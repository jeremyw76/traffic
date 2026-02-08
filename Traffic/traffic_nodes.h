#pragma once
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

class Car;
class Lane;

class Enterable {
public:
	virtual bool canEnter(Lane* fromLane) const = 0;
	virtual void accept(Lane* fromLane, Car* car) = 0;
	virtual void processBeforeTick() = 0;
	virtual ~Enterable() = default;
};

class Exitable {
public:
	virtual void processAfterTick() = 0;
	virtual ~Exitable() = default;
};

class Terminal : public Enterable {
private:
	std::vector<Car*> _carBuffer;
public:
	bool canEnter(Lane* fromLane) const override { return true; }
	void accept(Lane* fromLane, Car* car);
	void processBeforeTick() override;
};

class Intersection : public Enterable, public Exitable {
public:
	enum Colors { Red, Yellow, Green };
private:
	class StopLight {
	private:
		Colors _state;
		int _waitTicks = 0;
		bool _isRunning = false;
	public:
		StopLight(Colors initialSignal);
		void processTick();
		void advance();
		Colors getState() const { return _state; }
	};

	std::map<Lane*, Lane*> _connections;
	std::map<Lane*, std::unique_ptr<StopLight>> _stopLights;
	std::map<Lane*, Car*> _carBuffer;
	std::future<void> _cycleFuture;
	bool _areStopLightsRunning = false;
	mutable std::mutex _stopLightMutex;
public:
	bool canEnter(Lane* fromLane) const override;
	void accept(Lane* fromLane, Car* car);
	void processBeforeTick() override;
	void processAfterTick() override;
	void createConnection(Lane* fromLane, Lane* toLane, Intersection::Colors initialSignal);
	void startTrafficSignals();
	void stopTrafficSignals();
	Colors getSignal(Lane* lane) const;
};

class Origin : public Exitable {
private:
	Lane* _lane = nullptr;
public:
	void setLane(Lane* lane) { _lane = lane; }
	Lane* getLane() const { return _lane; }
	void processAfterTick() override;
};