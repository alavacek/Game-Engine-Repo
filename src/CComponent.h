#ifndef CCOMPONENT_H
#define CCOMPONENT_H

#include "Entity.h"

class Entity;

class CComponent
{
public:
	CComponent() 
	{
		owningEntity = nullptr;
		isEnabled = true;
	}

	virtual void Start() {};
	virtual void Update() {};
	virtual void LateUpdate() {};

	Entity* owningEntity;
	bool isEnabled;
};

#endif
