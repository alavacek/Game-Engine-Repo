#include "Rigidbody.h"

void Rigidbody2DLuaRef::Start()
{
	// first rigid body added to scene
	if (b2WorldDB::b2WorldInstance == nullptr)
	{
		b2WorldDB::b2WorldInstance = new b2World(b2Vec2(0.0f, 9.8f));

		b2WorldDB::b2WorldInstance->SetContactListener(new ContactListener());
	}

	b2BodyDef b2BodyDef;

	b2BodyDef.position.Set(x, y);
	b2BodyDef.angle = rotation * (b2_pi / 180.0f);

	if (bodyType == "dynamic")
	{
		b2BodyDef.type = b2_dynamicBody;
	}
	else if (bodyType == "static")
	{
		b2BodyDef.type = b2_staticBody;
	}
	else if (bodyType == "kinematic")
	{
		b2BodyDef.type = b2_kinematicBody;
	}
	else
	{
		b2BodyDef.type = b2_dynamicBody;
		ErrorHandling::ReportString(owningEntity->entityName + ": Unknown type set for Rigidbody, defaulting to dynamic type");
	}

	b2BodyDef.bullet = precise;

	b2BodyDef.gravityScale = gravityScale;

	b2BodyDef.angularDamping = angularFriction;

	b2Body = b2WorldDB::b2WorldInstance->CreateBody(&b2BodyDef);

	if (!hasCollider && !hasTrigger)
	{
		b2PolygonShape phantomShape;
		phantomShape.SetAsBox(colliderWidth * 0.5f, colliderHeight * 0.5f);

		b2FixtureDef phantomFixtureDef;
		phantomFixtureDef.shape = &phantomShape;
		phantomFixtureDef.density = density;

		// bc is sensor (w no callback), no collisions will ever occur
		phantomFixtureDef.isSensor = true;
		b2Body->CreateFixture(&phantomFixtureDef);
	}
	else
	{
		if (hasCollider)
		{
			if (colliderType == "box")
			{
				b2PolygonShape myShape;
				myShape.SetAsBox(colliderWidth * 0.5f, colliderHeight * 0.5f);

				b2FixtureDef fixture;
				fixture.shape = &myShape;
				fixture.density = density;
				fixture.friction = colliderFriction;
				fixture.restitution = colliderBounciness;
				fixture.isSensor = false;

				// Reference to owning Entity, used in collision detection me thinks
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);

				b2Body->CreateFixture(&fixture);
			}
			else if (colliderType == "circle")
			{
				b2CircleShape myShape;
				myShape.m_radius = colliderRadius;

				b2FixtureDef fixture;
				fixture.shape = &myShape;
				fixture.density = density;
				fixture.friction = colliderFriction;
				fixture.restitution = colliderBounciness;
				fixture.isSensor = false;

				// Reference to owning Entity, used in collision detection me thinks
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);

				b2Body->CreateFixture(&fixture);
			}
			else
			{
				ErrorHandling::ReportString(owningEntity->entityName + ": Unknown type set for collider, defaulting to box type");
				
				b2PolygonShape myShape;
				myShape.SetAsBox(colliderWidth * 0.5f, colliderHeight * 0.5f);

				b2FixtureDef fixture;
				fixture.shape = &myShape;
				fixture.density = density;
				fixture.friction = colliderFriction;
				fixture.restitution = colliderBounciness;
				fixture.isSensor = false;

				// Reference to owning Entity, used in collision detection me thinks
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);

				b2Body->CreateFixture(&fixture);
			}
		}

		if (hasTrigger)
		{
			if (triggerType == "box")
			{
				b2PolygonShape myShape;
				myShape.SetAsBox(triggerWidth * 0.5f, triggerHeight * 0.5f);

				b2FixtureDef fixture;
				fixture.shape = &myShape;
				fixture.density = density;
				fixture.isSensor = true;

				// Reference to owning Entity, used in collision detection me thinks
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);

				b2Body->CreateFixture(&fixture);
			}
			else if (triggerType == "circle")
			{
				b2CircleShape myShape;
				myShape.m_radius = triggerRadius;

				b2FixtureDef fixture;
				fixture.shape = &myShape;
				fixture.density = density;
				fixture.isSensor = true;

				// Reference to owning Entity, used in collision detection me thinks
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);

				b2Body->CreateFixture(&fixture);
			}
			else
			{
				ErrorHandling::ReportString(owningEntity->entityName + ": Unknown type set for collider, defaulting to box type");

				b2PolygonShape myShape;
				myShape.SetAsBox(triggerWidth * 0.5f, triggerHeight * 0.5f);

				b2FixtureDef fixture;
				fixture.shape = &myShape;
				fixture.density = density;
				fixture.isSensor = true;

				// Reference to owning Entity, used in collision detection me thinks
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);

				b2Body->CreateFixture(&fixture);
			}
		}

	}
	
}

