#include "Generator.h"

#include <cinder/interface/Imgui.h>
#include <cinder/utilities/Simplex.h>

#include <sethex/Common.h>
#include <sethex/Graphics.h>

#include <utilities/Mathematics.h>

using namespace cinder;
using namespace constf;
using namespace std;

namespace sethex {

	shared<Channel32f> elevation_buffer;
	shared<Texture> elevation_texture;
	shared<Texture> terrain_texture;

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
		static bool update_noise = true, update_postprocessing = true;
		static unsigned water_pixels;
		static float elevation_minimum, elevation_maximum;

		static BiomeThresholds thresholds, default_thresholds;

		ui::ScopedWindow ui_window("Noise Texture", ImGuiWindowFlags_HorizontalScrollbar);

		auto assign_elevation = [](const Surface::Iter& iterator, float elevation) {
			if (elevation < elevation_minimum) elevation_minimum = elevation;
			if (elevation > elevation_maximum) elevation_maximum = elevation;
			if (elevation > 0.33f * sealevel + thresholds.snowcap) { assign(iterator, { 255, 255, 255 }); return; } // snowcap
			if (elevation > 0.5f * sealevel + thresholds.mountain) { assign(iterator, { 128, 128, 128 }); return; } // mountain
			if (elevation > 0.66f * sealevel + thresholds.forrest) { assign(iterator, { 0, 160, 0 }); return; } // forrest
			if (elevation > sealevel + thresholds.prairie) { assign(iterator, { 32, 204, 32 }); return; } // prairie
			if (elevation > sealevel + thresholds.beach) { assign(iterator, { 240, 240, 160 }); return; } // beach
			water_pixels++;
			if (elevation > sealevel + thresholds.coast) { assign(iterator, { 16, 16, 160 }); return; } // coast
			if (elevation > sealevel + thresholds.ocean) { assign(iterator, { 0, 0, 128 }); return; } // ocean
			assign(iterator, { 0, 0, 96 }); // deep ocean
		};

