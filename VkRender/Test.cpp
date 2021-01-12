
#include <GLFW/glfw3.h>

int main() {
	if (glfwInit()) {
		GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
		if (window) {

		}

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	return 0;
}