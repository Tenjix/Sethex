#include "TileSystem.h"

#include <chrono>

#include <cinder/utilities/Assets.h>
#include <cinder/utilities/Shaders.h>
#include <cinder/utilities/Watchdog.h>

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>

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

		void TileSystem::mark(const Lot<hex::Coordinates>& coordinates) {
			float3* mapped_instance_colors = static_cast<float3*>(instance_colors->mapWriteOnly());
			float3 color(0.15f);
			for (auto& c : coordinates) {
				mapped_instance_colors[map.index(c)] = color;
			}
			instance_colors->unmap();
		}

		optional<Entity> TileSystem::get_tile(float2 mouse_position) const {
			Display& display = world->find_entity("Main Display").get<Display>();
			Ray ray = display.camera.generateRay(mouse_position, display.size);
			float distance;
			bool hit_ground = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
			if (hit_ground) {
				auto ground_position = ray.calcPosition(distance);
				auto camera_position = display.camera.getEyePoint();
				auto line = Coordinates::line(Coordinates::of(camera_position), Coordinates::of(ground_position), true);
				auto extrusion = float3(0, hexagon_extrusion / 2, 0);
				for (auto& coordinates : line) {
					auto tile = tiles[map.index(coordinates)];
					auto position = tile.get<Geometry>().position() + extrusion;
					//debug("check ", tile, " ", coordinates, " ", position);
					bool hit_elevation = ray.calcPlaneIntersection(position, float3(0, 1, 0), &distance);
					if (not hit_elevation) continue;
					auto intersection = ray.calcPosition(distance);
					vec2 plain_position = intersection.xz() - position.xz();
					bool hit_hexagon = hexagon_shape.contains(plain_position);
					//debug("hex shape ", hit_hexagon ? "contains" : "doesn't contain ", plain_position);
					if (hit_hexagon) return tile;
					//debug("hit ", coordinates);
				}
			}
			return {};
		}

		void TileSystem::focus(const hex::Coordinates& coordinates) {
			auto tile = tiles[map.index(coordinates)];
			focus(tile);
		}

		void TileSystem::focus(const Entity& tile) {
			auto position = tile.get<Geometry>().position() + float3(0, hexagon_extrusion / 2, 0);
			focus(position);
		}

		void TileSystem::focus(const float3& position) {
			previous_focus_coordinates = focus_coordinates;
			previous_focus_position = target_focus_position;
			target_focus_position = position;
			focusing = true;
		}

		void TileSystem::initialize() {

			Font font = Font(Assets::get("fonts/Nunito.ttf"), 100.0f);

			Display& display = world->find_entity("Main Display").get<Display>();

			// create hexagon shape

			hexagon_shape.clear();
			for (auto& vertex : UnitHexagon.vertices) {
				if (hexagon_shape.empty()) {
					hexagon_shape.moveTo(vertex);
				} else {
					hexagon_shape.lineTo(vertex);
				}
			}

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
			material = Material::create();
			material->name("Shared Tile Material");
			//material->add(hexagon_texure);

			// make tiles instantiable

			instantiable = Instantiable::create();

			wd::watch("shaders/Material.*", [this](const fs::path& path) {
				String vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
				shader::define(vertex_shader, "INSTANTIATION");
				String fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
				try {
					material->shader = Shader::create(vertex_shader, fragment_shader);
				} catch (GlslProgExc exception) {
					error(exception.what());
					return;
				}
				material->shader->setLabel("Tile Shader");
				if (not mesh) return;
				Batch::AttributeMapping attributes;
				attributes.emplace(Attrib::CUSTOM_0, "InstancePosition");
				attributes.emplace(Attrib::CUSTOM_1, "InstanceColor");
				instantiable->batch = Batch::create(mesh, material->shader, attributes);
			});

			resize({ 16, 9 });

			display.window->getSignalMouseMove().connect([&](MouseEvent event) {
				if (event.isAltDown()) {
					auto mouse_position = event.getPos();
					auto tile = get_tile(mouse_position);
					if (tile) mark({ tile->get<Tile>().coordinates });
				}
			});

			display.window->getSignalMouseDown().connect([&](MouseEvent event) {
				mouse_down_position = event.getPos();
			});

			display.window->getSignalMouseDrag().connect([&](MouseEvent event) {
				focusing = false;
			});

			display.window->getSignalMouseUp().connect([&](MouseEvent event) {
				auto mouse_position = event.getPos();
				if (mouse_position != mouse_down_position) return;
				auto tile = get_tile(mouse_position);
				if (tile) {
					focus(*tile);
					static Entity& player = world->find_entity_tagged("Player");
					if (player.is_active) {
						player.get<Geometry>().position = target_focus_position + float3(0, 0.2, 0);
					}
				}
			});

			display.window->getSignalKeyDown().connect([&](KeyEvent event) {
				Coordinates coordinates = focus_coordinates;
				switch (event.getCode()) {
					case KeyEvent::KEY_RIGHT:
						coordinates.shift(Direction::East, map.width / 5);
						break;
					case KeyEvent::KEY_LEFT:
						coordinates.shift(Direction::West, map.width / 5);
						break;
					default:
						break;
				}
				if (coordinates != focus_coordinates) focus(coordinates);
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
					case KeyEvent::KEY_HOME:
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
					focus(coordinates);
					static Entity& player = world->find_entity_tagged("Player");
					if (player.is_active) {
						player.get<Geometry>().position = target_focus_position + float3(0, 0.2, 0);
					}
				}
			});
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
					delta *= 0.1f;
				} else {
					focusing = false;
				}
				focus_position += delta;
				eye_position += delta;
				display.camera.lookAt(eye_position, focus_position);
				mouse_down_position = display.window->getApp()->getMousePos();
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
		}

		void TileSystem::update(Entity& entity, float delta_time) {
			auto geometry = entity.get_shared<Geometry>();
			if (not geometry) return;
			auto& tile = entity.get<Tile>();
			auto& position = geometry->position();
			if (not focus_range.contains(position.x)) {
				position.x += map.width * UnitHexagon.width * sign(focus_position.x - position.x);
				mapped_instance_positions[map.index(tile.coordinates)] = position;
			}
		}

		void TileSystem::resize(unsigned2 size) {

			// build map coordinates

			map = hex::Map(size.x, size.y);
			tiles.reserve(map.coordinates().size());

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

			mesh = Mesh::create(Extrude(hexagon_shape, hexagon_extrusion) >> Rotate(quaternion(float3(-Pi_Half, 0.0f, 0.0f))));
			mesh->appendVbo(BufferLayout({ { Attrib::CUSTOM_0, 3, 0, 0, 1 } }), instance_positions);
			mesh->appendVbo(BufferLayout({ { Attrib::CUSTOM_1, 3, 0, 0, 1 } }), instance_colors);

			Batch::AttributeMapping attributes;
			attributes.emplace(Attrib::CUSTOM_0, "InstancePosition");
			attributes.emplace(Attrib::CUSTOM_1, "InstanceColor");
			instantiable->batch = Batch::create(mesh, material->shader, attributes);

			// create entities

			for (auto tile : tiles) {
				world->destroy_entity(tile);
			}
			tiles.clear();

			print("generate tiles");
			auto start = chrono::system_clock::now();

			auto coordinate_iterator = map.coordinates().begin();
			auto position_iterator = positions.begin();
			world->create_entities(map.coordinates().size(), "Tile #", [&](Entity entity) {
				entity.add<Tile>().coordinates = *coordinate_iterator++;
				entity.add<Geometry>().mesh(mesh).position(*position_iterator++);
				entity.add(material);
				entity.add(instantiable);
				tiles.push_back(entity);
			});

			auto end = chrono::system_clock::now();
			print("tiles generated");
			auto duration = end - start;
			print("in ", chrono::duration_cast<chrono::milliseconds>(duration).count(), " milliseconds");

			focus_expansion = (map.width / 2 + 1) * UnitHexagon.size.x;
			print(map.width, "x", map.height);
		}

		float3* mapped_instance_colors;

		void TileSystem::update(shared<Surface32f> biome_map, shared<Channel32f> elevation_map, float scale, float power) {
			float2 biome_map_size, elevation_map_size;
			if (elevation_map) {
				mapped_instance_positions = static_cast<float3*>(instance_positions->mapReplace());
				elevation_map_size = elevation_map->getSize();
			}
			if (biome_map) {
				mapped_instance_colors = static_cast<float3*>(instance_colors->mapReplace());
				biome_map_size = biome_map->getSize();
			}
			if (elevation_map or biome_map) {
				scale *= glm::length(map.cartesian_size()) * 0.02f;
				for (auto& entity : get_entities()) {
					auto& coordinates = entity.get<Tile>().coordinates;
					auto& position = entity.get<Geometry>().position();
					auto texinates = map.texinates(coordinates);
					if (elevation_map) {
						auto elevation = *elevation_map->getData(elevation_map_size * texinates);
						position.y = (pow(elevation + 1.0f, power) - 1.0f) * scale;
						mapped_instance_positions[map.index(coordinates)] = position;
					}
					if (biome_map) {
						auto biome = biome_map->getPixel(biome_map_size * texinates);
						float3 color(biome.r, biome.g, biome.b);
						mapped_instance_colors[map.index(coordinates)] = color;
					}
				}
			}
			if (elevation_map) instance_positions->unmap();
			if (biome_map) instance_colors->unmap();
		}

	}

}
