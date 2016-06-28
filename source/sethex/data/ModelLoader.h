#pragma once

#include <sethex/data/Model.h>

namespace sethex {

	shared<ModelSource> loadModel(const shared<ci::DataSource>& file_name);

}
