#include "recorder.h"
#include "Defs.hpp"
#include "Object.hpp"
#include "Thread.hpp"
#include <Viewport.hpp>
#include <ViewportTexture.hpp>
#include <Image.hpp>
#include <CanvasItem.hpp>
#include <Transform.hpp>
#include <String.hpp>
#include <Label.hpp>
#include <GlobalConstants.hpp>
#include <Engine.hpp>
#include <SceneTree.hpp>
#include <OS.hpp>
#include <Directory.hpp>
#include <Timer.hpp>
#include <PoolArrays.hpp>

namespace godot
{    
   void Recorder::_register_methods()
   {
      register_method("_process", &Recorder::_process);
      register_property<Recorder, float>("frames_per_second", &Recorder::frames_per_second, 15);
      register_property<Recorder, String>("output_folder", &Recorder::output_folder, String("../out"));
      register_method("timer_complete", &Recorder::_timer_complete);
   }
   Recorder::Recorder()
      : _viewport(nullptr),
	_thread(nullptr)
   {
   }
   Recorder::~Recorder()
   {
      // add your cleanup here
      if(_viewport != nullptr)
      {
#pragma message("Fix this")
      }
      if(_thread != nullptr)
      {
#pragma message("Fix this")
      }
      if(_label != nullptr)
      {
#pragma message("Fix this")	 
      }
      if(_timer != nullptr)
      {
#pragma message("Fix this")	 
      }
   }
   void Recorder::_init()
   {
      // initialize any variables here
      _viewport = get_viewport();
      _frametick = 1.0 / frames_per_second;
      _thread = Thread::_new();
      _label = Label::_new();
      // # If running on editor, DONT override process and input
      set_process(false);
      set_process_input(false);
      auto engine = Engine::get_singleton();
      if(!engine->is_editor_hint())
      {
	 set_process(true);
	 set_process_input(true);
      }
      //# Some recorder info to show onscreen
      _viewport->call_deferred("add_child", _label);
     //#		print(get_tree().root)
     //#		get_tree().root.add_child(_label)
     //#		_label.text = "Waiting for capture...\nNumber of frames recorded: " + str(_images.size())
      _label->set_anchor(GlobalConstants::MARGIN_BOTTOM, 1);
      _label->set_margin(GlobalConstants::MARGIN_BOTTOM, -10);
      _label->set_anchor(GlobalConstants::MARGIN_TOP, 1);
      _label->set_margin(GlobalConstants::MARGIN_TOP, -40);
      _label->hide();
      _timer = Timer::_new();
      _timer->connect("timeout", this, "timer_complete");
   }
   void Recorder::_process(float delta)
   {
      if (_running)
      {
	 _frametick += delta;
	 if (_frametick > 1.0/frames_per_second)
	 {
	    _frametick -= 1.0/frames_per_second;
	    Ref<Image> image = _viewport->get_texture()->get_data();
	    Vector2 pos = get_global_transform_with_canvas().get_origin();
	    Rect2 rect = Rect2(Vector2(pos.x,_viewport->get_size().y - (pos.y + get_rect().size.y)), get_rect().size);
	    image->blit_rect(image, rect, Vector2(0,0));
	    _images.append(image);
	 }
      }
   }
   void Recorder::_toggle_record()
   {
      if (_thread->is_active() || _saving)
      {
   	 return;
      }
      _running = !_running;
      _label->hide();
      if (!_running)
      {
   	 _label->show();
      }
      if (use_thread)
      {
   	 if(not _thread->is_active())
   	 {
   	    auto err = _thread->start(this, "save_frames");
   	 }
      }
      else
      {
   	 _save_frames(nullptr);
      }
   }
   void Recorder::_save_frames(void* user_data)
   {
      _saving = true;
	   //# userdata wont be used, is just for the thread calling
      String scene_name = get_tree()->get_current_scene()->get_name();
      Dictionary date_time = OS::get_singleton()->get_datetime();
      Object* hour = Object::cast_to<Object>(date_time["hour"]);
      Object* minute = Object::cast_to<Object>(date_time["minute"]);
      Object* second = Object::cast_to<Object>(date_time["second"]);
      if(hour && minute && second)
      {
	 String time_str = hour->to_string()+"-"+minute->to_string()+"-"+second->to_string();
	 Directory* dir = Directory::_new();
	 String path = "res://" + output_folder+"/"+scene_name+"_"+time_str+"/";
	 dir->make_dir(path);
	 if (dir->open(path) != Error::OK)
	 {
	    // print("An error occurred when trying to create the output folder.");
	    ERR_PRINT("An error occurred when trying to create the output folder.");
	 }
	
	 for (int i = 0; i < _images.size(); i++)
	 {
	    Ref<Image> image = _images[i];
	    _label->set_text("Saving frames...("+String(i) + "/"+String(_images.size())+")");
	    image->crop(get_rect().size.x, get_rect().size.y);
	    if (flip_y)
	    {
	       image->flip_y();
	    }
	    image->save_png(path + String(i) + ".png");
	 }
	 _images.clear();
	
	 if(_thread->is_active())
	 {
	    _thread->wait_to_finish();
	 }
	 Array output;
	 String script  = "addons/Recorder/folder_to_gif.py";
	 path = output_folder+"/"+scene_name+"_"+time_str+"/";
	 float fps = frames_per_second;
	 PoolStringArray array;
	 array.append(script);
	 array.append(path);
	 array.append(fps);
	 OS::get_singleton()->execute("python", array, false, output);
	 _label->set_text("Done!");
	 _timer->start(1);
      }
   }
   void Recorder::_timer_complete()
   {
      _label->set_text("");
      _label->hide();
      _saving = false;
   }
} // namespace godot
