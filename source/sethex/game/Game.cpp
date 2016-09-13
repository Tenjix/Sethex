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
using namespace sethex::hexagonal;

namespace sethex {

	void Game::setup(CameraUi& camera_ui) {
		Display& display = world.create_entity("Main Display").add<Display>();

		camera_ui.setCamera(&display.camera);
		auto font_type = loadAsset("fonts/Nunito.ttf");
		font = TextureFont::create(Font(font_type, 20.0f), TextureFont::Format(), TextureFont::defaultChars() + u8"äöüß\ue000\ue001\ue002\ue003\ue004\ue005\ue006");
		font_color = Color::white();
		background = Texture::create(loadImage(loadAsset("images/Background.jpg")));
		display.camera.lookAt(float3(0, 2.5, 2.5), float3(0));

		shared<Mesh> mesh = Mesh::create(geom::Plane() >> geom::Rotate(quaternion(float3())));

		if (not executable) return;

		Entity entity = world.create_entity("test-plane");
		entity.add<Geometry>().mesh(mesh).position(float3(0.0, 0.01, 0.0));
		auto& material = entity.add<Material>()
			.add(Texture::create(loadImage(loadAsset("textures/test.diffuse.png"))))
			.add(Texture::create(loadImage(loadAsset("textures/test.specular.png"))))
			.add(Texture::create(loadImage(loadAsset("textures/test.emissive.png"))))
			.add(Texture::create(loadImage(loadAsset("textures/test.normal.png"))));
		entity.deactivate();

		wd::watch("shaders/*", [this, &shader = material.shader](const fs::path& path) {
			print("compiling shader ...");
			try {
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

			} catch (GlslProgCompileExc exception) {
				error(exception.what());
				message = exception.what();

			}
		});

		world.add<RenderSystem>();
		world.add<TileSystem>(input);
	}

	void Game::resize() {
		static Display& display = world.find_entity("Main Display").get<Display>();
		display.size = getWindowSize();
		if (display.size.x == 0 or display.size.y == 0) return;
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
		static Display& display = world.find_entity("Main Display").get<Display>();
		if (display.size.x == 0 or display.size.y == 0) return;

		static bool render_background = true;
		static bool render_world = true;
		static bool render_entity = false;
		static bool render_overlay = true;
		static bool render_interface = false;
		static bool render_generator = true;
		static bool vertical_synchronization = true;
		{
			ui::ScopedWindow ui_window("", ImGuiWindowFlags_NoTitleBar);
			ui::Checkbox("Background", &render_background);
			if (executable) {
				if (ui::Checkbox("World", &render_world)) {
					auto tiles = world.find_entities_beginning("Tile #");
					for each (auto tile in tiles) {
						if (render_world) tile.activate();
						else tile.deactivate();
					}
				}
				if (ui::Checkbox("Entity", &render_entity)) {
					auto entity = world.find_entity("test-plane");
					if (render_entity) entity.activate();
					else entity.deactivate();
				}
			}
			ui::Checkbox("Overlay", &render_overlay);
			ui::Checkbox("Interface", &render_interface);
			ui::Checkbox("Generator", &render_generator);
			if (ui::Checkbox("V-Sync", &vertical_synchronization)) enableVerticalSync(vertical_synchronization);
		}

		setMatricesWindow(display.size);
		if (render_background) draw(background);
		else clear();

		if (executable and (render_world or render_entity)) world.get<RenderSystem>().render();
		if (render_interface) ui::ShowTestWindow();
		if (render_generator) generator.display();

		if (not render_overlay) return;
		drawString(u8"\ue000 Sethex", float2(10, 10), font_color, font->getFont());
		drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), font_color, font->getFont());
		drawString(message, float2(5, display.size.y - 50), font_color, font->getFont());
		drawString(to_string(frames_per_second) + " FPS", float2(5, display.size.y - 15));
		drawStringRight(u8"Thomas Würstle", float2(display.size.x - 5, display.size.y - 15));
	}

}
