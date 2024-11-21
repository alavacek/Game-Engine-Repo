#ifndef RIGIDBODY2D_H
#define RIGIDBODY2D_H

#include "b2WorldDB.h"
#include "box2d/box2d.h"
#include "CComponent.h"
#include "ErrorHandling.h"

//class Rigidbody2D : public Component
//{
//public:
//	Rigidbody2D(const std::shared_ptr<luabridge::LuaRef>& luaRef, const std::string type,
//		bool hasStart, bool hasUpdate, bool hasLateUpdate) : Component(luaRef, type, hasStart, hasUpdate, hasLateUpdate) {};
//
//	void Start() override;
//	void Update() override;
//	void LateUpdate() override;
//};



class Rigidbody2DLuaRef : public CComponent
{
public:
	Rigidbody2DLuaRef(float x = 0.0f, float y = 0.0f) : CComponent(), x(x), y(y) {};

	Rigidbody2DLuaRef(const Rigidbody2DLuaRef& other)
	{
		bodyType = other.bodyType;

		x = other.x;
		y = other.y;

		gravityScale = other.gravityScale;
		density = other.density;
		angularFriction = other.angularFriction;
		rotation = other.rotation;

		precise = other.precise;
		hasCollider = other.hasCollider;
		hasTrigger = other.hasTrigger;
	}

	b2Vec2 GetPosition() { return b2Vec2(x, y); }
	float GetRotation() { return rotation; }

	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	b2Body* b2Body;

	float x;
	float y;

	std::string bodyType = "dynamic";

	float gravityScale = 1.0f;
	float density = 1.0f;
	float angularFriction = 0.3f;
	float rotation = 0.0f;

	bool precise = true;
	bool hasCollider = true;
	bool hasTrigger = true;

	~Rigidbody2DLuaRef();
};

#endif
