#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>

namespace tenjix {

	namespace sethex {

		class Generator {

			const Lot<String> elevation_source_list { "CPU Simplex Noise", "GPU Simplex Noise", "Bathymetry & Topography Maps" };
			enum Elevation_Source { CPU_Noise, GPU_Noise, Elevation_Maps };
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

			shared<ImageSource> biomes;
			shared<ImageSource> elevation;

			void display();

		};

	}

}