void Rigidbody2DLuaRef::Update()
{
	x = b2Body->GetPosition().x;
	y = b2Body->GetPosition().y;

	rotation = b2Body->GetAngle() * (180.0f / b2_pi);
}

void Rigidbody2DLuaRef::LateUpdate()
{
#ifdef DRAW_COLLISIONS
	SDL_Renderer* renderer = Renderer::GetRenderer();

	if (hasCollider && colliderType == "box")
	{
		float colliderHalfWidth = (colliderWidth / 2);
		float colliderHalfHeight = (colliderHeight / 2);

		glm::vec2 upperLeftCorner = glm::vec2(
			(x - colliderHalfWidth),
			(y - colliderHalfHeight)
		);

		glm::vec2 lowerRightCorner = glm::vec2(
			(x + colliderHalfWidth),
			(y + colliderHalfHeight)
		);


		ImageDB::DrawLine(upperLeftCorner.x, upperLeftCorner.y, lowerRightCorner.x, upperLeftCorner.y, 255, 0, 0, 255);
		ImageDB::DrawLine(lowerRightCorner.x, upperLeftCorner.y, lowerRightCorner.x, lowerRightCorner.y, 255, 0, 0, 255);
		ImageDB::DrawLine(lowerRightCorner.x, lowerRightCorner.y, upperLeftCorner.x, lowerRightCorner.y, 255, 0, 0, 255);
		ImageDB::DrawLine(upperLeftCorner.x, lowerRightCorner.y, upperLeftCorner.x, upperLeftCorner.y, 255, 0, 0, 255);
	}

	if (hasTrigger && triggerType == "box")
	{
		float triggerHalfWidth = (triggerWidth / 2);
		float triggerHalfHeight = (triggerHeight / 2);

		glm::vec2 upperLeftCorner = glm::vec2(
			(x - triggerHalfWidth),
			(y - triggerHalfHeight)
		);

		glm::vec2 lowerRightCorner = glm::vec2(
			(x + triggerHalfWidth),
			(y + triggerHalfHeight)
		);


		ImageDB::DrawLine(upperLeftCorner.x, upperLeftCorner.y, lowerRightCorner.x, upperLeftCorner.y, 0, 255, 0, 255);
		ImageDB::DrawLine(lowerRightCorner.x, upperLeftCorner.y, lowerRightCorner.x, lowerRightCorner.y, 0, 255, 0, 255);
		ImageDB::DrawLine(lowerRightCorner.x, lowerRightCorner.y, upperLeftCorner.x, lowerRightCorner.y, 0, 255, 0, 255);
		ImageDB::DrawLine(upperLeftCorner.x, lowerRightCorner.y, upperLeftCorner.x, upperLeftCorner.y, 0, 255, 0, 255);
	}

#endif // DRAW_COLLISIONS
}

