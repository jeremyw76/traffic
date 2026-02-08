#include "renderer.h"
#include "screenwriter.h"

#include <string>

void Renderer::renderLanes() {
	for (int i = 0; i < VERT_LANE_HEIGHT; i++) {
		ScreenWriter::put(40, i + VERT_OFFSET, "|", ScreenWriter::WHITE); // Lane A
		ScreenWriter::put(42, i + VERT_OFFSET, "|"); // Lane G
		ScreenWriter::put(40, i + VERT_OFFSET + VERT_LANE_HEIGHT + INTERSECTION_HEIGHT, "|"); // Lane E
		ScreenWriter::put(42, i + VERT_OFFSET + VERT_LANE_HEIGHT + INTERSECTION_HEIGHT, "|"); // Lane C
	}

	for (int i = 0; i < HORIZ_LANE_LENGTH; i++) {
		ScreenWriter::put(i + HORIZ_OFFSET, 21, "-"); // Lane H
		ScreenWriter::put(i + HORIZ_OFFSET, 22, "-"); // Lane B
		ScreenWriter::put(i + HORIZ_OFFSET + HORIZ_LANE_LENGTH + INTERSECTION_WIDTH, 21, "-"); // Lane D
		ScreenWriter::put(i + HORIZ_OFFSET + HORIZ_LANE_LENGTH + INTERSECTION_WIDTH, 22, "-"); // Lane F
	}
}

void Renderer::renderStopLight(Axis a, const std::string& color) {
	switch (a) {
	case Top:
		ScreenWriter::put(40, 20, "---", color);
		break;
	case Left:
		ScreenWriter::put(39, 21, "|", color);
		ScreenWriter::put(39, 22, "|", color);
		break;
	case Right:
		ScreenWriter::put(43, 21, "|", color);
		ScreenWriter::put(43, 22, "|", color);
		break;
	case Bottom:
		ScreenWriter::put(40, 23, "---", color);
		break;
	}
}

void Renderer::renderCar(Axis a, Heading h, int pos, int laneLength) {
	int row = 0;
	int col = 0;

	switch (h) {
	case In:
		switch (a) {
		case Top:
			col = 40;
			row = static_cast<int>(pos * (VERT_LANE_HEIGHT - 1) / laneLength + VERT_OFFSET);
			break;
		case Left:
			row = 22;
			col = static_cast<int>(pos * (HORIZ_LANE_LENGTH - 1) / laneLength + HORIZ_OFFSET);
			break;
		case Bottom:
			col = 42;
			row = static_cast<int>(VERT_OFFSET + INTERSECTION_HEIGHT + (VERT_LANE_HEIGHT * 2) - (pos * (VERT_LANE_HEIGHT - 1) / laneLength) - 1);
			break;
		case Right:
			row = 21;
			col = static_cast<int>(HORIZ_OFFSET + INTERSECTION_WIDTH + (HORIZ_LANE_LENGTH * 2) - (pos * (HORIZ_LANE_LENGTH - 1) / laneLength) - 1);
			break;
		}
		break;
	case Out:
		switch (a) {
		case Top:
			col = 42;
			row = static_cast<int>(VERT_OFFSET + VERT_LANE_HEIGHT - (pos * (VERT_LANE_HEIGHT - 1) / laneLength) - 1);
			break;
		case Left:
			row = 21;
			col = static_cast<int>(HORIZ_OFFSET + HORIZ_LANE_LENGTH - (pos * (HORIZ_LANE_LENGTH - 1) / laneLength) - 1);
			break;
		case Bottom:
			col = 40;
			row = static_cast<int>((pos * (VERT_LANE_HEIGHT - 1) / laneLength) + VERT_OFFSET + VERT_LANE_HEIGHT + INTERSECTION_HEIGHT);
			break;
		case Right:
			row = 22;
			col = static_cast<int>((pos * (HORIZ_LANE_LENGTH - 1) / laneLength) + HORIZ_OFFSET + HORIZ_LANE_LENGTH + INTERSECTION_WIDTH);
			break;
		}
		break;
	}

	ScreenWriter::put(col, row, "O", ScreenWriter::BLUE);
}

void Renderer::renderVolumeGraph(int volume) {
	for (int i = 3; i < 18; i++) {
		ScreenWriter::put(10, i, "|                   ", ScreenWriter::WHITE);
	}
	ScreenWriter::put(10, 18, "--------------------");
	ScreenWriter::put(12, 19, "Traffic volume");

	_historicalVolume.push_back(volume);
	if (_historicalVolume.size() > 19) {
		_historicalVolume.pop_front();
	}

	int col = 11;
	for (int volume : _historicalVolume) {
		int row = 17 - (static_cast<int>(volume) / 10);
		std::string color = ScreenWriter::WHITE;
		if (row < 3) {
			row = 3;
			color = ScreenWriter::RED;
		}
		ScreenWriter::put(col, row, "-", color);
		col++;
	}
}
