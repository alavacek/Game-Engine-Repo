#include "Rigidbody.h"

void Rigidbody2DLuaRef::Start()
{
	b2BodyDef b2BodyDef;
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

	// b2BodyDef.density = 

	b2BodyDef.angularDamping = angularFriction;

	b2Body = b2WorldDB::b2WorldInstance->CreateBody(&b2BodyDef);

	b2PolygonShape myShape;
	myShape.SetAsBox(0.5f, 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &myShape;
	fixture.density = 1.0f;
	b2Body->CreateFixture(&fixture);
}

void Rigidbody2DLuaRef::Update()
{

}

void Rigidbody2DLuaRef::LateUpdate()
{

}

Rigidbody2DLuaRef::~Rigidbody2DLuaRef()
{
	//b2Body delete or memory leak?
}