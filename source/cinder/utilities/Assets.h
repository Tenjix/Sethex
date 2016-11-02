#pragma once

#include <unordered_map>

#include <cinder/app/App.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>

namespace cinder {

	namespace utilities {

		class Assets {

			static std::unordered_map<std::size_t, std::shared_ptr<DataSource>> assets;

		public:

			static std::shared_ptr<DataSource> get(const fs::path& path);

			static void release(const fs::path& path);

			static std::shared_ptr<DataSource> load(const fs::path& path);

			//static String load_string(const ci::fs::path& path);

			//static shared<ImageSource> load_image(const ci::fs::path& path, const ci::ImageSource::Options& options = ci::ImageSource::Options(), const String& extension = "");

		};

	}

}

