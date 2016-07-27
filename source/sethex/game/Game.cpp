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

	shared<Texture> noise_texture;

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
			static bool threshold = false;
			static bool update_noise = true;

			ui::ScopedWindow ui_window("Noise Texture", ImGuiWindowFlags_HorizontalScrollbar);

			if (update_noise) {
				//debug("update noise");
				if (seed < 0 || seed > seed_maximum) seed = seed_maximum;
				Simplex::seed(seed);
				Channel channel(512, 512);
				auto iterator = channel.getIter();
				while (iterator.line()) {
					while (iterator.pixel()) {
						float2 position = iterator.getPos();
						position += shift * float2(channel.getSize());
						position *= scale * 0.01f;
						float noise = Simplex::noise(position, current_options);
						//float noise = amplitude * Simplex::fBm(position * frequency, octaves, lacunarity, persistence);
						//if (positive) noise = (noise + 1) / 2;
						//noise = amplitude * pow(noise, exponent);
						if (threshold) {
							iterator.v() = noise < 0.0f ? 0 : 255;
						} else {
							iterator.v() = static_cast<uint8>((noise + 1) / 2 * 255);
						}
					}
				}
				noise_texture = Texture::create(channel);
				update_noise = false;
			}

			ui::BeginChild("l", noise_texture->getSize(), false, ImGuiWindowFlags_HorizontalScrollbar);
			ui::Image(noise_texture, noise_texture->getSize());
			ui::EndChild();

			ui::SameLine();

			ui::BeginChild("r", noise_texture->getSize());
			ui::Text("Presets:");
			if (ui::Button("Reset")) {
				shift = {};
				current_options = {};
				update_noise = true;
			}
			ui::SameLine();
			if (ui::Button("Mist")) {
				shift = { -0.5f, -0.5f };
				current_options = {};
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
			update_noise |= ui::Checkbox("Positive", current_options.positive); ui::Tooltip("sets noise range to [0,1] instead of [-1,1]");
			ui::SameLine();
			update_noise |= ui::Checkbox("Threshold", threshold); ui::Tooltip("map positive values to white, negative ones to black");
			ui::EndChild();
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
