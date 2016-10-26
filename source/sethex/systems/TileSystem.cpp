#include "TileSystem.h"

#include <chrono>

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
		Font font = Font(Assets::get("fonts/Nunito.ttf"), 100.0f);

		// build map coordinates

		unsigned n = 10;
		map = Map(16 * n, 9 * n);

		// create material and mesh

		String vertex_shader = loadString(loadAsset("shaders/Tile.vertex.shader"));
		String fragment_shader = loadString(loadAsset("shaders/Tile.fragment.shader"));
		//shader::define(fragment_shader, "DIFFUSE_TEXTURE");
		shared<Shader> shader = Shader::create(vertex_shader, fragment_shader);
		shader->setLabel("Tile Shader");
		//shader->uniform("uDiffuseTexture", 0);
		//shader->uniform("uTextureRotation", glm::mat2x2(0.0, 1.0, -1.0, 0.0));
		//shared<Texture> hexagon_texure = Texture::create(loadImage(loadAsset("textures/pointy.png")), Texture::Format().mipmap());
		auto material = Material::create(shader);
		material->name("Shared Tile Material");
		//material->add(hexagon_texure);

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
		instance_colors = VertexBuffer::create(GL_ARRAY_BUFFER, colors, GL_DYNAMIC_DRAW);

		mesh->appendVbo(geom::BufferLayout({ { geom::Attrib::CUSTOM_0, 3, 0, 0, 1 } }), instance_positions);
		mesh->appendVbo(geom::BufferLayout({ { geom::Attrib::CUSTOM_1, 3, 0, 0, 1 } }), instance_colors);

		Batch::AttributeMapping attributes;
		attributes.emplace(geom::Attrib::CUSTOM_0, "instancePosition");
		attributes.emplace(geom::Attrib::CUSTOM_1, "instanceColor");
		auto batch = Batch::create(mesh, material->shader, attributes);
		auto instantiable = Instantiable::create(batch);

		// create entities

		print("generate tiles");
		auto start = chrono::system_clock::now();

		auto coordinate_iterator = map.coordinates().begin();
		auto position_iterator = positions.begin();
		world->create_entities(map.coordinates().size(), "Tile #", [&coordinate_iterator, &position_iterator, &mesh, &material, &instantiable](Entity entity) {
			entity.add<Tile>().coordinates = *coordinate_iterator++;
			entity.add<Geometry>().mesh(mesh).position(*position_iterator++);
			entity.add(material);
			entity.add(instantiable);
		});

		auto end = chrono::system_clock::now();
		print("tiles generated");
		auto duration = end - start;
		print("in ", chrono::duration_cast<chrono::milliseconds>(duration).count(), " milliseconds");

		focus_expansion = (map.width / 2 + 1) * Coordinates::Tilesize.x;
		print(map.width, "x", map.height);

	}

	float3* mapped_instance_positions;

	void TileSystem::update(float delta_time) {
		static Display& display = world->find_entity("Main Display").get<Display>();
		if (display.size.x == 0 or display.size.y == 0) return;
		auto pivot_point = display.camera.getPivotPoint();
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
	}

	void TileSystem::update(Entity& entity, float delta_time) {
		auto& geometry = entity.has<Geometry>();
		if (not geometry) return;
		auto& tile = entity.get<Tile>();
		auto& position = geometry->position();
		if (not focus_range.contains(position.x)) {
			position.x += map.width * Coordinates::Spacing.x * signum(focus_position.x - position.x);
			mapped_instance_positions[map.index(tile.coordinates)] = position;
		}
	}

	float3* mapped_instance_colors;

	void TileSystem::update(shared<Surface32f> biome_map, shared<Channel32f> elevation_map) {
		float2 map_size = float2(map.width * Coordinates::Spacing.x, map.height * Coordinates::Spacing.y);
		float2 biome_map_size, elevation_map_size;
		if (elevation_map) {
			mapped_instance_positions = static_cast<float3*>(instance_positions->mapReplace());
			elevation_map_size = elevation_map->getSize();
		}
		if (biome_map) {
			mapped_instance_colors = static_cast<float3*>(instance_colors->mapReplace());
			biome_map_size = biome_map->getSize();
		}
		if (elevation_map or biome_map) for (auto& entity : get_entities()) {
			auto& coordinates = entity.get<Tile>().coordinates;
			auto& position = entity.get<Geometry>().position();
			auto texinates = coordinates.to_cartesian() / map_size + 0.5f;
			if (elevation_map) {
				auto elevation = *elevation_map->getData(elevation_map_size * texinates);
				position.y = elevation * glm::length(map_size) * 0.1f;
				mapped_instance_positions[map.index(coordinates)] = position;
			}
			if (biome_map) {
				auto biome = biome_map->getPixel(biome_map_size * texinates);
				float3 color(biome.r, biome.g, biome.b);
				mapped_instance_colors[map.index(coordinates)] = color;
			}
		}
		if (elevation_map) instance_positions->unmap();
		if (biome_map) instance_colors->unmap();
	}

}
