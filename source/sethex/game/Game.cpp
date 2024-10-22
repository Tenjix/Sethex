#include "Game.h"

#include <cinder/ObjLoader.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>
#include <cinder/app/App.h>
#include <cinder/utilities/Watchdog.h>
#include <cinder/interface/Imgui.h>
#include <cinder/utilities/Shaders.h>

#include <sethex/data/ModelLoader.h>
#include <sethex/systems/RenderSystem.h>
#include <sethex/systems/TileSystem.h>

using namespace cinder;
using namespace cinder::geom;
using namespace cinder::app;
using namespace cinder::gl;
using namespace std;

namespace tenjix {

	namespace sethex {

		void Game::setup(const shared<Window>& window) {
			Display& display = world.create_entity("Main Display").add<Display>();
			display.window = window;

			window->getSignalMouseMove().connect(bind(&Game::mouseMove, this, placeholders::_1));
			window->getSignalMouseDrag().connect(bind(&Game::mouseDrag, this, placeholders::_1));
			window->getSignalMouseDown().connect(bind(&Game::mouseDown, this, placeholders::_1));
			window->getSignalMouseUp().connect(bind(&Game::mouseUp, this, placeholders::_1));
			window->getSignalMouseWheel().connect(bind(&Game::mouseWheel, this, placeholders::_1));
			window->getSignalKeyDown().connect(bind(&Game::keyDown, this, placeholders::_1));
			window->getSignalKeyUp().connect(bind(&Game::keyUp, this, placeholders::_1));

			camera_ui.setCamera(&display.camera);
			auto font_type = loadAsset("fonts/Nunito.ttf");
			font = TextureFont::create(Font(font_type, 20.0f), TextureFont::Format(), TextureFont::defaultChars() + u8"����\ue000\ue001\ue002\ue003\ue004\ue005\ue006");
			font_color = Color::white();
			background = Texture::create(loadImage(loadAsset("images/Earth.jpg")));
			display.camera.lookAt(float3(0, 250, 0.001), float3(0));
			display.camera.setFarClip(1000.0f);

			Entity test_object = world.create_entity("test-object", [](Entity entity) {
				entity.tag("Player");
				entity.add<Geometry>()
					//.mesh(Mesh::create(geom::Plane() >> geom::Rotate(quaternion(float3()))))
					.mesh(Mesh::create(geom::Cube()))
					.scaling(float3(0.25f))
					.position(float3(0.0, 0.01, 0.0));
				entity.add<Material>()
					.add(Texture::create(loadImage(loadAsset("textures/test.diffuse.png"))))
					.add(Texture::create(loadImage(loadAsset("textures/test.specular.png"))))
					.add(Texture::create(loadImage(loadAsset("textures/test.emissive.png"))))
					.add(Texture::create(loadImage(loadAsset("textures/test.normal.png"))));
				entity.deactivate();
			});

			wd::watch("shaders/*", [this, test_object](const fs::path& path) {
				//print("compiling shader ...");
				try {
					shared<Shader> shader;
					if (false) {
						string vertex_shader = loadString(loadAsset("shaders/Wireframe.vertex.shader"));
						string fragment_shader = loadString(loadAsset("shaders/Wireframe.fragment.shader"));
						string geometry_shader = loadString(loadAsset("shaders/Wireframe.geometry.shader"));
						shader = Shader::create(vertex_shader, fragment_shader, geometry_shader);
					} else {
						string vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
						//shader::define(vertex_shader, "HEIGHT_MAP");
						string fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
						shader::define(fragment_shader, "DIFFUSE_TEXTURE", "SPECULAR_TEXTURE", "EMISSIVE_TEXTURE", "NORMAL_MAP");
						shader = Shader::create(vertex_shader, fragment_shader);
						shader->uniform("uDiffuseTexture", 0);
						shader->uniform("uSpecularTexture", 1);
						shader->uniform("uEmissiveTexture", 2);
						shader->uniform("uNormalMap", 3);
						//shader->uniform("uOverlayTexture", 3);
						//shader->uniform("uHeightMap", 4);
						shader->uniform("uSpecularity", 1.0f);
						shader->uniform("uLuminosity", 1.0f);
					}
					message = "shader compiled successfully";
					test_object.get<Material>().shader = shader;
				} catch (GlslProgExc exception) {
					error(exception.what());
					message = exception.what();
				}
			});

			world.add<RenderSystem>();
			world.add<TileSystem>();
		}

		void Game::resize() {
			Display& display = world.find_entity("Main Display").get<Display>();
			display.size = getWindowSize();
			if (display.minimized()) return;
			camera_ui.setWindowSize(getWindowSize());
			display.camera.setAspectRatio(getWindowAspectRatio());
			display.framebuffer = FrameBuffer::create(display.size.x, display.size.y, FrameBuffer::Format().samples(16).coverageSamples(16));
		}

		void Game::update(float elapsed_seconds, unsigned frames_per_second) {
			this->frames_per_second = frames_per_second;
			time_delta = elapsed_seconds;
			time += time_delta;
			world.update(elapsed_seconds);
		}

