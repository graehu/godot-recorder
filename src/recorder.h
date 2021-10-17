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
   protected:
      void _toggle_record();
      void _save_frames(void* user_data);
      void _timer_complete();
      float frames_per_second;
      String output_folder;
      bool flip_y = true;
      bool use_thread = false;

   private:
      float _frametick;
      // vector<Ref<Image>> _images;
      Array _images;
      Viewport* _viewport;
      Label* _label;
      // var _label = Label.new();
      bool _saving = false;
      bool _running = false;
      Thread* _thread;
      Timer* _timer;
   };

}

#endif//RECORDER_H
