#include "Game.h"

#include <cinder/ObjLoader.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>
#include <cinder/app/App.h>

#include <sethex/data/ModelLoader.h>
#include <sethex/systems/RenderSystem.h>
#include <sethex/systems/TileSystem.h>

//#include <utilities/cinder/Watchdog.h>
//#include <utilities/cinder/Shaders.h>

using namespace cinder;
using namespace cinder::geom;
using namespace cinder::app;
using namespace cinder::gl;
using namespace std;
using namespace sethex::hexagonal;

namespace sethex {

	void Game::setup(CameraUi& camera_ui) {
		Display& display = world.create_entity("Main Display").add<Display>();

		camera_ui.setCamera(&display.camera);
		auto font_type = loadAsset("fonts/Nunito.ttf");
		font = TextureFont::create(Font(font_type, 20.0f), TextureFont::Format(), TextureFont::defaultChars() + u8"äöüß\ue000\ue001\ue002\ue003\ue004\ue005\ue006");
		font_color = Color::white();
		background = Texture::create(loadImage(loadAsset("images/Background.jpg")));
		display.camera.lookAt(float3(0, 2.5, 2.5), float3(0));
		enableVerticalSync(false);

		//shared<Texture> texinate_texture = Texture::create(loadImage(loadAsset("images/Texinates.jpg")));
		//shared<Texture> checker_texture = Texture::create(loadImage(loadAsset("images/Checker.jpg")));

		//shared<Shader> unlit_texture_shader = getStockShader(ShaderDef().texture());
		//shared<Shader> lit_texture_shader = getStockShader(ShaderDef().texture().lambert());
		//shared<Shader> lambert_shader = getStockShader(ShaderDef().lambert());

		//shared<Mesh> mesh = Mesh::create(geom::Circle() >> geom::Rotate(quaternion(float3(-Pi_Half, Pi_Half, 0.0f))));

		//string vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
		//string fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
		//shader::define(fragment_shader, "DIFFUSE_TEXTURE", "OVERLAY_TEXTURE");
		//shared<Shader> shader = Shader::create(vertex_shader, fragment_shader);
		//shader->setLabel("Tile Shader");
		//shader->uniform("uDiffuseTexture", 0);
		//shader->uniform("uOverlayTexture", 1);
		//shared<Texture> hexagon_texure = Texture::create(loadImage(loadAsset("textures/pointy.png")), Texture::Format().mipmap());

		//Font coordinate_font = Font(font_type, 100.0f);
		//map = Map(9, 8);
		//auto iterator = map.coordinates().begin();
		//world.create_entities(map.coordinates().size(), "Tile #", [&iterator, &mesh, &shader, &coordinate_font, &hexagon_texure, &labels = this->labels](Entity entity) {
		//	const Coordinates& coordinates = *iterator++;
		//	entity.add<Geometry>().mesh(mesh).position(coordinates.to_position());
		//	auto& material = entity.add<Material>().name(entity.name + " Material").shader(shader).add_texture(hexagon_texure);
		//	TextLayout layout;
		//	layout.setFont(coordinate_font);
		//	layout.setColor(Color(1, 1, 1));
		//	layout.addLine(coordinates.w + "     " + coordinates.u);
		//	layout.addCenteredLine("" + coordinates.v);
		//	auto texture = Texture::create(layout.render(true, false));
		//	labels.push_back(texture);
		//	material.add_texture(texture);
		//});

		world.add<RenderSystem>();
		world.add<TileSystem>(input);
	}

	void Game::resize() {
		static Display& display = world.find_entity("Main Display").get<Display>();
		display.size = getWindowSize();
		if (display.size.x == 0 or display.size.y == 0) return;
		display.camera.setAspectRatio(getWindowAspectRatio());
		display.framebuffer = FrameBuffer::create(display.size.x, display.size.y, FrameBuffer::Format().samples(16).coverageSamples(16));
	}

	void Game::update(float elapsed_seconds, unsigned frames_per_second) {
		this->frames_per_second = frames_per_second;
		time_delta = elapsed_seconds;
		time += time_delta;
		world.update(elapsed_seconds);
	}

