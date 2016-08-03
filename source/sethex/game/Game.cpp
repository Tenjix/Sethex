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

	shared<Channel> elevation_channel;
	shared<Texture> elevation_texture;
	shared<Surface> terrain_surface;
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

	struct BiomeThresholds {
		float ocean = -0.5f;
		float coast = -0.1f;
		float beach = 0.0f;
		float prairie = 0.05f;
		float forrest = 0.33f;
		float mountain = 0.5f;
		float snowcap = 0.66f;

		bool operator==(const BiomeThresholds& other) {
			return ocean == other.ocean
				and coast == other.coast
				and beach == other.beach
				and prairie == other.prairie
				and forrest == other.forrest
				and mountain == other.mountain
				and snowcap == other.snowcap;
		}
		bool operator!=(const BiomeThresholds& other) {
			return not operator==(other);
		}
	};

	void Game::update(float elapsed_seconds, unsigned frames_per_second) {
		this->frames_per_second = frames_per_second;
		time_delta = elapsed_seconds;
		time += time_delta;
		world.update(elapsed_seconds);

		{
			static int seed = 0, seed_maximum = 1000000000;
			static float2 shift;
			static signed2 drag;
			static float scale = 1.0f, roll = 0.0f;
			static Simplex::Options current_options;
			static Simplex::Options saved_options;
			static float continent_frequency = 1.0f, sealevel = 0.0f;
			static bool use_continents = false;
			static bool threshold = false;
			static bool update_noise = true;
			static unsigned water_pixels;

			static BiomeThresholds thresholds, default_thresholds;

			ui::ScopedWindow ui_window("Noise Texture", ImGuiWindowFlags_HorizontalScrollbar);

			auto assign_elevation = [](const Surface::Iter& iterator, float elevation) {
				if (elevation > 0.33f * sealevel + thresholds.snowcap) { assign(iterator, { 255, 255, 255 }); return; } // snowcap
				if (elevation > 0.5f * sealevel + thresholds.mountain) { assign(iterator, { 128, 128, 128 }); return; } // mountain
				if (elevation > 0.66f * sealevel + thresholds.forrest) { assign(iterator, { 16, 160, 0 }); return; } // forrest
				if (elevation > sealevel + thresholds.prairie) { assign(iterator, { 50, 220, 20 }); return; } // prairie
				if (elevation > sealevel + thresholds.beach) { assign(iterator, { 240, 240, 64 }); return; } // beach
				water_pixels++;
				if (elevation > sealevel + thresholds.coast) { assign(iterator, { 25, 25, 150 }); return; } // coast
				if (elevation > sealevel + thresholds.ocean) { assign(iterator, { 0, 0, 128 }); return; } // ocean
				assign(iterator, { 0, 0, 96 }); // deep ocean
			};

			if (update_noise) {
				//debug("update noise");
				if (seed < 0 || seed > seed_maximum) seed = seed_maximum;
				water_pixels = 0;
				Simplex::seed(seed);
				auto channel = Channel::create(800, 450);
				auto surface = Surface::create(channel->getWidth(), channel->getHeight(), false, SurfaceChannelOrder::RGB);
				float2 size = channel->getSize();
				float2 center = size / 2.0f;
				auto channel_iterator = channel->getIter();
				auto surface_iterator = surface->getIter();
				scale = clamp(scale * (1.0f - roll), 0.1f, 10.0f);
				shift -= float2(drag) / scale;
				while (channel_iterator.line() and surface_iterator.line()) {
					while (channel_iterator.pixel() and surface_iterator.pixel()) {
						signed2 pixel = channel_iterator.getPos();
						if (drag != zero) {
							bool x_copyable = drag.x == 0 or drag.x > 0 and pixel.x >= drag.x or drag.x < 0 and pixel.x < size.x + drag.x;
							bool y_copyable = drag.y == 0 or drag.y > 0 and pixel.y >= drag.y or drag.y < 0 and pixel.y < size.y + drag.y;
							if (x_copyable and y_copyable) {
								channel_iterator.v() = elevation_channel->getValue(pixel - drag);
								assign_elevation(surface_iterator, channel_iterator.v() / 255.0f * 2 - 1);
								continue;
							}
						}
						float2 position = pixel;
						position = (position - center) / scale + center;
						position += shift;
						float elevation = Simplex::noise(position * 0.01f, current_options);
						if (use_continents) {
							float continents = Simplex::noise(position * 0.01f * continent_frequency);
							elevation = (elevation + continents) / (current_options.amplitude + 1.0f);
						}
						if (threshold) {
							channel_iterator.v() = elevation < 0.0f ? 0 : 255;
						} else {
							channel_iterator.v() = static_cast<uint8>((elevation + 1) / 2 * 255);
						}
						assign_elevation(surface_iterator, elevation);
					}
				}
				elevation_channel = channel;
				if (elevation_texture) elevation_texture->update(*elevation_channel);
				else elevation_texture = Texture::create(*elevation_channel);
				terrain_surface = surface;
				if (terrain_texture) terrain_texture->update(*terrain_surface);
				else terrain_texture = Texture::create(*terrain_surface);
				update_noise = false;
			}

			auto region_size = elevation_texture->getSize() + 6;

			ui::BeginChild("elevation map", region_size);
			ui::Image(elevation_texture, elevation_texture->getSize());
			ui::EndChild();

			ui::SameLine();

			ui::BeginChild("elevation map properties", region_size);
			ui::Text("Presets:");
			if (ui::Button("Reset")) {
				seed = 0;
				scale = 1.0f;
				shift = {};
				current_options = {};
				use_continents = false;
				continent_frequency = 0.5f;
				sealevel = 0.0f;
				thresholds = default_thresholds;
				update_noise = true;
			}
			ui::SameLine();
			if (ui::Button("Continents")) {
				current_options = {};
				current_options.amplitude = 0.5f;
				current_options.octaves = 5;
				use_continents = true;
				continent_frequency = 0.25f;
				sealevel = 0.25f;
				update_noise = true;
			}
			ui::SameLine();
			if (ui::Button("Islands")) {
				current_options = {};
				current_options.amplitude = 2.0f;
				current_options.octaves = 5;
				use_continents = true;
				continent_frequency = 0.5f;
				sealevel = 0.25f;
				update_noise = true;
			}
			update_noise |= ui::DragInt("Seed", seed, 1.0f, 0, seed_maximum, "%.0f", 0);
			update_noise |= ui::SliderFloat("Scale", scale, 0.1f, 10.0f, "%.2f", 3.45f, 1.0f);
			update_noise |= ui::DragFloat2("Shift", shift, 1.0f, 0.0f, 0.0f, "%.2f", 1.0f, zero);
			update_noise |= ui::SliderUnsigned("Octaves", current_options.octaves, 1, 15, "%.0f", saved_options.octaves);
			update_noise |= ui::SliderFloat("Amplitude", current_options.amplitude, 0.0f, 10.0f, "%.2f", 1.0f, saved_options.amplitude); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Frequency", current_options.frequency, 0.0f, 10.0f, "%.2f", 1.0f, saved_options.frequency); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Lacunarity", current_options.lacunarity, 0.0f, 10.0f, "%.2f", 1.0f, saved_options.lacunarity); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Persistence", current_options.persistence, 0.0f, 2.0f, "%.2f", 1.0f, saved_options.persistence); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::SliderFloat("Power", current_options.power, 0.1f, 10.0f, "%.2f", 1.0f, saved_options.power); ui::Hint("Ctrl+Click to enter an exact value");
			update_noise |= ui::Checkbox("Positive", current_options.positive); ui::Tooltip("sets noise range to [0,1] instead of [-1,1]");
			ui::SameLine();
			update_noise |= ui::Checkbox("Threshold", threshold); ui::Tooltip("map positive values to white, negative ones to black");
			ui::SameLine();
			update_noise |= ui::Checkbox("Continents##use", use_continents);
			update_noise |= ui::SliderFloat("Continent Frequency", continent_frequency, 0.0f, 2.0f, "%.2f", 1.0f, 0.5f);
			unsigned pixels = terrain_texture->getWidth() * terrain_texture->getHeight();
			float water_percentage = 100.0f * water_pixels / pixels;
			ui::Text("%.1f%% Water, %.1f%% Land", water_percentage, 100.0f - water_percentage);
			ui::EndChild();

			ui::BeginChild("world map", region_size);
			ui::Image(terrain_texture, terrain_texture->getSize());
			if (ui::IsItemHoveredRect()) {
				roll = ui::GetIO().MouseWheel * 0.1f;
				drag = ui::IsMouseDragging() ? signed2(ui::GetIO().MouseDelta) : zero;
				update_noise = drag != zero or roll != zero;
			}
			ui::EndChild();

			ui::SameLine();

			ui::BeginChild("world map properties", region_size);
			static float waterlevel = 0.0;
			update_noise |= ui::SliderFloat("Sealevel", sealevel, -1.0f, 1.0f, "%.2f", 1.0f, 0.0);
			ui::Text("Thresholds:");
			if (thresholds != default_thresholds) {
				ui::SameLine();
				if (ui::SmallButton("Reset##thresholds")) {
					thresholds = default_thresholds;
					update_noise = true;
				}
			}
			update_noise |= ui::SliderFloat("Snowcap", thresholds.snowcap, thresholds.mountain, 1.0f, "%.2f");
			update_noise |= ui::SliderFloat("Mountain", thresholds.mountain, thresholds.forrest, thresholds.snowcap, "%.2f");
			update_noise |= ui::SliderFloat("Forrest", thresholds.forrest, thresholds.prairie, thresholds.mountain, "%.2f");
			update_noise |= ui::SliderFloat("Prairie", thresholds.prairie, thresholds.beach, thresholds.forrest, "%.2f");
			update_noise |= ui::SliderFloat("Beach", thresholds.beach, thresholds.coast, thresholds.prairie, "%.2f");
			update_noise |= ui::SliderFloat("Coast", thresholds.coast, thresholds.ocean, thresholds.beach, "%.2f");
			update_noise |= ui::SliderFloat("Ocean", thresholds.ocean, -1.0f, thresholds.coast, "%.2f");
			ui::Text("dragged x=%i, y=%i", drag.x, drag.y);
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
