#ifndef b2WorldDB_H
#define b2WorldDB_H

#include "box2d/box2d.h"

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
};

#endif
