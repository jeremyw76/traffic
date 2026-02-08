#include "notifications.h"

#include <any>
#include <map>
#include <string>
#include <vector>

std::map<std::string, std::vector<Subscriber*>> Notifications::_subscriptions;

void Notifications::emit(const std::string& message, const std::any& data) {
	if (_subscriptions.contains(message)) {
		for (Subscriber* s : _subscriptions[message]) {
			s->notify(message, data);
		}
	}
}

void Notifications::subscribe(const std::string& message, Subscriber* subscriber) {
	_subscriptions[message].push_back(subscriber);
}