#!confply/confply.py --in
# generated using:
# python confply/confply.py --config cpp_compiler build.py
import sys
sys.path.append('confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config_hash = '8ff317685643ed2d41cc17cdbb356768'
############# modify_below ################

config.confply.log_topic = "recorder"
log.normal("loading build.py with confply_args: "+str(config.confply.args))
config.confply.tool = options.tool.clangpp
config.standard = options.standard.cpp17
config.source_files = [
    "src/recorder.cpp",
    "src/init.cpp",
    "src/mpeg_writer.cpp"
]
config.include_paths = ["godot-cpp/godot-headers",
                        "godot-cpp/include",
                        "godot-cpp/include/core",
                        "godot-cpp/include/gen",
]
ff_dir = "FFmpeg/"
# ff_libs = ["avformat", "avcodec", "swresample", "swscale", "avutil"]
# x264 isn't linking correctly in godot yet, there's an issue with the resolved paths being used in the librecorder.so
# ff_libs = ["avformat", "avcodec", "swresample", "swscale", "avutil", "x264"]
ff_libs = ["avformat", "avcodec", "swresample", "swscale", "avutil", "avdevice", "avfilter", "x264"]
ff_lib_dirs = [ff_dir+"lib"+lib for lib in ff_libs]
config.library_paths.append("recorder/bin/")
config.library_paths.append("godot-cpp/bin/")
config.link_libraries.extend(ff_libs)
config.link_libraries.append("godot-cpp.linux.debug.64")
config.compile_commands = True
config.position_independent = True
config.output_file = "recorder/bin/librecorder.so"
config.output_type = options.output_type.dll
# config.run_paths = ["'$ORIGIN'"]# , "'$ORIGIN/recorder/bin/'"
config.run_paths = ["'$ORIGIN'"]


def post_load():
    import os
    if not os.path.exists("godot-cpp/bin/libgodot-cpp.linux.debug.64.a"):
        os.system("cd godot-cpp; scons platform=linux generate_bindings=yes -j4")
    if not os.path.exists(ff_dir+"config.h"):
        os.system("cd "+ff_dir+"; ./configure --enable-shared --enable-libx264 --enable-gpl")
    os.system("cd "+ff_dir+"; make")
    log.normal("ffmpeg built")
    pass


def post_run():
    import shutil
    import os
    to_path = "../recorder-demo/recorder"
    # #todo: maybe don't always do this.
    if os.path.exists(to_path):
        shutil.rmtree(to_path)
    for lib_dir in ff_lib_dirs:
        if os.path.exists(lib_dir):
            for obj in os.listdir(lib_dir):
                if ".so." in obj:
                    cp_file = os.path.join(lib_dir, obj)
                    shutil.copy(cp_file, "recorder/bin/")
                    # #todo: I don't like this! patchelf doesn't come on every system
                    os.system("patchelf --set-rpath "+(":".join(config.run_paths))+" recorder/bin/"+obj)
                    print(f"copied {obj} to recorder/bin/{obj}")
    os.system("patchelf --set-rpath "+(":".join(config.run_paths))+" recorder/bin/libx264.so.160")
    shutil.copytree("recorder", to_path)
    # test it!
    if "--test" in config.confply.args:
        os.system("export DISPLAY=:1; ~/godot --path ~/Projects/godot/recorder-demo")


config.confply.post_load = post_load
config.confply.post_run = post_run
