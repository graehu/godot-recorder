#ifndef RECORDER_H
#define RECORDER_H

#include "Array.hpp"
#include <Godot.hpp>
#include <ReferenceRect.hpp>

class mpeg_writer;
namespace godot
{
   class Thread;
   class Image;
   class String;
   class Label;
   class Timer;
   class Recorder : public ReferenceRect
   {
      GODOT_CLASS(Recorder, ReferenceRect);
   public:
      static void _register_methods();

      Recorder();
      ~Recorder();

      void _init();
      void _ready();
      void _process(float delta);
      void record_duration(float duration);
      void toggle_record();

   protected:
      void _save_frames();
      void _save_timer_complete();
      void _toggle_timer_complete();
      float frames_per_second;
      String output_folder;
      bool flip_y;
      bool use_thread;

   private:
      Array _images;
      Viewport* _viewport;
      Label* _label;
      Ref<Thread> _thread;
      Timer* _save_timer;
      Timer* _toggle_timer;
      float _frametick;
      bool _saving = false;
      bool _running = false;
   };
}

#endif//RECORDER_H
