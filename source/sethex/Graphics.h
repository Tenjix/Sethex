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
	using Textures = std::vector<ci::gl::TextureRef>;
	using Shader = ci::gl::GlslProg;
	using Mesh = ci::gl::VboMesh;
	using Batch = ci::gl::Batch;
	using VertexArray = ci::gl::Vao;
	using VertexBuffer = ci::gl::Vbo;
	using FrameBuffer = ci::gl::Fbo;

}

namespace cinder {

	// asignes r,g,b of "color" to "iterator"
	inline void operator<<(const Surface::Iter& iterator, const Color8u& color) {
		iterator.r() = color.r;
		iterator.g() = color.g;
		iterator.b() = color.b;
	}

	// asignes r,g,b,a of "color" to "iterator"
	inline void operator<<(const Surface::Iter& iterator, const ColorA8u& color) {
		iterator.r() = color.r;
		iterator.g() = color.g;
		iterator.b() = color.b;
		iterator.a() = color.a;
	}

	inline void operator<<(const Surface::Iter& iterator, const Surface::Iter& other_iterator) {
		iterator.r() = other_iterator.r();
		iterator.g() = other_iterator.g();
		iterator.b() = other_iterator.b();
	}

	// returns an iterator of "iterable" pointing to the first pixel
	//template <typename Iterable>
	//typename Iterable::Iter initialized_iterator(std::shared_ptr<Iterable>& iterable) {
	//	auto iterator = iterable->getIter();
	//	iterator.line();
	//	iterator.pixel();
	//	return iterator;
	//}

}