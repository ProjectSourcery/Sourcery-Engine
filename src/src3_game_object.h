#pragma once

#include "src3_model.h"
#include "texture/src3_texture.h"
#include "src3_swap_chain.h"

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

	struct GameObjectBufferData {
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	class SrcGameObjectManager;

	class SrcGameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, SrcGameObject>;

		SrcGameObject(const SrcGameObject&) = delete;
		SrcGameObject& operator=(const SrcGameObject&) = delete;
		SrcGameObject(SrcGameObject&&) = default;
		SrcGameObject& operator=(SrcGameObject&&) = delete;

		id_t getId() { return id; }

		VkDescriptorBufferInfo getBufferInfo(int frameIndex);

		glm::vec3 color{};
		TransformComponent transform{};

		std::shared_ptr<SrcModel> model{};
		std::shared_ptr<SrcTexture> diffuseMap = nullptr;
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		SrcGameObject(id_t objId, const SrcGameObjectManager &manager);

		id_t id;
		const SrcGameObjectManager &gameObjectManager;

		friend class SrcGameObjectManager;
	};

	class SrcGameObjectManager{
		public:
			static constexpr int MAX_GAME_OBJECTS = 1000;
			
			SrcGameObjectManager(SrcDevice& device);
			SrcGameObjectManager(const SrcGameObjectManager&) = delete;
			SrcGameObjectManager &operator=(const SrcGameObjectManager&) = delete;
			SrcGameObjectManager(const SrcGameObjectManager&&) = delete;
			SrcGameObjectManager &operator=(const SrcGameObjectManager&&) = delete;

			SrcGameObject & createGameObject() {
				assert(currentId < MAX_GAME_OBJECTS && "Max SrcGameObjects limit reached!");
				auto gameObject = SrcGameObject{currentId++,*this};
				auto gameObjectId = gameObject.getId();
				gameObject.diffuseMap = textureDefault;
				gameObjects.emplace(gameObjectId,std::move(gameObject));
				return gameObjects.at(gameObjectId);
			}

			SrcGameObject &makePointLight(
      			float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

			VkDescriptorBufferInfo getBufferInfoForGameObject(int frameIndex, SrcGameObject::id_t gameObjectId) const {
				return uboBuffers[frameIndex]->descriptorInfoForIndex(gameObjectId);
			}

			void updateBuffer(int frameIndex);

			SrcGameObject::Map gameObjects{};
			std::vector<std::unique_ptr<SrcBuffer>> uboBuffers{SrcSwapChain::MAX_FRAMES_IN_FLIGHT};

		private:
			SrcGameObject::id_t currentId = 0;
			std::shared_ptr<SrcTexture> textureDefault;
	};
}