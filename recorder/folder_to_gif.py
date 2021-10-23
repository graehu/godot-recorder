#!/bin/python

def main():

    import os
    import sys
    import imageio

    if len(sys.argv) > 1:
        path = sys.argv[1]
        framerate = float(0.1)
        if len(sys.argv) > 2:
            framerate = sys.argv[2]
        files = os.listdir(path)
        print(framerate)
        print(path)
        print(files)
        files.sort()
        try:
            with imageio.get_writer(path.strip("/")+'.mp4', fps=float(framerate)) as writer:
                for filename in files:
                    image = imageio.imread(path+"/"+filename)
                    writer.append_data(image)
        except Exception as e:
            with open(path+"/log.txt","w") as log_file:
                log_file.write(str(e))
            return


if __name__ == "__main__":
    main()
    pass