	void Game::render() {
		static Display& display = world.find_entity("Main Display").get<Display>();
		if (display.size.x == 0 or display.size.y == 0) return;
		float2 screen_size = display.size;
		setMatricesWindow(display.size);
		draw(background);

		world.get<RenderSystem>().render();

		//display.framebuffer->bindFramebuffer();
		//enableDepth(true);
		//clear(Color(0, 0, 0, 0));
		//setMatrices(display.camera);
		//world.get<RenderSystem>().render();
		//enableDepth(false);

		//getStockShader(ShaderDef().color())->bind();
		//float2 mouse_position = mouse;
		//Ray ray = display.camera.generateRay(mouse_position, display.size);
		//float distance;
		//bool hit = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
		//float3 intersection_position;
		//Coordinates intersection_coordinates;
		//if (hit) {
		//	intersection_position = ray.calcPosition(distance);
		//	intersection_coordinates = Coordinates::of(intersection_position);
		//	//drawCube(intersection_coordinates.to_position(), float3(0.1f));
		//}
		//drawVector(ray.getOrigin(), ray.getOrigin() + ray.getDirection() * 10.0f);
		//drawCoordinateFrame();
		//display.framebuffer->unbindFramebuffer();

		//setMatricesWindow(display.size);
		//draw(display.framebuffer->getColorTexture());

		//int i = 0;
		//Area display_area(getWindowBounds());
		//for (const Coordinates& coordinates : map.coordinates()) {
		//	auto texture = labels[i++];
		//	float3 world_position = coordinates.to_position();
		//	bool behind_camera = camera.worldToEyeDepth(world_position) > 0.0;
		//	if (behind_camera) continue;
		//	float2 screen_position = camera.worldToScreen(world_position, screen_size.x, screen_size.y);
		//	bool on_screen = display_area.contains(screen_position);
		//	if (not on_screen) continue;
		//	draw(texture, screen_position - float2(texture->getSize()) / 2.0f);

		//	//String text = coordinates.to_string();
		//	//if (hit and coordinates == intersection_coordinates) {
		//	//	String text = "hit";
		//	//	vec2 offset = font->measureString(text) / 2.0f;
		//	//	offset.x = -offset.x;
		//	//	font->drawString(text, screen_position + offset);
		//	//}

		//	//drawStringCentered(coordinates.to_string(), screen_position, font_color, font);
		//}

		//bool hit = false;
		//Coordinates intersection_coordinates;
		//if (hit) {
		//	bool on_map = map.contains(intersection_coordinates);
		//	String text = on_map ? "in" : "out";
		//	vec2 offset = font->measureString(text) / 2.0f;
		//	offset.x = -offset.x;
		//	float3 world_position = intersection_coordinates.to_position();
		//	float2 screen_position = display.camera.worldToScreen(world_position, screen_size.x, screen_size.y);
		//	Color color(0, 1, 0);
		//	font->drawString(text, screen_position + offset);
		//	auto draw_coordinates = [&instance = *this, &screen_size, &color](const Coordinates& coordinates) {
		//		float2 screen_position = display.camera.worldToScreen(coordinates.to_position(), screen_size.x, screen_size.y);
		//		TextLayout layout;
		//		layout.setFont(instance.font->getFont());
		//		layout.setColor(color);
		//		layout.addLine(coordinates.w + "     " + coordinates.u);
		//		layout.addCenteredLine("" + coordinates.v);
		//		auto texture = Texture::create(layout.render(true, false));
		//		draw(texture, screen_position - float2(texture->getSize()) / 2.0f);
		//	};
		//	Coordinates reprojected_coordinates = map.reproject(intersection_coordinates);
		//	draw_coordinates(reprojected_coordinates);
		//	color = Color(0, 0, 1);
		//	map.neighbor(reprojected_coordinates, Direction::NorthEast).then(draw_coordinates);
		//	color = Color(0, 0.5f, 1);
		//	map.neighbor(reprojected_coordinates, Direction::East).then(draw_coordinates);
		//	color = Color(0, 1, 1);
		//	map.neighbor(reprojected_coordinates, Direction::SouthEast).then(draw_coordinates);
		//	color = Color(1, 0, 0);
		//	map.neighbor(reprojected_coordinates, Direction::SouthWest).then(draw_coordinates);
		//	color = Color(1, 0.5f, 0);
		//	map.neighbor(reprojected_coordinates, Direction::West).then(draw_coordinates);
		//	color = Color(1, 1, 0);
		//	map.neighbor(reprojected_coordinates, Direction::NorthWest).then(draw_coordinates);
		//}

		//color(1.0, 0.0, 0.0, 1.0);
		//texture_font->drawString(u8"\ue000 Sethex by Thomas Würstle \ue001", float2(250, 250));
		drawString(u8"\ue000 Sethex", float2(10, 10), font_color, font->getFont());
		drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), font_color, font->getFont());
		drawString(message, float2(5, display.size.y - 50), font_color, font->getFont());
		drawString(to_string(frames_per_second) + " FPS", float2(5, display.size.y - 15));
		drawStringRight(u8"Thomas Würstle", float2(display.size.x - 5, display.size.y - 15));
	}

}
