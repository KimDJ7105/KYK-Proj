
#include "first_app.h"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
	lve::FirstApp app{};

	// 오류가 발생할 때를 대비한 try catch
	// 예외 객체를 캐치하고 해당 예외 객체를 참조하는 방법을 지정
	try {
		app.run();
	}
	catch (const std::exception& e) {
		//만약 에러가 발생한다면 콘솔창에 출력하도록 한다.
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}