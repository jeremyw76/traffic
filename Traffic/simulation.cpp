#include "simulation.h"

#include "notifications.h"
#include "renderer.h"
#include "screenwriter.h"
#include "traffic_nodes.h"

#include <algorithm>
#include <any>
#include <array>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/// <summary>
/// Simulation architecture:
/// 
/// This is a simulation of traffic moving through a single intersection. Important entities are as follows:
/// 
/// Car - An object that travels along Lanes, beginning at an Origin and ending at a Terminal.
/// Lane - A one-dimensional path along which Cars travel in a single direction.
/// Exitable - Each Lane has an Exitable object at its beginning. This object is capable of emitting cars
///     into the lane.
/// Enterable - Each Lane has an Enterable object at its end. This object is capable of accepting cars
///     from the lane.
/// Origin - Inherits Exitable: it governs the generation of Cars for a given Lane.
/// Intersection - Inherits Enterable and Exitable: it allows multiple Lanes to connect. It also contains the logic that
///     governs when Cars can travel through the Intersection.
/// Terminal - Inherits Exitable: it represents a car's destination. It signals that cars should be deleted.
/// 
/// An Intersection contains a StopLight for each incoming lane that connects to it. It runs a single thread
/// which sends out a pulse. The StopLights count these pulses to advance their own state (Green -> Yellow -> Red -> Green, etc.)
/// 
/// The Simulation owns all Cars, Lanes, and Enterable/Exitables. Lanes and their components are long-lived; their lifespan is
/// essentially the same as the Simulation's. Cars are ephemeral, and while Origins and Terminals are responsible for signalling
/// the beginning and end of a Car's life, the Simulation is responsible for the actual creation and deletion of Cars.
/// 
/// A monitor thread was created for the purpose of testing the simulation. It detects collisions between cars in the same lane.
/// This surfaced several bugs during development.
/// 
/// 
/// 
/// </summary>

const int Car::MIN_ACCEL_INTERVAL = 2;

Car* Lane::findCarAt(int position) const {
	for (Car* car : _cars) {
		if (car->getPosition() == position) {
			return car;
		}
	}

	return nullptr;
}

void Lane::removeCar(Car* car) {
	std::erase(_cars, car);
}

void Car::accelerate()
{
	_speed += MIN_ACCEL_INTERVAL;
}

void Car::decelerate()
{
	_speed -= MIN_ACCEL_INTERVAL;

	if (_speed < 0) {
		_speed = 0;
	}
}

bool Car::canMove() {
	int laneLength = _lane->getLength();
	int interval = _speed;

	if (!isMoving()) {
		interval = MIN_ACCEL_INTERVAL;
	}

	int futurePosition = _position + interval;

	if (futurePosition > laneLength) {
		return _lane->getEnd().canEnter(_lane);
	}
	else if (Car* interferingCar = _lane->findCarAt(futurePosition); interferingCar != nullptr) {
		return interferingCar->canMove();
	}
	else {
		return true;
	}
}

void Car::move()
{
	int laneLength = _lane->getLength();
	int futurePosition = _position + _speed;

	if (futurePosition > laneLength) {
		if (_lane->getEnd().canEnter(_lane)) {
			_lane->getEnd().accept(_lane, this);
			_lane->removeCar(this);
			_lane = nullptr;
		}

		return;
	}

	_position = futurePosition;
}

Simulation::Simulation() :
	laneA(originA, intersection, 50),
	laneB(originB, intersection, 50),
	laneC(originC, intersection, 50),
	laneD(originD, intersection, 50),
	laneE(intersection, terminalE, 50),
	laneF(intersection, terminalF, 50),
	laneG(intersection, terminalG, 50),
	laneH(intersection, terminalH, 50)
{
	originA.setLane(&laneA);
	originB.setLane(&laneB);
	originC.setLane(&laneC);
	originD.setLane(&laneD);

	intersection.createConnection(&laneA, &laneE, Intersection::Red);
	intersection.createConnection(&laneB, &laneF, Intersection::Green);
	intersection.createConnection(&laneC, &laneG, Intersection::Red);
	intersection.createConnection(&laneD, &laneH, Intersection::Green);

	Notifications::subscribe(Notifications::DELETE_CAR_MESSAGE, this);
	Notifications::subscribe(Notifications::CREATE_CAR_MESSAGE, this);
}

void Simulation::notify(const std::string& message, const std::any& data) {

	if (message == Notifications::DELETE_CAR_MESSAGE) {
		try {
			Car* car = std::any_cast<Car*>(data);

			auto iter = std::find_if(_cars.begin(), _cars.end(), [car](std::unique_ptr<Car>& uniqueCar) {
				return uniqueCar.get() == car;
				});

			_cars.erase(iter);
		}
		catch (const std::bad_any_cast& e) {
			// Log to observability service
			std::cerr << "Bad any_cast in notify(): " << e.what() << std::endl;
		}
	}
	else if (message == Notifications::CREATE_CAR_MESSAGE) {
		try {
			Lane* lane = std::any_cast<Lane*>(data);
			if (!lane->findCarAt(0)) {
				std::unique_ptr<Car> car = std::make_unique<Car>();
				car->setLane(lane);
				lane->addCar(car.get());
				_cars.push_back(std::move(car));
			}
		}
		catch (const std::bad_any_cast& e) {
			// Log to observability service
			std::cerr << "Bad any_cast in notify(): " << e.what() << std::endl;
		}
	}
}

