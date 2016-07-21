#pragma once

#include <unordered_map>

#include <cinder/app/App.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>

#include <utilities/Standard.h>

class Assets {

	static std::unordered_map<uint, shared<ci::DataSource>> assets;

public:

	static shared<ci::DataSource> get(const ci::fs::path& path);

	static void release(const ci::fs::path& path);

	static shared<ci::DataSource> load(const ci::fs::path& path);

	//static String load_string(const ci::fs::path& path);

	//static shared<ImageSource> load_image(const ci::fs::path& path, const ci::ImageSource::Options& options = ci::ImageSource::Options(), const String& extension = "");

};
