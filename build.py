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

config.confply.log_topic = "cpp_compiler"
log.normal("loading build.py with confply_args: "+str(config.confply.args))
# #todo: ensure configure + make are called on ffmpeg prior to build.py
config.confply.tool = options.tool.clangpp
config.standard = options.standard.cpp11
config.source_files = [
    "src/recorder.cpp",
    "src/init.cpp"
]
config.include_paths = ["godot-cpp/godot-headers",
                        "godot-cpp/include",
                        "godot-cpp/include/core",
                        "godot-cpp/include/gen",
]
ff_dir = "FFmpeg/"
ff_libs = ["avformat", "avcodec", "swresample", "swscale", "avutil", "x264"]
config.library_paths.extend([ff_dir+"lib"+lib for lib in ff_libs])
config.link_libraries.extend(ff_libs)
config.compile_commands = True
config.position_independent = True
config.output_file = "recorder/bin/librecorder.so"
config.output_type = options.output_type.dll


def post_load():
    import os
    if not os.path.exists("godot-cpp/include/gen/Viewport.hpp"):
        os.system("cd godot-cpp; scons platform=linux generate_bindings=yes -j4")
    if not os.path.exists(ff_dir+"config.h"):
        os.system("cd "+ff_dir+"; ./configure")
    os.system("cd "+ff_dir+"; make")
    log.normal("ffmpeg built")
    pass


config.confply.post_load = post_load
