#include "Game.h"

#include <cinder/ObjLoader.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>

#include <sethex/data/ModelLoader.h>
#include <sethex/systems/RenderSystem.h>

#include <utilities/cinder/Watchdog.h>
#include <utilities/cinder/ShaderUtilities.h>

using namespace cinder;
using namespace cinder::geom;
using namespace cinder::app;
using namespace cinder::gl;
using namespace std;
using namespace sethex::hexagonal;

namespace sethex {

	void Game::setup(CameraUi& camera_ui) {
		camera_ui.setCamera(&camera);
		Font sethex = Font(loadAsset("fonts/Nunito.ttf"), 20.0f);
		font = TextureFont::create(sethex, TextureFont::Format(), TextureFont::defaultChars() + u8"äöüß\ue000\ue001\ue002\ue003\ue004\ue005\ue006");
		font_color = Color::white();
		background = Texture::create(loadImage(loadAsset("images/Background.jpg")));
		camera.lookAt(float3(0, 2.5, 2.5), float3(0));
		enableVerticalSync(false);

		shared<Texture> texinate_texture = Texture::create(loadImage(loadAsset("images/Texinates.jpg")));
		shared<Texture> checker_texture = Texture::create(loadImage(loadAsset("images/Checker.jpg")));

		shared<Shader> unlit_texture_shader = getStockShader(ShaderDef().texture());
		shared<Shader> lit_texture_shader = getStockShader(ShaderDef().texture().lambert());
		shared<Shader> lambert_shader = getStockShader(ShaderDef().lambert());

		shared<Mesh> mesh = Mesh::create(geom::Circle() >> geom::Rotate(quaternion(float3(-Pi_Half, Pi_Half, 0.0f))));
		shared<Material> material = Material::create();
		material->add_texture(Texture::create(loadImage(loadAsset("textures/flat.png"))));

		//Entity entity = world.create_entity();
		//entity.add<Geometry>().mesh(mesh).position(float3(0, 0, 0));
		//entity.add(material);
			//.add_texture(Texture::create(loadImage(loadAsset("textures/test.diffuse.png"))))
			//.add_texture(Texture::create(loadImage(loadAsset("textures/test.specular.png"))))
			//.add_texture(Texture::create(loadImage(loadAsset("textures/test.emission.png"))))
			//.add_texture(Texture::create(loadImage(loadAsset("textures/test.normal.png"))))

		map = Map(4, 3);
		auto iterator = map.coordinates().begin();
		world.create_entities(map.coordinates().size(), "", [&iterator, &mesh, &material, &sethex, &labels = this->labels](Entity entity) {
			const Coordinates& coordinates = *iterator++;
			//print(coordinates, " -> ", coordinates.to_position());
			entity.add<Geometry>().mesh(mesh).position(coordinates.to_position());
			entity.add(material);

			TextLayout layout;
			//layout.clear(Color(0.2f, 0.2f, 0.2f, 0.2f));
			layout.setFont(sethex);
			layout.setColor(Color(1, 1, 1));
			layout.addLine(coordinates.w + "     " + coordinates.u);
			layout.addCenteredLine("" + coordinates.v);
			labels.push_back(Texture::create(layout.render(true, false)));
		});

		wd::watch("shaders/*", [&game = *this, &shader = material->shader](const fs::path& path) {
			print("compiling shader ...");
			try {
				if (false) {
					string vertex_shader = loadString(loadAsset("shaders/Wireframe.vertex.shader"));
					string fragment_shader = loadString(loadAsset("shaders/Wireframe.fragment.shader"));
					string geometry_shader = loadString(loadAsset("shaders/Wireframe.geometry.shader"));
					shader = Shader::create(vertex_shader, fragment_shader, geometry_shader);
				} else {
					string vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
					//shader::define(vertex_shader, "HEIGHT_MAP");
					string fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
					//shader::define(fragment_shader, "DIFFUSE_TEXTURE", "SPECULAR_TEXTURE", "EMISSIVE_TEXTURE", "NORMAL_MAP");
					shader::define(fragment_shader, "DIFFUSE_TEXTURE");
					shader = Shader::create(vertex_shader, fragment_shader);
					//shader->uniform("uDiffuseTexture", 0);
					//shader->uniform("uSpecularTexture", 1);
					//shader->uniform("uEmissiveTexture", 2);
					//shader->uniform("uNormalMap", 3);
					//shader->uniform("uHeightMap", 4);
					//shader->uniform("uSpecularity", 1.0f);
					//shader->uniform("uLuminosity", 1.0f);
				}
				game.message = "shader compiled successfully";
			} catch (GlslProgCompileExc exception) {
				error(exception.what());
				game.message = exception.what();
			}
		});

		world.add<RenderSystem>();
	}