void Rigidbody2DLuaRef::OnCollisionEnter(Collision2D collision)
{
	for (const auto& componentKey : owningEntity->componentsKeysAlphabeticalOrder)
	{
		if (owningEntity->wasDestroyed)
		{
			return;
		}
		
		Component* component = owningEntity->components[componentKey];

		std::shared_ptr<luabridge::LuaRef> luaRefPtr = component->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
		{
			try
			{
				// Run if enabled
				luabridge::LuaRef isEnabled = (luaRef)["enabled"];
				if (isEnabled.isBool() && isEnabled)
				{
					luabridge::LuaRef onCollisionEnterFunc = (luaRef)["OnCollisionEnter"];
					if (onCollisionEnterFunc.isFunction())
					{
						onCollisionEnterFunc(luaRef, collision);
					}
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ErrorHandling::ReportError(component->owningEntityName, e);
			}
		}

		owningEntity->components[componentKey]->wasInstantiated = true;
	}
}

void Rigidbody2DLuaRef::OnCollisionExit(Collision2D collision)
{
	for (const auto& componentKey : owningEntity->componentsKeysAlphabeticalOrder)
	{
		if (owningEntity->wasDestroyed)
		{
			return;
		}

		Component* component = owningEntity->components[componentKey];

		std::shared_ptr<luabridge::LuaRef> luaRefPtr = component->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
		{
			try
			{
				// Run if enabled
				luabridge::LuaRef isEnabled = (luaRef)["enabled"];
				if (isEnabled.isBool() && isEnabled)
				{
					luabridge::LuaRef onCollisionExitFunc = (luaRef)["OnCollisionExit"];
					if (onCollisionExitFunc.isFunction())
					{
						onCollisionExitFunc(luaRef, collision);
					}
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ErrorHandling::ReportError(component->owningEntityName, e);
			}
		}

		owningEntity->components[componentKey]->wasInstantiated = true;
	}
}

void Rigidbody2DLuaRef::OnTriggerEnter(Collision2D collision)
{
	for (const auto& componentKey : owningEntity->componentsKeysAlphabeticalOrder)
	{
		if (owningEntity->wasDestroyed)
		{
			return;
		}

		Component* component = owningEntity->components[componentKey];

		std::shared_ptr<luabridge::LuaRef> luaRefPtr = component->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
		{
			try
			{
				// Run if enabled
				luabridge::LuaRef isEnabled = (luaRef)["enabled"];
				if (isEnabled.isBool() && isEnabled)
				{
					luabridge::LuaRef onTriggerEnterFunc = (luaRef)["OnTriggerEnter"];
					if (onTriggerEnterFunc.isFunction())
					{
						onTriggerEnterFunc(luaRef, collision);
					}
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ErrorHandling::ReportError(component->owningEntityName, e);
			}
		}

		owningEntity->components[componentKey]->wasInstantiated = true;
	}
}

void Rigidbody2DLuaRef::OnTriggerExit(Collision2D collision)
{
	for (const auto& componentKey : owningEntity->componentsKeysAlphabeticalOrder)
	{
		if (owningEntity->wasDestroyed)
		{
			return;
		}

		Component* component = owningEntity->components[componentKey];

		std::shared_ptr<luabridge::LuaRef> luaRefPtr = component->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && (luaRefPtr->isTable() || luaRefPtr->isUserdata()))
		{
			try
			{
				// Run if enabled
				luabridge::LuaRef isEnabled = (luaRef)["enabled"];
				if (isEnabled.isBool() && isEnabled)
				{
					luabridge::LuaRef onTriggerExitFunc = (luaRef)["OnTriggerExit"];
					if (onTriggerExitFunc.isFunction())
					{
						onTriggerExitFunc(luaRef, collision);
					}
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ErrorHandling::ReportError(component->owningEntityName, e);
			}
		}

		owningEntity->components[componentKey]->wasInstantiated = true;
	}
}

void Rigidbody2DLuaRef::AddForce(b2Vec2 force)
{
	b2Body->ApplyForceToCenter(force, true);
}

void Rigidbody2DLuaRef::SetVelocity(b2Vec2 velocity)
{
	b2Body->SetLinearVelocity(velocity);
}

void Rigidbody2DLuaRef::SetPosition(b2Vec2 position)
{
	b2Body->SetTransform(position, b2Body->GetAngle());
}

void Rigidbody2DLuaRef::SetRotation(float degreesClockwise)
{
	b2Body->SetTransform(b2Body->GetPosition(), degreesClockwise * (b2_pi / 180.0f));
}

void Rigidbody2DLuaRef::SetAngularVelocity(float degreesClockwise)
{
	b2Body->SetAngularVelocity(degreesClockwise * (b2_pi / 180.0f));
}

void Rigidbody2DLuaRef::SetGravityScale(float scale)
{
	b2Body->SetGravityScale(scale);
}

void Rigidbody2DLuaRef::SetUpDirection(b2Vec2 direction)
{
	direction.Normalize();

	// bc we have -1 as y's "up", we gotta make y negative here
	b2Body->SetTransform(b2Body->GetPosition(), glm::atan(direction.x, -1 * direction.y));
}

void Rigidbody2DLuaRef::SetRightDirection(b2Vec2 direction)
{
	direction.Normalize();

	b2Body->SetTransform(b2Body->GetPosition(), glm::atan(direction.x, -1 * direction.y) - (b2_pi / 2.0f));
}

b2Vec2 Rigidbody2DLuaRef::GetVelocity()
{
	return b2Body->GetLinearVelocity();
}

float Rigidbody2DLuaRef::GetAngularVelocity()
{
	return b2Body->GetAngularVelocity();
}

float Rigidbody2DLuaRef::GetGravityScale()
{
	return b2Body->GetGravityScale();
}

b2Vec2 Rigidbody2DLuaRef::GetUpDirection()
{
	float angle = b2Body->GetAngle();
	b2Vec2 result = b2Vec2(glm::sin(angle), -1 *  glm::cos(angle));
	result.Normalize();
	return result;
}

b2Vec2 Rigidbody2DLuaRef::GetRightDirection()
{
	float angle = b2Body->GetAngle();
	b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
	result.Normalize();
	return result;
	
}

Rigidbody2DLuaRef::~Rigidbody2DLuaRef()
{
	//b2Body delete or memory leak?
}

// Contact Listener
void ContactListener::BeginContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	Rigidbody2DLuaRef* rbA = reinterpret_cast<Rigidbody2DLuaRef*>(fixtureA->GetUserData().pointer);

	// phantom collider
	if (!rbA)
	{
		return;
	}

	Entity* entityA = rbA->owningEntity;

	b2Fixture* fixtureB = contact->GetFixtureB();
	Rigidbody2DLuaRef* rbB = reinterpret_cast<Rigidbody2DLuaRef*>(fixtureB->GetUserData().pointer);
	Entity* entityB = rbB->owningEntity;

	// phantom collider
	if (!rbB)
	{
		return;
	}

	b2Vec2 relativeVelocityA = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
	b2Vec2 relativeVelocityB =  fixtureB->GetBody()->GetLinearVelocity() - fixtureA->GetBody()->GetLinearVelocity();

	// Collider Collision
	if (!fixtureA->IsSensor() && !fixtureB->IsSensor())
	{
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		Collision2D collisionA(entityB, worldManifold.points[0], relativeVelocityA, worldManifold.normal);
		rbA->OnCollisionEnter(collisionA);

		Collision2D collisionB(entityA, worldManifold.points[1], relativeVelocityB, worldManifold.normal);
		rbB->OnCollisionEnter(collisionB);
	}
	// Trigger Collision
	else if (fixtureA->IsSensor() && fixtureB->IsSensor())
	{
		b2Vec2 invalid = b2Vec2(-999.0f, -999.0f);

		Collision2D collisionA(entityB, invalid, relativeVelocityA, invalid);
		rbA->OnTriggerEnter(collisionA);

		Collision2D collisionB(entityA, invalid, relativeVelocityB, invalid);
		rbB->OnTriggerEnter(collisionB);
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	Rigidbody2DLuaRef* rbA = reinterpret_cast<Rigidbody2DLuaRef*>(fixtureA->GetUserData().pointer);
	Entity* entityA = rbA->owningEntity;

	// phantom collider
	if (!rbA)
	{
		return;
	}

	b2Fixture* fixtureB = contact->GetFixtureB();
	Rigidbody2DLuaRef* rbB = reinterpret_cast<Rigidbody2DLuaRef*>(fixtureB->GetUserData().pointer);
	Entity* entityB = rbB->owningEntity;

	// phantom collider
	if (!rbB)
	{
		return;
	}

	b2Vec2 invalid = b2Vec2(-999.0f, -999.0f);

	b2Vec2 relativeVelocityA = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
	b2Vec2 relativeVelocityB = fixtureB->GetBody()->GetLinearVelocity() - fixtureA->GetBody()->GetLinearVelocity();

	Collision2D collisionA(entityB, invalid, relativeVelocityA, invalid);
	Collision2D collisionB(entityA, invalid, relativeVelocityB, invalid);
	

	// Collider Collision
	if (!fixtureA->IsSensor() && !fixtureB->IsSensor())
	{
		rbA->OnCollisionExit(collisionA);
		rbB->OnCollisionExit(collisionB);
	}
	// Trigger Collision
	else if (fixtureA->IsSensor() && fixtureB->IsSensor())
	{
		rbA->OnTriggerExit(collisionA);
		rbB->OnTriggerExit(collisionB);
	}
}