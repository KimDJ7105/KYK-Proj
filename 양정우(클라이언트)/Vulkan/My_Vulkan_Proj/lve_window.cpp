// 윈도우 창을 띄우기 위한 함수를 정의

#include "lve_window.h"

// std
#include <stdexcept>

namespace lve {
	
	// 윈도우 생성자
	LveWindow::LveWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	// 윈도우 소멸자
	LveWindow::~LveWindow() {
		// 윈도우를 제거, 메모리 해제
		glfwDestroyWindow(window);

		// GLFW 라이브러리를 종료, 리소스 정리
		glfwTerminate();
	}
	
	// 윈도우 초기화 구현
	void LveWindow::initWindow() {
		glfwInit();

		// GLFW를 사용하여 윈도우를 생성할 때 OpenGL 컨텍스트를 생성하지 않도록 지정하는 명령
		// Vulkan을 사용할 것이기에 GLFW는 윈도우를 만들 때만 사용하기 위한 명령처리
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// 윈도우 크기조절을 위한 힌트코드
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// (폭, 높이, C스타일의 string, 전체화면을 위한 파라미터, openGL 문장을 사용할것인가에 대한 파라미터)
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		// 챕터 8 에서 추가한 내용
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	// 챕터 8에서 추가한 내용
	void LveWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
		lveWindow->framebufferResized = true;
		lveWindow->width = width;
		lveWindow->height = height;

		// VULKAN에서 window size dependent 객체들을 다 날리고, 새로 생성해야 한다.

	}

}	// namespace lve