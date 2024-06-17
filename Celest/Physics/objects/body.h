#pragma once
#include "../../cfg.h"
#include "../collider.h"
#include "../quaternion.h"

namespace PhysicsObject {

	struct BodyDescriptor {
		std::string uid;

		glm::vec3 position = glm::vec3(0);
		DataObject::Quaternion orientation = DataObject::Quaternion();

		glm::vec3 velocity = glm::vec3(0);
		glm::vec3 angularVelocity = glm::vec3(0);

		double invMass = 1.0;

		double coefRestitution = 1.0;
		double coefFriction = 1.0;

		Collision::HitboxDescriptor hitboxDescriptor;
	};

	enum BodyType {
		STATIC,
		DYNAMIC
	};

	class Body {
	public:
		std::string uid;
		BodyType type = BodyType::STATIC;

		glm::vec3 position;
		DataObject::Quaternion orientation;

		glm::mat4 translationMatrix;

		double coefRestitution;
		double coefFriction;

		Collision::Collider* hitbox;

		Body(BodyDescriptor bodyDescriptor);

		virtual bool checkColliding(Body* obj, Collision::CollisionInfo* collisionInfo) = 0;

		~Body();
	};
}