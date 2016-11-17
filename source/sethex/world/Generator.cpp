#include "Generator.h"

#include <cinder/interface/Imgui.h>
#include <cinder/utilities/Assets.h>
#include <cinder/utilities/Shaders.h>
#include <cinder/utilities/Simplex.h>
#include <cinder/utilities/Watchdog.h>

#include <utilities/Exceptions.h>
#include <utilities/Mathematics.h>

using namespace cinder;
using namespace tenjix::float_constants;
using namespace std;

namespace tenjix {

	namespace sethex {

		shared<Channel32f> elevation_buffer;
		shared<Channel32f> elevation_map;
		shared<Channel> temperature_map;
		shared<Channel> evapotranspiration_map;
		shared<Surface> circulation_map;
		shared<Channel> humidity_map;
		shared<Channel> precipitation_map;
		shared<Surface> terrain_map;
		shared<Surface> biome_map;

		shared<Texture> map_texture;
		shared<Texture> world_texture;

		class Frame {

			fs::path vertex_shader_path = "shaders/Square.vertex.shader";
			fs::path geometry_shader_path = "shaders/Square.geometry.shader";
			fs::path fragment_shader_path;

			bool flip_origin = false;
			bool compiled = false;
			unsigned current = 0;

			shared<FrameBuffer> buffers[2];
			shared<Shader> shader;
			shared<Texture> result;

		public:

			Frame& framebuffer(unsigned2 size, GLint format = GL_RGBA32F, int samples = 0) {
				buffers[0] = FrameBuffer::create(size.x, size.y, FrameBuffer::Format().samples(samples).disableDepth().colorTexture(Texture::Format().internalFormat(format)));
				return *this;
			}

			Frame& dual_framebuffer(unsigned2 size, GLint format = GL_RGBA32F, int samples = 0) {
				if (not buffers[0]) framebuffer(size, format);
				buffers[1] = FrameBuffer::create(size.x, size.y, FrameBuffer::Format().samples(samples).disableDepth().colorTexture(Texture::Format().internalFormat(format)));
				return *this;
			}

			bool initialized() { return buffers[0] != nullptr; }

			unsigned active() { return current; }

			unsigned inactive() { return 1 - current; }

			Frame& activate(unsigned index) {
				assert(index == 0 or index == 1);
				current = index;
			}

			Frame& swap() { current = 1 - current; return *this; }

			FrameBuffer& buffer() { return *buffers[current]; }

			FrameBuffer& buffer(unsigned index) {
				assert(index == 0 or index == 1);
				return *buffers[index];
			}

			Frame& set(shared<Texture> texture) {
				result = texture;
				return *this;
			}

			shared<Texture> texture() {
				runtime_assert(buffers[current], "buffers[", current, "] isn't initialized");
				return result ? result : buffers[current]->getColorTexture();
			}

			shared<Texture> texture(unsigned index) {
				assert(index == 0 or index == 1);
				return buffers[index]->getColorTexture();
			}

			Frame& flip_horizontally() {
				flip_origin = true;
				return *this;
			}

			Frame& fragment(const String& fragment_shader_path, bool& update) {
				//wd::unwatch(this->fragment_shader_path);
				this->fragment_shader_path = fragment_shader_path;
				wd::watch(fragment_shader_path, [&](const fs::path& path) {
					if (compile()) update = true;
				});
				return *this;
			}

			template<class Type>
			void uniform(const String& name, Type value) {
				if (not compiled) return;
				shader->uniform(name, value);
			}

			bool compile() {
				try {
					debug("compiling ", fragment_shader_path.filename(), " ...");
					String vertex_shader = loadString(app::loadAsset(vertex_shader_path));
					String geometry_shader = loadString(app::loadAsset(geometry_shader_path));
					if (flip_origin) shader::define(geometry_shader, "ORIGIN_UPPER_LEFT");
					String fragment_shader = loadString(app::loadAsset(fragment_shader_path));
					if (flip_origin) shader::define(fragment_shader, "ORIGIN_UPPER_LEFT");
					shader = Shader::create(vertex_shader, fragment_shader, geometry_shader);
					compiled = true;
				} catch (gl::GlslProgCompileExc exception) {
					error(exception.what());
					compiled = false;
				}
				return compiled;
			}

			void render(std::initializer_list<shared<Texture>> textures = {}) {
				if (not compiled) return;
				debug("rendering ", fragment_shader_path.filename(), " ...");
				auto buffer = buffers[current];
				assert(buffer);
				assert(shader);
				using namespace gl;
				ScopedFramebuffer scoped_framebuffer(buffer);
				ScopedViewport scoped_viewport(buffer->getSize());
				ScopedGlslProg scoped_shader(shader);
				uint8 unit = 0;
				for (auto& texture : textures) {
					assert(texture);
					texture->bind(unit++);
				}
				drawArrays(GL_POINTS, 0, 1);
				unit = 0;
				for (auto& texture : textures) {
					texture->unbind(unit++);
				}
			}

		};

		Frame elevation_frame;
		Frame temperature_frame;
		Frame evapotranspiration_frame;
		Frame circulation_frame;
		Frame humidity_frame;
		Frame precipitation_frame;
		Frame biome_frame;

		float lower_threshold = One_Third;
		float upper_threshold = Two_Thirds;

		struct TerrainThresholds {
			float ocean = -0.5f;
			float coast = -0.1f;
			float beach = 0.0f;
			float prairie = 0.05f;
			float forrest = 0.33f;
			float mountain = 0.5f;
			float snowcap = 0.66f;

			bool operator==(const TerrainThresholds& other) {
				return ocean == other.ocean
					and coast == other.coast
					and beach == other.beach
					and prairie == other.prairie
					and forrest == other.forrest
					and mountain == other.mountain
					and snowcap == other.snowcap;
			}
			bool operator!=(const TerrainThresholds& other) {
				return not operator==(other);
			}
		};

