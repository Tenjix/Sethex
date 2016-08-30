#include "Generator.h"

#include <cinder/interface/Imgui.h>
#include <cinder/utilities/Simplex.h>

#include <sethex/Common.h>
#include <sethex/Graphics.h>

#include <utilities/Exceptions.h>
#include <utilities/Mathematics.h>

using namespace cinder;
using namespace constf;
using namespace std;

namespace sethex {

	shared<Channel32f> elevation_buffer;
	shared<Channel32f> elevation_map;
	shared<Channel> temperature_map;
	shared<Channel> precipitation_map;
	shared<Surface> terrain_map;
	shared<Texture> map_texture;

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

	struct HumidityCell {
		vector<float> humidity;
		unsigned start;
		unsigned end;
	};

	template <class Type>
	float calculate_elevation(const Type& position, const Simplex::Options& options, bool continents, float continent_frequency, float continent_amplitude) {
		float elevation = Simplex::noise(position * 0.01f, options);
		if (continents) {
			float continental_elevation = Simplex::noise(position * 0.01f * continent_frequency);
			if (options.positive) continental_elevation = Simplex::to_unsigned(continental_elevation);
			elevation = (elevation + continent_amplitude * continental_elevation) / (options.amplitude + continent_amplitude);
		}
		return elevation;
	}

