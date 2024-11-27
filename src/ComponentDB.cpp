#include "ComponentDB.h"

std::unordered_map<std::string, Component*> ComponentDB::components;
int ComponentDB::numRuntimeAddedComponents = 0;
lua_State* ComponentDB::luaState;

void ComponentDB::LoadComponents()
{
	std::string componentDirectoryPath = "resources/component_types";
	luaState = LuaStateManager::GetLuaState();

	if (std::filesystem::exists(componentDirectoryPath))
	{
		for (const auto& entry : std::filesystem::directory_iterator(componentDirectoryPath)) {
			if (entry.is_regular_file() && entry.path().extension() == ".lua") 
			{
				std::string fileNameWithoutExtension = entry.path().stem().string();			

				std::string filePath = entry.path().string();
				std::replace(filePath.begin(), filePath.end(), '\\', '/');

				if (luaL_dofile(luaState, filePath.c_str()) != LUA_OK)
				{
					std::cout << "problem with lua file " << fileNameWithoutExtension;
					exit(0);
				}

				luabridge::LuaRef luaRef = luabridge::getGlobal(luaState, fileNameWithoutExtension.c_str());

				// all components should start as enabled, since these are templates for instances of components
				// we can just set these ones to enabled
				// NOTE: not sure this is actually doing something, had to try something else in instantiate function
				luaRef["enabled"] = true;

				std::shared_ptr<luabridge::LuaRef> luaRefPtr = std::make_shared<luabridge::LuaRef>(luaRef);
				
				bool hasStart = false;
				bool hasUpdate = false;
				bool hasLateUpdate = false;

				// has start
				luabridge::LuaRef onStartFunc = (luaRef)["OnStart"];
				if (onStartFunc.isFunction())
				{
					hasStart = true;
				}

				// has update
				luabridge::LuaRef onUpdateFunc = (luaRef)["OnUpdate"];
				if (onUpdateFunc.isFunction())
				{
					hasUpdate = true;
				}

				// has late update
				luabridge::LuaRef onLateUpdateFunc = (luaRef)["OnLateUpdate"];
				if (onLateUpdateFunc.isFunction())
				{
					hasLateUpdate = true;
				}

				Component* newComponentBase = new Component(luaRefPtr, fileNameWithoutExtension, hasStart, hasUpdate, hasLateUpdate);

				components[fileNameWithoutExtension] = newComponentBase;
			}
		}
	}

    // C++ Components
    // For now just called Rigidbody since this is a 2D engine
    luabridge::getGlobalNamespace(luaState)
        .beginClass<Rigidbody2DLuaRef>("Rigidbody")
        // Component Properties
        .addFunction("OnStart", &Rigidbody2DLuaRef::Start)
        .addFunction("OnUpdate", &Rigidbody2DLuaRef::Update)
        .addFunction("OnLateUpdate", &Rigidbody2DLuaRef::LateUpdate)
        .addProperty("entity", &Rigidbody2DLuaRef::owningEntity)
        .addProperty("enabled", &Rigidbody2DLuaRef::isEnabled)
        // Rigidbody Properties
        .addFunction("GetRotation", &Rigidbody2DLuaRef::GetRotation)
        .addFunction("GetPosition", &Rigidbody2DLuaRef::GetPosition)
        .addProperty("x", &Rigidbody2DLuaRef::x)
        .addProperty("y", &Rigidbody2DLuaRef::y)
        .addProperty("body_type", &Rigidbody2DLuaRef::bodyType)
        .addProperty("precise", &Rigidbody2DLuaRef::precise)
        .addProperty("gravity_scale", &Rigidbody2DLuaRef::gravityScale)
        .addProperty("density", &Rigidbody2DLuaRef::density)
        .addProperty("angular_friction", &Rigidbody2DLuaRef::angularFriction)
        .addProperty("rotation", &Rigidbody2DLuaRef::rotation)
        .addProperty("has_collider", &Rigidbody2DLuaRef::hasCollider)
        .addProperty("has_trigger", &Rigidbody2DLuaRef::hasTrigger)
        .addFunction("AddForce", &Rigidbody2DLuaRef::AddForce)
        .addFunction("SetVelocity", &Rigidbody2DLuaRef::SetVelocity)
        .addFunction("SetPosition", &Rigidbody2DLuaRef::SetPosition)
        .addFunction("Rotation", &Rigidbody2DLuaRef::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody2DLuaRef::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody2DLuaRef::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody2DLuaRef::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody2DLuaRef::SetRightDirection)
        .addFunction("GetVelocity", &Rigidbody2DLuaRef::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody2DLuaRef::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody2DLuaRef::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody2DLuaRef::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody2DLuaRef::GetRightDirection)
        // Collider properties
        .addProperty("collider_type", &Rigidbody2DLuaRef::colliderType)
        .addProperty("width", &Rigidbody2DLuaRef::colliderWidth)
        .addProperty("height", &Rigidbody2DLuaRef::colliderHeight)
        .addProperty("radius", &Rigidbody2DLuaRef::colliderRadius)
        .addProperty("friction", &Rigidbody2DLuaRef::colliderFriction)
        .addProperty("bounciness", &Rigidbody2DLuaRef::colliderBounciness)
        .endClass();

    Rigidbody2DLuaRef* RB = new Rigidbody2DLuaRef();
    luabridge::push(luaState, RB);

    luabridge::LuaRef luaRef = luabridge::LuaRef::fromStack(luaState, -1); // top of stack
    std::shared_ptr<luabridge::LuaRef> luaRefPtr = std::make_shared<luabridge::LuaRef>(luaRef);

    components["Rigidbody"] = new Component(luaRefPtr, "Rigidbody", true, true, true);

    // Debug Namesapce
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Debug")
		.addFunction("Log", &ComponentDB::CppDebugLog)
		.addFunction("LogError", &ComponentDB::CppDebugLogError)
		.endNamespace();
}

