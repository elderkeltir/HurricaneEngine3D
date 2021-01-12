:: Set MSBuild
for /f "usebackq delims=" %%I in (`call "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -property installationpath -version "[16.0,17.0)"`) DO (
    call "%%I\VC\Auxiliary\Build\vcvarsall.bat" x64 %*
    goto :found
)
echo There is no VS2019 installed!
PAUSE
EXIT

:found

:: GLFW
cd glfw
cmake -S . -B build

cd build
msbuild GLFW.sln /t:GLFW3\glfw:Rebuild /p:Configuration="Debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild GLFW.sln /t:GLFW3\glfw:Rebuild /p:Configuration="Release" /p:Platform="x64" /p:BuildProjectReferences=false

:: PhysX
cd ..\..\PhysX\physx
call generate_projects.bat vc16win64

cd compiler\vc16win64
msbuild PhysXSDK.sln /t:"PhysX SDK\FastXml:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\LowLevel:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\LowLevelAABB:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\LowLevelDynamics:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysX:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXCommon:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXCooking:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXExtensions:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXFoundation:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXPvdSDK:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXTask:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXVehicle:Rebuild" /p:Configuration="debug" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\FastXml:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\LowLevel:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\LowLevelAABB:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\LowLevelDynamics:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysX:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXCommon:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXCooking:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXExtensions:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXFoundation:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXPvdSDK:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXTask:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false
msbuild PhysXSDK.sln /t:"PhysX SDK\PhysXVehicle:Rebuild" /p:Configuration="release" /p:Platform="x64" /p:BuildProjectReferences=false