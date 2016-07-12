#pragma once

#include <cinder/gl/gl.h>

namespace sethex {

	using PerspectiveCamera = ci::CameraPersp;
	using OrthogonalCamera = ci::CameraOrtho;
	using OpaqueColor = ci::Color;
	using Color = ci::ColorA;
	using Font = ci::Font;

	using TextureFont = ci::gl::TextureFont;
	using Texture = ci::gl::Texture;
	using Textures = vector<shared<Texture>>;
	using Shader = ci::gl::GlslProg;
	using Mesh = ci::gl::VboMesh;
	using Batch = ci::gl::Batch;
	using VertexArray = ci::gl::Vao;
	using VertexBuffer = ci::gl::Vbo;
	using FrameBuffer = ci::gl::Fbo;

}