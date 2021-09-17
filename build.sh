g++ -std=c++11 -fPIC -I godot-headers/ -c src/recorder.cpp -o objects/recorder.o
g++ -rdynamic -shared objects/recorder.o -o recorder/bin/librecorder.so
