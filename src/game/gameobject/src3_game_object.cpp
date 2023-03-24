#include "src3_game_object.h"

#include <numeric>

namespace src3 {
	glm::mat4 TransformComponent::mat4() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f} };
	}
	glm::mat3 TransformComponent::normalMatrix()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.0f / scale;

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1)
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3)
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2)
			}
		};
	}
}

RTTR_REGISTRATION{ // a sacrifice must be made
    using namespace rttr;
    using namespace src3;

    //                      TransformComponent
    registration::class_<TransformComponent>("TransformComponent")
            .property("translation",&TransformComponent::translation)
            .property("scale",&TransformComponent::scale)
            .property("rotation",&TransformComponent::rotation)
            .method("mat4",&TransformComponent::mat4)
            .method("normalMatrix",&TransformComponent::normalMatrix);

    //                      PointLightComponent
    registration::class_<PointLightComponent>("PointLightComponent")
            .property("lightIntensity",&PointLightComponent::lightIntensity);

    //                      ColorComponent
    registration::class_<ColorComponent>("ColorComponent")
            .property("color",&ColorComponent::color);

    //                      ModelComponent
    registration::class_<ModelComponent>("ModelComponent")
            .property("modelFile",&ModelComponent::modelFile)
            .property("textureFile",&ModelComponent::textureFile)
            .property_readonly("model",&ModelComponent::model)
            .property_readonly("texture",&ModelComponent::texture);

    //                      PhysicsComponent
    registration::class_<PhysicsComponent>("PhysicsComponent")
            .property("physicsBodyID",&PhysicsComponent::physicsBodyID)
            .property("position",&PhysicsComponent::position)
            .property("rotation",&PhysicsComponent::rotation)
            .property("velocity",&PhysicsComponent::velocity)
            .property("motionType",&PhysicsComponent::motionType)
            .property("objectLayer",&PhysicsComponent::objectLayer);
};