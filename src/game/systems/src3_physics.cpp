// needs to be before all of the other includes
#include <Jolt/Jolt.h>

// src3
#include "game/gameobject/src3_game_object.h"

// entt
#include "game/ecs/entt.hpp"

// std
#include <iostream>
#include <cstdarg>
#include <thread>
#include <sstream>
#include <string>

// jolt
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/ObjectStream/ObjectStreamTextIn.h>
#include <Jolt/ObjectStream/ObjectStreamTextOut.h>
#include <Jolt/ObjectStream/SerializableObject.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

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
    std::cout << "[ASSERT_F][" << inFile << "]: (" << inExpr << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

    return true;
}

#endif // JPH_ENABLE_ASSERTS

namespace src3 {
    namespace Layers
    {
        static constexpr uint8 NON_MOVING = 0;
        static constexpr uint8 MOVING = 1;
        static constexpr uint8 NUM_LAYERS = 2;
    };

    struct PhysicsComponent {
        BodyID physicsBodyID    = BodyID();

        RVec3 position{};                  // optional to set, if not set, it will use TransformComponent's position
        Quat rotation = Quat::sIdentity(); // optional to set, if not set, it will use TransformComponent's rotation
        Vec3 velocity{};                   // optional to set

        EMotionType motionType  = EMotionType::Dynamic; // optional
        ObjectLayer objectLayer = Layers::MOVING; // optional
    };

    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

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
            SrcPhysicsSystem(entt::registry &entityComponentSystem) : ecs{entityComponentSystem}
            {
                RegisterDefaultAllocator();

                Trace = traceImpl;
                JPH_IF_ENABLE_ASSERTS(AssertFailed = assertFailedImpl);

                Factory::sInstance = new Factory();

                RegisterTypes();

                tempAllocator = std::make_unique<TempAllocatorImpl>(8*1024*1024);
                jobSystem = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency() - 1));

                physicsSystem.Init(maxBodies,numBodyMutexes,maxBodyPairs,maxContactConstraints,
                                broadPhaseLayerInterface,objectVsBroadphaseLayerFilter,objectVsObjectLayerFilter);
                
                physicsSystem.SetBodyActivationListener(srcBodyActivationListener);
                physicsSystem.SetContactListener(srcContactListener);
            }

            ~SrcPhysicsSystem()
            {
                for (auto [entity,phys] : ecs.view<PhysicsComponent>().each()) {
                    bodyInterface.RemoveBody(phys.physicsBodyID);
                    bodyInterface.DestroyBody(phys.physicsBodyID);
                }

                delete Factory::sInstance;
                Factory::sInstance = nullptr;
            }

            void update()
            {
                for (auto [entity,phys,transform] : ecs.group<PhysicsComponent,TransformComponent>().each()) {
                    if (bodyInterface.IsActive(phys.physicsBodyID)) {
                        RVec3 pos = bodyInterface.GetCenterOfMassPosition(phys.physicsBodyID);
                        Quat  rot = bodyInterface.GetRotation(phys.physicsBodyID);

                        // magic start

                        ecs.patch<TransformComponent>(entity,[&](TransformComponent &trans_c){
                            trans_c.translation.x = pos.GetX(); trans_c.translation.y = pos.GetY(); trans_c.translation.z = pos.GetZ();
                            trans_c.rotation.x    = rot.GetX(); trans_c.rotation.y    = rot.GetY(); trans_c.rotation.z    = rot.GetZ();
                        });

                        // magic end
                    }
                }
                physicsSystem.Update(deltaTime,collisionSteps,integerationSubSteps,tempAllocator.get(), jobSystem.get());
            }

            BodyID registerPhysicsBody(entt::entity entity,Shape *shape,PhysicsComponent options,EActivation activationMode = EActivation::DontActivate){
                auto transform = ecs.get<TransformComponent>(entity);
                RVec3 pos = options.position == RVec3          () ? RVec3(transform.translation.x,transform.translation.y,transform.translation.z) : options.position;
                Quat  rot = options.rotation == Quat::sIdentity() ? Quat (transform.rotation.x,transform.rotation.y,transform.rotation.z,0)        : options.rotation;
                BodyCreationSettings bcs{
                    shape, 
                    pos,
                    rot,
                    options.motionType,
                    options.objectLayer
                };
                BodyID bodyid = bodyInterface.CreateAndAddBody(bcs,activationMode);
                bodyInterface.SetLinearVelocity(bodyid,options.velocity);
                ecs.emplace_or_replace<PhysicsComponent>(entity,bodyid,pos,rot,options.velocity,options.motionType,options.objectLayer);
                return bodyid;
            }

            const float deltaTime = 1.f / 200.f;
            const int collisionSteps = 2;
            const int integerationSubSteps = 2;
        private:
            const uint maxBodies = 65536;
            const uint numBodyMutexes = 0; // auto-detect
            const uint maxBodyPairs = 65536;
            const uint maxContactConstraints = 10240;

            std::unique_ptr<TempAllocatorImpl> tempAllocator;
            std::unique_ptr<JobSystemThreadPool> jobSystem;

            BPLayerInterfaceImpl broadPhaseLayerInterface{};
            ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter{};
            ObjectLayerPairFilterImpl objectVsObjectLayerFilter{};

            SrcBodyActivationListener *srcBodyActivationListener;
            SrcContactListener *srcContactListener;

            PhysicsSystem physicsSystem;
            BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

            entt::registry &ecs;
    };
}