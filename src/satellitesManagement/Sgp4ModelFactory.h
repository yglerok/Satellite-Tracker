#pragma once

#include "Sgp4Model.h"

#include <memory>
#include <vector>

class Sgp4ModelFactory
{
public:
	static std::shared_ptr<Sgp4Model> create(const std::string& tleLine1, const std::string& tleLine2) {
		try {
			auto model = std::make_shared<Sgp4Model>(tleLine1, tleLine2);
			return model->isValid() ? model : nullptr;
		}
		catch (const std::exception e) {
			std::cerr << "Exception in Sgp4ModelFactory: " << e.what() << std::endl;
			return nullptr;
		}
	}
};