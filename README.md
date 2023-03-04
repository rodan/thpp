

## thpp

Thermal processing panel is viewer/analyzer of thermal images. It aims to be a rewrite of a project of mine from the 2000s with the same name hosted on sourceforge. The old project was Windows 95-XP oriented and it was written in Borland C++. This new one's target is mostly linux use and it attempts to also support radiometric jpeg images generated by a Flir camera.

```
 source:        https://github.com/rodan/thpp
 author:        Petre Rodan <2b4eda@subdimension.ro>
 license:       GNU GPLv3

png decode/encode functionality provided by
 source:        https://github.com/lvandeve/lodepng
 author:        Lode Vandevenne
 license:       zlib (GPL compatible)

error checking based on the Linux Programming Interface book's library
 source:        https://nostarch.com/tlpi/
 author:        Michael Kerrisk
 license:       GNU GPLv3
```

### functionality

currently supported image formats:

camera | image type
--- | ---
Irtis 2000 | DTV 256x248 pixel, single frame
Flir ThermaCAM E25 | radiometric JPG 160x120

currently supported functions:

image type | export to png | palette change | rescale | zoom
--- | --- | --- | --- | --- 
dtv | yes | yes | yes | yes
rJPG| yes | yes | yes | yes


### Build requirements

the code depends on the headers and libraries provided by the following packages:

media-libs/exiftool-12.42
dev-libs/json-c-0.16
dev-libs/apr-util-1.6.3

compilation is simple, one only needs to
```
cd ./src
make
```

### Usage


### Testing

the code itself is static-scanned by [llvm's scan-build](https://clang-analyzer.llvm.org/), [cppcheck](http://cppcheck.net/). Dynamic memory allocation in the PC applications is checked with [valgrind](https://valgrind.org/).


