#pragma once
#include "../../cfg.h"

namespace util {
	class FileLoader {

	public:
		FileLoader(std::string filename) {
			this->filepath = filename;
		}

		virtual bool load() = 0;

	protected:
		std::string filepath;
	};
}