#include "recorder.h"

namespace godot
{    
   void Recorder::_register_methods()
   {
      register_method("_process", &Recorder::_process);
      register_property<Recorder, float>("test_property", &Recorder::test_prop, 10);
   }
   Recorder::Recorder()
   {
   }
   Recorder::~Recorder()
   {
      // add your cleanup here
   }
   void Recorder::_init()
   {
      // initialize any variables here
      test_prop = 0.0;
   }
   void Recorder::_process(float delta)
   {
      test_prop += delta;
   }
}
