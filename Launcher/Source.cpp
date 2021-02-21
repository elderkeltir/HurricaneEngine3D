#include "Physics.h"
#include "Render.h"
#include <Windows.h>
#include <filesystem>

int main(int argc, const char** argv) {
	wchar_t * exePath = new wchar_t[MAX_PATH];
	std::string obj_path;
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule)
	{
		GetModuleFileName(hModule, exePath, MAX_PATH);

		std::filesystem::path p = std::filesystem::path(exePath);
		obj_path = p.parent_path().parent_path().parent_path().string() + "\\extern\\meshoptimizer\\demo\\pirate.obj";
	}

	PhysSDK p;
	p.Init();

	main_render(obj_path.c_str());

	p.Shutdown();
}