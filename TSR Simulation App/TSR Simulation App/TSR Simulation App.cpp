#include "TSR_Simulation.h"

int main() {
	try {
		TSR_Simulation sim;
		return sim.Run();
	} catch (const std::exception& e) {
		std::cerr << "Error occured: " << e.what();
		return EXIT_FAILURE;
	}
}