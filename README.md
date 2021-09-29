# HurricaneEngine3D

Educational project to get familiar with Vulkan, PhysX.
Maybe it will work out and new game engine will be born ^_^

Supported platforms:
Windows,
Linux,
Mac Os X,
FreeBSD(see https://github.com/Esentiel/PhysX-FreeBSD for PhysX port)

Supported compilers*:
Visual C++ (2019),
clang(11+)

**other compilers were not tested, try it*

Requirements:
1. Vulkan SDK should be already set up according to official guidelines

Setup process:
1. Sync all the git submodules (git submodule update --init --recursive)
2. Build GLWF as a static lib into "build" folder (cd extern/glfw && cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cd build && make -j8)
3. Build PhysX (read below wall of text && cd extern/PhysX/physx && ./generate_projects.{sh|bat} && cd compiler/\<platform-type\> && make -j8).
Building process on windows is pretty simple and straightforward (except you only need to fix one error in code for VS2019 compiler - https://github.com/NVIDIAGameWorks/PhysX/issues/164).
Disable -Werror in "PhysX/physx/source/compiler/cmake/\<platform\>/CMakeLists.txt" to build PhysX on Linux or Mac OS X. Seems like PhysX team will not fix any warnings in 4.1 branch so there is basically no point in treating warnings like errors. On Linux you have to fix this bug in sources as well to build the project - https://github.com/NVIDIAGameWorks/PhysX/issues/283
 On non-debian derived distro also fix this one https://github.com/NVIDIAGameWorks/PhysX/issues/39
And last but not least, PhysX is built in static libs. That's not we want, so we have to fix "PhysX/physx/buildtools/presets/public/\<platform\>.xml": change "PX_GENERATE_STATIC_LIBRARIES" to "False"(also you may want to save some time by not compiling samples and snippets, thare are options for this as well in the file). That's expected to be False on both Linux and Mac OS X, or you can change cmake file in PxPhys project instead to use link to PhysX statically.

Notes:
Only debug configuration is now supported in cmake, sadly.