void ComponentDB::EstablishLuaInheritance(luabridge::LuaRef& instanceTable, luabridge::LuaRef& parentTable)
{
	lua_State* luaState = LuaStateManager::GetLuaState();

	// create metatable to establish inheritance
	luabridge::LuaRef newMetatable = luabridge::newTable(luaState);
	newMetatable["__index"] = parentTable;

	// must use raw lua C-API to perform a "setmetable"
	instanceTable.push(luaState);
	newMetatable.push(luaState);
	lua_setmetatable(luaState, -2);
	lua_pop(luaState, 1);
}

void ComponentDB::CppDebugLog(const std::string& message)
{
	std::cout << message << std::endl;
}

void ComponentDB::CppDebugLogError(const std::string& message)
{
	std::cerr << message << std::endl;
}

luabridge::LuaRef ComponentDB::CreateInstanceTable(const std::string& componentName, const std::string& componentType)
{
    luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
    instanceTable["key"] = componentName;

    Component* parentComponent = ComponentDB::components[componentType];
    luabridge::LuaRef parentTable = *(parentComponent->luaRef);

    if (componentType == "Rigidbody")
    {
        Rigidbody2DLuaRef* RB = new Rigidbody2DLuaRef(parentTable.cast<Rigidbody2DLuaRef>());
        luabridge::push(luaState, RB);

        instanceTable = luabridge::LuaRef::fromStack(luaState, -1); // top of stack
    }
    else // this is gross, consider having C++Component Base class instead
    {
        ComponentDB::EstablishLuaInheritance(instanceTable, parentTable);
    }

    return instanceTable;
}

luabridge::LuaRef ComponentDB::CreateInstanceTableFromTemplate(const std::string& componentName, const std::string& componentType, luabridge::LuaRef templateTable)
{
    luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
    instanceTable["key"] = componentName;

    if (componentType == "Rigidbody")
    {
        Rigidbody2DLuaRef* RB = new Rigidbody2DLuaRef(templateTable.cast<Rigidbody2DLuaRef>());
        luabridge::push(luaState, RB);

        instanceTable = luabridge::LuaRef::fromStack(luaState, -1); // top of stack
    }
    else // this is gross, consider having C++Component Base class instead
    {
        ComponentDB::EstablishLuaInheritance(instanceTable, templateTable);
    }

    return instanceTable;
}

Component* ComponentDB::LoadComponentInstance(const rapidjson::Value& component, const std::string& componentName)
{
    // Check if the component has a "type" field and extract it
    if (component.HasMember("type")) {
        std::string componentType = component["type"].GetString();

        if (ComponentDB::components.find(componentType) != ComponentDB::components.end())
        {
            luabridge::LuaRef instanceTable = CreateInstanceTable(componentName, componentType);
            Component* parentComponent = ComponentDB::components[componentType];

            // inject property overrides
            for (auto propItr = component.MemberBegin(); propItr != component.MemberEnd(); ++propItr) {
                std::string propName = propItr->name.GetString();

                if (propName != "type")
                { // Exclude "type" field itself
                    if (propItr->value.IsString())
                    {
                        instanceTable[propName] = propItr->value.GetString();
                    }
                    else if (propItr->value.IsInt())
                    {
                        instanceTable[propName] = propItr->value.GetInt();
                    }
                    else if (propItr->value.IsDouble())
                    {
                        instanceTable[propName] = propItr->value.GetDouble();
                    }
                    else if (propItr->value.IsBool())
                    {
                        instanceTable[propName] = propItr->value.GetBool();
                    }
                    else if (propItr->value.IsArray())
                    {
                        // Create a Lua table for arrays
                        luabridge::LuaRef luaArray = luabridge::newTable(luaState);
                        int index = 1; // Lua uses 1-based indexing

                        // Iterate over array elements
                        for (auto& arrayElem : propItr->value.GetArray())
                        {
                            if (arrayElem.IsString())
                            {
                                luaArray[index++] = arrayElem.GetString();
                            }
                            else if (arrayElem.IsInt())
                            {
                                luaArray[index++] = arrayElem.GetInt();
                            }
                            else if (arrayElem.IsDouble())
                            {
                                luaArray[index++] = arrayElem.GetDouble();
                            }
                            else if (arrayElem.IsBool())
                            {
                                luaArray[index++] = arrayElem.GetBool();
                            }
                        }

                        instanceTable[propName] = luaArray;
                    }
                    else
                    {
                        std::cout << "error: could not override " << propName << " because type is not supported!";
                        exit(0);
                    }
                }
            }

            std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
            return new Component(instanceTablePtr, componentType, parentComponent->hasStart, parentComponent->hasUpdate, parentComponent->hasLateUpdate);
        }
        else
        {
            std::cout << "error: failed to locate component " << componentName;
            exit(0);
        }
    }
    else {
        std::cout << "error: component " << componentName << " is missing a type" << std::endl;
        exit(0);
    }
}

ComponentDB::~ComponentDB()
{
	for (auto& component : components)
	{
		delete(component.second);
	}
}