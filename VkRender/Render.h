#ifdef VKRENDER_EXPORTS
#undef VKRENDER_EXPORTS
#define VKRENDER_EXPORTS __declspec(dllexport)
#else
#define VKRENDER_EXPORTS __declspec(dllimport)
#endif

int main_render(const char* path);