#include "Game.h"

#include <cinder/ObjLoader.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>
#include <cinder/app/App.h>
#include <cinder/utilities/Watchdog.h>
#include <cinder/interface/CinderImGui.h>
#include <cinder/utilities/Shaders.h>
#include <cinder/utilities/Simplex.h>

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

	shared<Texture> elevation_texture;
	shared<Texture> terrain_texture;

	void Game::setup(CameraUi& camera_ui) {
		Display& display = world.create_entity("Main Display").add<Display>();

		camera_ui.setCamera(&display.camera);
		auto font_type = loadAsset("fonts/Nunito.ttf");
		font = TextureFont::create(Font(font_type, 20.0f), TextureFont::Format(), TextureFont::defaultChars() + u8"äöüß\ue000\ue001\ue002\ue003\ue004\ue005\ue006");
		font_color = Color::white();
		background = Texture::create(loadImage(loadAsset("images/Background.jpg")));
		display.camera.lookAt(float3(0, 2.5, 2.5), float3(0));

		shared<Mesh> mesh = Mesh::create(geom::Plane() >> geom::Rotate(quaternion(float3())));

		Entity terrain = world.create_entity();
		terrain.add<Geometry>().mesh(mesh).position(float3(0.0, 0.01, 0.0));
		auto& material = terrain.add<Material>()
			.add(Texture::create(loadImage(loadAsset("textures/test.diffuse.png"))))
			.add(Texture::create(loadImage(loadAsset("textures/test.specular.png"))))
			.add(Texture::create(loadImage(loadAsset("textures/test.emissive.png"))))
			.add(Texture::create(loadImage(loadAsset("textures/test.normal.png"))));
		terrain.deactivate();

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

		{
			static int seed = 0, seed_maximum = 1000000000;
			static float2 shift;
			static float scale = 1.0f;
			static Simplex::Options current_options;
			static Simplex::Options saved_options;
			static float details = 1.0f, default_details = 1.0f;
			static bool threshold = false;
			static bool update_noise = true;

			ui::ScopedWindow ui_window("Noise Texture", ImGuiWindowFlags_HorizontalScrollbar);

			auto assign = [](const Surface::Iter& iterator, const Color8u& color) {
				iterator.r() = color.r;
				iterator.g() = color.g;
				iterator.b() = color.b;
			};
			auto assign_elevation = [&assign](const Surface::Iter& iterator, float elevation) {
				if (elevation < -0.25f) { assign(iterator, { 0, 0, 128 }); return; } // ocean
				if (elevation < -0.0f) { assign(iterator, { 25, 25, 150 }); return; } // coast
				if (elevation < 0.1f) { assign(iterator, { 240, 240, 64 }); return; } // beach
				if (elevation < 0.33f) { assign(iterator, { 50, 220, 20 }); return; } // prairie
				if (elevation < 0.5f) { assign(iterator, { 16, 160, 0 }); return; } // forrest
				if (elevation < 0.66f) { assign(iterator, { 128, 128, 128 }); return; } // mountain
				assign(iterator, { 255, 255, 255 }); // snowcap
			};

			if (update_noise) {
				//debug("update noise");
				if (seed < 0 || seed > seed_maximum) seed = seed_maximum;
				Simplex::seed(seed);
				Channel channel(512, 512);
				Surface surface(channel.getWidth(), channel.getHeight(), false, SurfaceChannelOrder::RGB);
				float2 size = channel.getSize();
				float2 center = size / 2.0f;
				auto channel_iterator = channel.getIter();
				auto surface_iterator = surface.getIter();
				while (channel_iterator.line() and surface_iterator.line()) {
					while (channel_iterator.pixel() and surface_iterator.pixel()) {
						float2 position = float2(channel_iterator.getPos());
						position = (position - center) / scale + center;
						position += shift * size;
						float elevation = Simplex::noise(position * 0.01f, current_options);
						if (threshold) {
							channel_iterator.v() = elevation < 0.0f ? 0 : 255;
						} else {
							channel_iterator.v() = static_cast<uint8>((elevation + 1) / 2 * 255);
						}
						assign_elevation(surface_iterator, elevation);
					}
				}
				elevation_texture = Texture::create(channel);
				terrain_texture = Texture::create(surface);
				update_noise = false;
			}

			ui::BeginChild("l", elevation_texture->getSize(), false);
			ui::Image(elevation_texture, elevation_texture->getSize());
			ui::EndChild();

			ui::SameLine();

			ui::BeginChild("r", elevation_texture->getSize());
			ui::Text("Presets:");
			if (ui::Button("Reset")) {
				seed = 0;
				scale = 1.0f;
				shift = {};
				current_options = {};
				details = default_details;
				update_noise = true;
			}
			ui::SameLine();
			if (ui::Button("World")) {
				seed = 0;
				scale = 2.0f;
				shift = {};
				current_options = {};
				details = default_details;
				current_options.octaves = 5;
				update_noise = true;
			}
			update_noise |= ui::DragInt("Seed", seed, 1.0f, 0, seed_maximum);
			update_noise |= ui::SliderFloat("Scale", scale, 0.1f, 10.0f, "%.2f", 3.45f);
			update_noise |= ui::SliderFloat2("Shift", shift, -1.0f, 1.0f, "%.2f");
			update_noise |= ui::SliderUnsigned("Octaves", current_options.octaves, 1, 15, "%.0f", &saved_options.octaves);
			update_noise |= ui::SliderFloat("Amplitude", current_options.amplitude, 0.0f, 10.0f, "%.2f", 1.0f, &saved_options.amplitude); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Frequency", current_options.frequency, 0.0f, 10.0f, "%.2f", 1.0f, &saved_options.frequency); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Lacunarity", current_options.lacunarity, 0.0f, 10.0f, "%.2f", 1.0f, &saved_options.lacunarity); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Persistence", current_options.persistence, 0.0f, 2.0f, "%.2f", 1.0f, &saved_options.persistence); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Power", current_options.power, 0.1f, 10.0f, "%.2f", 1.0f, &saved_options.power); ui::Hint("Ctrl+Click to enter an exact value");
			if (ui::SliderFloat("Details", details, 0.0f, 1.0f, "%.2f", 1.0f, &default_details)) {
				current_options.lacunarity = 2.0f * details;
				current_options.persistence = 1.0f / current_options.lacunarity;
				update_noise = true;
			}
			update_noise |= ui::Checkbox("Positive", current_options.positive); ui::Tooltip("sets noise range to [0,1] instead of [-1,1]");
			ui::SameLine();
			update_noise |= ui::Checkbox("Threshold", threshold); ui::Tooltip("map positive values to white, negative ones to black");
			ui::Text("0.5 ^ %.1f -> %.1f", current_options.power, glm::pow(0.5, current_options.power));
			ui::EndChild();

			ui::Image(terrain_texture, terrain_texture->getSize());
		}
	}

	void Game::render() {
		static Display& display = world.find_entity("Main Display").get<Display>();
		if (display.size.x == 0 or display.size.y == 0) return;

		static bool render_background = true;
		static bool render_world = true;
		static bool render_overlay = true;
		static bool render_interface = false;
		static bool vertical_synchronization = true;
		{
			ui::ScopedWindow ui_window("", ImGuiWindowFlags_NoTitleBar);
			ui::Checkbox("Background", &render_background);
			ui::Checkbox("World", &render_world);
			ui::Checkbox("Overlay", &render_overlay);
			ui::Checkbox("Interface", &render_interface);
			if (ui::Checkbox("V-Sync", &vertical_synchronization)) enableVerticalSync(vertical_synchronization);
		}

		setMatricesWindow(display.size);
		if (render_background) draw(background);
		else clear();

		if (render_world) world.get<RenderSystem>().render();
		if (render_interface) ui::ShowTestWindow();

		if (not render_overlay) return;
		drawString(u8"\ue000 Sethex", float2(10, 10), font_color, font->getFont());
		drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), font_color, font->getFont());
		drawString(message, float2(5, display.size.y - 50), font_color, font->getFont());
		drawString(to_string(frames_per_second) + " FPS", float2(5, display.size.y - 15));
		drawStringRight(u8"Thomas Würstle", float2(display.size.x - 5, display.size.y - 15));
	}

}
