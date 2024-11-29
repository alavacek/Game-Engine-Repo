#ifndef RIGIDBODY2D_H
#define RIGIDBODY2D_H

#include "b2WorldDB.h"
#include "box2d/box2d.h"
#include "CComponent.h"
#include "Component.h"
#include "ErrorHandling.h"


class ContactListener;
struct Collision2D;

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

		colliderType = other.colliderType;
		colliderWidth = other.colliderWidth;
		colliderHeight = other.colliderHeight;
		colliderRadius = other.colliderRadius;
		colliderFriction = other.colliderFriction;
		colliderBounciness = other.colliderBounciness;

		triggerType = other.triggerType;
		triggerWidth = other.triggerWidth;
		triggerHeight = other.triggerHeight;
		triggerRadius = other.triggerRadius;
	}

	void AddForce(b2Vec2 force);
	void SetVelocity(b2Vec2 velocity);
	void SetPosition(b2Vec2 position);
	void SetRotation(float degreesClockwise);
	void SetAngularVelocity(float degreesClockwise);
	void SetGravityScale(float scale);
	void SetUpDirection(b2Vec2 direction);
	void SetRightDirection(b2Vec2 direction);

	b2Vec2 GetVelocity();
	float GetAngularVelocity();
	float GetGravityScale();
	b2Vec2 GetUpDirection();
	b2Vec2 GetRightDirection();

	b2Vec2 GetPosition() { return b2Vec2(x, y); }
	float GetRotation() { return rotation; }

	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void OnDestroy() override;

	void OnCollisionEnter(Collision2D collision);
	void OnCollisionExit(Collision2D collision);

	void OnTriggerEnter(Collision2D collision);
	void OnTriggerExit(Collision2D collision);

	b2Body* b2Body;

	// Basic Properties
	float x; // TODO: hide and only make available during setup
	float y;

	std::string bodyType = "dynamic";

	b2Vec2 startingVelocity = b2Vec2(0,0);
	float startingAngularVelocity = 0.0f;

	float gravityScale = 1.0f;
	float density = 1.0f;
	float angularFriction = 0.3f;
	float rotation = 0.0f;

	bool precise = true;
	bool hasCollider = true;
	bool hasTrigger = true;

	// Collider Properties
	std::string colliderType = "box";
	float colliderWidth = 1.0f;
	float colliderHeight = 1.0f;
	float colliderRadius = 0.5f;
	float colliderFriction = 0.3f;
	float colliderBounciness = 0.3f;

	//Trigger Properties
	std::string triggerType = "box";
	float triggerWidth = 1.0f;
	float triggerHeight = 1.0f;
	float triggerRadius = 0.5f;


	~Rigidbody2DLuaRef();
};

class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
};



#endif
