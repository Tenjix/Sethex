#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>

namespace tenjix {

	namespace sethex {

		class Generator {

			const Lot<String> elevation_source_list { "CPU Simplex Noise", "GPU Simplex Noise", "Bathymetry & Topography Maps", "Elevation Map" };
			enum Elevation_Source { CPU_Noise, GPU_Noise, Elevation_Maps, Elevation_Map };
			int elevation_source = CPU_Noise;

			const Lot<String> circulation_type_list { "Coriolis dependend Linear Interpolation", "Coriolis deflected Pressure Differential" };
			enum Circulation_Type { Linear, Deflected };
			int circulation_type = Linear;

			const Lot<String> biome_determination_list { "Elevation based", "Elevation & Temperature & Precipitation based" };
			enum Biome_Determination { Elevation_Based, Climate_Based };
			int biome_determination = Elevation_Based;

			const Lot<String> map_display_list_circulation_deflected { "Biome", "Elevation", "Temperature", "Circulation", "Evapotranspiration", "Humidity", "Precipitation" };
			const Lot<String> map_display_list_circulation_linear { "Biome", "Elevation", "Temperature", "Precipitation" };
			const Lot<String> map_display_list_elevation { "Biome", "Elevation" };
			enum Map_Display { Biome, Elevation, Temperature, Circulation, Evapotranspiration, Humidity, Precipitation };
			int map_display = Biome;

			ci::Color8u determine_biome(float elevation, float temperature, float precipitation);

			bool all_compiled();

		public:

			struct BiomeColors {
				ci::Color8u ice = { 255, 255, 255 };
				ci::Color8u tundra = { 128, 128, 64 };
				ci::Color8u taiga = { 0, 160, 80 };
				ci::Color8u steppe = { 192, 192, 96 };
				ci::Color8u prairie = { 0, 160, 0 };
				ci::Color8u forest = { 0, 96, 0 };
				ci::Color8u desert = { 255, 255, 128 };
				ci::Color8u savanna = { 192, 240, 64 };
				ci::Color8u rainforest = { 0, 64, 0 };
				ci::Color8u hot_rock = { 160, 128, 128 };
				ci::Color8u rock = { 128, 128, 128 };
				ci::Color8u cold_rock = { 128, 128, 160 };
				ci::Color8u beach = { 240, 240, 128 };
				ci::Color8u coast = { 16, 16, 160 };
				ci::Color8u ocean = { 0, 0, 128 };
				ci::Color8u deep_ocean = { 0, 0, 92 };
			};

			static BiomeColors biome_colors;

			static const char* get_biome_name(ci::Color8u biome_color) {
				if (biome_color == biome_colors.ice) return "Ice";
				if (biome_color == biome_colors.tundra) return "Tundra";
				if (biome_color == biome_colors.taiga) return "Taiga";
				if (biome_color == biome_colors.steppe) return "Steppe";
				if (biome_color == biome_colors.prairie) return "Prairie";
				if (biome_color == biome_colors.forest) return "Forest";
				if (biome_color == biome_colors.desert) return  "Desert";
				if (biome_color == biome_colors.savanna) return "Savanna";
				if (biome_color == biome_colors.rainforest) return "Rainforest";
				if (biome_color == biome_colors.hot_rock) return "Hot Rock";
				if (biome_color == biome_colors.rock) return "Rock";
				if (biome_color == biome_colors.cold_rock) return "Cold Rock";
				if (biome_color == biome_colors.beach) return "Beach";
				if (biome_color == biome_colors.coast) return "Coast";
				if (biome_color == biome_colors.ocean) return "Ocean";
				if (biome_color == biome_colors.deep_ocean) return "Deep Ocean";
				return "Unknown";
			}

			shared<ImageSource> biomes;
			shared<ImageSource> elevation;

			void display();

		};

	}

}
