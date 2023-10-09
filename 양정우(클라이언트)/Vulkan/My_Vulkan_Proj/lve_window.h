// vulkan 엔진을 활용한 윈도우를 만들기 위한 헤더

#pragma once

#define GLFW_INCLUDE_VULKAN				// GLFW가 Vulkan을 포함하고있다는 것을 명시해줌
#include <GLFW/glfw3.h>					// 윈도우를 띄우기 위한 헤더
										// 플랫폼에 종속되지 않기 위해 GLFW를 사용하여 윈도우 창을 만듬

#include <string>

namespace lve {
	
	// 윈도우를 위한 클래스
	class LveWindow {

	public:
		LveWindow(int w, int h, std::string name);		// 윈도우를 만들기 위한 생성자
		~LveWindow();									// 소멸자

		// 복사생성자와 복사연산자를 제거
		// glfw 윈도우 포인터를 사용하기 때문에 제거하는것이다.
		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		// run중 윈도우를 닫을지 말지를 판단하는 함수
		bool shouldClose() { return glfwWindowShouldClose(window); }

		// 챕터 5에서 추가한 내용
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		// 챕터 8에서 추가한 내용
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const { return window; }

		// 챕터 3에서 추가된 lve_device에 존재하는 함수를 정의내리기 위한 추가 코드
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		// 챕터 8에서 추가한 내용
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		void initWindow();

		int					width;								// 윈도우의 폭
		int					height;								// 윈도우의 높이
		bool				framebufferResized = false;			// 윈도우 사이즈가 바뀌는것을 알려주기 위함

		std::string			windowName;					// 윈도우 이름

		GLFWwindow*			window;						// GLFW윈도우를 만들기 위한 private 변수
	};
}	// namespace lve