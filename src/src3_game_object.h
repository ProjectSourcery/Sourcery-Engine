#pragma once

#include "src3_model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace src3 {

	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f,1.f,1.f};
		glm::vec3 rotation;

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};

	class SrcGameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, SrcGameObject>;

		static SrcGameObject createGameObject() {
			static id_t currentId = 0;
			return SrcGameObject(currentId++);
		}

		static SrcGameObject makePointLight(
      		float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		SrcGameObject(const SrcGameObject&) = delete;
		SrcGameObject& operator=(const SrcGameObject&) = delete;
		SrcGameObject(SrcGameObject&&) = default;
		SrcGameObject& operator=(SrcGameObject&&) = default;

		id_t getId() { return id; }

		glm::vec3 color{};
		TransformComponent transform{};

		std::shared_ptr<SrcModel> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		SrcGameObject(id_t objId) : id{objId} {}

		id_t id;
	};
}