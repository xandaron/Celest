#pragma once
#include "../../cfg.h"
#include "../collider.h"
#include "../quaternion.h"
#include "body.h"

namespace PhysicsObject {

	class DynamicBody : public Body {
	public:
		glm::vec3 velocity;
		glm::vec3 force;

		glm::vec3 angularVelocity;
		glm::vec3 torque;

		glm::mat3 orientationMatrix;

		double invMass;
		glm::mat3 invInertia;
		glm::mat3 invInertiaOrientated;

		DynamicBody(BodyDescriptor bodyDescriptor);

		bool checkColliding(Body* obj, Collision::CollisionInfo* collisionInfo);

		void updateInvInertiaOrientated();

		void applyForce(glm::vec3 force);

		void applyForceAtPoint(glm::vec3 force, glm::vec3 point);

		void applyCollisionImpulse(glm::vec3 force, glm::vec3 distance);

		void move(glm::vec3 movement);

		void rotate(glm::vec3 rotation);

		glm::vec3 momentum();

		~DynamicBody();
	};
}