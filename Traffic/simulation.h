#pragma once
#include "notifications.h"
#include "renderer.h"
#include "traffic_nodes.h"

#include <any>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class Lane {
private:
	int _length;
	std::vector<Car*> _cars;
	Exitable& _beginning;
	Enterable& _end;
public:
	Lane(Exitable& beginning, Enterable& end, int length) : _beginning(beginning), _end(end), _length(length) {}
	int getLength() const { return _length; }
	Enterable& getEnd() const { return _end; }
	Exitable& getBeginning() const { return _beginning; }
	void addCar(Car* car) { _cars.push_back(car); }
	void removeCar(Car* car);
	Car* findCarAt(int position) const;
};

class Car {
private:
	static const int MIN_ACCEL_INTERVAL;
	int _speed = 0;
	Lane* _lane = nullptr;
	int _position = 0;
public:
	void setLane(Lane* lane) { _lane = lane; }
	Lane* getLane() const { return _lane; }
	bool isMoving() const { return _speed > 0; }
	bool canMove();
	int getPosition() const { return _position; }
	void resetPosition() { _position = 0; }
	void accelerate();
	void decelerate();
	void move();
};

class Simulation : public Subscriber
{
private:
	Intersection intersection;
	Origin originA, originB, originC, originD;
	Terminal terminalE, terminalF, terminalG, terminalH;
	Lane laneA, laneB, laneC, laneD, laneE, laneF, laneG, laneH;

	std::vector<std::unique_ptr<Car>> _cars;
	std::mutex _simulationMutex;
	bool _isRunning = false;

	std::future<void> _simulationCycle;
	std::future<void> _monitorCycle;

	Renderer _renderer;

	void runSimulationThread();
	void runMonitorThread();
	void render();
	const std::string& convertSignalToScreen(Intersection::Colors color) const;
public:
	Simulation();
	void start();
	void stop();
	void notify(const std::string& message, const std::any& data) override;
};