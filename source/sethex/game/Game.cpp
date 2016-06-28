#include "Game.h"

#include <cinder/ObjLoader.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>

#include <sethex/systems/RenderSystem.h>

#include <sethex/data/ModelLoader.h>

#include <utilities/cinder/Watchdog.h>
#include <utilities/cinder/ShaderUtilities.h>

using namespace cinder;
using namespace cinder::geom;
using namespace cinder::app;
using namespace cinder::gl;
using namespace std;

namespace sethex {

	void Game::setup(CameraUi& camera_ui) {
		camera_ui.setCamera(&camera);
		font = Font(loadAsset("fonts/Icomoon.ttf"), 20.0f);
		font_color = Color::white();
		background = Texture::create(loadImage(loadAsset("images/Background.jpg")));
		camera.lookAt(float3(0, 0, 2.5), float3(0));
		enableVerticalSync(false);

		vector<float3> positions = {
			{ -0.5f, -0.5f, 0.0f },
			{ 0.5f, -0.5f, 0.0f },
			{ 0.5f, 0.5f, 0.0f },
			{ -0.5f, 0.5f, 0.0f },
		};
		vector<float2> texinates = {
			{ 0, 0 },
			{ 1, 0 },
			{ 1, 1 },
			{ 0, 1 },
		};
		vector<unsigned short> indices = { 0, 1, 2, 2, 3, 0 };
		vector<Mesh::Layout> buffer_layout = {
			Mesh::Layout().usage(GL_STATIC_DRAW).attrib(Attrib::POSITION, 3).attrib(Attrib::TEX_COORD_0, 2)
		};
		shared<Mesh> plane_mesh = Mesh::create(static_cast<int>(positions.size()), GL_TRIANGLES, buffer_layout, static_cast<int>(indices.size()));
		plane_mesh->bufferAttrib(Attrib::POSITION, positions);
		plane_mesh->bufferAttrib(Attrib::TEX_COORD_0, texinates);
		plane_mesh->bufferIndices(indices.size() * sizeof(unsigned short), indices.data());
		shared<Texture> texinate_texture = Texture::create(loadImage(loadAsset("images/Texinates.jpg")));
		shared<Texture> checker_texture = Texture::create(loadImage(loadAsset("images/Checker.jpg")));

		shared<Shader> unlit_texture_shader = getStockShader(ShaderDef().texture());
		shared<Shader> lit_texture_shader = getStockShader(ShaderDef().texture().lambert());
		shared<Shader> lambert_shader = getStockShader(ShaderDef().lambert());

		//Entity entity1 = world.create_entity();
		//entity1.add<Geometry>().mesh = plane_mesh;
		//Material& material = entity1.add<Material>();
		//material.shader = unlit_texture_shader;
		//material.texture = texinate_texture;

		//Entity entity2 = world.create_entity();
		//entity2.add<Geometry>().mesh(plane_mesh).position(float3(2, 0, 0));
		//entity2.add<Material>().shader(unlit_texture_shader).texture(checker_texture);


		//ObjLoader loader(loadAsset("models/Cube.obj"));
		//shared<Texture> cube_texture = Texture::create(loadImage(loadAsset("models/Cube.diffuse.png")));
		//auto x = loadAsset("textures/test.diffuse.png");
		//shared<Texture> cube_texture = Texture::create(loadImage(loadAsset("textures/test.diffuse.png")));

		//Entity cube = world.create_entity();
		//cube.add<Geometry>().mesh(Mesh::create(loader)).position(float3(-2, 0, 0)).scaling(float3(0.5));
		//cube.add<Material>().shader(lit_texture_shader).texture(cube_texture);

		//shared<Model> model = Model::create(loadModel(loadAsset("models/Cube.dae")));
		//Entity c = world.create_entity();
		//cube.add<Geometry>().mesh(model->geometry->mesh).position(float3(2, 0, 0)).scaling(float3(0.5));
		//cube.add<Material>().shader(model->material->shader).texture(model->material->texture);

		//Entity test = world.create_entity();
		//test.add<Geometry>().mesh(Mesh::create(geom::Sphere())).position(float3(0, 3, 0));

		//mesh = Mesh::create(Icosphere());
		//mesh = Mesh::create(Icosahedron());
		//mesh = Mesh::create(Cube());
		//mesh = Mesh::create(geom::Sphere());

		//string vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
		//shader::define(vertex_shader, "HEIGHT_MAP");
		//string fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
		//shader::define(fragment_shader, "DIFFUSE_TEXTURE", "SPECULAR_TEXTURE", "EMISSIVE_TEXTURE", "NORMAL_MAP");
		//shared<Shader> material_shader = Shader::create(vertex_shader, fragment_shader);
		//material_shader->uniform("uDiffuseTexture", 0);
		//material_shader->uniform("uSpecularTexture", 1);
		//material_shader->uniform("uEmissiveTexture", 2);
		//material_shader->uniform("uNormalMap", 3);

		Entity cube = world.create_entity();
		cube.add<Geometry>().mesh(Mesh::create(Cube())).position(float3(0, 0, 0));
		Material& cube_material = cube.add<Material>()
			.add_texture(Texture::create(loadImage(loadAsset("textures/test.diffuse.png"))))
			.add_texture(Texture::create(loadImage(loadAsset("textures/test.specular.png"))))
			.add_texture(Texture::create(loadImage(loadAsset("textures/test.emission.png"))))
			.add_texture(Texture::create(loadImage(loadAsset("textures/test.normal.png"))));

		Entity transparent_cube = world.create_entity();
		transparent_cube.add<Geometry>().mesh(Mesh::create(Cube())).position(float3(0, 0, 0));
		Material& transparent_cube_material = transparent_cube.add<Material>().transparent(true)
			.add_texture(Texture::create(loadImage(loadAsset("textures/frame.png"))));

		//shared<Mesh> plane = Mesh::create(geom::Plane() >> geom::Scale(0.5));
		//shared<Mesh> plane = plane_mesh;

		//world.create_entity("top", [&plane, &cube](Entity entity) {
		//	entity.add<Geometry>().mesh(plane).position(float3(0.0, 0.5, 0.0));
		//	entity.add_shared<Material>(cube);
		//});
		//world.create_entity("bottom", [&plane, &cube](Entity entity) {
		//	entity.add<Geometry>().mesh(plane).position(float3(0.0, -0.5, 0.0)).rotation(quaternion(vec3(M_PI, 0.0, 0.0)));
		//	entity.add_shared<Material>(cube);
		//});
		//world.create_entity("right", [&plane, &cube](Entity entity) {
		//	entity.add<Geometry>().mesh(plane).position(float3(0.5, 0.0, 0.0)).rotation(quaternion(vec3(0.0, 0.0, -0.5 * M_PI)));
		//	entity.add_shared<Material>(cube);
		//});
		//world.create_entity("left", [&plane, &cube](Entity entity) {
		//	entity.add<Geometry>().mesh(plane).position(float3(-0.5, 0, 0)).rotation(quaternion(vec3(0.0, 0.0, 0.5 * M_PI)));
		//	entity.add_shared<Material>(cube);
		//});
		//world.create_entity("front", [&plane, &cube](Entity entity) {
		//	entity.add<Geometry>().mesh(plane).position(float3(0.0, 0.0, 0.5)).rotation(quaternion(vec3(0.5 * M_PI, 0.0, 0.0)));
		//	entity.add_shared<Material>(cube);
		//});
		//world.create_entity("back", [&plane, &cube](Entity entity) {
		//	entity.add<Geometry>().mesh(plane).position(float3(0.0, 0.0, -0.5)).rotation(quaternion(vec3(-0.5 * M_PI, 0.0, 0.0)));
		//	entity.add_shared<Material>(cube);
		//});

		//Entity e = world.create_entity();
		//e.add<Geometry>().mesh(Mesh::create(WireIcosahedron()));
		//e.add<Material>().shader(lambert_shader);

		//Entity earth = world.create_entity();
		//earth.add<Geometry>().mesh(Mesh::create(geom::Icosphere().subdivisions(4))).position(float3(0, 0, 0));
		//Material& earth_material = earth.add<Material>()
		//	.add_texture(Texture::create(loadImage(loadAsset("textures/earth/day.jpg"))))
		//	.add_texture(Texture::create(loadImage(loadAsset("textures/earth/specular.jpg"))))
		//	.add_texture(Texture::create(loadImage(loadAsset("textures/earth/lights.jpg"))))
		//	.add_texture(Texture::create(loadImage(loadAsset("textures/earth/normal.jpg"))))
		//	.add_texture(Texture::create(loadImage(loadAsset("textures/earth/topography.jpg"))));
		//.add_texture(Texture::create(loadImage(loadAsset("textures/earth/bathymetry.jpg"))))
		//.add_texture(Texture::create(loadImage(loadAsset("textures/earth/night.jpg"))))
		//.add_texture(Texture::create(loadImage(loadAsset("textures/earth/clouds.png"))));

		wd::watch("shaders/*", [&game = *this, &shader = cube_material.shader](const fs::path& path) {
			print("compiling shader ...");
			try {
				if (true) {
					string vertex_shader = loadString(loadAsset("shaders/Wireframe.vertex.shader"));
					string fragment_shader = loadString(loadAsset("shaders/Wireframe.fragment.shader"));
					string geometry_shader = loadString(loadAsset("shaders/Wireframe.geometry.shader"));
					shader = Shader::create(vertex_shader, fragment_shader, geometry_shader);
				} else {
					string vertex_shader = loadString(loadAsset("shaders/Material.vertex.shader"));
					//shader::define(vertex_shader, "HEIGHT_MAP");
					string fragment_shader = loadString(loadAsset("shaders/Material.fragment.shader"));
					shader::define(fragment_shader, "DIFFUSE_TEXTURE", "SPECULAR_TEXTURE", "EMISSIVE_TEXTURE", "NORMAL_MAP");
					//shader::define(fragment_shader, "DIFFUSE_TEXTURE");
					shader = Shader::create(vertex_shader, fragment_shader);
					shader->uniform("uDiffuseTexture", 0);
					shader->uniform("uSpecularTexture", 1);
					shader->uniform("uEmissiveTexture", 2);
					shader->uniform("uNormalMap", 3);
					//shader->uniform("uHeightMap", 4);
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
		framebuffer->unbindFramebuffer();

		setMatricesWindow(display_size);
		draw(framebuffer->getColorTexture());
		drawString(u8"\ue000 Sethex", float2(10, 10), font_color, font);
		drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), font_color, font);
		drawString(message, float2(5, display_size.y - 50), font_color, font);
		drawString(to_string(frames_per_second) + " FPS", float2(5, display_size.y - 15));
		drawStringRight(u8"Thomas Würstle", float2(display_size.x - 5, display_size.y - 15));
	}

}
