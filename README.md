# wrx-engine
wrx-engine is a portable, simple graphical client that runs on all Desktop computers. It is currently backed by a simple OpenGL bitmap buffer base, and hosts Lua 5.4 wrx applications in a protected sandbox allowing little access to the hosting machine.

# dependencies
wrx-engine employs only common libraries which can be found in three main repositories: MSYS2, Brew, and Debian packages. This allows it to be easily built on Windows, Mac, and Debian/Ubuntu operating systems. Here is a list of all the dependencies:

* Lua 5.4
* PhysFS
* json-C
* OpenGL
* opus
* cairo
* FreeImage