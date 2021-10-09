#ifndef RECORDER_H
#define RECORDER_H

#include <Godot.hpp>
#include <Node.hpp>

namespace godot
{
   class Recorder : public Node
   {
      GODOT_CLASS(Recorder, Node);
   public:
      static void _register_methods();

      Recorder();
      ~Recorder();

      void _init();
      void _process(float delta);
   protected:
      float test_prop;
   };

}

#endif//RECORDER_H
