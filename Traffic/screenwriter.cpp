#include "screenwriter.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

void ScreenWriter::init() {
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
#endif
	std::cout << "\033[?25l";
}

const std::string ScreenWriter::RED = "\033[31m";
const std::string ScreenWriter::YELLOW = "\033[33m";
const std::string ScreenWriter::GREEN = "\033[32m";
const std::string ScreenWriter::BLUE = "\033[34m";
const std::string ScreenWriter::WHITE = "\033[0m";

void ScreenWriter::clearScreen() {
	std::cout << "\033[2J\033[1;1H\033[0m" << std::flush;
}

void ScreenWriter::put(int col, int row, const std::string& text, const std::string& color) {
	std::cout << "\033[" << row << ";" << col << "H";
	std::cout << color << text << std::flush;
}