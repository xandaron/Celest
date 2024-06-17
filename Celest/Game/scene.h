#pragma once
#include "../vkCfg.h"
#include "../Physics/objects/body.h"
#include "camera.h"
#include "entity.h"
#include "player_controller.h"
#include <unordered_map>

namespace Game {

	struct AssetPack {
		std::vector<std::string> objectTypes;
		std::vector<std::string> modelFilenames;
		std::vector<std::string> textureFilenames;
		std::vector<glm::mat3> preTransforms;
		bool skyboxRepeatTexture = false;
		std::array<std::string, 6> skybox;
	};

	class Scene {

	public:

		Scene(const char* filepath);

		void load(const char* sceneFilepath);

		void update(double delta);

		Camera* getCamera();

		void setCamera(Camera* camera);

		AssetPack getAssetPack();

		std::vector<PhysicsObject::Body*> getPhysicsObjects();

		std::unordered_map<std::string, std::vector<Entitys::Entity*>> getMappedObjects();

		void cycleCamera(Controller::PlayerController* pc);

		~Scene();

	private:
		AssetPack assetPack;
		int cameraArrayPointer = 0;
		Camera* camera;
		std::vector<Camera*> cameras;
		std::vector<PhysicsObject::Body*> physicsObjects;
		std::unordered_map<std::string, std::vector<Entitys::Entity*>> mappedObjects;
	};
}