		struct BiomeColors {
			Color8u ice = { 255, 255, 255 };
			Color8u tundra = { 128, 128, 64 };
			Color8u taiga = { 0, 160, 80 };
			Color8u steppe = { 192, 192, 96 };
			Color8u prairie = { 0, 160, 0 };
			Color8u forest = { 0, 96, 0 };
			Color8u desert = { 255, 255, 128 };
			Color8u savanna = { 192, 240, 64 };
			Color8u rainforest = { 0, 64, 0 };
			Color8u hot_rock = { 160, 128, 128 };
			Color8u rock = { 128, 128, 128 };
			Color8u cold_rock = { 128, 128, 160 };
			Color8u coast = { 16, 16, 160 };
			Color8u ocean = { 0, 0, 128 };
			Color8u abyssal = { 0, 0, 92 };
		};

		BiomeColors biome_colors;

		template <class Type>
		float calculate_elevation(const Type& position, const Simplex::Options& options, bool continents, float continent_frequency, float continent_amplitude) {
			float elevation = Simplex::noise(position * 0.01f, options);
			if (continents) {
				float continental_elevation = Simplex::noise(position * 0.01f * continent_frequency);
				elevation = (elevation + continent_amplitude * continental_elevation) / (options.amplitude + continent_amplitude);
			}
			return elevation;
		}

		Color8u determine_biome(float temperature, float precipitation) {
			if (temperature < lower_threshold) {
				// polar & dry -> ice
				if (precipitation < lower_threshold) return biome_colors.ice;
				// polar & nor -> tundra
				if (precipitation < upper_threshold) return biome_colors.tundra;
				// polar & wet -> taiga
				return biome_colors.taiga;
			}
			if (temperature < upper_threshold) {
				// temperate & dry -> steppe
				if (precipitation < lower_threshold) return biome_colors.steppe;
				// temperate & nor -> prairie
				if (precipitation < upper_threshold) return biome_colors.prairie;
				// temperate & wet -> forest
				return biome_colors.forest;
			}
			// tropical & dry -> desert
			if (precipitation < lower_threshold) return biome_colors.desert;
			// tropical & nor -> savanna
			if (precipitation < upper_threshold) return biome_colors.savanna;
			// tropical & wet -> rainforest
			return biome_colors.rainforest;
		}

		String get_biome_name(Color8u biome_color) {
			if (biome_color == biome_colors.desert) return "Desert";
			if (biome_color == biome_colors.forest) return "Forest";
			if (biome_color == biome_colors.ice) return "Ice";
			if (biome_color == biome_colors.prairie) return "Prairie";
			if (biome_color == biome_colors.rainforest) return "Rainforest";
			if (biome_color == biome_colors.rock) return "Rock";
			if (biome_color == biome_colors.savanna) return "Savanna";
			if (biome_color == biome_colors.steppe) return "Steppe";
			if (biome_color == biome_colors.taiga) return "Taiga";
			if (biome_color == biome_colors.tundra) return "Tundra";
			if (biome_color == biome_colors.coast) return "Coast";
			if (biome_color == biome_colors.ocean) return "Ocean";
			if (biome_color == biome_colors.abyssal) return "Abyssal";
			return "Unknown";
		}

