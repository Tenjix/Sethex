#pragma once

#include <cinder/gl/gl.h>

namespace sethex {

	using PerspectiveCamera = ci::CameraPersp;
	using OrthogonalCamera = ci::CameraOrtho;
	using OpaqueColor = ci::Color;
	using Color = ci::ColorA;
	using Font = ci::Font;

	using Texture = ci::gl::Texture;
	using Shader = ci::gl::GlslProg;
	using Mesh = ci::gl::VboMesh;
	using VertexArray = ci::gl::Vao;
	using VertexBuffer = ci::gl::Vbo;
	using FrameBuffer = ci::gl::Fbo;

}