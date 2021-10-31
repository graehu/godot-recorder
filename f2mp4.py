#!confply/confply.py --in
# generated using:
# python confply/confply.py --config cpp_compiler f2mp4.py
import sys
sys.path.append('confply')
import confply.cpp_compiler.config as config
import confply.cpp_compiler.options as options
import confply.log as log
config_hash = 'e092a432c0f821911977b8fcfb9ca79e'
############# modify_below ################

config.confply.log_topic = "f2mp4"
log.normal("loading f2mp4.py with confply_args: "+str(config.confply.args))

config.include_paths.extend(
    [
        "src/"
    ]
)
config.library_paths.extend(
    [
        "recorder/bin"
    ]
)
config.source_files.extend(
    [
        "f2mp4/main.cpp"
    ]
)
config.run_paths = ["'$ORIGIN/recorder/bin'"]
config.standard = options.standard.cpp17
config.link_libraries.extend(
    [
        "recorder"
    ]
)

config.output_file = "f2mp4/f2mp4.exe"
config.confply.dependencies.append("build.py")

def post_load():
    import shutil
    import os
    to_path = "f2mp4/recorder"
    if os.path.exists(to_path):
        shutil.rmtree(to_path)
    shutil.copytree("recorder", to_path)

def post_run():
    import os
    os.system("cd f2mp4; ./f2mp4.exe")


config.confply.post_load = post_load
config.confply.post_run = post_run
