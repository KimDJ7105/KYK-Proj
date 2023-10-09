
#include "first_app.h"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
	lve::FirstApp app{};

	// ������ �߻��� ���� ����� try catch
	// ���� ��ü�� ĳġ�ϰ� �ش� ���� ��ü�� �����ϴ� ����� ����
	try {
		app.run();
	}
	catch (const std::exception& e) {
		//���� ������ �߻��Ѵٸ� �ܼ�â�� ����ϵ��� �Ѵ�.
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}