#include "src3_physics.h"

namespace src3 {
    SrcPhysicsSystem::SrcPhysicsSystem(entt::registry &entityComponentSystem) : tempAllocator{8*1024*1024}, jobSystem{cMaxPhysicsJobs,cMaxPhysicsBarriers,thread::hardware_concurrency() - 1}, ecs{entityComponentSystem}
    {
        RegisterDefaultAllocator();

        Trace = traceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = assertFailedImpl);

        Factory::sInstance = factory;

        RegisterTypes();

        physicsSystem.Init(maxBodies,numBodyMutexes,maxBodyPairs,maxContactConstraints,
                           broadPhaseLayerInterface,objectVsBroadpahseLayerFilter,objectVsObjectLayerFilter);
        
        physicsSystem.SetBodyActivationListener(srcBodyActivationListener);
        physicsSystem.SetContactListener(srcContactListener);

        
    }

    SrcPhysicsSystem::~SrcPhysicsSystem()
    {

    }

    void SrcPhysicsSystem::updateObjects()
    {
        
    }

    void SrcPhysicsSystem::update()
    {

    }
}