#include "recorder.h"
#include "Defs.hpp"
#include "GodotGlobal.hpp"
#include "Object.hpp"
#include "ReferenceRect.hpp"
#include "Thread.hpp"
#include "Variant.hpp"
#include "gdnative/gdnative.h"
#include "gdnative/string.h"
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
#include <bits/stdint-uintn.h>
#include "mpeg_writer.h"

namespace godot
{
   #define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define GD_LINE godot::Godot::print(__FILE__ ": " LINE_STRING)
   void Recorder::_register_methods()
   {
      //methods
      register_method("_process", &Recorder::_process);
      register_method("_ready", &Recorder::_ready);
      register_method("_save_frames", &Recorder::_save_frames);
      register_method("record_duration", &Recorder::record_duration);
      register_method("toggle_record", &Recorder::toggle_record);
      register_method("save_timer_complete", &Recorder::_save_timer_complete);
      register_method("toggle_timer_complete", &Recorder::_toggle_timer_complete);
      //props
      register_property<Recorder, float>("frames_per_second", &Recorder::frames_per_second, 15);
      register_property<Recorder, String>("output_folder", &Recorder::output_folder, String("../out"));
   }
   //#todo: lots of uninitialised vars here.
   Recorder::Recorder()
      : frames_per_second(15),
	output_folder("../out"),
	_viewport(nullptr),
	_thread(nullptr),
	_save_timer(nullptr),
	_toggle_timer(nullptr),
	use_thread(true),
	flip_y(true)
	// _writer(nullptr)
   {
   }
   Recorder::~Recorder()
   {
      _viewport = nullptr;
      // add your cleanup here
      if(_thread != nullptr)
      {
	 if(_thread->is_active())
	 {
	    _thread->wait_to_finish();
	 }
	 _thread->unreference();
      }
      // the rest of these are associated with the scene, might need to add refs if I want to clean them up here.
      // #todo: seems like these might be leaking, but they don't come with ref class accessors. Not sure what the clean-up pattern is.
      // if(_label != nullptr)
      // {
      // 	 _label->free();
      // 	 _label = nullptr;
      // }
      // GD_LINE;
      // if(_save_timer != nullptr)
      // {
      // 	 _save_timer->free();
      // 	 _save_timer = nullptr;
      // }
      // GD_LINE;
      // if(_toggle_timer != nullptr)
      // {
      // 	 _toggle_timer->free();
      // 	 _toggle_timer = nullptr;
      // }
      // GD_LINE;
      // if(_writer != nullptr)
      // {
      // 	 delete _writer;
      // 	 _writer = nullptr;
      // }
   }
   void Recorder::_init(){}
   void Recorder::_ready()
   {
      // Initialize any variables here
      _viewport = get_viewport();
      _frametick = 1.0 / frames_per_second;
      _thread = Ref<Thread>(Thread::_new());
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


      _save_timer = Timer::_new();
      _toggle_timer = Timer::_new();
      _save_timer->connect("timeout", this, "save_timer_complete");
      _save_timer->set_one_shot(true);
      _toggle_timer->connect("timeout", this, "toggle_timer_complete");
      _toggle_timer->set_one_shot(true);
      _viewport->call_deferred("add_child", _save_timer);
      _viewport->call_deferred("add_child", _toggle_timer);
      Godot::print("recorder ready");  
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
   void Recorder::toggle_record()
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
	 if (use_thread)
	 {
	    if(!_thread->is_active())
	    {
	       Godot::print("threaded mp4 save");
	       auto err = _thread->start(this, "_save_frames");
	    }
	 }
	 else
	 {	    
	    _save_frames();
	 }
      }
   }
   void Recorder::_save_frames()
   {
      Godot::print("saving frames to file...");
      _saving = true;
      //# userdata wont be used, is just for the thread calling
      String scene_name = get_tree()->get_current_scene()->get_name();
      Dictionary date_time = OS::get_singleton()->get_datetime();
      const String hour = date_time["hour"];
      const String minute = date_time["minute"];
      const String second = date_time["second"];
      // if(hour && minute && second)
      {
	 String time_str = hour+"-"+minute+"-"+second;
	 Directory* dir = Directory::_new();
	 String path = "res://" + output_folder+"/"+scene_name+"_"+time_str+"/";
	 // Godot::print(output_folder);
	 // Godot::print(path);
	 // dir->make_dir(path);
	 // if (dir->open(path) != Error::OK)
	 // {
	 //    ERR_PRINT("An error occurred when trying to create the output folder.");
	 // }
	 auto rect = ReferenceRect::get_rect();
	 String writer_path = (output_folder+"/"+scene_name+"_"+time_str);
	 {
	    const char* c_string = writer_path.alloc_c_string();
	    mpeg_writer*writer = new mpeg_writer(c_string, rect.get_size().x, rect.get_size().y, frames_per_second);
	    for (int i = 0; i < _images.size(); i++)
	    {
	       Ref<Image> image = _images[i];
	       _label->set_text("Saving frames...("+String::num(i) + "/"+String::num(_images.size())+")");
	       image->crop(get_rect().size.x, get_rect().size.y);
	       if (flip_y)
	       {
		  image->flip_y();
	       }
	       image->convert(Image::FORMAT_RGB8);
	       auto pool = image->get_data();
	       auto* ptr = pool.read().ptr();
	       writer->add_frame((uint8_t*)ptr);
	    }
	    Godot::print("wrote "+String::num(_images.size())+"frames");
	    delete writer;
	    writer = nullptr;
	    Godot::print("so far so good....");
	 }
	 _images.clear();
	 // Array output;
	 // String script  = "recorder/folder_to_gif.py";
	 // path = output_folder+"/"+scene_name+"_"+time_str+"/";
	 // float fps = frames_per_second;
	 // PoolStringArray array;
	 // array.append(script);
	 // array.append(path);
	 // array.append(fps);
	 // OS::get_singleton()->execute("python", array, false, output);
	 _label->set_text("Done!");
	 _save_timer->start(1);
	 Godot::print("wrote: "+writer_path);
      }
   }
   void Recorder::record_duration(float duration = 5)
   {
      if (_running) { return; }
      toggle_record();
      _toggle_timer->start(duration);
   }
   void Recorder::_save_timer_complete()
   {
      _label->set_text("");
      _label->hide();
      _saving = false;
   }
   void Recorder::_toggle_timer_complete()
   {
      toggle_record();
   }
} // namespace godot