	void Generator::display() {
		static int seed = 0, seed_maximum = 1000000000;
		static float2 shift;
		static signed2 drag;
		static float scale = 1.0f, roll = 0.0f;
		static Simplex::Options current_options;
		static Simplex::Options saved_options;
		static float continent_frequency = 0.5f, continent_amplitude = 1.0f, sealevel = 0.0f;
		static float equator_distance_factor = 0.0f;
		static int equator_distance_power = 10;
		static bool wrap_horizontally = false;
		static bool use_continents = false;
		static bool landmass = false;
		static bool update_noise = true, update_world = true;
		static unsigned water_pixels;
		static float elevation_minimum, elevation_maximum;

		static BiomeThresholds thresholds, default_thresholds;

		ui::ScopedWindow ui_window("Noise Texture", ImGuiWindowFlags_HorizontalScrollbar);

		auto assign_elevation = [](const Surface::Iter& iterator, float elevation) {
			if (elevation < elevation_minimum) elevation_minimum = elevation;
			if (elevation > elevation_maximum) elevation_maximum = elevation;
			if (elevation > 0.33f * sealevel + thresholds.snowcap) { iterator << Color8u(255, 255, 255); return; } // snowcap
			if (elevation > 0.5f * sealevel + thresholds.mountain) { iterator << Color8u(128, 128, 128); return; } // mountain
			if (elevation > 0.66f * sealevel + thresholds.forrest) { iterator << Color8u(0, 160, 0); return; } // forrest
			if (elevation > sealevel + thresholds.prairie) { iterator << Color8u(32, 204, 32); return; } // prairie
			if (elevation > sealevel + thresholds.beach) { iterator << Color8u(240, 240, 160); return; } // beach
			water_pixels++;
			if (elevation > sealevel + thresholds.coast) { iterator << Color8u(16, 16, 160); return; } // coast
			if (elevation > sealevel + thresholds.ocean) { iterator << Color8u(0, 0, 128); return; } // ocean
			iterator << Color8u(0, 0, 96); // deep ocean
		};

		signed2 map_resolution = { 800, 450 };
		update_world |= update_noise;
		if (update_world) {
			elevation_map = Channel32f::create(map_resolution.x, map_resolution.y);
			if (update_noise) {
				elevation_minimum = elevation_maximum = zero;
				if (seed < 0 || seed > seed_maximum) seed = seed_maximum;
				Simplex::seed(seed);
				auto elevation_iterator = elevation_map->getIter();
				float2 center = float2(map_resolution) / 2.0f;
				scale = clamp(scale * (1.0f - roll), 0.1f, 10.0f);
				shift -= wrap_horizontally ? float2(drag.x, drag.y / scale) : float2(drag) / scale;
				while (elevation_iterator.line()) {
					while (elevation_iterator.pixel()) {
						signed2 pixel = elevation_iterator.getPos();
						if (drag != zero) {
							bool x_copyable = drag.x == 0 or drag.x > 0 and pixel.x >= drag.x or drag.x < 0 and pixel.x < map_resolution.x + drag.x;
							bool y_copyable = drag.y == 0 or drag.y > 0 and pixel.y >= drag.y or drag.y < 0 and pixel.y < map_resolution.y + drag.y;
							if (x_copyable and y_copyable) {
								elevation_iterator.v() = elevation_buffer->getValue(pixel - drag);
								continue;
							}
						}
						float elevation;
						float2 position = pixel;
						if (wrap_horizontally) {
							float repeat_interval = map_resolution.x / scale;
							position.y = (position.y - center.y) / scale + center.y;
							position += shift;
							float radians = position.x / map_resolution.x * Tau;
							float3 cylindrical_position;
							cylindrical_position.x = sin(radians) / Tau * repeat_interval;
							cylindrical_position.y = position.y;
							cylindrical_position.z = cos(radians) / Tau * repeat_interval;
							elevation = calculate_elevation(cylindrical_position, current_options, use_continents, continent_frequency, continent_amplitude);
						} else {
							position = (position - center) / scale + center;
							position += shift;
							elevation = calculate_elevation(position, current_options, use_continents, continent_frequency, continent_amplitude);
						}
						elevation_iterator.v() = elevation;
					}
				}
				elevation_buffer = Channel32f::create(*elevation_map);
				update_noise = false;
			}

			temperature_map = Channel::create(map_resolution.x, map_resolution.y);
			precipitation_map = Channel::create(map_resolution.x, map_resolution.y);
			terrain_map = Surface::create(map_resolution.x, map_resolution.y, false, SurfaceChannelOrder::RGB);
			vector<vector<float>> humidity_map { 6, vector<float>(map_resolution.x) };

			auto buffer_iterator = elevation_buffer->getIter();
			auto elevation_iterator = elevation_map->getIter();
			auto temperature_iterator = temperature_map->getIter();
			auto precipitation_iterator = precipitation_map->getIter();
			auto terrain_iterator = terrain_map->getIter();
			water_pixels = 0;
			while (buffer_iterator.line() and elevation_iterator.line() and temperature_iterator.line() and precipitation_iterator.line() and terrain_iterator.line()) {
				float y = static_cast<float>(elevation_iterator.y()) / map_resolution.y; // y position [0.0, 1.0]
				float distance_to_equator = abs(2.0f * (y - 0.5f)); // distance to equator [0.0, 1.0]
				float elevation_change = pow(distance_to_equator, equator_distance_power) * equator_distance_factor;
				while (buffer_iterator.pixel() and elevation_iterator.pixel() and temperature_iterator.pixel() and precipitation_iterator.pixel() and terrain_iterator.pixel()) {
					float elevation = buffer_iterator.v();
					elevation = clamp(elevation + elevation_change, current_options.positive ? 0.0f : -1.0f, 1.0f);
					float elevation_above_sealevel = max(elevation - sealevel, 0.0f) / (1.0f - sealevel);
					if (landmass) {
						elevation_iterator.v() = elevation_above_sealevel;
					} else {
						elevation_iterator.v() = current_options.positive ? elevation : (elevation + 1.0f) / 2.0f;
					}
					float2 position = elevation_iterator.getPos();
					float temperature = 1.0f - distance_to_equator * distance_to_equator;
					//float temperature_noise = Simplex::to_unsigned(Simplex::noise(elevation_iterator.getPos().x * 0.02f * continent_frequency));
					float temperature_noise = Simplex::to_unsigned(Simplex::noise(position * 0.02f * continent_frequency));
					temperature = mix(temperature, temperature_noise, 0.1f);
					if (elevation > sealevel) temperature -= elevation_above_sealevel * elevation_above_sealevel;
					else temperature -= 0.1f;
					temperature = clamp(temperature, 0.0f, 1.0f);
					//temperature = ceil(max(temperature, 0.1f), 3);
					temperature_iterator.v() = static_cast<uint8>(temperature * 255.0f + 0.5f);
					precipitation_iterator.v() = elevation - sealevel > 0.0f ? 0 : 255;
					assign_elevation(terrain_iterator, elevation);
				}
			}
		}

		ui::BeginChild("elevation map", float2(map_resolution.x, 0));
		int selected_map = 0;
		ui::PushItemWidth(125);
		if (ui::Combo("Map##selection", selected_map, { "Terrain", "Elevation", "Temperature", "Precipitation" }) or update_world) {
			switch (selected_map) {
				case 0:
					map_texture = Texture::create(*terrain_map);
					break;
				case 1:
					map_texture = Texture::create(*elevation_map);
					break;
				case 2:
					map_texture = Texture::create(*temperature_map);
					break;
				case 3:
					map_texture = Texture::create(*precipitation_map);
					break;
				default: throw_runtime_exception("invalid map");
			}
			update_world = false;
		}
		ui::PopItemWidth();
		ui::ImageButton(map_texture, map_texture->getSize(), 0);
		bool map_hovered = ui::IsItemHoveredRect();
		signed2 map_position = ui::GetItemRectMin();
		if (ui::IsItemHovered()) {
			roll = ui::GetIO().MouseWheel * 0.1f;
			drag = ui::IsMouseDragging() ? signed2(ui::GetIO().MouseDelta) : zero;
			update_noise = drag != zero or roll != zero;
		}
		ui::EndChild();

		ui::SameLine();

		ui::BeginChild("elevation map properties");
		int selected_preset = 0;
		if (ui::Combo("Preset##selection", selected_preset, { "Default", "Alpha World", "Beta World" })) switch (selected_preset) {
			case 0:
				seed = 0;
				scale = 1.0f;
				shift = {};
				current_options = {};
				use_continents = false;
				continent_frequency = 0.5f;
				continent_amplitude = 1.0f;
				sealevel = 0.0f;
				thresholds = default_thresholds;
				equator_distance_factor = 0.0f;
				equator_distance_power = 10;
				wrap_horizontally = false;
				update_noise = true;
				break;
			case 1:
				seed = 0;
				scale = 1.0f;
				shift = { -150, -5200 };
				current_options = {};
				current_options.amplitude = 0.5f;
				current_options.octaves = 5;
				use_continents = true;
				continent_frequency = 0.25f;
				continent_amplitude = 1.0f;
				sealevel = 0.25f;
				thresholds.ocean = -0.20f;
				thresholds.coast = -0.02f;
				thresholds.beach = 0.0f;
				thresholds.prairie = 0.02f;
				thresholds.forrest = 0.25f;
				thresholds.mountain = 0.40f;
				thresholds.snowcap = 0.55f;
				equator_distance_factor = 0.0f;
				wrap_horizontally = true;
				update_noise = true;
				break;
			case 2:
				seed = 0;
				scale = 0.66f;
				shift = { -160, -4650 };
				current_options = {};
				current_options.amplitude = 0.5f;
				current_options.octaves = 5;
				use_continents = true;
				continent_frequency = 0.25f;
				continent_amplitude = 1.0f;
				sealevel = 0.25f;
				thresholds.ocean = -0.20f;
				thresholds.coast = -0.02f;
				thresholds.beach = 0.0f;
				thresholds.prairie = 0.02f;
				thresholds.forrest = 0.25f;
				thresholds.mountain = 0.40f;
				thresholds.snowcap = 0.55f;
				equator_distance_factor = -0.15f;
				equator_distance_power = 15;
				wrap_horizontally = true;
				update_noise = true;
				break;
			default: throw_runtime_exception("invalid preset");

		}

		auto mouse_position = signed2(ui::GetIO().MousePos) - map_position;
		auto coordinates = (float2(mouse_position) / float2(map_resolution) * 2.0f - 1.0f) * float2(180, 90);
		auto elevation = (elevation_map->getValue(mouse_position) * 2.0f - 1.0f - sealevel) * 10000.0f;
		auto temperature = temperature_map->getValue(mouse_position) / 255.0f * 70.0f - 25.0f;
		auto precipitation = precipitation_map->getValue(mouse_position) / 255.0f * 100.0f;
		if (map_hovered) ui::Text(u8"Position: %i, %i\nCoordinates: %+4.1f�, %+4.1f�\nElevation: %.1fm\nTemperature: %.1f�\nPrecipitation: %.1f%%",
								  mouse_position.x, mouse_position.y, coordinates.x, coordinates.y, elevation, temperature, precipitation);
		else ui::Text("Use the mouse pointer to view map details.");
		unsigned pixels = map_texture->getWidth() * map_texture->getHeight();
		float water_percentage = 100.0f * water_pixels / pixels;
		ui::Text("%.1f%% Water, %.1f%% Land", water_percentage, 100.0f - water_percentage);

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
		update_noise |= ui::Checkbox("Landmass", landmass); ui::Tooltip("display elevation above sea level");
		ui::SameLine();
		update_noise |= ui::Checkbox("Wrap Horizontally", wrap_horizontally);
		ui::SameLine();
		update_noise |= ui::Checkbox("Continents##use", use_continents);
		if (use_continents) {
			update_noise |= ui::SliderFloat("Continent Amplitude", continent_amplitude, 0.0f, 10.0f, "%.2f", 1.0f, 1.0f);
			update_noise |= ui::SliderFloat("Continent Frequency", continent_frequency, 0.0f, 2.0f, "%.2f", 1.0f, 0.5f);
		}

		static float waterlevel = 0.0;
		update_world |= ui::SliderPercentage("Sealevel", sealevel, -1.0f, 1.0f, "%+.0f%%", 1.0f, 0.0f);
		update_world |= ui::SliderFloat("Equator Distance Factor", equator_distance_factor, -1.0f, 1.0f, "%.2f", 1.0f, 0.0f);
		update_world |= ui::SliderInt("Equator Distance Power", equator_distance_power, 1, 15, "%.0f", 10);
		ui::Text("Thresholds:");
		if (thresholds != default_thresholds) {
			ui::SameLine();
			if (ui::SmallButton("Reset##thresholds")) {
				thresholds = default_thresholds;
				update_world = true;
			}
		}
		update_world |= ui::SliderFloat("Snowcap", thresholds.snowcap, thresholds.mountain, 1.0f, "%.2f");
		update_world |= ui::SliderFloat("Mountain", thresholds.mountain, thresholds.forrest, thresholds.snowcap, "%.2f");
		update_world |= ui::SliderFloat("Forrest", thresholds.forrest, thresholds.prairie, thresholds.mountain, "%.2f");
		update_world |= ui::SliderFloat("Prairie", thresholds.prairie, thresholds.beach, thresholds.forrest, "%.2f");
		update_world |= ui::SliderFloat("Beach", thresholds.beach, thresholds.coast, thresholds.prairie, "%.2f");
		update_world |= ui::SliderFloat("Coast", thresholds.coast, thresholds.ocean, thresholds.beach, "%.2f");
		update_world |= ui::SliderFloat("Ocean", thresholds.ocean, -1.0f, thresholds.coast, "%.2f");
		ui::Text("elevation range [%.5f, %.5f] (%s [-1, +1])", elevation_minimum, elevation_maximum, (elevation_minimum >= -1.0f and elevation_maximum <= 1.0f) ? "lies within" : "exceeds");
		//auto mouse_position = signed2(ui::GetIO().MousePos) - map_position;
		//ui::Text("mouse position (%i, %i)", mouse_position.x, mouse_position.y);
		ui::EndChild();
	}

}
