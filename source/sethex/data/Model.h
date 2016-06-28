#pragma once

#include <cinder/DataSource.h>

#include <sethex/components/Geometry.h>
#include <sethex/components/Material.h>

#include <sethex/data/ModelSource.h>

namespace sethex {

	class Model {

	public:

		String name;
		shared<Geometry> geometry;
		shared<Material> material;

		Model(const String& name = "") : name(name) {}

		static shared<Model> create(const String& name = "") { return std::make_shared<Model>(name); }

		static shared<Model> create(const shared<ModelSource>& model_source) {
			auto model = std::make_shared<Model>(model_source->name);
			model->geometry = Geometry::create(Mesh::create(*model_source));
			model->material = Material::create();
			return model;
		}

	};

}
