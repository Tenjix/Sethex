#pragma once

#include <sethex/data/Model.h>

namespace tenjix {

	namespace sethex {

		shared<ModelSource> loadModel(const shared<DataSource>& file_name);

	}

}
