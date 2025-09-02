#pragma once

#include <unordered_map>
#include <functional>
#include <string>

class EventBus
{
public:
	using Callback = std::function<void()>;

	void subscribe(const std::string& eventType, Callback callback) {
		subscribers[eventType].push_back(callback);
	}

	void publish(const std::string& eventType) {
		if (subscribers.find(eventType) != subscribers.end())
			for (auto& callback : subscribers[eventType])
				callback();
	}

private:
	std::unordered_map<std::string, std::vector<Callback>> subscribers;
};