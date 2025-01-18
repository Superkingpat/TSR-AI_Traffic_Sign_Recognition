#include "TSR_Simulation.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main() {
	try {
		RUN_COMUNICATION = true;

		std::thread comunicationThread(start);

		TSR_Simulation sim;
		int result = sim.Run();

		RUN_COMUNICATION = false;
		comunicationThread.join();

		return result;
	} catch (const std::exception& e) {
		std::cerr << "Error occured: " << e.what();
		return EXIT_FAILURE;
	}
}