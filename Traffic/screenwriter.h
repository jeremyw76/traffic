#pragma once
#include <string>

class ScreenWriter
{
public:
	static const std::string RED;
	static const std::string YELLOW;
	static const std::string GREEN;
	static const std::string BLUE;
	static const std::string WHITE;

	static void init();
	static void clearScreen();
	static void put(int col, int row, const std::string& text, const std::string& color = "");
};