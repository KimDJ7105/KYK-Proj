// 메인 애플리케이션

#pragma once

#include "lve_device.h"
#include "lve_game_object.h"
#include "lve_renderer.h"
#include "lve_window.h"


// std
#include <memory>		// 파이프라인 오브젝트를 만들기 시작하면서 추가한 메모리, 유니크포인터를 사용할것이다.
#include <vector>

namespace lve {
	class FirstApp {
		// FirstApp가 만들어질 때 윈도우가 만들어지고 실행될것이다.
		// FirstApp가 종료될 때 윈도우는 자동으로 종료될것이다.
		// 포인터와 동적 메모리 할당을 사용하지 않았다.

	public:
		// 이곳에서 윈도우의 크기를 조절할 수 있다.
		static constexpr int WIDTH = 800;										// 윈도우의 폭
		static constexpr int HEIGHT = 600;										// 윈도우의 높이

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();

	private:

		void loadGameObjects();

		LveWindow lveWindow{ WIDTH , HEIGHT, "2019182023_LORG"};
		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice };

		std::vector<LveGameObject> gameObjects;
	};
}	// namespace lve