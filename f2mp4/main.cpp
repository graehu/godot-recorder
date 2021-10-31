#include "../src/mpeg_writer.h"
#include "../../../C++/framework/src/graphics/resources/bitmap.h"
#include <bits/stdint-uintn.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


int main(int argv, char** argc)
{
   int w = 0, h = 0, c = 0;
    std::string path = "images/";
    std::vector<std::string> paths = {
       "images/1.png",
       "images/2.png",
       "images/3.png",
       "images/4.png",
       "images/5.png",
       "images/6.png",
       "images/7.png",
       "images/8.png",
       "images/9.png",
       "images/10.png",
       "images/11.png",
       "images/12.png",
       "images/13.png",
       "images/14.png"
    };
    {
       
       mpeg_writer writer("out", 1024, 600, 15);
       for(int i = 0; i < 5; i++)
       {
	  for(auto image : paths)
	  {
	     const unsigned char* data = stbi_load(image.c_str(), &w, &h, &c, 0);
	     writer.add_frame((uint8_t*)data);
	     stbi_image_free((void*)data);
	  }
       }
    }
    
   // int bmp_data_size = 3*frame->width*frame->height;
   // bitmap bmp(rgb_frame->width, rgb_frame->height, (signed char*)&rgb_frame->data[0][0], bmp_data_size);
   // bmp.save("decoded.pkg.bmp");
   // mpeg_writer derp()		// 
   //hello
}
