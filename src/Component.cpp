#include "Component.h"

void Component::Start()
{
	std::shared_ptr<luabridge::LuaRef> luaRefPtr = luaRef; // Get the LuaRef pointer
	luabridge::LuaRef luaRef = *luaRefPtr;

	if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
	{
		try
		{
			// Run if enabled
			// Note, The OnStart function does not run again if a component is re-enabled. 
			// A component will only ever try and run its OnStart function once (upon creation)
			luabridge::LuaRef isEnabled = (luaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				luabridge::LuaRef onStartFunc = (luaRef)["OnStart"];
				if (onStartFunc.isFunction())
				{
					onStartFunc(luaRef);
				}
			}

		}
		catch (const luabridge::LuaException& e)
		{
			ErrorHandling::ReportError(owningEntityName, e);
		}

	}
}

void Component::Update()
{
	std::shared_ptr<luabridge::LuaRef> luaRefPtr = luaRef; // Get the LuaRef pointer
	luabridge::LuaRef luaRef = *luaRefPtr;

	if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
	{
		try
		{
			// Run if enabled
			luabridge::LuaRef isEnabled = (luaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				luabridge::LuaRef onUpdateFunc = (luaRef)["OnUpdate"];
				if (onUpdateFunc.isFunction())
				{
					onUpdateFunc(luaRef);
				}
			}
		}
		catch (const luabridge::LuaException& e)
		{
			ErrorHandling::ReportError(owningEntityName, e);
		}
	}
}

void Component::LateUpdate()
{
	std::shared_ptr<luabridge::LuaRef> luaRefPtr = luaRef; // Get the LuaRef pointer
	luabridge::LuaRef luaRef = *luaRefPtr;

	if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
	{
		try
		{
			// Run if enabled
			luabridge::LuaRef isEnabled = (luaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				luabridge::LuaRef onLateUpdateFunc = (luaRef)["OnLateUpdate"];
				if (onLateUpdateFunc.isFunction())
				{
					onLateUpdateFunc(luaRef);
				}
			}
		}
		catch (const luabridge::LuaException& e)
		{
			ErrorHandling::ReportError(owningEntityName, e);
		}
	}
}

//void Component::OnCollisionEnter(Collision2D collision)
//{
//	std::shared_ptr<luabridge::LuaRef> luaRefPtr = luaRef; // Get the LuaRef pointer
//	luabridge::LuaRef luaRef = *luaRefPtr;
//
//	if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
//	{
//		try
//		{
//			// Run if enabled
//			luabridge::LuaRef isEnabled = (luaRef)["enabled"];
//			if (isEnabled.isBool() && isEnabled)
//			{
//				luabridge::LuaRef onCollisionEnterFunc = (luaRef)["OnCollisionEnter"];
//				if (onCollisionEnterFunc.isFunction())
//				{
//					onCollisionEnterFunc(luaRef);
//				}
//			}
//		}
//		catch (const luabridge::LuaException& e)
//		{
//			ErrorHandling::ReportError(owningEntityName, e);
//		}
//	}
//}
//
//void Component::OnCollisionExit(Collision2D collision)
//{
//	std::shared_ptr<luabridge::LuaRef> luaRefPtr = luaRef; // Get the LuaRef pointer
//	luabridge::LuaRef luaRef = *luaRefPtr;
//
//	if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
//	{
//		try
//		{
//			// Run if enabled
//			luabridge::LuaRef isEnabled = (luaRef)["enabled"];
//			if (isEnabled.isBool() && isEnabled)
//			{
//				luabridge::LuaRef onCollisionExitFunc = (luaRef)["OnCollisionExit"];
//				if (onCollisionExitFunc.isFunction())
//				{
//					onCollisionExitFunc(luaRef);
//				}
//			}
//		}
//		catch (const luabridge::LuaException& e)
//		{
//			ErrorHandling::ReportError(owningEntityName, e);
//		}
//	}
//}