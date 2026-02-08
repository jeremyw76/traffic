#include "screenwriter.h"
#include "simulation.h"

#include <conio.h>

#ifdef RUN_TESTS
#include "gtest/gtest.h"
#else
int main()
{
	ScreenWriter::init();
	ScreenWriter::clearScreen();
	Simulation simulation;
	simulation.start();
	
	bool isRunning = true;
	while (isRunning) {
		if (_kbhit()) {
			isRunning = false;
		}
	}

	simulation.stop();
}
#endif