#include "mpeg_writer.h"
#include "GodotGlobal.hpp"
#include <cstring>
#include <string>
#include <Godot.hpp>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define GD_LINE godot::Godot::print(__FILE__ ":" LINE_STRING)

// needed because ffmpeg is a pure C library.
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <libavutil/error.h>
}

mpeg_writer::mpeg_writer(const char* _filename, unsigned int _width, unsigned int _height, unsigned int _fps, float _scale) :
   frame(nullptr),
   packet(nullptr),
   filename(_filename),
   out_file(nullptr),
   codec_context(nullptr),
   sws_context(nullptr),
   width(_width),
   height(_height),
   fps(_fps),
   scale(_scale)
{
   // fprintf(stderr, "\n");
   fprintf(stderr, "w %d\n", width);
   fprintf(stderr, "h %d\n", height);
   fprintf(stderr, "f %d\n", fps);
   fprintf(stderr, "s %f\n", scale);
   packet = new AVPacket();
   //allocate h264 codec
   auto codec_id = AV_CODEC_ID_H264;
   AVCodecID e_codec_id = static_cast<AVCodecID>(codec_id);
   auto* codec = avcodec_find_encoder(e_codec_id);
   if (codec == nullptr)
   {
      fprintf(stderr, "Codec not found\n");
      
      exit(1);
   }
   codec_context = avcodec_alloc_context3(codec);
   if (codec_context == nullptr)
   {
      fprintf(stderr, "Could not allocate video codec context\n");
      exit(1);
   }
   //setup codec
   codec_context->bit_rate = 400000;
   codec_context->width = width*scale;
   codec_context->height = height*scale;
   codec_context->time_base.num = 1;
   codec_context->time_base.den = fps;
   codec_context->gop_size = 10;
   codec_context->max_b_frames = 1;
   codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
   //if (codec_id == AV_CODEC_ID_H264)
   av_opt_set(codec_context->priv_data, "preset", "slow", 0);
   
   // open codec #todo: error handling, ret < 0 get error strings.
   int ret = avcodec_open2(codec_context, codec, nullptr);
   if (ret < 0)
   {
      char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
      av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
      fprintf(stderr, "Could not open codec %s\n", error_buf);
      exit(1);
   }
   char float_str[16];
   snprintf(float_str, 16,".%.2f.h264", scale);
   std::string file_str = std::string(filename)+std::string(float_str);
   out_file = fopen(file_str.c_str(), "wb");
   if (out_file == nullptr)
   {
      fprintf(stderr, "Could not open %s\n", file_str.c_str());
      exit(1);
   }
   //allocate frames
   frame = av_frame_alloc();
   if (frame == nullptr)
   {
      fprintf(stderr, "Could not allocate video frame\n");
      exit(1);
   }
   frame->format = codec_context->pix_fmt;
   frame->width  = width;
   frame->height = height;
   frame->pts = -1;
   AVPixelFormat format = static_cast<AVPixelFormat>(frame->format);
   // #todo: align might be better 64?
   ret = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, format, 32);
   // #todo: add av_strerror
   if (ret < 0)
   {
      char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
      av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
      fprintf(stderr, "Could not allocate raw picture buffer: %s\n", error_buf);
      exit(1);
   }
   scaled_frame = av_frame_alloc();
   if (scaled_frame == nullptr)
   {
      fprintf(stderr, "Could not allocate video frame\n");
      exit(1);
   }
   scaled_frame->format = codec_context->pix_fmt;
   scaled_frame->width  = width*scale;
   scaled_frame->height = height*scale;
   scaled_frame->pts = -1;
   // #todo: align might be better 64?
   ret = av_image_alloc(scaled_frame->data, scaled_frame->linesize, scaled_frame->width, scaled_frame->height, format, 32);
   // #todo: add av_strerror
   if (ret < 0)
   {
      char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
      av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
      fprintf(stderr, "Could not allocate raw picture buffer: %s\n", error_buf);
      exit(1);
   }
   //next bit
}
mpeg_writer::~mpeg_writer()
{
   uint8_t endcode[] = { 0, 0, 1, 0xb7 };
   int ret;
   do
   {
      fflush(stdout);
      ret = avcodec_receive_packet(codec_context, packet);
      if (ret >= 0)
      {
	 fwrite(packet->data, 1, packet->size, out_file);
	 av_packet_unref(packet);
      }
      else if (ret == AVERROR_EOF)
      {
	 break;
      }
      else if(ret < 0 && ret != AVERROR(EAGAIN))
      {
	 char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
	 av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
	 fprintf(stderr, "Error encoding frame %s\n", error_buf);
	 exit(1);
      }
   } while (ret == EAGAIN);
   fwrite(endcode, 1, sizeof(endcode), out_file);
   fclose(out_file);
   avcodec_close(codec_context);
   av_free(codec_context);
   av_freep(&frame->data[0]);
   av_frame_free(&frame);
   av_freep(&scaled_frame->data[0]);
   av_frame_free(&scaled_frame);
   delete packet;
 }
int mpeg_writer::add_frame(uint8_t *rgb, int pix_format)
{
   if(rgb != nullptr)
   {
      const int in_linesize[1] = { 3 * frame->width };
      //yuv conversion
      sws_context = sws_getCachedContext(sws_context,
					 // #todo: don't assume the format?
					 frame->width, frame->height, AV_PIX_FMT_RGB24,
					 frame->width, frame->height, AV_PIX_FMT_YUV420P,
					 SWS_BICUBIC, nullptr, nullptr, nullptr);
      sws_scale(sws_context, (const uint8_t * const *)&rgb, in_linesize, 0,
		frame->height, frame->data, frame->linesize);
      //scale
      if (scale != 1.0f)
      {
	 
	 sws_context = sws_getCachedContext(sws_context,
					    frame->width, frame->height, AV_PIX_FMT_YUV420P,
					    scaled_frame->width, scaled_frame->height, AV_PIX_FMT_YUV420P,
					    SWS_BICUBIC, nullptr, nullptr, nullptr);

	 sws_scale(sws_context, (const uint8_t * const *)frame->data, frame->linesize, 0,
		   frame->height, scaled_frame->data, scaled_frame->linesize);  
      }
      int ret = 0;
      av_init_packet(packet);
      packet->data = nullptr;
      packet->size = 0;
      frame->pts++;
      scaled_frame->pts = frame->pts;
      if (scale != 1.0f)
      {
	 ret = avcodec_send_frame(codec_context, scaled_frame);  
      }
      else
      {
	 ret = avcodec_send_frame(codec_context, frame);
      }
      if (ret < 0)
      {
	 char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
	 av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
	 fprintf(stderr, "Error encoding frame %s\n", error_buf);
	 godot::Godot::print("Error encoding frame %s\n", error_buf);
	 exit(1);
      }
      ret = avcodec_receive_packet(codec_context, packet);
      if (ret >= 0)
      {
	 fwrite(packet->data, 1, packet->size, out_file);
	 av_packet_unref(packet);
      }
   }
   return -1;
}
