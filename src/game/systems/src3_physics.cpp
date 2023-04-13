// needs to be before all of the other includes
#include <Jolt/Jolt.h>

// std
#include <iostream>
#include <cstdarg>
#include <thread>
#include <string>

// jolt
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/ObjectStream/ObjectStreamTextIn.h>
#include <Jolt/ObjectStream/ObjectStreamTextOut.h>

// src3
#include "game/gameobject/src3_game_object.h"

// entt
#include "game/ecs/entt.hpp"

JPH_SUPPRESS_WARNINGS

using namespace JPH;

using namespace JPH::literals;

static void traceImpl(const char *inFMT,...) {
    std::va_list list;
    va_start(list,inFMT);
    char buffer[1024];
    std::vsnprintf(buffer,sizeof(buffer), inFMT, list);
    va_end(list);

    std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

static bool assertFailedImpl(const char *inExpr, const char *inMessage, const char *inFile, uint inLine) {
    std::cout << "[ASSERT_F][" << inFile << ":"<< inLine <<"]: (" << inExpr << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

    return true;
}

#endif // JPH_ENABLE_ASSERTS

namespace src3 {
    // Class that determines if two object layers can collide
    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
        public:
            virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
            {
                switch (inObject1)
                {
                case Layers::NON_MOVING:
                    return inObject2 == Layers::MOVING; // Non moving only collides with moving
                case Layers::MOVING:
                    return true; // Moving collides with everything
                default:
                    JPH_ASSERT(false);
                    return false;
                }
            }
    };

    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
    {
        public:
            BPLayerInterfaceImpl()
            {
                // Create a mapping table from object to broad phase layer
                mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
                mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            }

            virtual uint GetNumBroadPhaseLayers() const override
            {
                return BroadPhaseLayers::NUM_LAYERS;
            }

            virtual BroadPhaseLayer	GetBroadPhaseLayer(ObjectLayer inLayer) const override
            {
                JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
                return mObjectToBroadPhase[inLayer];
            }

        #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            virtual const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
            {
                switch ((BroadPhaseLayer::Type)inLayer)
                {
                case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
                case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
                default:													JPH_ASSERT(false); return "INVALID";
                }
            }
        #endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

        private:
            BroadPhaseLayer	mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    // Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
    {
        public:
            virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
            {
                switch (inLayer1)
                {
                case Layers::NON_MOVING:
                    return inLayer2 == BroadPhaseLayers::MOVING;
                case Layers::MOVING:
                    return true;	
                default:
                    JPH_ASSERT(false);
                    return false;
                }
            }
    };

    class SrcContactListener : public ContactListener
    {
        public:
            // See: ContactListener
            virtual ValidateResult OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
            {
                std::cout << "Contact validate callback" << std::endl;

                // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
                return ValidateResult::AcceptAllContactsForThisBodyPair;
            }

            virtual void OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
            {
                std::cout << "A contact was added" << std::endl;
            }

            virtual void OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
            {
                std::cout << "A contact was persisted" << std::endl;
            }

            virtual void OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
            { 
                std::cout << "A contact was removed" << std::endl;
            }
    };

    class SrcBodyActivationListener : public BodyActivationListener
    {
        public:
            virtual void OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
            {
                std::cout << "A body got activated" << std::endl;
            }

            virtual void OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
            {
                std::cout << "A body went to sleep" << std::endl;
            }
    };

    class SrcPhysicsSystem {
        public:
            explicit SrcPhysicsSystem(entt::registry &entityComponentSystem) : ecs{entityComponentSystem}
            {
                Trace = traceImpl;
                JPH_IF_ENABLE_ASSERTS(AssertFailed = assertFailedImpl);

                Factory::sInstance = new Factory();

                RegisterTypes();

                physicsSystem.Init(maxBodies,numBodyMutexes,maxBodyPairs,maxContactConstraints,
                                broadPhaseLayerInterface,objectVsBroadphaseLayerFilter,objectVsObjectLayerFilter);
                
                physicsSystem.SetBodyActivationListener(srcBodyActivationListener);
                physicsSystem.SetContactListener(srcContactListener);

                physicsSystem.SetGravity(/* - */(physicsSystem.GetGravity()));
            }

            ~SrcPhysicsSystem()
            {
                for (auto [entity,phys] : ecs.view<PhysicsComponent>().each()) {
                    BodyID bodyid{phys.physicsBodyID};
                    bodyInterface.RemoveBody(bodyid);
                    bodyInterface.DestroyBody(bodyid);
                }

                delete Factory::sInstance;
                Factory::sInstance = nullptr;
            }

            void update()
            {
                for (auto [entity,phys,transform] : ecs.view<PhysicsComponent,TransformComponent>().each()) {
                    if (BodyID bodyid = BodyID(phys.physicsBodyID); bodyInterface.IsActive(bodyid)) {
                        RVec3 pos = bodyInterface.GetCenterOfMassPosition(bodyid);
                        Quat  rot = bodyInterface.GetRotation(bodyid);

                        // magic start

                        ecs.patch<TransformComponent>(entity,[&](TransformComponent &trans_c){
                            trans_c.translation.x = pos.GetX(); trans_c.translation.y = (-pos.GetY()) - 1.f/*TODO: fix this -- i've had enough*/; trans_c.translation.z = pos.GetZ();
                            trans_c.rotation.x    = rot.GetX(); trans_c.rotation.y    = rot.GetY(); trans_c.rotation.z    = rot.GetZ();
                        });
                        std::cout << "Y: " << pos.GetY() << std::endl;

                        // magic end
                    }
                }
                physicsSystem.Update(deltaTime,collisionSteps,integerationSubSteps,&tempAllocator, &jobSystem);
            }

            BodyID registerPhysicsBody(entt::entity entity,Shape *shape,PhysicsComponent options, EActivation activationMode = EActivation::DontActivate) {
                auto transform = ecs.get<TransformComponent>(entity);
                RVec3 pos = RVec3(transform.translation.x,transform.translation.y,transform.translation.z);
                Quat  rot = Quat(transform.rotation.x,transform.rotation.y,transform.rotation.z,1);
                Vec3  vel = Vec3(transform.velocity.x,transform.velocity.y,transform.velocity.z);
                BodyCreationSettings bcs{
                    shape, 
                    pos,
                    rot.Normalized(),
                    options.motionType,
                    options.objectLayer
                };
                BodyID bodyid = bodyInterface.CreateAndAddBody(bcs,activationMode);
                bodyInterface.SetLinearVelocity(bodyid,vel);
                ecs.emplace_or_replace<PhysicsComponent>(entity,bodyid.GetIndexAndSequenceNumber(),options.motionType,options.objectLayer);
                return bodyid;
            }

            const float deltaTime = 1.f / 60.f;
            const int collisionSteps = 1;
            const int integerationSubSteps = 2;
        private:
            const uint maxBodies = 65536;
            const uint numBodyMutexes = 0; // auto-detect
            const uint maxBodyPairs = 65536;
            const uint maxContactConstraints = 10240;

            TempAllocatorMalloc tempAllocator{ /*8 * 1024 * 1024*/ };
            JobSystemThreadPool jobSystem{ cMaxPhysicsJobs, cMaxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency() - 1) };

            BPLayerInterfaceImpl broadPhaseLayerInterface{};
            ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter{};
            ObjectLayerPairFilterImpl objectVsObjectLayerFilter{};

            SrcBodyActivationListener *srcBodyActivationListener{};
            SrcContactListener *srcContactListener{};

            PhysicsSystem physicsSystem;
            BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

            entt::registry &ecs;
    };
}