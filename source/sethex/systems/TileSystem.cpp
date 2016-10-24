#include "TileSystem.h"

#include <cinder/utilities/Assets.h>
#include <cinder/utilities/Shaders.h>

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>
#include <sethex/components/Instantiable.h>
#include <sethex/components/Material.h>
#include <sethex/components/Tile.h>

using namespace std;
using namespace cinder;
using namespace cinder::app;
using namespace cinder::gl;
using namespace sethex::hexagonal;
using namespace constf;

namespace sethex {

	void TileSystem::initialize() {
		using namespace glm;
		Font font = Font(Assets::get("fonts/Nunito.ttf"), 100.0f);

		// build map coordinates

		map = Map(16, 9);

		// create material and mesh

		String vertex_shader = loadString(loadAsset("shaders/Tile.vertex.shader"));
		String fragment_shader = loadString(loadAsset("shaders/Tile.fragment.shader"));
		shader::define(fragment_shader, "DIFFUSE_TEXTURE");
		shared<Shader> shader = Shader::create(vertex_shader, fragment_shader);
		shader->setLabel("Tile Shader");
		shader->uniform("uDiffuseTexture", 0);
		shader->uniform("uTextureRotation", mat2x2(0.0, 1.0, -1.0, 0.0));
		shared<Texture> hexagon_texure = Texture::create(loadImage(loadAsset("textures/pointy.png")), Texture::Format().mipmap());
		auto material = Material::create(shader);
		material->name("Shared Tile Material");
		material->add(hexagon_texure);

		shared<Mesh> mesh = Mesh::create(geom::Circle() >> geom::Rotate(quaternion(float3(-Pi_Half, Pi_Half, 0.0f))));

		// make tiles instantiable

		vector<float3> positions;
		vector<float3> colors;
		positions.reserve(map.coordinates().size());
		colors.reserve(map.coordinates().size());
		for (auto& coordinates : map.coordinates()) {
			positions.push_back(coordinates.to_position());
			colors.push_back(glm::mix(float3(1.0), coordinates.to_floats(), 0.25));
		}
		instance_positions = VertexBuffer::create(GL_ARRAY_BUFFER, positions, GL_DYNAMIC_DRAW);
		auto instance_colors = VertexBuffer::create(GL_ARRAY_BUFFER, colors, GL_STATIC_DRAW);

		mesh->appendVbo(geom::BufferLayout({ { geom::Attrib::CUSTOM_0, 3, 0, 0, 1 } }), instance_positions);
		mesh->appendVbo(geom::BufferLayout({ { geom::Attrib::CUSTOM_1, 3, 0, 0, 1 } }), instance_colors);

		Batch::AttributeMapping attributes;
		attributes.emplace(geom::Attrib::CUSTOM_0, "instancePosition");
		attributes.emplace(geom::Attrib::CUSTOM_1, "instanceColor");
		auto batch = Batch::create(mesh, material->shader, attributes);
		auto instantiable = Instantiable::create(batch);

		// create entities

		auto iterator = map.coordinates().begin();
		print("generate tiles");
		world->create_entities(map.coordinates().size(), "Tile #", [&iterator, &mesh, &shader, &font, &hexagon_texure, &material, &instantiable](Entity entity) {
			const Coordinates& coordinates = *iterator++;
			entity.add<Geometry>().mesh(mesh).position(coordinates.to_position());
			entity.add<Tile>().coordinates = coordinates;
			entity.add(material);
			entity.add(instantiable);
		});
		print("tiles generated");

		focus_expansion = (map.width / 2 + 1) * Coordinates::Tilesize.x;
		print(map.width, "x", map.height);


	}

	float3* mapped_instance_positions;

	void TileSystem::update(float delta_time) {
		static Display& display = world->find_entity("Main Display").get<Display>();
		if (display.size.x == 0 or display.size.y == 0) return;
		focus_position = display.camera.getPivotPoint();
		focus_position.z = 0;
		focus_coordinates = Coordinates::of(focus_position);
		focus_range = { focus_position.x - focus_expansion, focus_position.x + focus_expansion };

		mapped_instance_positions = static_cast<float3*>(instance_positions->mapWriteOnly());
		System::update(delta_time);
		instance_positions->unmap();

		static Entity& player = world->find_entity_tagged("Player");
		if (player.is_active) {
			Ray ray = display.camera.generateRay(input.mouse.position, display.size);
			float distance;
			bool hit = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
			float3 intersection_position;
			Coordinates intersection_coordinates;
			if (hit) {
				intersection_position = ray.calcPosition(distance);
				intersection_coordinates = Coordinates::of(intersection_position);
				player.get<Geometry>().position = intersection_coordinates.to_position();
			}
		}
	};

	void TileSystem::update(Entity& entity, float delta_time) {
		auto& geometry = entity.has<Geometry>();
		if (not geometry) return;
		auto& tile = entity.get<Tile>();
		auto& position = geometry->position();
		if (not focus_range.contains(position.x)) {
			position.x += map.width * Coordinates::Spacing.x * signum(focus_position.x - position.x);
			mapped_instance_positions[map.index(tile.coordinates)] = position;
		}
		//auto area = Coordinates::rectangle(map.width, map.height, focus_coordinates);
		//for (auto coordinates : area) {
		//	auto reprojected_coordinates = map.reproject(coordinates);
		//	auto index = map.index(reprojected_coordinates);
		//	map.coordinates()[index];
		//}
		//map.index(focus)
	};

}
