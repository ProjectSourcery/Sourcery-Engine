#include "src3_window.h"

#include <stdexcept>

namespace src3 {
	SrcWindow::SrcWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
		initWindow();
	}

	SrcWindow::~SrcWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void SrcWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void SrcWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}
	void SrcWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto srcWindow = reinterpret_cast<SrcWindow*>(glfwGetWindowUserPointer(window));
		srcWindow->framebufferResized = true;
		srcWindow->width = width;
		srcWindow->height = height;
	}
}