		void Generator::display() {
			static int seed = 0, seed_maximum = 1000000000;
			static float2 shift;
			static signed2 drag;
			static float scale = 1.0f, roll = 0.0f;
			static Simplex::Options current_options;
			static Simplex::Options saved_options;
			static float continent_frequency = 0.5f, continent_amplitude = 1.0f, sealevel = 0.0f, equator = 0.0f;
			static float equator_distance_factor = 0.0f;
			static int equator_distance_power = 10, lapse_power = 1;
			static bool wrap_horizontally = false;
			static bool use_continents = false;
			static bool circulation = false;
			static bool update_tectonic = true, update_topography, update_climate, update_display, update_biomes;
			static unsigned water_pixels;
			static float elevation_minimum, elevation_maximum, lapse_rate = 10.0f;
			static float evaporation_factor = 1.0f, transpiration_factor = 0.25f, precipitation_factor = 1.0f, precipitation_decay = 0.05f, slope_scale = 0.01f;
			static float humidity_saturation = 25.0f;
			static bool upper_precipitation = true;
			static bool gpu_compute = false, earth = false, debug = false;
			static unsigned circulation_iterations = 25;
			static float circulation_intensity = 1.0, orograpic_effect = 1.0;
			static float bathymetry_scale = 1.0, topography_scale = 1.0;

			const unsigned2 map_resolution = { 800, 450 };

			if (gpu_compute and not temperature_frame.initialized()) {
				temperature_frame.framebuffer(map_resolution, GL_R32F).fragment("shaders/generation/Temperature.fragment.shader", update_topography);
				evapotranspiration_frame.framebuffer(map_resolution, GL_R32F).fragment("shaders/generation/Evapotranspiration.fragment.shader", update_climate);
				circulation_frame.framebuffer(map_resolution, GL_RGB32F).fragment("shaders/generation/Circulation.fragment.shader", update_climate);
				humidity_frame.dual_framebuffer(map_resolution, GL_R32F).fragment("shaders/generation/Humidity.fragment.shader", update_climate);
				precipitation_frame.framebuffer(map_resolution, GL_R32F, 16).fragment("shaders/generation/Precipitation.fragment.shader", update_climate);
			}

			static TerrainThresholds thresholds, default_thresholds;

			ui::ScopedWindow ui_window("Noise Texture", ImGuiWindowFlags_HorizontalScrollbar);

			auto assign_elevation = [](const Surface::Iter& iterator, float elevation) {
				if (elevation < elevation_minimum) elevation_minimum = elevation;
				if (elevation > elevation_maximum) elevation_maximum = elevation;
				if (elevation > 0.33f * sealevel + thresholds.snowcap) { iterator << biome_colors.ice; return; } // snowcap
				if (elevation > 0.5f * sealevel + thresholds.mountain) { iterator << biome_colors.rock; return; } // mountain
				if (elevation > 0.66f * sealevel + thresholds.forrest) { iterator << biome_colors.forest; return; } // forrest
				if (elevation > sealevel + thresholds.prairie) { iterator << biome_colors.prairie; return; } // prairie
				if (elevation > sealevel + thresholds.beach) { iterator << biome_colors.desert; return; } // beach
				water_pixels++;
				if (elevation > sealevel + thresholds.coast) { iterator << biome_colors.coast; return; } // coast
				if (elevation > sealevel + thresholds.ocean) { iterator << biome_colors.ocean; return; } // ocean
				iterator << biome_colors.abyssal; // deep ocean
			};

			if (update_tectonic or update_topography or update_climate or update_biomes) {
				print("===== update =====");

				if (update_tectonic) {
					print("update tectonic");
					scale = clamp(scale * (1.0f - roll), 0.1f, 10.0f);
					shift -= wrap_horizontally ? float2(drag.x, drag.y / scale) : float2(drag) / scale;
					if (earth) {
						if (not elevation_frame.initialized()) elevation_frame.framebuffer(map_resolution, GL_R32F);
						elevation_frame.fragment("shaders/generation/Elevation-Composing.fragment.shader", update_tectonic);
						auto bathymetry_map = loadImage(app::loadAsset("maps/Bathymetry.png"));
						auto topography_map = loadImage(app::loadAsset("maps/Topography.png"));
						auto bathymetry_texture = Texture::create(bathymetry_map);
						auto topography_texture = Texture::create(topography_map);
						elevation_frame.uniform("uBathymetryMap", 0);
						elevation_frame.uniform("uTopographyMap", 1);
						elevation_frame.uniform("uBathymetryScale", bathymetry_scale);
						elevation_frame.uniform("uTopographyScale", topography_scale);
						elevation_frame.render({ bathymetry_texture, topography_texture });
						elevation_map = Channel32f::create(elevation_frame.texture()->createSource());
						if (not elevation_buffer) elevation_buffer = Channel32f::create(*elevation_map);
						else elevation_buffer->copyFrom(*elevation_map, elevation_map->getBounds());
					} else if (gpu_compute) {
						if (not elevation_frame.initialized()) elevation_frame.framebuffer(map_resolution, GL_R32F);
						elevation_frame.fragment("shaders/generation/Elevation.fragment.shader", update_tectonic).flip_horizontally();
						elevation_buffer = nullptr;
						elevation_frame.uniform("uWrapping", wrap_horizontally);
						elevation_frame.uniform("uResolution", map_resolution);
						//elevation_frame.shader->uniform("uSeed", seed);
						elevation_frame.uniform("uShift", shift);
						elevation_frame.uniform("uScale", scale);
						elevation_frame.uniform("uOctaces", current_options.octaves);
						elevation_frame.uniform("uAmplitude", current_options.amplitude);
						elevation_frame.uniform("uFrequency", current_options.frequency);
						elevation_frame.uniform("uLacunarity", current_options.lacunarity);
						elevation_frame.uniform("uPersistence", current_options.persistence);
						elevation_frame.uniform("uPower", current_options.power);
						elevation_frame.uniform("uContinentalAmplitudeFactor", continent_amplitude);
						elevation_frame.uniform("uContinentalFrequencyFactor", continent_frequency);
						//elevation_frame.uniform("uContinentalShift", seed);
						elevation_frame.uniform("uEquatorDistanceFactor", equator_distance_factor);
						elevation_frame.uniform("uEquatorDistancePower", equator_distance_power);
						elevation_frame.render();
						elevation_map = Channel32f::create(elevation_frame.texture()->createSource());
					} else {
						elevation_frame.set(nullptr);
						if (not elevation_map) elevation_map = Channel32f::create(map_resolution.x, map_resolution.y);
						elevation_minimum = elevation_maximum = Zero;
						if (seed < 0 || seed > seed_maximum) seed = seed_maximum;
						Simplex::seed(seed);
						auto elevation_iterator = elevation_map->getIter();
						signed2 resolution = map_resolution;
						float2 center = float2(map_resolution) / 2.0f;
						while (elevation_iterator.line()) {
							while (elevation_iterator.pixel()) {
								signed2 pixel = elevation_iterator.getPos();
								if (drag != Zero and elevation_buffer) {
									bool x_copyable = drag.x == 0 or drag.x > 0 and pixel.x >= drag.x or drag.x < 0 and pixel.x < resolution.x + drag.x;
									bool y_copyable = drag.y == 0 or drag.y > 0 and pixel.y >= drag.y or drag.y < 0 and pixel.y < resolution.y + drag.y;
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
						if (not elevation_buffer) elevation_buffer = Channel32f::create(*elevation_map);
						else elevation_buffer->copyFrom(*elevation_map, elevation_map->getBounds());
					}
					update_tectonic = false;
					update_topography = true;
				}

				if (update_topography) {
					print("update topography");
					if (gpu_compute) {
						temperature_frame.uniform("uElevationMap", 0);
						temperature_frame.uniform("uSeaLevel", sealevel);
						temperature_frame.uniform("uEquator", equator);
						temperature_frame.uniform("uLapseRate", lapse_rate);
						temperature_frame.render({ elevation_frame.texture() });
						temperature_map = Channel::create(temperature_frame.texture()->createSource());
					} else {
						if (not temperature_map) temperature_map = Channel::create(map_resolution.x, map_resolution.y);
						auto buffer_iterator = elevation_buffer->getIter();
						auto elevation_iterator = elevation_map->getIter();
						auto temperature_iterator = temperature_map->getIter();
						while (buffer_iterator.line() and elevation_iterator.line() and temperature_iterator.line()) {
							float y = static_cast<float>(elevation_iterator.y()) / map_resolution.y; // y position [0.0, 1.0] 
							float distance_to_equator = abs(2.0f * (y - 0.5f)); // distance to equator [0.0, 1.0] 
							float elevation_change = pow(distance_to_equator, equator_distance_power) * equator_distance_factor;
							while (buffer_iterator.pixel() and elevation_iterator.pixel() and temperature_iterator.pixel()) {
								float elevation = buffer_iterator.v();
								elevation = clamp(elevation + elevation_change, -1.0f, 1.0f);
								float elevation_above_sealevel = max(elevation - sealevel, 0.0f) / (1.0f - sealevel);
								elevation_iterator.v() = elevation;
								float2 position = elevation_iterator.getPos();
								float temperature = 1.0f - distance_to_equator * distance_to_equator;
								//float temperature_noise = Simplex::to_unsigned(Simplex::noise(position.x * 0.02f * continent_frequency)); 
								float temperature_noise = Simplex::to_unsigned(Simplex::noise(position * 0.02f * continent_frequency));
								temperature = mix(temperature, temperature_noise, 0.1f);
								if (elevation > sealevel) {
									if (lapse_power == 1) {
										float elevation_above_sealevel_in_km = (elevation - sealevel) * 10.0f;
										temperature -= lapse_rate * elevation_above_sealevel_in_km / 70.0f;
									} else {
										temperature -= elevation_above_sealevel * elevation_above_sealevel * lapse_rate / 20.0f;
									}
								} else temperature -= 0.1f;
								temperature = clamp(temperature, 0.0f, 1.0f);
								temperature_iterator.v() = static_cast<uint8>(temperature * 255.0f + 0.5f);
							}
						}
					}
					update_topography = false;
					update_climate = true;
				}

				if (update_climate) {
					print("update climate");
					if (gpu_compute and circulation) {
						// calculate circulation
						circulation_frame.uniform("uElevationMap", 0);
						circulation_frame.uniform("uTemperatureMap", 1);
						circulation_frame.uniform("uEquator", equator);
						circulation_frame.uniform("uDebug", debug);
						circulation_frame.render({ elevation_frame.texture(), temperature_frame.texture() });
						circulation_map = Surface::create(circulation_frame.texture()->createSource());
						// calculate evapotranspiration
						evapotranspiration_frame.uniform("uElevationMap", 0);
						evapotranspiration_frame.uniform("uTemperatureMap", 1);
						evapotranspiration_frame.uniform("uCirculationMap", 2);
						evapotranspiration_frame.uniform("uSeaLevel", sealevel);
						evapotranspiration_frame.uniform("uEquator", equator);
						evapotranspiration_frame.uniform("uEvaporation", evaporation_factor);
						evapotranspiration_frame.uniform("uTranspiration", transpiration_factor);
						evapotranspiration_frame.render({ elevation_frame.texture(), temperature_frame.texture(), circulation_frame.texture() });
						evapotranspiration_map = Channel::create(evapotranspiration_frame.texture()->createSource());
						// calculate humidity by simulating calculation
						humidity_frame.uniform("uElevationMap", 0);
						humidity_frame.uniform("uTemperatureMap", 1);
						humidity_frame.uniform("uCirculationMap", 2);
						humidity_frame.uniform("uHumidityMap", 3);
						humidity_frame.uniform("uSeaLevel", sealevel);
						humidity_frame.uniform("uIntensity", circulation_intensity);
						shared<Texture> humidity_texture = evapotranspiration_frame.texture();
						for (unsigned iteration = 1; iteration <= circulation_iterations; iteration++) {
							humidity_frame.uniform("uIteration", iteration);
							humidity_frame.swap().render({ elevation_frame.texture(), temperature_frame.texture(), circulation_frame.texture(), humidity_texture });
							humidity_texture = humidity_frame.texture();
						}
						humidity_map = Channel::create(humidity_frame.texture()->createSource());
						// calculate precipitation
						precipitation_frame.uniform("uElevationMap", 0);
						precipitation_frame.uniform("uTemperatureMap", 1);
						precipitation_frame.uniform("uCirculationMap", 2);
						precipitation_frame.uniform("uHumidityMap", 3);
						precipitation_frame.uniform("uResolution", map_resolution);
						precipitation_frame.uniform("uSeaLevel", sealevel);
						precipitation_frame.uniform("uOrograpicEffect", orograpic_effect);
						precipitation_frame.render({ elevation_frame.texture(), temperature_frame.texture(), circulation_frame.texture(), humidity_texture });
						precipitation_map = Channel::create(precipitation_frame.texture()->createSource());
					} else {
						if (not precipitation_map) precipitation_map = Channel::create(map_resolution.x, map_resolution.y);
						vector<vector<float>> humidity_map { 6, vector<float>(map_resolution.x) };
						unsigned map_height_sixth = map_resolution.y / 6;

						auto calculate_evapotranspiration_precipitation = [&](unsigned belt, unsigned slot, unsigned& x, unsigned y, int x_delta, float& previous_elevation) {
							float elevation = *elevation_map->getData(x, y);
							float elevation_above_sealevel = max(elevation - sealevel, 0.0f) / (1.0f - sealevel);
							float temperature = *temperature_map->getData(x, y) / 255.0f;
							float evapotranspiration = 0.0f;
							float precipitation = 0.0f;
							float humidity = humidity_map[belt][slot];
							float slope = 0.0f;
							bool land = elevation > sealevel;
							evapotranspiration = temperature * (land ? transpiration_factor : evaporation_factor);
							evapotranspiration = min(evapotranspiration, humidity_saturation * temperature - humidity);
							humidity += evapotranspiration;
							if (land) {
								if (y > 0) {
									if (isnan(previous_elevation)) {
										previous_elevation = *elevation_map->getData(x - x_delta, y - 1);
									}
									slope = (elevation - previous_elevation) / slope_scale;
								}
								//precipitation = clamp(slope * elevation_above_sealevel * elevation_above_sealevel + 0.1f, 0.0f, 1.0f) * precipitation_factor;
								precipitation = clamp(slope * elevation_above_sealevel * elevation_above_sealevel + humidity / (humidity_saturation * temperature), 0.0f, 1.0f) * precipitation_factor;
								precipitation = min(precipitation, humidity);
								humidity -= precipitation;
							}
							*precipitation_map->getData(x, y) = static_cast<uint8>(precipitation * 255.0f + 0.5f);
							humidity_map[belt][slot] = humidity;
							previous_elevation = elevation;
							x = project(x + x_delta, 0, map_resolution.x - 1);
						};

						//srand(15);
						auto calculate_precipitation = [&](unsigned belt, unsigned slot, unsigned& x, unsigned y, int x_delta) {
							float elevation = *elevation_map->getData(x, y);
							bool land = elevation > sealevel;
							float humidity = humidity_map[belt][slot];
							float precipitation = humidity * precipitation_decay;
							//precipitation = min(precipitation, humidity);
							precipitation = min(precipitation * precipitation, 1.0f);
							humidity -= precipitation;
							if (land) {
								uint8& mapped_precipitation = *precipitation_map->getData(x, y);
								float existing_precipitation = mapped_precipitation / 255.0f;
								mapped_precipitation = static_cast<uint8>(min(existing_precipitation + precipitation, 1.0f) * 255.0f + 0.5f);
							}
							humidity_map[belt][slot] = humidity;
							//bool drift = (rand() % 100) < 10;
							x = project(x + x_delta, 0, map_resolution.x - 1);
						};

						// low altitude wind
						for (unsigned slot = 0; slot < map_resolution.x; slot++) {
							float previous_elevation = NAN;
							unsigned x = slot;
							for (unsigned y = 0; y < map_height_sixth; y++) {
								calculate_evapotranspiration_precipitation(0, slot, x, y, -1, previous_elevation);
							}
							previous_elevation = NAN;
							x = slot;
							for (unsigned y = 2 * map_height_sixth - 1; y >= map_height_sixth; y--) {
								calculate_evapotranspiration_precipitation(1, slot, x, y, +1, previous_elevation);
							}
							previous_elevation = NAN;
							x = slot;
							for (unsigned y = 2 * map_height_sixth; y < 3 * map_height_sixth; y++) {
								calculate_evapotranspiration_precipitation(2, slot, x, y, -1, previous_elevation);
							}
							previous_elevation = NAN;
							x = slot;
							for (unsigned y = 4 * map_height_sixth - 1; y >= 3 * map_height_sixth; y--) {
								calculate_evapotranspiration_precipitation(3, slot, x, y, -1, previous_elevation);
							}
							previous_elevation = NAN;
							x = slot;
							for (unsigned y = 4 * map_height_sixth; y < 5 * map_height_sixth; y++) {
								calculate_evapotranspiration_precipitation(4, slot, x, y, +1, previous_elevation);
							}
							previous_elevation = NAN;
							x = slot;
							for (unsigned y = 6 * map_height_sixth - 1; y >= 5 * map_height_sixth; y--) {
								calculate_evapotranspiration_precipitation(5, slot, x, y, -1, previous_elevation);
							}
						}
						if (upper_precipitation) {
							// humidity exchange of rising air
							for (unsigned slot = 0; slot < map_resolution.x; slot++) {
								humidity_map[0][slot] = humidity_map[1][slot] = (humidity_map[0][slot] + humidity_map[1][slot]) / 2.0f;
								humidity_map[2][slot] = humidity_map[3][slot] = (humidity_map[2][slot] + humidity_map[3][slot]) / 2.0f;
								humidity_map[4][slot] = humidity_map[5][slot] = (humidity_map[4][slot] + humidity_map[5][slot]) / 2.0f;
							}
							// high altitude wind
							for (unsigned slot = 0; slot < map_resolution.x; slot++) {
								unsigned x = slot;
								for (unsigned y = map_height_sixth; y-- > 0; ) {
									calculate_precipitation(0, slot, x, y, +1);
								}
								x = slot;
								for (unsigned y = map_height_sixth; y < 2 * map_height_sixth; y++) {
									calculate_precipitation(1, slot, x, y, -1);
								}
								x = slot;
								for (unsigned y = 3 * map_height_sixth - 1; y >= 2 * map_height_sixth; y--) {
									calculate_precipitation(2, slot, x, y, +1);
								}
								x = slot;
								for (unsigned y = 3 * map_height_sixth; y < 4 * map_height_sixth; y++) {
									calculate_precipitation(3, slot, x, y, +1);
								}
								x = slot;
								for (unsigned y = 5 * map_height_sixth - 1; y >= 4 * map_height_sixth; y--) {
									calculate_precipitation(4, slot, x, y, -1);
								}
								x = slot;
								for (unsigned y = 5 * map_height_sixth; y < 6 * map_height_sixth; y++) {
									calculate_precipitation(5, slot, x, y, +1);
								}
							}
						}
					}
					update_climate = false;
					update_biomes = true;
				}

				if (update_biomes) {
					print("update biomes");
					if (not biome_map) biome_map = Surface::create(map_resolution.x, map_resolution.y, false, SurfaceChannelOrder::RGB);
					if (not terrain_map) terrain_map = Surface::create(map_resolution.x, map_resolution.y, false, SurfaceChannelOrder::RGB);
					//if (gpu_compute) {
					//	if (not biome_framebuffer) biome_framebuffer = FrameBuffer::create(map_resolution.x, map_resolution.y);
					//	temperature_shader->uniform("uElevationMap", 0);
					//	temperature_shader->uniform("uTemperatureMap", 1);
					//	temperature_shader->uniform("uPrecipitationMap", 2);
					//	temperature_shader->uniform("uResolution", map_resolution);
					//	temperature_shader->uniform("uSeaLevel", sealevel);
					//	{
					//		using namespace gl;
					//		ScopedFramebuffer scoped_framebuffer(biome_framebuffer);
					//		ScopedViewport scoped_viewport(map_resolution);
					//		ScopedGlslProg scoped_shader(temperature_shader);
					//		ScopedTextureBind scoped_texture(elevation_texture);
					//		ScopedTextureBind scoped_texture(temperature_texture);
					//		ScopedTextureBind scoped_texture(precipitation_texture);
					//		drawArrays(GL_POINTS, 0, 1);
					//	}
					//	biome_texture = biome_framebuffer->getColorTexture();
					//} else {
					auto elevation_iterator = elevation_map->getIter();
					auto temperature_iterator = temperature_map->getIter();
					auto precipitation_iterator = precipitation_map->getIter();
					auto biome_iterator = biome_map->getIter();
					auto terrain_iterator = terrain_map->getIter();
					water_pixels = 0;
					while (elevation_iterator.line() and temperature_iterator.line() and precipitation_iterator.line() and biome_iterator.line() and terrain_iterator.line()) {
						while (elevation_iterator.pixel() and temperature_iterator.pixel() and precipitation_iterator.pixel() and biome_iterator.pixel() and terrain_iterator.pixel()) {
							float elevation = elevation_iterator.v();
							assign_elevation(terrain_iterator, elevation);
							if (elevation > sealevel) {
								biome_iterator << determine_biome(temperature_iterator.v() / 255.0f, precipitation_iterator.v() / 255.0f);
							} else {
								biome_iterator << terrain_iterator;
							}
						}
					}
					//}
					update_biomes = false;
					update_display = true;
				}
			}

			ui::BeginChild("map display", float2(map_resolution.x, 0));
			static int selected_map = 0;
			ui::PushItemWidth(180);
			static Lot<String> cpu_map_names { "Biome", "Terrain", "Elevation", "Temperature", "Precipitation" };
			static Lot<String> gpu_map_names { "Biome", "Terrain", "Elevation", "Temperature", "Circulation", "Evapotranspiration", "Humidity", "Precipitation" };
			bool gpu = gpu_compute and circulation;
			Lot<String>& map_names = gpu ? gpu_map_names : cpu_map_names;
			static shared<ImageSource> image_source;
			if (ui::Combo("Map##selection", selected_map, map_names) or update_display) {
				switch (selected_map) {
					case 0:
						image_source = *biome_map;
						break;
					case 1:
						image_source = *terrain_map;
						break;
					case 2:
						image_source = *elevation_map;
						break;
					case 3:
						image_source = *temperature_map;
						break;
					case 4:
						if (gpu) image_source = *circulation_map;
						else image_source = *precipitation_map;
						break;
					case 5:
						image_source = *evapotranspiration_map;
						break;
					case 6:
						image_source = *humidity_map;
						break;
					case 7:
						image_source = *precipitation_map;
						break;
					default: throw_runtime_exception("invalid map");
				}
				map_texture = Texture::create(image_source);
				world_texture = Texture::create(*terrain_map);
				biomes = image_source;
				elevation = *elevation_map;
				update_display = false;

			}
			ui::PopItemWidth();
			ui::SameLine();
			if (ui::Button("Save")) writeImage("exported/" + map_names[selected_map] + "_Map.jpg", image_source);
			ui::SameLine(ui::GetWindowWidth() - 150);
			update_tectonic |= ui::Checkbox("GPU Compute", gpu_compute);
			ui::ImageButton(map_texture, map_texture->getSize(), 0);
			bool map_hovered = ui::IsItemHoveredRect() and ui::IsWindowHovered();
			signed2 map_position = ui::GetItemRectMin();
			if (map_hovered) {
				roll = ui::GetIO().MouseWheel * 0.1f;
				drag = ui::IsMouseDragging() ? signed2(ui::GetIO().MouseDelta) : Zero;
				update_tectonic = drag != Zero or roll != Zero;
			}
			ui::Image(world_texture, world_texture->getSize());
			ui::EndChild();

			ui::SameLine();

			ui::BeginChild("map properties");
			static int selected_preset = 0;
			if (ui::Combo("Preset##selection", selected_preset, { "Default", "Alpha World", "Beta World", "Continents", "Islands" })) switch (selected_preset) {
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
					update_tectonic = true;
					break;
				case 1:
					seed = 0;
					scale = 1.0f;
					shift = { -150, -5200 };
					current_options = {};
					current_options.octaves = 5;
					use_continents = true;
					continent_frequency = 0.25f;
					continent_amplitude = 2.0f;
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
					update_tectonic = true;
					break;
				case 2:
					seed = 0;
					scale = 0.66f;
					shift = { -160, -4650 };
					current_options = {};
					current_options.octaves = 5;
					use_continents = true;
					continent_frequency = 0.25f;
					continent_amplitude = 2.0f;
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
					update_tectonic = true;
					break;
				case 3:
					gpu_compute = true;
					seed = 0;
					scale = 0.6f;
					shift = { 185, -7925 };
					current_options = {};
					current_options.octaves = 5;
					use_continents = true;
					continent_amplitude = 2.0f;
					continent_frequency = 0.25f;
					sealevel = 0.25f;
					thresholds.ocean = -0.20f;
					thresholds.coast = -0.02f;
					thresholds.beach = 0.0f;
					thresholds.prairie = 0.02f;
					thresholds.forrest = 0.25f;
					thresholds.mountain = 0.40f;
					thresholds.snowcap = 0.55f;
					equator_distance_factor = -0.4f;
					equator_distance_power = 15;
					transpiration_factor = 0.5f;
					slope_scale = 1.0f;
					wrap_horizontally = true;
					update_tectonic = true;
					break;
				case 4:
					gpu_compute = true;
					seed = 0;
					scale = 1.0f;
					shift = { 330, -2330 };
					current_options = {};
					current_options.octaves = 5;
					use_continents = true;
					continent_amplitude = 1.0f;
					continent_frequency = 0.5f;
					sealevel = 0.25f;
					thresholds.ocean = -0.20f;
					thresholds.coast = -0.02f;
					thresholds.beach = 0.0f;
					thresholds.prairie = 0.02f;
					thresholds.forrest = 0.25f;
					thresholds.mountain = 0.40f;
					thresholds.snowcap = 0.55f;
					equator_distance_factor = -0.4f;
					equator_distance_power = 15;
					transpiration_factor = 0.5f;
					slope_scale = 1.0f;
					wrap_horizontally = true;
					update_tectonic = true;
					break;
				default: throw_runtime_exception("invalid preset");

			}

			auto mouse_position = signed2(ui::GetIO().MousePos) - map_position;
			auto coordinates = (float2(mouse_position) / float2(map_resolution) * 2.0f - 1.0f) * float2(180, 90);
			auto elevation = (elevation_map->getValue(mouse_position) - sealevel) * 10000.0f;
			auto temperature = temperature_map->getValue(mouse_position) / 255.0f * 70.0f - 25.0f;
			auto precipitation = precipitation_map->getValue(mouse_position) / 255.0f * 80.0f;
			auto biome = get_biome_name(biome_map->getPixel(mouse_position));

			unsigned pixels = map_texture->getWidth() * map_texture->getHeight();
			float water_percentage = 100.0f * water_pixels / pixels;

			ui::Text("%.1f%% Water, %.1f%% Land", water_percentage, 100.0f - water_percentage);

			if (map_hovered) {
				ui::Text(u8"Position: %i, %i \nCoordinates: %+4.1f°, %+4.1f° \nElevation: %.1fm \nTemperature: %.1f°C \nPrecipitation: %.1fkg/m² \nBiome: %s",
						 mouse_position.x, mouse_position.y, coordinates.x, coordinates.y, elevation, temperature, precipitation, biome.c_str());
			} else {
				ui::PushItemWidth(-250);

				bool use_earth = ui::Checkbox("Earth", earth);
				if (use_earth) {
					bathymetry_scale = 0.8f;
					topography_scale = 0.7f;
					sealevel = 0.0f;
					circulation_iterations = 0;
					thresholds.ocean = -0.50f;
					thresholds.coast = -0.10f;
					thresholds.beach = 0.0f;
					thresholds.prairie = 0.005f;
					thresholds.forrest = 0.05f;
					thresholds.mountain = 0.20f;
					thresholds.snowcap = 0.25f;
				}
				update_tectonic |= use_earth;
				if (earth) {
					update_tectonic |= ui::SliderFloat("Bathymetry Scaling", bathymetry_scale, 0.0f, 1.0f, "%.2f", 1.0f, 1.0f);
					update_tectonic |= ui::SliderFloat("Topography Scaling", topography_scale, 0.0f, 1.0f, "%.2f", 1.0f, 1.0f);
				} else {
					ui::Text("Use the mouse pointer to view map details.");
					update_tectonic |= ui::DragInt("Seed", seed, 1.0f, 0, seed_maximum, "%.0f", 0);
					update_tectonic |= ui::SliderFloat("Scale", scale, 0.1f, 10.0f, "%.2f", 3.45f, 1.0f);
					update_tectonic |= ui::DragFloat2("Shift", shift, 1.0f, 0.0f, 0.0f, "%.2f", 1.0f, Zero);
					update_tectonic |= ui::SliderUnsigned("Octaves", current_options.octaves, 1, 15, "%.0f", saved_options.octaves);
					update_tectonic |= ui::SliderFloat("Amplitude", current_options.amplitude, 0.0f, 10.0f, "%.2f", 1.0f, saved_options.amplitude); ui::Hint("Ctrl+Click to enter an exact value");
					update_tectonic |= ui::SliderFloat("Frequency", current_options.frequency, 0.0f, 10.0f, "%.2f", 1.0f, saved_options.frequency); ui::Hint("Ctrl+Click to enter an exact value");
					update_tectonic |= ui::SliderFloat("Lacunarity", current_options.lacunarity, 0.0f, 10.0f, "%.2f", 1.0f, saved_options.lacunarity); ui::Hint("Ctrl+Click to enter an exact value");
					update_tectonic |= ui::SliderFloat("Persistence", current_options.persistence, 0.0f, 2.0f, "%.2f", 1.0f, saved_options.persistence); ui::Hint("Ctrl+Click to enter an exact value");
					update_tectonic |= ui::SliderFloat("Power", current_options.power, 0.1f, 10.0f, "%.2f", 1.0f, saved_options.power); ui::Hint("Ctrl+Click to enter an exact value");
					update_tectonic |= ui::SliderFloat("Continent Amplitude", continent_amplitude, 0.0f, 10.0f, "%.2f", 1.0f, 1.0f);
					update_tectonic |= ui::SliderFloat("Continent Frequency", continent_frequency, 0.0f, 2.0f, "%.2f", 1.0f, 0.5f);
					update_tectonic |= ui::Checkbox("Wrap Horizontally", wrap_horizontally);
					ui::SameLine();
					update_tectonic |= ui::Checkbox("Continents##use", use_continents);
				}

				update_topography |= ui::SliderPercentage("Sealevel", sealevel, -1.0f, 1.0f, "%+.0f%%", 1.0f, 0.0f);
				update_topography |= ui::SliderPercentage("Equator", equator, -1.0f, 1.0f, "%+.0f%%", 1.0f, 0.0f);

				if (not earth) {
					bool& update_optimized = gpu_compute ? update_tectonic : update_topography;
					update_optimized |= ui::SliderFloat("Equator Distance Factor", equator_distance_factor, -1.0f, 1.0f, "%.2f", 1.0f, 0.0f);
					update_optimized |= ui::SliderInt("Equator Distance Power", equator_distance_power, 1, 15, "%.0f", 10);

					update_topography |= ui::SliderInt("Lapse Power", lapse_power, 1, 2, "%.0f", 1);
				}
				update_topography |= ui::SliderFloat("Lapse Rate", lapse_rate, 0.0f, 20.0f, u8"%.1f (°C/km)", 1.0f, 10.0f);

				update_climate |= ui::SliderFloat("Evaporation", evaporation_factor, 0.0f, 10.0f, "%.3f", 2.0f, 1.0f);
				update_climate |= ui::SliderFloat("Transpiration", transpiration_factor, 0.0f, 10.0f, "%.3f", 2.0f, 0.25f);

				if (gpu_compute) {
					update_climate |= ui::Checkbox("Circulation", circulation);
					ui::SameLine();
					update_climate |= ui::Checkbox("Debug", debug);
				}

				if (gpu_compute and circulation) {
					update_climate |= ui::SliderUnsigned("Circulation Iterations", circulation_iterations, 0, 50, "%.0f", 25);
					update_climate |= ui::SliderFloat("Circulation Intensity", circulation_intensity, 0.0f, 1.0f, "%.3f", 1.0f, 1.0f);
					update_climate |= ui::SliderFloat("Orograpic Effect", orograpic_effect, 0.0f, 1.0f, "%.3f", 1.0f, 1.0f);
				} else {
					update_climate |= ui::SliderFloat("Precipitation", precipitation_factor, 0.0f, 10.0f, "%.3f", 2.0f, 1.0f);
					update_climate |= ui::SliderFloat("Precipitation Decay", precipitation_decay, 0.0f, 1.0f, "%.3f", 2.0f, 0.05f);
					update_climate |= ui::SliderFloat("Slope Scale", slope_scale, 0.001f, 1.0f, "%.3f", 3.0f, 0.01f);
					update_climate |= ui::SliderFloat("Humidity Saturation", humidity_saturation, 1.0f, 100.0f, "%.3f", 3.0f, 25.0f);
					update_climate |= ui::Checkbox("Upper Precipitation", upper_precipitation);
				}

				update_biomes |= ui::SliderFloat("Lower Threshold", lower_threshold, 0.0f, upper_threshold, "%.2f", 1.0f, One_Third);
				update_biomes |= ui::SliderFloat("Upper Threshold", upper_threshold, lower_threshold, 1.0f, "%.2f", 1.0f, Two_Thirds);

				static bool show_biome_colors = false;
				if (ui::SmallButton("Biome Colors:")) show_biome_colors = not show_biome_colors;
				if (show_biome_colors) {
					update_biomes |= ui::ColorEdit3("Ice", biome_colors.ice);
					update_biomes |= ui::ColorEdit3("Tundra", biome_colors.tundra);
					update_biomes |= ui::ColorEdit3("Taiga", biome_colors.taiga);
					update_biomes |= ui::ColorEdit3("Steppe", biome_colors.steppe);
					update_biomes |= ui::ColorEdit3("Prairie", biome_colors.prairie);
					update_biomes |= ui::ColorEdit3("Forest", biome_colors.forest);
					update_biomes |= ui::ColorEdit3("Desert", biome_colors.desert);
					update_biomes |= ui::ColorEdit3("Savanna", biome_colors.savanna);
					update_biomes |= ui::ColorEdit3("Rainforest", biome_colors.rainforest);
					update_biomes |= ui::ColorEdit3("Coast", biome_colors.coast);
					update_biomes |= ui::ColorEdit3("Ocean", biome_colors.ocean);
					update_biomes |= ui::ColorEdit3("Abyssal", biome_colors.abyssal);
				}

				static bool show_thresholds = false;
				if (ui::SmallButton("Thresholds:")) show_thresholds = not show_thresholds;
				if (show_thresholds) {
					if (thresholds != default_thresholds) {
						ui::SameLine();
						if (ui::SmallButton("Reset##thresholds")) {
							thresholds = default_thresholds;
							update_topography = true;
						}
					}
					update_topography |= ui::SliderFloat("Snowcap", thresholds.snowcap, thresholds.mountain, 1.0f, "%.2f");
					update_topography |= ui::SliderFloat("Mountain", thresholds.mountain, thresholds.forrest, thresholds.snowcap, "%.2f");
					update_topography |= ui::SliderFloat("Forrest", thresholds.forrest, thresholds.prairie, thresholds.mountain, "%.2f");
					update_topography |= ui::SliderFloat("Prairie", thresholds.prairie, thresholds.beach, thresholds.forrest, "%.2f");
					update_topography |= ui::SliderFloat("Beach", thresholds.beach, thresholds.coast, thresholds.prairie, "%.2f");
					update_topography |= ui::SliderFloat("Coast", thresholds.coast, thresholds.ocean, thresholds.beach, "%.2f");
					update_topography |= ui::SliderFloat("Ocean", thresholds.ocean, -1.0f, thresholds.coast, "%.2f");
				}
				ui::Text("elevation range [%.5f, %.5f] (%s [-1, +1])", elevation_minimum, elevation_maximum, (elevation_minimum >= -1.0f and elevation_maximum <= 1.0f) ? "lies within" : "exceeds");

				ui::PopItemWidth();
			}
			ui::EndChild();
		}

	}

}
