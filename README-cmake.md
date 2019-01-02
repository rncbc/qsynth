# What is CMake?

CMake is a cross platform build system, that can be used to replace the old
auto-tools, providing a nice building environment and advanced features.

Some of these features are:
* Out of sources build: CMake allows you to build your software into a directory
  different to the source tree. You can safely delete the build directory and
  all its contents once you are done.
* Multiple generators: classic makefiles can be generated for Unix and MinGW,
  but also Visual Studio, XCode and Eclipse CDT projects among other types.
* Graphic front-ends for configuration and build options.

More information and documentation is available at the [CMake website](https://www.cmake.org)

CMake is free software. You can get the sources and pre-compiled packages for
Linux and other systems [here](https://www.cmake.org/cmake/resources/software.html).


## How to use it?

1. You need CMake 2.6 or newer to build Qsynth

2. Unpack the Qsynth sources somewhere, or checkout the repository,
   and create a build directory. For instance, using a command line shell:
```bash
$ tar -xvzf Downloads/qsynth-x.y.z.tar.gz
$ cd qsynth-x.y.z
$ mkdir build
```

2. Execute CMake from the build directory, providing the source directory
   location and optionally, the build options. There are several ways.

* From a command line shell:
```bash
$ pwd
qsynth-x.y.z
$ cd build
$ cmake .. -DCONFIG_STACKTRACE=1 -DCMAKE_BUILD_TYPE=debug
```

Available boolean options:
  * CONFIG_GRADIENT, enabled by default
  * CONFIG_SYSTEM_TRAY, enabled by default
  * CONFIG_STACKTRACE, disabled by default
Valid values for boolean options are: 1, 0, YES, NO, ON, OFF, TRUE and FALSE.

* There are also several alternative CMake front-ends, if you don't want to use
  the command line interface:
  * ncurses based program, for Linux and Unix: ccmake
  * GUI, Qt4 based program, multiplatform: cmake-gui
  * GUI, Windows native program: CMakeSetup.exe (deprecated)

3. Execute the build command. If you used the Makefiles generator (the default
   in Linux and other Unix systems) then execute make, gmake, or mingw32-make.
   If you generated a project file, use your IDE to build it.


## Compiling with make

There are many targets available. To see a complete list of them, type:
```bash
$ make help
```
The build process usually hides the compiler command lines, to show them:
```bash
$ make VERBOSE=1
```
There is a "clean" target, but not a "distclean" one. You should use a build
directory different to the source tree. In this case, the "distclean" target
would be equivalent to simply removing the build directory.


## If something fails

If there is an error message while executing CMake, this probably means that a
required package is missing in your system. You should install the missing
component and run CMake again.

If there is an error executing the build process, after running a flawless CMake
configuration process, this means that there may be an error in the source code,
or in the build system, or something incompatible in 3rd party libraries.

The CMake build system for Qsynth is experimental. It will take a while
until it becomes stable and fully tested. You can help providing feedback,
please send a report containing your problems to the Qsynth development
mailing list, https://lists.sourceforge.net/mailman/listinfo/qsynth-devel
