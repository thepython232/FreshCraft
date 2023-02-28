#include "glm/glm.hpp"
#include <iostream>
#include <stdlib.h>

#include "Core\App.h"

int main(int argc, char* argv[]) {
	try {
		App app;
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}