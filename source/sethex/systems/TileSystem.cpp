#include "TileSystem.h"

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>
#include <sethex/components/Material.h>
#include <sethex/components/Tile.h>

#include <utilities/cinder/Assets.h>
#include <utilities/cinder/Shaders.h>

using namespace std;
using namespace cinder;
using namespace cinder::app;
using namespace cinder::gl;
using namespace sethex::hexagonal;

namespace sethex {

	void TileSystem::initialize() {
		Font font = Font(Assets::get("fonts/Nunito.ttf"), 100.0f);
		map = Map(9, 8);

		shared<Mesh> mesh = Mesh::create(geom::Circle() >> geom::Rotate(quaternion(float3(-Pi_Half, Pi_Half, 0.0f))));

		String vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
		String fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
		shader::define(fragment_shader, "DIFFUSE_TEXTURE", "OVERLAY_TEXTURE");
		shared<Shader> shader = Shader::create(vertex_shader, fragment_shader);
		shader->setLabel("Tile Shader");
		shader->uniform("uDiffuseTexture", 0);
		shader->uniform("uOverlayTexture", 1);
		shared<Texture> hexagon_texure = Texture::create(loadImage(loadAsset("textures/pointy.png")), Texture::Format().mipmap());
		//auto material = Material::create(shader);
		//material->add(hexagon_texure);

		auto iterator = map.coordinates().begin();
		world->create_entities(map.coordinates().size(), "Tile #", [&iterator, &mesh, &shader, &font, &hexagon_texure/*, &material*/](Entity entity) {
			const Coordinates& coordinates = *iterator++;
			entity.add<Geometry>().mesh(mesh).position(coordinates.to_position());
			entity.add<Tile>().coordinates = coordinates;
			//entity.add(material);
			auto& material = entity.add<Material>().name(entity.name + " Material").shader(shader).add(hexagon_texure);
			TextLayout layout;
			layout.setFont(font);
			layout.setColor(Color(1, 1, 1));
			layout.addLine(coordinates.w + "     " + coordinates.u);
			layout.addCenteredLine("" + coordinates.v);
			auto texture = Texture::create(layout.render(true, false));
			material.add(texture);
		});
		//for (size_t i = 0; i < map.coordinates().size(); i++) {
		//	auto& coordinates = map.coordinates()[i];
		//	print(i, ": ", coordinates, " ", map.index(coordinates));
		//}
		focus_expansion = (map.width / 2 + 1) * Coordinates::Tilesize.x;
		print(map.width, "x", map.height);
	}

	void TileSystem::update(float delta_time) {
		static Display& display = world->find_entity("Main Display").get<Display>();
		focus_position = display.camera.getPivotPoint();
		focus_position.z = 0;
		focus_coordinates = Coordinates::of(focus_position);
		focus_range = { focus_position.x - focus_expansion, focus_position.x + focus_expansion };

		//Ray ray = display.camera.generateRay(input.mouse.position, display.size);
		//float distance;
		//bool hit = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
		//float3 intersection_position;
		//Coordinates intersection_coordinates;
		//if (hit) {
		//	intersection_position = ray.calcPosition(distance);
		//	intersection_coordinates = Coordinates::of(intersection_position);
		//	//drawCube(intersection_coordinates.to_position(), float3(0.1f));
		//}
		System::update(delta_time);
	};

	void TileSystem::update(Entity& entity, float delta_time) {
		auto& geometry = entity.get<Geometry>();
		auto& tile = entity.get<Tile>();
		auto& position = geometry.position();
		if (not focus_range.contains(position.x)) {
			geometry.position->x += map.width * Coordinates::Spacing.x * signum(focus_position.x - position.x);
			//position.x += map.width * Coordinates::Spacing.x * signum(focus_position.x - position.x);
		}
		//auto area = Coordinates::rectangle(map.width, map.height, focus_coordinates);
		//for (auto coordinates : area) {
		//	auto reprojected_coordinates = map.reproject(coordinates);
		//	auto index = map.index(reprojected_coordinates);
		//	map.coordinates()[index];
		//}
		//map.index(focus)
	};

}
