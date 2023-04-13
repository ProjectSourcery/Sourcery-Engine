#pragma once

#include "render/model/src3_model.h"
#include "render/texture/src3_texture.h"
#include "render/systems/simple_render_system.h"
#include "game/ecs/entt.hpp"

#include <rttr/type>
#include <rttr/registration>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>
#include <iostream>
#include <string>

using namespace JPH;
using namespace rttr;

namespace src3 {
    namespace Layers
    {
        static constexpr uint8 NON_MOVING = 0;
        static constexpr uint8 MOVING = 1;
        static constexpr uint8 NUM_LAYERS = 2;
    };

    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f,1.f,1.f};
		glm::vec3 rotation{};
        glm::vec3 velocity{};

		glm::mat4 mat4();
		glm::mat3 normalMatrix();


		//const std::string fields[3]{"translation", "scale", "rotation"};
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;

		//const std::string fields[1]{"lightIntensity"};
	};

	struct ColorComponent {
		glm::vec3 color{};

		//const std::string fields[1]{"color"};
	};

	struct ModelComponent {
        std::shared_ptr<SrcModel> model;
        std::shared_ptr<SrcTexture> texture;
		std::string modelFile{};
		std::string textureFile{};

		//const std::string fields[2]{"modelFile","textureFile"};
	};

	struct PhysicsComponent {
        unsigned int physicsBodyID = BodyID::cInvalidBodyID;

        EMotionType motionType  = EMotionType::Dynamic; // optional
        ObjectLayer objectLayer = Layers::MOVING; // optional

        //const std::string fields[6]{"physicsBodyID","position","rotation","velocity","motionType","objectLayer"};
    };

	inline entt::entity makePointLight(
		entt::registry& ecs,
		float intensity = 10.f,
		float radius = 0.1f,
		glm::vec3 color = glm::vec3(1.f)
		) 
	{
		entt::entity ent = ecs.create();

		ecs.emplace<TransformComponent>(ent);
		ecs.patch<TransformComponent>(ent,[&](TransformComponent transform){ transform.scale.x = radius; });
		ecs.emplace<ColorComponent>(ent,color);
		ecs.emplace<PointLightComponent>(ent,intensity);

		return ent;
	}
}