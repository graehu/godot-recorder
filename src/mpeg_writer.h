#ifndef MPEG_WRITER_H
#define MPEG_WRITER_H

#include "stdint.h"
#include "stdio.h"

struct AVCodecContext;
struct SwsContext;
struct AVPacket;
struct AVFrame;

class mpeg_writer
{
public:
   mpeg_writer(const char* _filename, unsigned int _width, unsigned int _height, unsigned int _fps, float _scale = 1.0);
   ~mpeg_writer();
   // #todo: make a proper type for image data
   //adds a frame to the mpeg
   int add_frame(uint8_t *rgb, int pix_format = 2);
private:
   AVFrame* frame;
   AVFrame* scaled_frame;
   AVPacket* packet;
   const char* filename;
   FILE* out_file;
   AVCodecContext* codec_context;
   SwsContext* sws_context;
   //
   unsigned int width; // in pixels
   unsigned int height;
   unsigned int fps; // frames per second
   //
   float scale;
   bool initialised;
};

#endif//MPEG_WRITER_H
