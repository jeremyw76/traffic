#pragma once
#include <string>
#include <deque>

class Renderer
{
private:
	static constexpr int VERT_LANE_HEIGHT = 15;
	static constexpr int HORIZ_LANE_LENGTH = 22;
	static constexpr int VERT_OFFSET = 5;
	static constexpr int HORIZ_OFFSET = 17;
	static constexpr int INTERSECTION_HEIGHT = 4;
	static constexpr int INTERSECTION_WIDTH = 5;
	std::deque<int> _historicalVolume;
public:
	enum Axis { Top, Left, Right, Bottom };
	enum Heading { In, Out };
	void renderLanes();
	void renderStopLight(Axis a, const std::string& color);
	void renderCar(Axis a, Heading h, int pos, int laneLength);
	void renderVolumeGraph(int volume);
};