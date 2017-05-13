#pragma once

#include <vector>

#include <cinder/DataSource.h>
#include <cinder/GeomIo.h>
#include <cinder/ImageIo.h>

#include <sethex/Common.h>

namespace tenjix {

	namespace sethex {

		class ModelSource : public ci::geom::Source {

		public:

			String name;
			std::vector<shared<ci::ImageSource>> texture_sources;
			std::vector<shared<ci::DataSource>> shader_sources;
			std::vector<float3> vertices;
			std::vector<uint> indices;

			std::vector<float3> positions;
			std::vector<float3> normals;
			std::vector<float3> colors;
			std::vector<float2> texinates;

			uint getNumVertices() const override { return vertices.size(); };
			uint getNumIndices() const override { return indices.size(); };
			ci::geom::Primitive getPrimitive() const override { return ci::geom::Primitive::TRIANGLES; };
			uint8 getAttribDims(ci::geom::Attrib attrib) const override;
			ci::geom::AttribSet getAvailableAttribs() const override;

			void loadInto(ci::geom::Target* target, const ci::geom::AttribSet& requestedAttribs) const override;
			ModelSource* clone() const override { return new ModelSource(*this); };




		};

	}

}