		void Game::render() {
			Display& display = world.find_entity("Main Display").get<Display>();
			if (display.size.x == 0 or display.size.y == 0) return;

			static bool render_background = false;
			static bool render_world = true;
			static bool render_entity = false;
			static bool render_test_object = false;
			static bool render_infos = false;
			static bool render_demo = false;
			static bool render_generator = true;
			static bool enable_tile_system = true;
			static bool vertical_synchronization = true;
			bool update_world = false;
			{
				ui::ScopedWindow ui_window("Menu", ImGuiWindowFlags_NoTitleBar);
				ui::Checkbox("Background", &render_background);
				if (ui::Checkbox("World", &render_world)) {
					world.get<TileSystem>().get_entities().begin()->get<Instantiable>().active = render_world;
				}
				if (ui::Checkbox("Entity", &render_entity)) {
					auto entity = world.find_entity("test-object");
					if (render_entity) entity.activate();
					else entity.deactivate();
				}
				ui::Checkbox("Infos", &render_infos);
				ui::Checkbox("Demo", &render_demo);
				update_world |= ui::Checkbox("Generator", &render_generator) and not render_generator;
				if (ui::Checkbox("Tile System", &enable_tile_system)) {
					if (enable_tile_system)	world.get<TileSystem>().activate();
					else world.get<TileSystem>().deactivate();
				}
				if (ui::Checkbox("V-Sync", &vertical_synchronization)) enableVerticalSync(vertical_synchronization);
			}

			if (render_world) {
				ui::ScopedWindow ui_window("Hexagonal World Map");
				static unsigned2 size = { 16, 9 };
				static float scale = 1.0f, power = 1.0f, resolution = 0.1f;
				bool resize_world = false;
				if (ui::SliderPercentage("Resolution", resolution, 0.1f, 2.0f, "%.0f%%", 1.0f, 1.0f)) {
					size = float2(160, 90) * resolution;
					resize_world = true;
				}
				resize_world |= ui::SliderUnsigned("Width", size.x, 16, 16 * 20);
				resize_world |= ui::SliderUnsigned("Height", size.y, 9, 9 * 20);
				if (resize_world) world.get<TileSystem>().resize(size);
				update_world |= resize_world;
				update_world |= ui::SliderFloat("Elevation Scale", scale, 0.1f, 10.0f, "%.2f", 3.45f, 1.0f);
				update_world |= ui::SliderFloat("Elevation Power", power, 0.1f, 10.0f, "%.2f", 3.45f, 1.0f);
				if (update_world) {
					world.get<TileSystem>().update(generator.biomes, generator.elevation, scale, power);
				}
			}

			setMatricesWindow(display.size);
			if (render_background) draw(background);
			else clear();

			if (world.has<RenderSystem>()) world.get<RenderSystem>().render();
			if (render_demo) ui::ShowTestWindow();
			if (render_generator) generator.display();

			auto& tile = world.get<TileSystem>().selected_tile;
			static double last = getElapsedSeconds();
			double now = getElapsedSeconds();
			if (tile or now - last < 0.15f) {
				static const char* biome_name = nullptr;
				if (tile) {
					biome_name = tile->biome;
					last = now;
				}
				if (biome_name != nullptr) {
					color(0.0f, 0.0f, 0.0f, 0.9f);
					drawSolidRect(Area(0, display.size.y - 100, 160, display.size.y - 60));
					color(Color::white());
					drawString(biome_name, float2(15, display.size.y - 100), font_color, font->getFont());
				}
			}

			if (not render_infos) return;

			drawString(u8"\ue000 Sethex", float2(10, 10), font_color, font->getFont());
			drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), font_color, font->getFont());
			//drawString(message, float2(5, display.size.y - 50), font_color, font->getFont());
			drawString(to_string(frames_per_second) + " FPS", float2(5, display.size.y - 15));
			drawStringRight(u8"Thomas W�rstle", float2(display.size.x - 5, display.size.y - 15));
			drawStringRight(stringify("Focus Position ", display.camera.getPivotPoint()), float2(display.size.x - 5, 5));
			drawStringRight(stringify("Eye Position ", display.camera.getEyePoint()), float2(display.size.x - 5, 20));
			drawStringRight(stringify("Focus Coordinates ", world.get<TileSystem>().focus_coordinates), float2(display.size.x - 5, 35));
			drawStringRight(stringify("Focus Coordinates Magnitude ", world.get<TileSystem>().focus_coordinates.magnitude()), float2(display.size.x - 5, 50));
		}

		void Game::mouseMove(MouseEvent event) {}

		void Game::mouseDrag(MouseEvent event) {
			camera_ui.mouseDrag(event);
		}

		void Game::mouseDown(MouseEvent event) {
			camera_ui.mouseDown(event);
		}

		void Game::mouseUp(MouseEvent event) {
			camera_ui.mouseUp(event);
		}

		void Game::mouseWheel(MouseEvent event) {
			camera_ui.mouseWheel(event);
		}

		void Game::keyDown(KeyEvent event) {}

		void Game::keyUp(KeyEvent event) {
			Display& display = world.find_entity("Main Display").get<Display>();
			switch (event.getCode()) {
				case KeyEvent::KEY_F11:
					display.window->setFullScreen(!display.window->isFullScreen());
					break;
				case KeyEvent::KEY_END:
					display.camera.lookAt(vec3(0, 2.5, 2.5), vec3(0));
					break;
				case KeyEvent::KEY_SPACE:
					display.camera.lookAt(float3(0, 250, 0.001), float3(0));
					break;
				default:
					break;
			}
		}

	}

}
