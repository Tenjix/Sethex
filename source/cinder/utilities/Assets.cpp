#include "Assets.h"

using namespace std;
using namespace cinder;
using namespace cinder::app;
using namespace cinder::fs;

unordered_map<uint, shared<DataSource>> Assets::assets;
//unordered_map<uint, shared<DataSource>> images;

shared<DataSource> Assets::get(const path& path) {
	auto hash = hash_value(path);
	auto iterator = assets.find(hash);
	if (iterator == assets.end()) {
		iterator = assets.emplace(hash, loadAsset(path)).first;
	}
	return iterator->second;
}

void Assets::release(const path& path) {
	assets.erase(hash_value(path));
}

shared<DataSource> Assets::load(const path& path) {
	return loadAsset(path);
}

//String Assets::load_string(const path& path) {
//	return loadString(loadAsset(path));
//}
//
//shared<ImageSource> Assets::load_image(const path& path, const ImageSource::Options& options, const String& extension) {
//	return loadImage(loadAsset(path), options, extension);
//}
