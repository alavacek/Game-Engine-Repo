#ifndef b2WorldDB_H
#define b2WorldDB_H

#include "box2d/box2d.h"
#include "Entity.h"

#define DRAW_COLLISIONS

class Entity;
class HitResult;
 
class b2WorldDB
{
public:
	static float RadiansToDegrees(float radians)
	{
		return radians * (180.f / b2_pi);
	}

	static float DegreesToRadians(float degrees)
	{
		return degrees * (b2_pi / 180.0f);
	}

	static HitResult* Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
	static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);

	static b2World* b2WorldInstance;

	~b2WorldDB()
	{
		delete b2WorldInstance;
	}
};

struct Collision2D
{
	Collision2D(Entity* other, b2Vec2 point, b2Vec2 relativeVelocity, b2Vec2 normal) :
		other(other), point(point), relativeVelocity(relativeVelocity), normal(normal) {};

	Entity* other;
	b2Vec2 point;
	b2Vec2 relativeVelocity;
	b2Vec2 normal;
};

struct HitResult
{
	HitResult(Entity* entity, b2Vec2 point, b2Vec2 normal, bool isTrigger) :
		entity(entity), point(point), normal(normal), isTrigger(isTrigger) {};

	Entity* entity;
	b2Vec2 point;
	b2Vec2 normal;
	bool isTrigger;
};

class RayCastCallback : public b2RayCastCallback
{
public:
	struct HitData {
		b2Fixture* hitFixture;
		b2Vec2 hitPoint;
		b2Vec2 hitNormal;
		float fraction;
		bool isTrigger;

		HitData(b2Fixture* f, const b2Vec2& p, const b2Vec2& n, float frac, bool trigger)
			: hitFixture(f), hitPoint(p), hitNormal(n), fraction(frac), isTrigger(trigger) {}
	};

	RayCastCallback()
		: closestHit(nullptr, b2Vec2(0,0), b2Vec2(0,0), 1, false) {}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;

	HitData closestHit;

	std::vector<HitData> hits;


};

#endif
