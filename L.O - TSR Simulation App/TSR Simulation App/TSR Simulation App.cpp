#include "TSR_Simulation.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main() {
	try {
		TSR_Simulation sim;
		return sim.Run();
	} catch (const std::exception& e) {
		std::cerr << "Error occured: " << e.what();
		return EXIT_FAILURE;
	}
}