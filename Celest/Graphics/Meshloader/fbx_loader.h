#pragma once
#include "mesh_loader.h"

namespace Meshloader {

	class FBX_Loader : public Mesh_Loader {
	public:
		FBX_Loader(std::string filedir, std::string filename, glm::mat4 preTransform);

		bool load();
	};
}