		if (update_noise || update_postprocessing) {
			auto channel = Channel32f::create(800, 450);
			auto surface = Surface::create(channel->getWidth(), channel->getHeight(), false, SurfaceChannelOrder::RGB);
			float2 size = channel->getSize();
			float2 center = size / 2.0f;
			if (update_noise) {
				elevation_minimum = elevation_maximum = zero;
				if (seed < 0 || seed > seed_maximum) seed = seed_maximum;
				Simplex::seed(seed);
				auto channel_iterator = channel->getIter();
				scale = clamp(scale * (1.0f - roll), 0.1f, 10.0f);
				shift -= float2(drag) / scale;
				while (channel_iterator.line()) {
					while (channel_iterator.pixel()) {
						signed2 pixel = channel_iterator.getPos();
						if (drag != zero) {
							bool x_copyable = drag.x == 0 or drag.x > 0 and pixel.x >= drag.x or drag.x < 0 and pixel.x < size.x + drag.x;
							bool y_copyable = drag.y == 0 or drag.y > 0 and pixel.y >= drag.y or drag.y < 0 and pixel.y < size.y + drag.y;
							if (x_copyable and y_copyable) {
								channel_iterator.v() = elevation_buffer->getValue(pixel - drag);
								continue;
							}
						}
						float elevation;
						float2 position = pixel;
						position = (position - center) / scale + center;
						position += shift;
						if (wrap_horizontally) {
							float3 cylindrical_position;
							float repeat_interval = size.x / scale;
							position.x = position.x + (repeat_interval - size.x) / Tau_Half;
							float radians = position.x / repeat_interval * Tau;
							cylindrical_position.x = sin(radians) / Tau * repeat_interval;
							cylindrical_position.y = position.y;
							cylindrical_position.z = cos(radians) / Tau * repeat_interval;
							elevation = calculate_elevation(cylindrical_position, current_options, use_continents, continent_frequency, continent_amplitude);
						} else {
							elevation = calculate_elevation(position, current_options, use_continents, continent_frequency, continent_amplitude);
						}
						channel_iterator.v() = elevation;
					}
				}
				elevation_buffer = Channel32f::create(*channel);
				update_noise = false;
				update_postprocessing = true;
			}
			if (update_postprocessing) {
				auto elevation_iterator = elevation_buffer->getIter();
				auto channel_iterator = channel->getIter();
				auto surface_iterator = surface->getIter();
				water_pixels = 0;
				while (surface_iterator.line()) {
					runtime_assert(elevation_iterator.line() and channel_iterator.line());
					float normalized_y = channel_iterator.y() / size.y;
					float distance_to_equator = abs(2.0f * (normalized_y - 0.5f));
					float elevation_change = pow(distance_to_equator, equator_distance_power) * equator_distance_factor;
					while (surface_iterator.pixel()) {
						runtime_assert(elevation_iterator.pixel() and channel_iterator.pixel());
						float elevation = elevation_iterator.v();
						elevation = clamp(elevation + elevation_change, current_options.positive ? 0.0f : -1.0f, 1.0f);
						if (landmass) {
							channel_iterator.v() = max(elevation - sealevel, 0.0f) / (1.0f - sealevel);
						} else {
							channel_iterator.v() = current_options.positive ? elevation : (elevation + 1.0f) / 2.0f;
						}
						assign_elevation(surface_iterator, elevation);
					}
				}
				update_postprocessing = false;
			}
			if (elevation_texture) elevation_texture->update(*channel);
			else elevation_texture = Texture::create(*channel);
			if (terrain_texture) terrain_texture->update(*surface);
			else terrain_texture = Texture::create(*surface);
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
			continent_amplitude = 1.0f;
			sealevel = 0.0f;
			thresholds = default_thresholds;
			equator_distance_factor = 0.0f;
			equator_distance_power = 10;
			wrap_horizontally = false;
			update_noise = true;
		}
		ui::SameLine();
		if (ui::Button("Continents")) {
			current_options = {};
			current_options.amplitude = 0.5f;
			current_options.octaves = 5;
			use_continents = true;
			continent_frequency = 0.25f;
			continent_amplitude = 1.0f;
			sealevel = 0.25f;
			update_noise = true;
		}
		ui::SameLine();
		if (ui::Button("Islands")) {
			current_options = {};
			current_options.octaves = 5;
			use_continents = true;
			continent_frequency = 0.5f;
			continent_amplitude = 1.0f;
			sealevel = 0.25f;
			update_noise = true;
		}
		ui::SameLine();
		if (ui::Button("Alpha World")) {
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
		}
		ui::SameLine();
		if (ui::Button("Beta World")) {
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
		update_noise |= ui::Checkbox("Landmass", landmass); ui::Tooltip("display elevation above sea level");
		ui::SameLine();
		update_noise |= ui::Checkbox("Wrap Horizontally", wrap_horizontally);
		ui::SameLine();
		update_noise |= ui::Checkbox("Continents##use", use_continents);
		if (use_continents) {
			update_noise |= ui::SliderFloat("Continent Amplitude", continent_amplitude, 0.0f, 10.0f, "%.2f", 1.0f, 1.0f);
			update_noise |= ui::SliderFloat("Continent Frequency", continent_frequency, 0.0f, 2.0f, "%.2f", 1.0f, 0.5f);
		}
		ui::EndChild();

		ui::BeginChild("world map", region_size);
		ui::ImageButton(terrain_texture, terrain_texture->getSize(), 0);
		signed2 map_position = ui::GetItemRectMin();
		if (ui::IsItemHovered()) {
			roll = ui::GetIO().MouseWheel * 0.1f;
			drag = ui::IsMouseDragging() ? signed2(ui::GetIO().MouseDelta) : zero;
			update_noise = drag != zero or roll != zero;
		}
		ui::EndChild();

		ui::SameLine();

		ui::BeginChild("world map properties", region_size);
		unsigned pixels = terrain_texture->getWidth() * terrain_texture->getHeight();
		float water_percentage = 100.0f * water_pixels / pixels;
		ui::Text("%.1f%% Water, %.1f%% Land", water_percentage, 100.0f - water_percentage);
		static float waterlevel = 0.0;
		update_postprocessing |= ui::SliderPercentage("Sealevel", sealevel, -1.0f, 1.0f, "%+.0f%%", 1.0f, 0.0f);
		update_postprocessing |= ui::SliderFloat("Equator Distance Factor", equator_distance_factor, -1.0f, 1.0f, "%.2f", 1.0f, 0.0f);
		update_postprocessing |= ui::SliderInt("Equator Distance Power", equator_distance_power, 1, 15, "%.0f", 10);
		ui::Text("Thresholds:");
		if (thresholds != default_thresholds) {
			ui::SameLine();
			if (ui::SmallButton("Reset##thresholds")) {
				thresholds = default_thresholds;
				update_postprocessing = true;
			}
		}
		update_postprocessing |= ui::SliderFloat("Snowcap", thresholds.snowcap, thresholds.mountain, 1.0f, "%.2f");
		update_postprocessing |= ui::SliderFloat("Mountain", thresholds.mountain, thresholds.forrest, thresholds.snowcap, "%.2f");
		update_postprocessing |= ui::SliderFloat("Forrest", thresholds.forrest, thresholds.prairie, thresholds.mountain, "%.2f");
		update_postprocessing |= ui::SliderFloat("Prairie", thresholds.prairie, thresholds.beach, thresholds.forrest, "%.2f");
		update_postprocessing |= ui::SliderFloat("Beach", thresholds.beach, thresholds.coast, thresholds.prairie, "%.2f");
		update_postprocessing |= ui::SliderFloat("Coast", thresholds.coast, thresholds.ocean, thresholds.beach, "%.2f");
		update_postprocessing |= ui::SliderFloat("Ocean", thresholds.ocean, -1.0f, thresholds.coast, "%.2f");
		ui::Text("elevation range [%.5f, %.5f] (%s [-1, +1])", elevation_minimum, elevation_maximum, (elevation_minimum >= -1.0f and elevation_maximum <= 1.0f) ? "lies within" : "exceeds");
		auto mouse_position = signed2(ui::GetIO().MousePos) - map_position;
		ui::Text("mouse position (%i, %i)", mouse_position.x, mouse_position.y);
		ui::EndChild();
	}

}