void Simulation::runSimulationThread() {
	_simulationCycle = std::async(std::launch::async, [this]() {
		std::array<Lane*, 8> lanes { &laneA, &laneB, &laneC, &laneD, &laneE, &laneF, &laneG, &laneH };
		static constexpr std::chrono::milliseconds SIMULATION_INTERVAL_MS(250);
		

		while (_isRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATION_INTERVAL_MS));
			std::lock_guard<std::mutex> lock(_simulationMutex);
			
			for (Lane* lane : lanes) {
				lane->getEnd().processBeforeTick();
			}

			for (const std::unique_ptr<Car>& car : _cars) {
				if (car->getLane() != nullptr) {
					if (car->canMove()) {
						if (!car->isMoving()) {
							car->accelerate();
						}
					}
					else {
						if (car->isMoving()) {
							car->decelerate();
						}
					}
					car->move();
				}
			}

			for (Lane* lane : lanes) {
				lane->getBeginning().processAfterTick();
			}

			render();
		}
		});
}

void Simulation::runMonitorThread() {
	_monitorCycle = std::async(std::launch::async, [this]() {
		std::vector<std::pair<Lane*, int>> lanePositions;
		lanePositions.resize(400);

		std::ofstream logFile;
		logFile.open("collisions.log", std::ofstream::trunc);
		if (!logFile.is_open()) {
			std::cerr << "Failed to open collisions.log" << std::endl;
			return;
		}

		while (_isRunning) {
			lanePositions.clear();
			std::this_thread::sleep_for(std::chrono::milliseconds(125));
			std::lock_guard<std::mutex> lock(_simulationMutex);
			int index = 0;
			for (auto& car : _cars) {
				int carPosition = car->getPosition();
				if (car->getLane() != nullptr) {
					for (std::pair<Lane*, int>& position : lanePositions) {
						if (car->getLane() == position.first && car->getPosition() == position.second) {
							logFile << "***Collision involving " << car << " (index " << index++ << ") at " << carPosition << " and " << position.second << " at lane " << position.first << std::endl;
						}
					}
					lanePositions.push_back(std::pair<Lane*, int>(car->getLane(), carPosition));
				}
			}
			logFile.flush();
		}
		});
}

void Simulation::start() {
	intersection.startTrafficSignals();
	_isRunning = true;
	runSimulationThread();
	runMonitorThread();
}

void Simulation::stop() {
	_isRunning = false;
	intersection.stopTrafficSignals();
}

void Simulation::render() {
	_renderer.renderLanes();

	Intersection::Colors color = intersection.getSignal(&laneA);	
	_renderer.renderStopLight(Renderer::Top, convertSignalToScreen(color));

	color = intersection.getSignal(&laneB);
	_renderer.renderStopLight(Renderer::Left, convertSignalToScreen(color));

	color = intersection.getSignal(&laneC);
	_renderer.renderStopLight(Renderer::Bottom, convertSignalToScreen(color));

	color = intersection.getSignal(&laneD);
	_renderer.renderStopLight(Renderer::Right, convertSignalToScreen(color));

	for (const std::unique_ptr<Car>& car : _cars) {
		if (car->getLane() == nullptr) {
			continue;
		}

		int pos = car->getPosition();
		int laneLength = car->getLane()->getLength();
		Renderer::Axis axis = Renderer::Top;
		Renderer::Heading heading = Renderer::In;

		// Lanes moving toward intersection, starting from top, counter-clockwise
		if (car->getLane() == &laneA) {
			axis = Renderer::Top;
			heading = Renderer::In;
		}
		else if (car->getLane() == &laneB) {
			axis = Renderer::Left;
			heading = Renderer::In;
		}
		else if (car->getLane() == &laneC) {
			axis = Renderer::Bottom;
			heading = Renderer::In;
		}
		else if (car->getLane() == &laneD) {
			axis = Renderer::Right;
			heading = Renderer::In;
		}
		// Lanes moving away from intersection, starting from bottom, counter-clockwise
		else if (car->getLane() == &laneE) {
			axis = Renderer::Bottom;
			heading = Renderer::Out;
		}
		else if (car->getLane() == &laneF) {
			axis = Renderer::Right;
			heading = Renderer::Out;
		}
		else if (car->getLane() == &laneG) {
			axis = Renderer::Top;
			heading = Renderer::Out;
		}
		else if (car->getLane() == &laneH) {
			axis = Renderer::Left;
			heading = Renderer::Out;
		}

		_renderer.renderCar(axis, heading, pos, laneLength);
	}

	_renderer.renderVolumeGraph(static_cast<int>(_cars.size()));
}

const std::string& Simulation::convertSignalToScreen(Intersection::Colors color) const
{
	switch (color) {
	case Intersection::Red:
		return ScreenWriter::RED;
		break;
	case Intersection::Yellow:
		return ScreenWriter::YELLOW;
		break;
	case Intersection::Green:
		return ScreenWriter::GREEN;
		break;
	default:
		// Should never reach here. All enum values are covered.
		static const std::string unknown = "unknown";
		return unknown;
	}
}