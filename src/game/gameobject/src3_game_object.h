#pragma once

#include "render/model/src3_model.h"
#include "render/texture/src3_texture.h"
#include "render/systems/simple_render_system.h"
#include "game/ecs/entt.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>
#include <iostream>

namespace src3 {

	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f,1.f,1.f};
		glm::vec3 rotation{};

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};

	struct ColorComponent {
		glm::vec3 color{};
	};

	struct ModelComponent {
		std::shared_ptr<SrcModel> model{};
		std::shared_ptr<SrcTexture> texture;
	};

	inline entt::entity makePointLight(
		entt::registry& ecs,
		float intensity = 10.f,
		float radius = 0.1f,
		glm::vec3 color = glm::vec3(1.f)
		) 
	{
		entt::entity ent = ecs.create();

		ecs.emplace_or_replace<TransformComponent>(ent);
		ecs.patch<TransformComponent>(ent,[&](TransformComponent &transform){ transform.scale.x = radius; });
		ecs.emplace<ColorComponent>(ent,color);
		ecs.emplace<PointLightComponent>(ent,intensity);

		return ent;
	}
}