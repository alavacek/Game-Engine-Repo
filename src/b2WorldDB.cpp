#include "b2WorldDB.h"

b2World* b2WorldDB::b2WorldInstance;

HitResult* b2WorldDB::Raycast(b2Vec2 pos, b2Vec2 dir, float dist)
{
    if (dist <= 0.0f || !b2WorldInstance) {
        return luabridge::LuaRef(LuaStateManager::GetLuaState());  // No physics world or invalid distance
    }

	b2Vec2 endPoint = pos + b2Vec2(dir.x * dist, dir.y * dist);

#ifdef DRAW_COLLISIONS
    ImageDB::DrawLine(pos.x, pos.y, endPoint.x, endPoint.y, 255, 0, 255, 255);
#endif 

	RayCastCallback callback;
	b2WorldInstance->RayCast(&callback, pos, endPoint);

    if (callback.closestHit.hitFixture) {
        // Retrieve the actor (user data)
        Rigidbody2DLuaRef* rb = reinterpret_cast<Rigidbody2DLuaRef*>(callback.closestHit.hitFixture->GetUserData().pointer);
        if (rb && rb->owningEntity) 
        {
            HitResult* hit = new HitResult(rb->owningEntity, callback.closestHit.hitPoint, callback.closestHit.hitNormal, callback.closestHit.isTrigger);
            return hit;  // Return the actor hit
        }
    }

	return luabridge::LuaRef(LuaStateManager::GetLuaState());
}

luabridge::LuaRef b2WorldDB::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist)
{
    if (dist <= 0.0f || !b2WorldInstance) {
        return luabridge::LuaRef(LuaStateManager::GetLuaState());  // No physics world or invalid distance
    }

    b2Vec2 endPoint = pos + b2Vec2(dir.x * dist, dir.y * dist);

    luabridge::LuaRef entityTable = luabridge::newTable(LuaStateManager::GetLuaState());
    int index = 1;

#ifdef DRAW_COLLISIONS
    ImageDB::DrawLine(pos.x, pos.y, endPoint.x, endPoint.y, 255, 0, 255, 255);
#endif 

    RayCastCallback callback;
    b2WorldInstance->RayCast(&callback, pos, endPoint);
    
    std::sort(callback.hits.begin(), callback.hits.end(),
        [](const RayCastCallback::HitData& a, const RayCastCallback::HitData& b) {
            return a.fraction < b.fraction;
        });

    for (const auto& hit : callback.hits) {
        Rigidbody2DLuaRef* rb = reinterpret_cast<Rigidbody2DLuaRef*>(hit.hitFixture->GetUserData().pointer);
        if (rb && rb->owningEntity) {
            HitResult* hitResult = new HitResult(rb->owningEntity, hit.hitPoint, hit.hitNormal, hit.isTrigger);
            entityTable[index] = hitResult;
            index++;
        }
    }

    return entityTable;


	//HitResult hit;

	return luabridge::LuaRef(LuaStateManager::GetLuaState());
}

float RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
{
    // no fixture or phantom fixture
    if (fixture == nullptr || !(reinterpret_cast<Rigidbody2DLuaRef*>(fixture->GetUserData().pointer)))
    {
        return -1;
    }

    // Update to the closest fixture
    if (fraction < closestHit.fraction)
    {
        closestHit = HitData(fixture, point, normal, fraction, fixture->IsSensor());
    }

    hits.emplace_back(fixture, point, normal, fraction, fixture->IsSensor());

    return 1;
}