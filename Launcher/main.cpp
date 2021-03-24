#include "Physics.h"
#include "VulkanBackend.h"
#ifdef WIN32
#include <Windows.h>
#endif
#ifdef LINUX
#include <unistd.h>
#endif
#ifdef APPLE
#include <mach-o/dyld.h>
#endif

#include <filesystem>
#include <string>

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define ROOT_FOLDER_NAME "HurricaneEngine3D"

#include "common/timer.h"

std::filesystem::path get_root_path(){
	std::filesystem::path root_path;

#ifdef WIN32
	char * exePath = new char[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	root_path = std::filesystem::path(exePath);

#endif

#ifdef LINUX
	char buff[MAX_PATH];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
      buff[len] = '\0';
      root_path = std::filesystem::path(buff);
    }
#endif

#ifdef APPLE
	char path[MAX_PATH];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0){
		root_path = std::filesystem::path(path);
	}
#endif

	while (root_path.filename().string() != ROOT_FOLDER_NAME){
		root_path = root_path.parent_path();
	}

	return root_path;
}

int main(int argc, const char** argv) {
	std::filesystem::path root_path = get_root_path();
	cmn::timer _timer;
	int cycles = 0u;
	float elapsed = 0.f;

	PhysSDK physEngine;
	physEngine.Init();

	VulkanBackend renderEngine;
	renderEngine.Initialize(root_path.string().c_str());
	do{
		cycles++;
		float delta = _timer.elapsed_time();
		elapsed+=delta;
		_timer.restart();

		if (elapsed > 5.0f){
			printf("FPS: %d for 5 secs\n", cycles/5);
			cycles = 0;
			elapsed = 0.f;
		}

		physEngine.Simulate(delta);
		renderEngine.Render(delta);
	}
	while(renderEngine.IsRunning());

	physEngine.Shutdown();
}
