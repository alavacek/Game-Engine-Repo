#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <unordered_map>
#include <vector>

#include "ErrorHandling.h"
#include "lua.hpp"
#include "LuaBridge.h"

struct SubscriptionUpdate
{
	SubscriptionUpdate(std::string eventType, luabridge::LuaRef component, luabridge::LuaRef function, bool addSubscription)
		: eventType(eventType), component(component), function(function), addSubscription(addSubscription) {};

	std::string eventType;
	luabridge::LuaRef component;
	luabridge::LuaRef function;
	bool addSubscription;
};

class Event
{
public:
	static void Publish(std::string eventType, luabridge::LuaRef eventObject);
	static void Subscribe(std::string eventType, luabridge::LuaRef component, luabridge::LuaRef function);
	static void Unsubscribe(std::string eventType, luabridge::LuaRef component, luabridge::LuaRef function);

	static void UpdateSubscriptions();
private:

	// event type - (lua function and table)
	static std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> subscriptions;

	static std::vector<SubscriptionUpdate> subscriptionUpdates;
};

#endif
