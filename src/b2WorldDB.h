#ifndef b2WorldDB_H
#define b2WorldDB_H

#include "box2d/box2d.h"
#include "Entity.h"

class Entity;
 
class b2WorldDB
{
public:
	static inline b2World* b2WorldInstance;

	static float RadiansToDegrees(float radians)
	{
		return radians * (180.f / b2_pi);
	}

	static float DegreesToRadians(float degrees)
	{
		return degrees * (b2_pi / 180.0f);
	}

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

#endif
