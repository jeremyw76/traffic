#include "traffic_nodes.h"
#include "notifications.h"
#include "simulation.h"

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <random>
#include <thread>

void Terminal::processBeforeTick() {
	for (Car* car : _carBuffer) {
		Notifications::emit(Notifications::DELETE_CAR_MESSAGE, car);
	}
	_carBuffer.clear();
}

void Terminal::accept(Lane* fromLane, Car* car) {
	_carBuffer.push_back(car);
}

void Origin::processAfterTick() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> distrib(1, 100);

	int roll = distrib(gen);

	if (roll % 5 == 0) {
		Notifications::emit(Notifications::CREATE_CAR_MESSAGE, _lane);
	}
}

bool Intersection::canEnter(Lane* fromLane) const {
	bool isRedLight = _stopLights.at(fromLane)->getState() == Red;

	if (!isRedLight) {
		auto search = _carBuffer.find(_connections.at(fromLane));
		return search == _carBuffer.end() || search->second == nullptr;
	}
	else {
		return false;
	}
}

void Intersection::createConnection(Lane* fromLane, Lane* toLane, Intersection::Colors initialSignal) {
	_connections[fromLane] = toLane;
	_stopLights[fromLane] = std::make_unique<StopLight>(initialSignal);
}

void Intersection::accept(Lane* fromLane, Car* car) {
	Lane* exit = _connections.at(fromLane);
	_carBuffer[exit] = car;
}

void Intersection::processAfterTick() {
	for (auto pair : _carBuffer) {
		Car* car = pair.second;
		if (car != nullptr) {
			Lane* nextLane = pair.first;

			if (nextLane->findCarAt(0) != nullptr) {
				continue;
			}

			nextLane->addCar(car);
			car->setLane(nextLane);
			car->resetPosition();
			
			_carBuffer[nextLane] = nullptr;
		}
	}
}

void Intersection::processBeforeTick() {}

void Intersection::startTrafficSignals() {
	_areStopLightsRunning = true;

	_cycleFuture = std::async(std::launch::async, [this]() {
		const int PULSE_INTERVAL_MS = 1000;
		while (_areStopLightsRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(PULSE_INTERVAL_MS));

			std::lock_guard<std::mutex> hold(_stopLightMutex);
			for (auto& pair : _stopLights) {
				StopLight* stoplight = pair.second.get();
				stoplight->processTick();
			}
		}
		});
}

void Intersection::stopTrafficSignals() {
	_areStopLightsRunning = false;
}

Intersection::Colors Intersection::getSignal(Lane* lane) const {
	std::lock_guard<std::mutex> hold(_stopLightMutex);
	return _stopLights.at(lane)->getState();
}

Intersection::StopLight::StopLight(Colors initialSignal) : _state(initialSignal) {
	switch (initialSignal) {
	case Red:
		_waitTicks = 5;
		break;
	case Green:
		_waitTicks = 4;
		break;
	case Yellow:
		_waitTicks = 1;
		break;
	}
}

void Intersection::StopLight::processTick() {
	_waitTicks--;

	if (_waitTicks == 0) {
		advance();
	}
}

void Intersection::StopLight::advance() {
	switch (_state) {
	case Red:
		_state = Green;
		_waitTicks = 4;
		break;
	case Green:
		_state = Yellow;
		_waitTicks = 1;
		break;
	case Yellow:
		_state = Red;
		_waitTicks = 5;
		break;
	}
}