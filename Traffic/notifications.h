#pragma once
#include <any>
#include <map>
#include <string>
#include <vector>

class Subscriber {
public:
	virtual void notify(const std::string& message, const std::any& data) = 0;
	virtual ~Subscriber() = default;
};

class Notifications {
private:
	static std::map<std::string, std::vector<Subscriber*>> _subscriptions;
public:
	static void emit(std::string& message, const std::any& data);
	static void subscribe(std::string& message, Subscriber* subscriber);

	inline const static std::string DELETE_CAR_MESSAGE = "delete_car";
	inline const static std::string CREATE_CAR_MESSAGE = "create_car";
};