#include "TileSystem.h"

#include <chrono>

#include <cinder/utilities/Assets.h>
#include <cinder/utilities/Shaders.h>
#include <cinder/utilities/Watchdog.h>

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>
#include <sethex/components/Instantiable.h>
#include <sethex/components/Material.h>
#include <sethex/components/Tile.h>

using namespace std;
using namespace cinder;
using namespace cinder::app;
using namespace cinder::geom;
using namespace cinder::gl;
using namespace cinder::utilities;
using namespace tenjix::float_constants;
using namespace tenjix::hexagonal;

namespace tenjix {

	namespace sethex {

		signed2 mouse_down_position;

		void TileSystem::initialize() {
			Font font = Font(Assets::get("fonts/Nunito.ttf"), 100.0f);

			Display& display = world->find_entity("Main Display").get<Display>();

			// build map coordinates

			unsigned n = 10;
			map = hex::Map(16 * n, 9 * n);
			float x = 3;
			x::Map<int, int>();

			display.window->getSignalMouseMove().connect([&](MouseEvent event) {

			});

			display.window->getSignalMouseDown().connect([&](MouseEvent event) {
				mouse_down_position = event.getPos();
			});

			display.window->getSignalMouseUp().connect([&](MouseEvent event) {
				auto mouse_position = event.getPos();
				if (mouse_position != mouse_down_position) return;

				static Entity& player = world->find_entity_tagged("Player");
				if (player.is_active) {
					Ray ray = display.camera.generateRay(mouse_position, display.size);
					float distance;
					bool hit = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
					if (hit) {
						auto intersection_position = ray.calcPosition(distance);
						auto intersection_coordinates = Coordinates::of(intersection_position);
						if (this->map.contains_vertically(intersection_coordinates)) {
							auto index = this->map.index(intersection_coordinates);
							auto entity = world->find_entity(stringify("Tile #", index));
							target_focus_position = entity.get<Geometry>().position() + float3(0, 2.7, 0);
							focusing = true;
							player.get<Geometry>().position = target_focus_position;
						} else {
							player.get<Geometry>().position = intersection_coordinates.to_position() + float3(0, 2.7, 0);
						}
					}
				}

			});

			display.window->getSignalKeyUp().connect([&](KeyEvent event) {
				Coordinates coordinates = focus_coordinates;
				if (event.isAltDown()) switch (event.getCode()) {
					case KeyEvent::KEY_KP1:
						coordinates.shift(Heading::SouthWestward);
						break;
					case KeyEvent::KEY_KP2:
						coordinates.shift(Heading::Southward);
						break;
					case KeyEvent::KEY_KP3:
						coordinates.shift(Heading::SouthEastward);
						break;
					case KeyEvent::KEY_KP4:
						coordinates.shift(Heading::Westward);
						break;
					case KeyEvent::KEY_KP6:
						coordinates.shift(Heading::Eastward);
						break;
					case KeyEvent::KEY_KP7:
						coordinates.shift(Heading::NorthWestward);
						break;
					case KeyEvent::KEY_KP8:
						coordinates.shift(Heading::Northward);
						break;
					case KeyEvent::KEY_KP9:
						coordinates.shift(Heading::NorthEastward);
						break;
					default:
						break;
				} else switch (event.getCode()) {
					case KeyEvent::KEY_SPACE:
						coordinates = Coordinates::Origin;
						break;
					case KeyEvent::KEY_KP1:
						coordinates.shift(Direction::SouthWest);
						break;
					case KeyEvent::KEY_KP3:
						coordinates.shift(Direction::SouthEast);
						break;
					case KeyEvent::KEY_KP4:
						coordinates.shift(Direction::West);
						break;
					case KeyEvent::KEY_KP6:
						coordinates.shift(Direction::East);
						break;
					case KeyEvent::KEY_KP7:
						coordinates.shift(Direction::NorthWest);
						break;
					case KeyEvent::KEY_KP9:
						coordinates.shift(Direction::NorthEast);
						break;
					default:
						break;
				}
				if (coordinates != focus_coordinates) {
					auto index = this->map.index(coordinates);
					auto entity = world->find_entity(stringify("Tile #", index));
					target_focus_position = entity.get<Geometry>().position() + float3(0, 2.7, 0);
					world->find_entity_tagged("Player").get<Geometry>().position = target_focus_position;
					focusing = true;
				}
			});


			// create material and mesh

			//String vertex_shader = loadString(loadAsset("shaders/Tile.vertex.shader"));
			//String fragment_shader = loadString(loadAsset("shaders/Tile.fragment.shader"));
			//shader::define(fragment_shader, "DIFFUSE_TEXTURE");
			//shared<Shader> shader;
			//try {
			//	shader = Shader::create(vertex_shader, fragment_shader);
			//} catch (GlslProgCompileExc exception) {
			//	error(exception.what());
			//	return;
			//}
			//shader->setLabel("Tile Shader");
			//shader->uniform("uDiffuseTexture", 0);
			//shader->uniform("uTextureRotation", glm::mat2x2(0.0, 1.0, -1.0, 0.0));
			//shared<Texture> hexagon_texure = Texture::create(loadImage(loadAsset("textures/pointy.png")), Texture::Format().mipmap());
			auto material = Material::create();
			material->name("Shared Tile Material");
			//material->add(hexagon_texure);

			Shape2d shape;
			{
				// hexagon side length
				float side = 1.0;
				// height of the six equilateral triangles of the hexagon
				float tirangle = Sqrt_3 / 2 * side;
				shape.moveTo(0, +side);
				shape.lineTo(-tirangle, +side / 2);
				shape.lineTo(-tirangle, -side / 2);
				shape.lineTo(0, -side);
				shape.lineTo(+tirangle, -side / 2);
				shape.lineTo(+tirangle, +side / 2);
				shape.close();
			}
			shared<Mesh> mesh = Mesh::create(Extrude(shape, 5.0f) >> Rotate(quaternion(float3(-Pi_Half, 0.0f, 0.0f))));

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

			mesh->appendVbo(BufferLayout({ { Attrib::CUSTOM_0, 3, 0, 0, 1 } }), instance_positions);
			mesh->appendVbo(BufferLayout({ { Attrib::CUSTOM_1, 3, 0, 0, 1 } }), instance_colors);

			auto instantiable = Instantiable::create();

			wd::watch("shaders/Tile.*", [instantiable, mesh, material](const fs::path& path) {
				String vertex_shader = loadString(loadAsset("shaders/Tile.vertex.shader"));
				shader::define(vertex_shader, "INSTANTIATION");
				String fragment_shader = loadString(loadAsset("shaders/Tile.fragment.shader"));
				try {
					material->shader = Shader::create(vertex_shader, fragment_shader);
				} catch (GlslProgCompileExc exception) {
					error(exception.what());
					return;
				}
				material->shader->setLabel("Tile Shader");
				Batch::AttributeMapping attributes;
				attributes.emplace(Attrib::CUSTOM_0, "InstancePosition");
				attributes.emplace(Attrib::CUSTOM_1, "InstanceColor");
				instantiable->batch = Batch::create(mesh, material->shader, attributes);
			});

			// create entities

			print("generate tiles");
			auto start = chrono::system_clock::now();

			auto coordinate_iterator = map.coordinates().begin();
			auto position_iterator = positions.begin();
			world->create_entities(map.coordinates().size(), "Tile #", [&](Entity entity) {
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
			Display& display = world->find_entity("Main Display").get<Display>();
			if (display.minimized()) return;

			focus_position = display.camera.getPivotPoint();
			auto eye_position = display.camera.getEyePoint();
			if (focusing) {
				auto delta = target_focus_position - focus_position;
				if (length(delta) > 0.1f) {
					focus_position += delta * 0.1f;
					eye_position += delta * 0.1f;
				} else {
					focus_position = target_focus_position;
					focusing = false;
				}
				display.camera.lookAt(eye_position, focus_position);
			}
			focus_coordinates = Coordinates::of(focus_position);
			focus_range = { focus_position.x - focus_expansion, focus_position.x + focus_expansion };

			static Coordinates previous_focus_coordinates;
			if (focus_coordinates != previous_focus_coordinates) {
				mapped_instance_positions = static_cast<float3*>(instance_positions->mapWriteOnly());
				System::update(delta_time);
				instance_positions->unmap();
			}
			previous_focus_coordinates = focus_coordinates;

			//static Entity& player = world->find_entity_tagged("Player");
			//if (player.is_active) {
			//	Ray ray = display.camera.generateRay(input.mouse.position, display.size);
			//	float distance;
			//	bool hit = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
			//	float3 intersection_position;
			//	Coordinates intersection_coordinates;
			//	if (hit) {
			//		intersection_position = ray.calcPosition(distance);
			//		intersection_coordinates = Coordinates::of(intersection_position);
			//		player.get<Geometry>().position = intersection_coordinates.to_position();
			//	}
			//}
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
					position.y = elevation * glm::length(map_size) * 0.05f;
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

}
