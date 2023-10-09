// ������ â�� ���� ���� �Լ��� ����

#include "lve_window.h"

// std
#include <stdexcept>

namespace lve {
	
	// ������ ������
	LveWindow::LveWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	// ������ �Ҹ���
	LveWindow::~LveWindow() {
		// �����츦 ����, �޸� ����
		glfwDestroyWindow(window);

		// GLFW ���̺귯���� ����, ���ҽ� ����
		glfwTerminate();
	}
	
	// ������ �ʱ�ȭ ����
	void LveWindow::initWindow() {
		glfwInit();

		// GLFW�� ����Ͽ� �����츦 ������ �� OpenGL ���ؽ�Ʈ�� �������� �ʵ��� �����ϴ� ���
		// Vulkan�� ����� ���̱⿡ GLFW�� �����츦 ���� ���� ����ϱ� ���� ���ó��
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// ������ ũ�������� ���� ��Ʈ�ڵ�
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// (��, ����, C��Ÿ���� string, ��üȭ���� ���� �Ķ����, openGL ������ ����Ұ��ΰ��� ���� �Ķ����)
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		// é�� 8 ���� �߰��� ����
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	// é�� 8���� �߰��� ����
	void LveWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
		lveWindow->framebufferResized = true;
		lveWindow->width = width;
		lveWindow->height = height;

		// VULKAN���� window size dependent ��ü���� �� ������, ���� �����ؾ� �Ѵ�.

	}

}	// namespace lve