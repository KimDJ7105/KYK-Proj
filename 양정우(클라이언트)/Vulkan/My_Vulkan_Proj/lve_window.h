// vulkan ������ Ȱ���� �����츦 ����� ���� ���

#pragma once

#define GLFW_INCLUDE_VULKAN				// GLFW�� Vulkan�� �����ϰ��ִٴ� ���� �������
#include <GLFW/glfw3.h>					// �����츦 ���� ���� ���
										// �÷����� ���ӵ��� �ʱ� ���� GLFW�� ����Ͽ� ������ â�� ����

#include <string>

namespace lve {
	
	// �����츦 ���� Ŭ����
	class LveWindow {

	public:
		LveWindow(int w, int h, std::string name);		// �����츦 ����� ���� ������
		~LveWindow();									// �Ҹ���

		// ��������ڿ� ���翬���ڸ� ����
		// glfw ������ �����͸� ����ϱ� ������ �����ϴ°��̴�.
		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		// run�� �����츦 ������ ������ �Ǵ��ϴ� �Լ�
		bool shouldClose() { return glfwWindowShouldClose(window); }

		// é�� 5���� �߰��� ����
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		// é�� 8���� �߰��� ����
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const { return window; }

		// é�� 3���� �߰��� lve_device�� �����ϴ� �Լ��� ���ǳ����� ���� �߰� �ڵ�
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		// é�� 8���� �߰��� ����
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		void initWindow();

		int					width;								// �������� ��
		int					height;								// �������� ����
		bool				framebufferResized = false;			// ������ ����� �ٲ�°��� �˷��ֱ� ����

		std::string			windowName;					// ������ �̸�

		GLFWwindow*			window;						// GLFW�����츦 ����� ���� private ����
	};
}	// namespace lve