	void Game::resize() {
		display_size = getWindowSize();
		if (display_size.x == 0 or display_size.y == 0) return;
		camera.setAspectRatio(getWindowAspectRatio());
		framebuffer = FrameBuffer::create(display_size.x, display_size.y, FrameBuffer::Format().samples(16).coverageSamples(16));
	}

	void Game::update(float elapsed_seconds, unsigned frames_per_second) {
		this->frames_per_second = frames_per_second;
		time_delta = elapsed_seconds;
		time += time_delta;
		world.update(elapsed_seconds);
	}

	void Game::render() {
		if (display_size.x == 0 or display_size.y == 0) return;
		setMatricesWindow(display_size);
		draw(background);

		framebuffer->bindFramebuffer();
		enableDepth(true);
		clear(Color(0, 0, 0, 0));
		setMatrices(camera);
		world.get<RenderSystem>().render();
		enableDepth(false);

		getStockShader(ShaderDef().color())->bind();
		float2 mouse_position = mouse;
		Ray ray = camera.generateRay(mouse_position, display_size);
		float distance;
		bool hit = ray.calcPlaneIntersection(float3(), float3(0, 1, 0), &distance);
		float3 intersection_position;
		Coordinates intersection_coordinates;
		if (hit) {
			intersection_position = ray.calcPosition(distance);
			intersection_coordinates = Coordinates::of(intersection_position);
			//drawCube(intersection_coordinates.to_position(), float3(0.1f));
		}
		drawVector(ray.getOrigin(), ray.getOrigin() + ray.getDirection() * 10.0f);
		drawCoordinateFrame();
		framebuffer->unbindFramebuffer();

		setMatricesWindow(display_size);
		draw(framebuffer->getColorTexture());
		int i = 0;
		Area display_area(getWindowBounds());
		for (const Coordinates& coordinates : map.coordinates()) {
			auto texture = labels[i++];
			float3 world_position = coordinates.to_position();
			bool behind_camera = camera.worldToEyeDepth(world_position) > 0.0;
			if (behind_camera) continue;
			float2 screen_position = camera.worldToScreen(world_position, static_cast<float>(display_size.x), static_cast<float>(display_size.y));
			bool on_screen = display_area.contains(screen_position);
			if (not on_screen) continue;
			draw(texture, screen_position - float2(texture->getSize()) / 2.0f);

			//String text = coordinates.to_string();
			//if (hit and coordinates == intersection_coordinates) {
			//	String text = "hit";
			//	vec2 offset = font->measureString(text) / 2.0f;
			//	offset.x = -offset.x;
			//	font->drawString(text, screen_position + offset);
			//}

			//drawStringCentered(coordinates.to_string(), screen_position, font_color, font);
		}
		if (hit) {
			String text = map.contains(intersection_coordinates) ? "in" : "out";
			vec2 offset = font->measureString(text) / 2.0f;
			offset.x = -offset.x;
			float3 world_position = intersection_coordinates.to_position();
			float2 screen_position = camera.worldToScreen(world_position, static_cast<float>(display_size.x), static_cast<float>(display_size.y));
			font->drawString(text, screen_position + offset);
		}
		//color(1.0, 0.0, 0.0, 1.0);
		//texture_font->drawString(u8"\ue000 Sethex by Thomas Würstle \ue001", float2(250, 250));
		drawString(u8"\ue000 Sethex", float2(10, 10), font_color, font->getFont());
		drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), font_color, font->getFont());
		drawString(message, float2(5, display_size.y - 50), font_color, font->getFont());
		drawString(to_string(frames_per_second) + " FPS", float2(5, display_size.y - 15));
		drawStringRight(u8"Thomas Würstle", float2(display_size.x - 5, display_size.y - 15));
	}

}
