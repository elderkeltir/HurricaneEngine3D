//#include "Physics.h"
#include "Render.h"
#ifdef WIN32
#include <Windows.h>
#endif
#ifdef LINUX
#include <unistd.h>
#endif
#ifdef APPLE
#include <mach-o/dyld.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#include <filesystem>
#include <string>


std::filesystem::path get_exe_path(){
	std::filesystem::path exe_path;

#ifdef WIN32
	wchar_t * exePath = new wchar_t[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	exe_path = std::filesystem::path(exePath);

#endif

#ifdef LINUX
	char buff[MAX_PATH];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
      buff[len] = '\0';
      exe_path = std::filesystem::path(buff);
    }
#endif

#ifdef APPLE
	char path[MAX_PATH];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0){
		exe_path = std::filesystem::path(path);
	}
#endif

return exe_path;
}

int main(int argc, const char** argv) {
	std::filesystem::path exe_path = get_exe_path();

	//std::string obj_path = exe_path.parent_path().parent_path().parent_path().string() + "\\extern\\meshoptimizer\\demo\\pirate.obj";

	//PhysSDK p;
	//p.Init();

	main_render(exe_path.string().c_str());

	//p.Shutdown();
}