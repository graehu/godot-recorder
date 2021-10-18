#ifndef RECORDER_H
#define RECORDER_H

#include "Array.hpp"
#include <Godot.hpp>
#include <ReferenceRect.hpp>

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
      void _process(float delta);
      void _record_duration(float duration);
      void _toggle_record();

   protected:
      void _save_frames(void* user_data);
      void _save_timer_complete();
      void _toggle_timer_complete();
      float frames_per_second;
      String output_folder;
      bool flip_y = true;
      bool use_thread = false;

   private:
      Array _images;
      Viewport* _viewport;
      Label* _label;
      Thread* _thread;
      Timer* _save_timer;
      Timer* _toggle_timer;
      float _frametick;
      bool _saving = false;
      bool _running = false;
   };
}

#endif//RECORDER_H
