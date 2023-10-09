// ���� ���ø����̼�

#pragma once

#include "lve_device.h"
#include "lve_game_object.h"
#include "lve_renderer.h"
#include "lve_window.h"


// std
#include <memory>		// ���������� ������Ʈ�� ����� �����ϸ鼭 �߰��� �޸�, ����ũ�����͸� ����Ұ��̴�.
#include <vector>

namespace lve {
	class FirstApp {
		// FirstApp�� ������� �� �����찡 ��������� ����ɰ��̴�.
		// FirstApp�� ����� �� ������� �ڵ����� ����ɰ��̴�.
		// �����Ϳ� ���� �޸� �Ҵ��� ������� �ʾҴ�.

	public:
		// �̰����� �������� ũ�⸦ ������ �� �ִ�.
		static constexpr int WIDTH = 800;										// �������� ��
		static constexpr int HEIGHT = 600;										// �������� ����

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