#include "Game.h"

#include <cinder/app/App.h>
#include <cinder/ObjLoader.h>

#include <systems/RenderSystem.h>

using namespace cinder;
using namespace cinder::geom;
using namespace cinder::app;
using namespace cinder::gl;
using namespace std;

namespace sethex {

	void Game::setup(CameraUi& camera_ui) {
		camera_ui.setCamera(&camera);
		font = Font(loadAsset("fonts/Icomoon.ttf"), 20.0f);
		background = Texture::create(loadImage(loadAsset("images/Background.jpg")));
		camera.lookAt(float3(0, 0, 2), float3(0));
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

		//ObjLoader loader(loadAsset("models/Hexcone.obj"));
		//mesh = VboMesh::create(loader);

		shared<Shader> unlit_texture_shader = getStockShader(ShaderDef().texture());

		Entity entity1 = world.create_entity();
		entity1.add<Geometry>().mesh = plane_mesh;
		Material& material = entity1.add<Material>();
		material.shader = unlit_texture_shader;
		material.texture = texinate_texture;

		Entity entity2 = world.create_entity();
		entity2.add<Geometry>().mesh(plane_mesh).position(float3(2, 0, 0));
		entity2.add<Material>().shader(unlit_texture_shader).texture(checker_texture);

		world.add<RenderSystem>();
	}

	void Game::resize() {
		camera.setAspectRatio(getWindowAspectRatio());
		display_size = getWindowSize();
		framebuffer = FrameBuffer::create(display_size.x, display_size.y, FrameBuffer::Format().samples(16).coverageSamples(16));
	}

	void Game::update(float elapsed_seconds, unsigned frames_per_second) {
		this->frames_per_second = frames_per_second;
		time_delta = elapsed_seconds;
		time += time_delta;
		world.update(elapsed_seconds);
	}

	void Game::render() {
		setMatricesWindow(display_size);
		draw(background);

		framebuffer->bindFramebuffer();
		clear(Color(0, 0, 0, 0));
		setMatrices(camera);
		enableDepth(true);
		world.get<RenderSystem>().render();
		enableDepth(false);
		framebuffer->unbindFramebuffer();

		setMatricesWindow(display_size);
		draw(framebuffer->getColorTexture());
		drawString(u8"\ue000 Sethex", float2(10, 10), Color(1, 1, 1), font);
		drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", float2(10, 40), Color(1, 1, 1), font);
		drawString(to_string(frames_per_second) + " FPS", float2(5, display_size.y - font.getAscent()));
		drawStringRight(u8"Thomas Würstle", float2(display_size.x - 5, display_size.y - font.getAscent()));
	}

}
