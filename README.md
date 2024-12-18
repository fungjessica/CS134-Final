# CS 134 Final Project: Space-Themed Landing Simulator

<div align="center">

![Cover Image](assets/CS_134_Final_Project_Cover_Lee_Rogers_Jessica_Fung.png)

![openFrameworks](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=C%2B%2B&logoColor=white)
![openFrameworks](https://img.shields.io/badge/openFrameworks-white?style=flat-square)
![OpenGL](https://img.shields.io/badge/OpenGL-blue?style=flat-square)

</div>

## Description

A fully featured landing simulator built on top of C++ and openFrameworks.

Created in conjunction with CS 134 @ SJSU.

## Trailer

[![Video Trailer](https://img.youtube.com/vi/VSlUb1KSigg/0.jpg)](https://www.youtube.com/watch?v=VSlUb1KSigg)

## Installation

To install, download version 0.12.0 of openFrameworks and generate a new project
with the ofxGui and ofxAssimpModelLoader add-ons. Once created, replace the files
in src with files in src from the project. Next, navigate to bin/data. Place geo,
images, shaders, shaders_gles, and sounds folders in. Lastly, compile and run the
files using Visual Studio 2022 (upgrade tools to v143).

NOTES:

Ensure that the model loader add-on file fix is included (comment out rotation at line #688).

Terrain model not included due to file size.

## Credits

-  [Jessica Fung](https://github.com/fungjessica)
-  [Lee Rogers](https://github.com/lrogerscs)
