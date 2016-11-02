#include "ModelLoader.h"

#include <cinder/Xml.h>

using namespace cinder;
using namespace std;

namespace tenjix {

	namespace sethex {

		shared<ModelSource> loadModel(const shared<DataSource>& data_source) {
			XmlTree root(data_source);

			auto geometries = root.getChild("collada/library_geometries");
			auto materials = root.getChild("collada/library_materials");

			//print("document path: ", document.getPath());

			auto model = make_shared<ModelSource>();

			return model;
		}

	}

}
