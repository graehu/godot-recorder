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
if "--x264" in config.confply.args:
    ff_libs = ["avformat", "avcodec", "swresample", "swscale", "avutil", "x264"]
else:
    ff_libs = ["avformat", "avcodec", "swresample", "swscale", "avutil", "openh264"]
ff_lib_dirs = [ff_dir+"lib"+lib for lib in ff_libs]
config.library_paths.append("recorder/bin/")
config.library_paths.append("godot-cpp/bin/")
config.library_paths.append("openh264/")
config.link_libraries.extend(ff_libs)
config.link_libraries.append("godot-cpp.linux.debug.64")
config.compile_commands = True
config.position_independent = True
config.output_file = "recorder/bin/librecorder.so"
config.output_type = options.output_type.dll
config.run_paths = ["'$ORIGIN'"]


def post_load():
    import os
    import shutil
    import pathlib
    libs = pathlib.Path("recorder/bin/").glob("*.so*")
    for lib in libs:
        os.remove(lib)
    if not os.path.exists("godot-cpp/bin/libgodot-cpp.linux.debug.64.a"):
        os.system("cd godot-cpp; scons platform=linux generate_bindings=yes -j4")
        log.normal("godot-cpp bindings generated")

    if "--x264" not in config.confply.args:
        if not os.path.exists("openh264/lib/libopenh264.so.2.1.1"):
            os.system("cd openh264; make install-shared OS=linux ARCH=x86_64 PREFIX=$(pwd)")
            log.normal("openh264 built")

    if not os.path.exists(ff_dir+"config.h"):
        configure = ["--enable-shared", "--disable-programs"]
        if "--x264" in config.confply.args:
            os.system("cd "+ff_dir+"; ./configure --enable-shared --enable-libx264 --enable-gpl")
        else:
            configure.extend(
                [
                    "--enable-libopenh264",
                ]
            )
            os.system("export PKG_CONFIG_PATH=$(pwd)/openh264/lib/pkgconfig; cd "+ff_dir+";./configure "+" ".join(configure))
    os.system("cd "+ff_dir+";  make")
    log.normal("ffmpeg built")
    for lib_dir in ff_lib_dirs:
        if os.path.exists(lib_dir):
            for obj in os.listdir(lib_dir):
                if ".so." in obj:
                    cp_file = os.path.join(lib_dir, obj)
                    shutil.copy(cp_file, "recorder/bin/")
    if "--x264" in config.confply.args:
        os.system("cp $(find /lib/ -name libx264.so.160) recorder/bin")
    else:
        shutil.copy("openh264/lib/libopenh264.so", "recorder/bin")
        shutil.copy("openh264/lib/libopenh264.so.2.1.1", "recorder/bin")
        shutil.copy("openh264/lib/libopenh264.so.6", "recorder/bin")
    pass


def post_run():
    import shutil
    import os
    to_path = "demo/recorder"
    # #todo: maybe don't always do this.
    if os.path.exists(to_path):
        shutil.rmtree(to_path)
    shutil.copytree("recorder", to_path)
    # test it!
    if "--test" in config.confply.args:
        os.system("export DISPLAY=:1; ~/godot --path demo")


config.confply.post_load = post_load
config.confply.post_run = post_run
