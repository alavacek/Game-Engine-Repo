#include "Event.h"

std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> Event::subscriptions;
std::vector<SubscriptionUpdate> Event::subscriptionUpdates;

void Event::Publish(std::string eventType, luabridge::LuaRef eventObject)
{
	for (std::pair<luabridge::LuaRef, luabridge::LuaRef> sub : subscriptions[eventType])
	{
		luabridge::LuaRef luaRef = sub.first; // Get the LuaRef component pointer

		if (luaRef && (luaRef.isTable() || luaRef.isUserdata()))
		{
			try
			{
				luabridge::LuaRef eventFunc = sub.second;
				if (eventFunc.isFunction())
				{
					eventFunc(luaRef, eventObject);
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ErrorHandling::ReportError("", e);
			}
		}
	}
}

void Event::Subscribe(std::string eventType, luabridge::LuaRef component, luabridge::LuaRef function)
{
	subscriptionUpdates.push_back(SubscriptionUpdate(eventType, component, function, true));
}

void Event::Unsubscribe(std::string eventType, luabridge::LuaRef component, luabridge::LuaRef function)
{
	subscriptionUpdates.push_back(SubscriptionUpdate(eventType, component, function, false));

	
}

void Event::UpdateSubscriptions()
{
	// Handle subscriptions and unsubscriptions at the end of the frame

	for (SubscriptionUpdate update : subscriptionUpdates)
	{

		if (update.addSubscription)
		{
			if (subscriptions.find(update.eventType) != subscriptions.end())
			{
				subscriptions[update.eventType].push_back(std::pair(update.component, update.function));
			}
			else
			{
				subscriptions[update.eventType];
				subscriptions[update.eventType].push_back(std::pair(update.component, update.function));
			}
		}
		// remove
		else
		{
			if (subscriptions.find(update.eventType) != subscriptions.end())
			{
				int indexToRemove = -1;

				for (std::pair<luabridge::LuaRef, luabridge::LuaRef> sub : subscriptions[update.eventType])
				{
					indexToRemove++;

					if (sub.first == update.component && sub.second == update.function)
					{
						break;
					}
				}

				if (indexToRemove != -1)
				{
					subscriptions[update.eventType].erase(subscriptions[update.eventType].begin() + indexToRemove);
				}
				else
				{
					// No subscriptions to this eventType, do I want to report a warning for this?
				}
			}
			else
			{
				// No subscriptions to this eventType, do I want to report a warning for this?

			}
		}
	}


	subscriptionUpdates.clear();
	
}