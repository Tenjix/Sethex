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

			const Lot<String> map_display_list_gpu { "Biome", "Terrain", "Elevation", "Temperature", "Circulation", "Evapotranspiration", "Humidity", "Precipitation" };
			const Lot<String> map_display_list_cpu { "Biome", "Terrain", "Elevation", "Temperature", "Precipitation" };
			enum Map_Display { Biome, Terrain, Elevation, Temperature, Circulation, Evapotranspiration, Humidity, Precipitation };
			int map_display = Biome;

		public:

			shared<ImageSource> biomes;
			shared<ImageSource> elevation;

			bool all_compiled();

			void display();

		};

	}

}
