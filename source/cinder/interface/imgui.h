/*
 Cinder-ImGui
 This code is intended for use with Cinder
 and Omar Cornut ImGui C++ libraries.

 http://libcinder.org
 https://github.com/ocornut

 Copyright (c) 2013-2015, Simon Geilfus - All rights reserved.
 Modified by Tenjix

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <cmath>
#include <memory>
#include <vector>
#include <map>

#include "cinder/CinderAssert.h"
#include "cinder/Color.h"
#include "cinder/Noncopyable.h"
#include "cinder/Vector.h"
#include "cinder/Filesystem.h"

 // forward declarations
namespace cinder {
	typedef std::shared_ptr<class DataSource> DataSourceRef;
	namespace app { typedef std::shared_ptr<class Window> WindowRef; }
	namespace gl { typedef std::shared_ptr<class Texture2d> Texture2dRef; }
}

// Custom implicit cast operators
#ifndef CINDER_IMGUI_NO_IMPLICIT_CASTS
#define IM_VEC2_CLASS_EXTRA                                             \
ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                        \
operator glm::vec2() const { return glm::vec2(x,y); }                   \
ImVec2(const glm::ivec2& f) { x = (float)f.x; y = (float)f.y; }         \
operator glm::ivec2() const { return glm::ivec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                             \
ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }      \
operator glm::vec4() const { return glm::vec4(x,y,z,w); }               \
ImVec4(const ci::ColorA& f) { x = f.r; y = f.g; z = f.b; w = f.a; }     \
operator ci::ColorA() const { return ci::ColorA(x,y,z,w); }             \
ImVec4(const ci::Color& f) { x = f.r; y = f.g; z = f.b; w = 1.0f; }     \
operator ci::Color() const { return ci::Color(x,y,z); }
#endif

#include "imgui/imgui.h"

namespace cinder {
	namespace imgui = ImGui;
	#ifndef CINDER_IMGUI_NO_NAMESPACE_ALIAS
	namespace ui = ImGui;
	#endif
}

//! cinder imgui namespace
namespace ImGui {

	struct Options {
		//! defaults to using the current window, the basic ImGui font and the dark theme
		Options();

		//! sets the window that will be used to connect the signals and render ImGui
		Options& window(const ci::app::WindowRef &window);
		//! species whether the block should call ImGui::NewFrame and ImGui::Render automatically. Default to true.
		Options& autoRender(bool autoRender);

		//! sets the font to use in ImGui
		Options& font(const ci::fs::path &fontPath, float size);
		//! sets the list of available fonts to use in ImGui
		Options& fonts(const std::vector<std::pair<ci::fs::path, float>> &fontPaths);
		//! sets the font to use in ImGui
		Options& fontGlyphRanges(const std::string &name, const std::vector<ImWchar> &glyphRanges);
		//! adds glyph ranges to the specified font, glyph_ranges_array has to be zero terminated
		Options& addFontGlyphRanges(const std::string & name, const ImWchar* glyph_ranges_array);
		//! sets global font scale
		Options& fontGlobalScale(float scale);

		//! Global alpha applies to everything in ImGui
		Options& alpha(float a);
		//! Padding within a window
		Options& windowPadding(const glm::vec2 &padding);
		//! Minimum window size
		Options& windowMinSize(const glm::vec2 &minSize);
		//! Radius of window corners rounding. Set to 0.0f to have rectangular windows
		Options& windowRounding(float rounding);
		//! Alignment for title bar text
		Options& windowTitleAlign(ImGuiAlign align);
		//! Radius of child window corners rounding. Set to 0.0f to have rectangular windows
		Options& childWindowRounding(float rounding);
		//! Padding within a framed rectangle (used by most widgets)
		Options& framePadding(const glm::vec2 &padding);
		//! Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
		Options& frameRounding(float rounding);
		//! Horizontal and vertical spacing between widgets/lines
		Options& itemSpacing(const glm::vec2 &spacing);
		//! Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
		Options& itemInnerSpacing(const glm::vec2 &spacing);
		//! Expand bounding box for touch-based system where touch position is not accurate enough (unnecessary for mouse inputs). Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget running. So dont grow this too much!
		Options& touchExtraPadding(const glm::vec2 &padding);
		//! Horizontal spacing when entering a tree node
		Options& indentSpacing(float spacing);
		//! Minimum horizontal spacing between two columns
		Options& columnsMinSpacing(float minSpacing);
		//! Width of the vertical scroll bar, Height of the horizontal scrollbar
		Options& scrollBarSize(float size);
		//! Radius of grab corners for scrollbar
		Options& scrollbarRounding(float rounding);
		//! Minimum width/height of a grab box for slider/scrollbar
		Options& grabMinSize(float minSize);
		//! Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
		Options& grabRounding(float rounding);
		//! Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
		Options& displayWindowPadding(const glm::vec2 &padding);
		//! If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
		Options& displaySafeAreaPadding(const glm::vec2 &padding);
		//! Enable anti-aliasing on lines/borders. Disable if you are really tight on CPU/GPU.
		Options& antiAliasedLines(bool antiAliasing);
		//! Enable anti-aliasing on filled shapes (rounded rectangles, circles, etc.)
		Options& antiAliasedShapes(bool antiAliasing);
		//! Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
		Options& curveTessellationTol(float tessTolerance);

		//! sets imgui ini file path
		Options& iniPath(const ci::fs::path &path);

		//! sets imgui original theme
		Options& defaultTheme();
		//! sets the dark theme
		Options& darkTheme();
		//! sets theme colors
		Options& color(ImGuiCol option, const ci::ColorA &color);

		//! returns whether the block should call ImGui::NewFrame and ImGui::Render automatically
		bool isAutoRenderEnabled() const { return mAutoRender; }
		//! returns the window that will be use to connect the signals and render ImGui
		ci::app::WindowRef getWindow() const { return mWindow; }
		//! returns the list of available fonts to use in ImGui
		const std::vector<std::pair<ci::fs::path, float>>& getFonts() const { return mFonts; }
		//! returns the glyph ranges if available for this font
		const ImWchar* getFontGlyphRanges(const std::string &name) const;
		//! returns the window that will be use to connect the signals and render ImGui
		const ImGuiStyle& getStyle() const { return mStyle; }
		//! returns imgui ini file path
		const ci::fs::path& getIniPath() const { return mIniPath; }

	protected:
		bool						mAutoRender;
		ImGuiStyle					mStyle;
		std::vector<std::pair<ci::fs::path, float>>	mFonts;
		std::map<std::string, std::vector<ImWchar>>	mFontsGlyphRanges;
		ci::app::WindowRef				mWindow;
		ci::fs::path					mIniPath;
	};

	//! initializes ImGui and the Renderer
	void    initialize(const Options &options = Options());
	//! connects window signals to imgui events
	void    connectWindow(ci::app::WindowRef window);
	//! disconnects window signals from imgui
	void    disconnectWindow(ci::app::WindowRef window);

	// Cinder Helpers

	void Image(const ci::gl::Texture2dRef &texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool ImageButton(const ci::gl::Texture2dRef &texture, const ImVec2& size, int frame_padding = -1, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& bg_col = ImVec4(0, 0, 0, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	void PushFont(const std::string& name = "");

	// Std Helpers

	bool ListBox(const char* label, int* current_item, const std::vector<std::string>& items, int height_in_items = -1);
	bool InputText(const char* label, std::string* buf, ImGuiInputTextFlags flags = 0, ImGuiTextEditCallback callback = NULL, void* user_data = NULL);
	bool InputTextMultiline(const char* label, std::string* buf, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiTextEditCallback callback = NULL, void* user_data = NULL);
	bool Combo(const char* label, int* current_item, const std::vector<std::string>& items, int height_in_items = -1);

	inline bool Combo(const char* label, int& current_item, const std::vector<std::string>& items, int height_in_items = -1) {
		return Combo(label, &current_item, items, height_in_items);
	}

	// Getters/Setters Helpers

	template<typename T>
	bool InputText(const char* label, T *object, std::string(T::*get)() const, void(T::*set)(const std::string&), ImGuiInputTextFlags flags = 0, ImGuiTextEditCallback callback = NULL, void* user_data = NULL);
	template<typename T>
	bool Checkbox(const char* label, T *object, bool(T::*get)() const, void(T::*set)(bool));
	template<typename T>
	bool Combo(const char* label, T *object, int(T::*get)() const, void(T::*set)(int), const std::vector<std::string>& items, int height_in_items = -1);
	template<typename T>
	bool DragFloat(const char* label, T *object, float(T::*get)() const, void(T::*set)(float), float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f);     // If v_min >= v_max we have no bound
	template<typename T>
	bool DragFloat2(const char* label, T *object, ci::vec2(T::*get)() const, void(T::*set)(const ci::vec2&), float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f);
	template<typename T>
	bool DragFloat3(const char* label, T *object, ci::vec3(T::*get)() const, void(T::*set)(const ci::vec3&), float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f);
	template<typename T>
	bool DragFloat4(const char* label, T *object, ci::vec4(T::*get)() const, void(T::*set)(const ci::vec4&), float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f);
	template<typename T>
	bool DragInt(const char* label, T *object, int(T::*get)() const, void(T::*set)(int), float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* display_format = "%.0f");     // If v_min >= v_max we have no bound
	template<typename T>
	bool DragInt2(const char* label, T *object, ci::ivec2(T::*get)() const, void(T::*set)(const ci::ivec2&), float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* display_format = "%.0f");
	template<typename T>
	bool DragInt3(const char* label, T *object, ci::ivec3(T::*get)() const, void(T::*set)(const ci::ivec3&), float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* display_format = "%.0f");
	template<typename T>
	bool DragInt4(const char* label, T *object, ci::ivec4(T::*get)() const, void(T::*set)(const ci::ivec4&), float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* display_format = "%.0f");

	// shows "tooltip" if the previous control is hovered
	inline void Tooltip(const char* tooltip = nullptr) {
		if (tooltip && IsItemHovered()) SetTooltip(tooltip);
	}
	// adds a hint on the same line indicated by an appended "indicator" which has "text" as tooltip
	inline void Hint(const char* text, const char* indicator = "*") {
		ImGui::SameLine(); ImGui::TextDisabled(indicator); Tooltip(text);
	}

	// internal
	namespace {

		template <class Type>
		bool ResetButton(bool changed, Type& value, const Type& reset_value, const char* label, const char* reset_label = "Reset") {
			if (value != reset_value) {
				PushID(label);
				SameLine(); if (Button(reset_label)) { value = reset_value; changed = true; }
				PopID();
			}
			return changed;
		}

	}

	// reference overloads

	inline bool Checkbox(const char* label, bool& value) {
		return Checkbox(label, &value);
	}
	inline bool ColorEdit3(const char* label, ci::Color& color) {
		return ColorEdit3(label, reinterpret_cast<float*>(&color));
	}
	inline bool ColorEdit4(const char* label, ci::ColorA& color, bool show_alpha = true) {
		return ColorEdit4(label, reinterpret_cast<float*>(&color), show_alpha);
	}
	inline bool DragFloat(const char* label, float& value, float value_speed = 1.0f, float value_minimum = 0.0f, float value_maximum = 0.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return DragFloat(label, &value, value_speed, value_minimum, value_maximum, display_format, power);
	}
	inline bool DragFloat(const char* label, float& value, float value_speed, float value_minimum, float value_maximum, const char* display_format, float power, const float reset_value, const char* reset_label = "Reset") {
		return ResetButton(DragFloat(label, &value, value_speed, value_minimum, value_maximum, display_format, power), value, reset_value, label, reset_label);
	}
	inline bool DragFloat2(const char* label, ci::vec2& value, float value_speed = 1.0f, float value_minimum = 0.0f, float value_maximum = 0.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return DragFloat2(label, reinterpret_cast<float*>(&value), value_speed, value_minimum, value_maximum, display_format, power);
	}
	inline bool DragFloat2(const char* label, ci::vec2& value, float value_speed, float value_minimum, float value_maximum, const char* display_format, float power, const ci::vec2& reset_value, const char* reset_label = "Reset") {
		return ResetButton(DragFloat2(label, value, value_speed, value_minimum, value_maximum, display_format, power), value, reset_value, label, reset_label);
	}
	inline bool DragFloat3(const char* label, ci::vec3& value, float value_speed = 1.0f, float value_minimum = 0.0f, float value_maximum = 0.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return DragFloat3(label, reinterpret_cast<float*>(&value), value_speed, value_minimum, value_maximum, display_format, power);
	}
	inline bool DragFloat4(const char* label, ci::vec4& value, float value_speed = 1.0f, float value_minimum = 0.0f, float value_maximum = 0.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return DragFloat4(label, reinterpret_cast<float*>(&value), value_speed, value_minimum, value_maximum, display_format, power);
	}
	inline bool DragInt(const char* label, int& value, float value_speed = 1.0f, int value_minimum = 0, int value_maximum = 0, const char* display_format = "%.0f") {
		return DragInt(label, &value, value_speed, value_minimum, value_maximum, display_format);
	}
	inline bool DragInt(const char* label, int& value, float value_speed, int value_minimum, int value_maximum, const char* display_format, const int reset_value, const char* reset_label = "Reset") {
		return ResetButton(DragInt(label, &value, value_speed, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool DragInt2(const char* label, ci::ivec2& value, float value_speed = 1.0f, int value_minimum = 0, int value_maximum = 0, const char* display_format = "%.0f") {
		return DragInt2(label, reinterpret_cast<int*>(&value), value_speed, value_minimum, value_maximum, display_format);
	}
	inline bool DragInt2(const char* label, ci::ivec2& value, float value_speed, int value_minimum, int value_maximum, const char* display_format, const ci::ivec2& reset_value, const char* reset_label = "Reset") {
		return ResetButton(DragInt2(label, value, value_speed, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool DragInt3(const char* label, ci::ivec3& value, float value_speed = 1.0f, int value_minimum = 0, int value_maximum = 0, const char* display_format = "%.0f") {
		return DragInt3(label, reinterpret_cast<int*>(&value), value_speed, value_minimum, value_maximum, display_format);
	}
	inline bool DragInt4(const char* label, ci::ivec4& value, float value_speed = 1.0f, int value_minimum = 0, int value_maximum = 0, const char* display_format = "%.0f") {
		return DragInt4(label, reinterpret_cast<int*>(&value), value_speed, value_minimum, value_maximum, display_format);
	}
	inline bool DragUnsigned(const char* label, unsigned& value, float value_speed = 1.0f, unsigned short value_minimum = 0, unsigned short value_maximum = 100, const char* display_format = "%.0f") {
		CI_ASSERT_MSG(value < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
		float slider_value = static_cast<float>(value);
		bool changed = DragFloat(label, &slider_value, value_speed, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		value = static_cast<unsigned>(slider_value);
		return changed;
	}
	inline bool DragUnsigned(const char* label, unsigned& value, float value_speed, unsigned short value_minimum, unsigned short value_maximum, const char* display_format, const unsigned reset_value, const char* reset_label = "Reset") {
		return ResetButton(DragUnsigned(label, value, value_speed, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool DragUnsigned2(const char* label, ci::uvec2& value, float value_speed = 1.0f, unsigned short value_minimum = 0, unsigned short value_maximum = 100, const char* display_format = "%.0f") {
		constexpr unsigned n = 2;
		float slider_value[n];
		for (unsigned i = 0; i < n; i++) {
			CI_ASSERT_MSG(value[i] < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
			slider_value[n] = static_cast<float>(value[n]);
		}
		bool changed = DragFloat2(label, slider_value, value_speed, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		for (unsigned i = 0; i < n; i++) value[i] = static_cast<unsigned>(slider_value[i]);
		return changed;
	}
	inline bool DragUnsigned2(const char* label, ci::uvec2& value, float value_speed, unsigned short value_minimum, unsigned short value_maximum, const char* display_format, const ci::uvec2& reset_value, const char* reset_label = "Reset") {
		return ResetButton(DragUnsigned2(label, value, value_speed, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool DragUnsigned3(const char* label, ci::uvec3& value, float value_speed = 1.0f, unsigned short value_minimum = 0, unsigned short value_maximum = 100, const char* display_format = "%.0f") {
		constexpr unsigned n = 3;
		float slider_value[n];
		for (unsigned i = 0; i < n; i++) {
			CI_ASSERT_MSG(value[i] < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
			slider_value[n] = static_cast<float>(value[n]);
		}
		bool changed = DragFloat3(label, slider_value, value_speed, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		for (unsigned i = 0; i < n; i++) value[i] = static_cast<unsigned>(slider_value[i]);
		return changed;
	}
	inline bool DragUnsigned4(const char* label, ci::uvec4& value, float value_speed = 1.0f, unsigned short value_minimum = 0, unsigned short value_maximum = 100, const char* display_format = "%.0f") {
		constexpr unsigned n = 4;
		float slider_value[n];
		for (unsigned i = 0; i < n; i++) {
			CI_ASSERT_MSG(value[i] < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
			slider_value[n] = static_cast<float>(value[n]);
		}
		bool changed = DragFloat4(label, slider_value, value_speed, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		for (unsigned i = 0; i < n; i++) value[i] = static_cast<unsigned>(slider_value[i]);
		return changed;
	}
	inline bool SliderAngle(const char* label, float& value_radians, float value_degrees_minimum = -360.0f, float value_degrees_maximum = +360.0f) {
		return SliderAngle(label, &value_radians, value_degrees_minimum, value_degrees_maximum);
	}
	inline bool SliderAngle(const char* label, float& value_radians, float value_degrees_minimum, float value_degrees_maximum, const float reset_value_radians, const char* reset_label = "Reset") {
		return ResetButton(SliderAngle(label, &value_radians, value_degrees_minimum, value_degrees_maximum), value_radians, reset_value_radians, label, reset_label);
	}
	inline bool SliderPercentage(const char* label, float& value, float value_minimum = 0.0f, float value_maximum = 1.0f, const char* display_format = "%+.1f%%", float power = 1.0f) {
		float percentage = value * 100.0f;
		bool changed = SliderFloat(label, &percentage, value_minimum * 100.0f, value_maximum * 100.0f, display_format, power);
		value = percentage / 100.0f;
		return changed;
	}
	inline bool SliderPercentage(const char* label, float& value, float value_minimum, float value_maximum, const char* display_format, float power, const float reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderPercentage(label, value, value_minimum, value_maximum, display_format, power), value, reset_value, label, reset_label);
	}
	inline bool SliderFloat(const char* label, float& value, float value_minimum = 0.0f, float value_maximum = 1.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return SliderFloat(label, &value, value_minimum, value_maximum, display_format, power);
	}
	inline bool SliderFloat(const char* label, float& value, float value_minimum, float value_maximum, const char* display_format, float power, const float reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderFloat(label, value, value_minimum, value_maximum, display_format, power), value, reset_value, label, reset_label);
	}
	inline bool SliderFloat2(const char* label, ci::vec2& value, float value_minimum = 0.0f, float value_maximum = 1.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return SliderFloat2(label, reinterpret_cast<float*>(&value), value_minimum, value_maximum, display_format, power);
	}
	inline bool SliderFloat2(const char* label, ci::vec2& value, float value_minimum, float value_maximum, const char* display_format, float power, const ci::vec2& reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderFloat2(label, value, value_minimum, value_maximum, display_format, power), value, reset_value, label, reset_label);
	}
	inline bool SliderFloat3(const char* label, ci::vec3& value, float value_minimum = 0.0f, float value_maximum = 1.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return SliderFloat3(label, reinterpret_cast<float*>(&value), value_minimum, value_maximum, display_format, power);
	}
	inline bool SliderFloat4(const char* label, ci::vec4& value, float value_minimum = 0.0f, float value_maximum = 1.0f, const char* display_format = "%.3f", float power = 1.0f) {
		return SliderFloat4(label, reinterpret_cast<float*>(&value), value_minimum, value_maximum, display_format, power);
	}
	inline bool SliderInt(const char* label, int& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		return SliderInt(label, &value, value_minimum, value_maximum, display_format);
	}
	inline bool SliderInt(const char* label, int& value, int value_minimum, int value_maximum, const char* display_format, const int reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderInt(label, value, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool SliderInt2(const char* label, ci::ivec2& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		return SliderInt2(label, reinterpret_cast<int*>(&value), value_minimum, value_maximum, display_format);
	}
	inline bool SliderInt2(const char* label, ci::ivec2& value, int value_minimum, int value_maximum, const char* display_format, const ci::ivec2& reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderInt2(label, reinterpret_cast<int*>(&value), value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool SliderInt3(const char* label, ci::ivec3& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		return SliderInt3(label, reinterpret_cast<int*>(&value), value_minimum, value_maximum, display_format);
	}
	inline bool SliderInt4(const char* label, ci::ivec4& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		return SliderInt4(label, reinterpret_cast<int*>(&value), value_minimum, value_maximum, display_format);
	}
	inline bool SliderUnsigned(const char* label, unsigned& value, unsigned short value_minimum = 0, unsigned short value_maximum = 100, const char* display_format = "%.0f") {
		CI_ASSERT_MSG(value < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
		float slider_value = static_cast<float>(value);
		bool changed = SliderFloat(label, &slider_value, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		value = static_cast<unsigned>(slider_value);
		return changed;
	}
	inline bool SliderUnsigned(const char* label, unsigned& value, unsigned short value_minimum, unsigned short value_maximum, const char* display_format, const unsigned reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderUnsigned(label, value, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool SliderUnsigned2(const char* label, ci::uvec2& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		constexpr unsigned n = 2;
		float slider_value[n];
		for (unsigned i = 0; i < n; i++) {
			CI_ASSERT_MSG(value[i] < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
			slider_value[n] = static_cast<float>(value[n]);
		}
		bool changed = SliderFloat2(label, slider_value, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		for (unsigned i = 0; i < n; i++) value[i] = static_cast<unsigned>(slider_value[i]);
		return changed;
	}
	inline bool SliderUnsigned2(const char* label, ci::uvec2& value, unsigned short value_minimum, unsigned short value_maximum, const char* display_format, const ci::uvec2& reset_value, const char* reset_label = "Reset") {
		return ResetButton(SliderUnsigned2(label, value, value_minimum, value_maximum, display_format), value, reset_value, label, reset_label);
	}
	inline bool SliderUnsigned3(const char* label, ci::uvec3& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		constexpr unsigned n = 3;
		float slider_value[n];
		for (unsigned i = 0; i < n; i++) {
			CI_ASSERT_MSG(value[i] < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
			slider_value[n] = static_cast<float>(value[n]);
		}
		bool changed = SliderFloat3(label, slider_value, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		for (unsigned i = 0; i < n; i++) value[i] = static_cast<unsigned>(slider_value[i]);
		return changed;
	}
	inline bool SliderUnsigned4(const char* label, ci::uvec4& value, int value_minimum = 0, int value_maximum = 100, const char* display_format = "%.0f") {
		constexpr unsigned n = 4;
		float slider_value[n];
		for (unsigned i = 0; i < n; i++) {
			CI_ASSERT_MSG(value[i] < INT_MAX - 63, "CinderImGui: unsigned integer slider value exceeded limit"); // assuming 32-bit int and IEEE-754 float
			slider_value[n] = static_cast<float>(value[n]);
		}
		bool changed = SliderFloat4(label, slider_value, static_cast<float>(value_minimum), static_cast<float>(value_maximum), display_format, 1.0f);
		for (unsigned i = 0; i < n; i++) value[i] = static_cast<unsigned>(slider_value[i]);
		return changed;
	}

	// Scoped objects goodness (push the state when created and pop it when destroyed)

	struct ScopedWindow : public ci::Noncopyable {
		ScopedWindow(const std::string &name = "Debug", ImGuiWindowFlags flags = 0);
		ScopedWindow(const std::string &name, glm::vec2 size, float fillAlpha = -1.0f, ImGuiWindowFlags flags = 0);
		~ScopedWindow();
	};
	struct ScopedChild : public ci::Noncopyable {
		ScopedChild(const std::string &name, glm::vec2 size = glm::vec2(0), bool border = false, ImGuiWindowFlags extraFlags = 0);
		~ScopedChild();
	};
	struct ScopedGroup : public ci::Noncopyable {
		ScopedGroup();
		~ScopedGroup();
	};
	struct ScopedFont : public ci::Noncopyable {
		ScopedFont(ImFont* font);
		ScopedFont(const std::string &name);
		~ScopedFont();
	};
	struct ScopedStyleColor : public ci::Noncopyable {
		ScopedStyleColor(ImGuiCol idx, const ImVec4& col);
		~ScopedStyleColor();
	};
	struct ScopedStyleVar : public ci::Noncopyable {
		ScopedStyleVar(ImGuiStyleVar idx, float val);
		ScopedStyleVar(ImGuiStyleVar idx, const ImVec2 &val);
		~ScopedStyleVar();
	};
	struct ScopedItemWidth : public ci::Noncopyable {
		ScopedItemWidth(float itemWidth);
		~ScopedItemWidth();
	};
	struct ScopedTextWrapPos : public ci::Noncopyable {
		ScopedTextWrapPos(float wrapPosX = 0.0f);
		~ScopedTextWrapPos();
	};
	struct ScopedId : public ci::Noncopyable {
		ScopedId(const std::string &name);
		ScopedId(const void *ptrId);
		ScopedId(const int intId);
		~ScopedId();
	};
	struct ScopedMainMenuBar : public ci::Noncopyable {
		ScopedMainMenuBar();
		~ScopedMainMenuBar();
	protected:
		bool mOpened;
	};
	struct ScopedMenuBar : public ci::Noncopyable {
		ScopedMenuBar();
		~ScopedMenuBar();
	protected:
		bool mOpened;
	};

	// Getters/Setters Helpers Implementation

	template<typename T>
	bool InputText(const char* label, T *object, std::string(T::*get)() const, void(T::*set)(const std::string&), ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data) {
		std::string text = (object->*get)();
		if (InputText(label, &text, flags, callback, user_data)) {
			(object->*set)(text);
			return true;
		}
		return false;
	}
	template<typename T>
	bool Checkbox(const char* label, T *object, bool(T::*get)() const, void(T::*set)(bool)) {
		bool value = (object->*get)();
		if (Checkbox(label, &value)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool Combo(const char* label, T *object, int(T::*get)() const, void(T::*set)(int), const std::vector<std::string>& items, int height_in_items) {
		int value = (object->*get)();
		if (Combo(label, &value, items, height_in_items)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}

	template<typename T>
	bool DragFloat(const char* label, T *object, float(T::*get)() const, void(T::*set)(float), float v_speed, float v_min, float v_max, const char* display_format, float power) {
		float value = (object->*get)();
		if (DragFloat(label, &value, v_speed, v_min, v_max, display_format, power)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool DragFloat2(const char* label, T *object, ci::vec2(T::*get)() const, void(T::*set)(const ci::vec2&), float v_speed, float v_min, float v_max, const char* display_format, float power) {
		ci::vec2 value = (object->*get)();
		if (DragFloat2(label, &value[0], v_speed, v_min, v_max, display_format, power)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool DragFloat3(const char* label, T *object, ci::vec3(T::*get)() const, void(T::*set)(const ci::vec3&), float v_speed, float v_min, float v_max, const char* display_format, float power) {
		ci::vec3 value = (object->*get)();
		if (DragFloat3(label, &value[0], v_speed, v_min, v_max, display_format, power)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool DragFloat4(const char* label, T *object, ci::vec4(T::*get)() const, void(T::*set)(const ci::vec4&), float v_speed, float v_min, float v_max, const char* display_format, float power) {
		ci::vec4 value = (object->*get)();
		if (DragFloat4(label, &value[0], v_speed, v_min, v_max, display_format, power)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}

	template<typename T>
	bool DragInt(const char* label, T *object, int(T::*get)() const, void(T::*set)(int), float v_speed, int v_min, int v_max, const char* display_format) {
		int value = (object->*get)();
		if (DragInt(label, &value, v_speed, v_min, v_max, display_format)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool DragInt2(const char* label, T *object, ci::ivec2(T::*get)() const, void(T::*set)(const ci::ivec2&), float v_speed, int v_min, int v_max, const char* display_format) {
		ci::ivec2 value = (object->*get)();
		if (DragInt2(label, &value[0], v_speed, v_min, v_max, display_format)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool DragInt3(const char* label, T *object, ci::ivec3(T::*get)() const, void(T::*set)(const ci::ivec3&), float v_speed, int v_min, int v_max, const char* display_format) {
		ci::ivec3 value = (object->*get)();
		if (DragInt3(label, &value[0], v_speed, v_min, v_max, display_format)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
	template<typename T>
	bool DragInt4(const char* label, T *object, ci::ivec4(T::*get)() const, void(T::*set)(const ci::ivec4&), float v_speed, int v_min, int v_max, const char* display_format) {
		ci::ivec4 value = (object->*get)();
		if (DragInt4(label, &value[0], v_speed, v_min, v_max, display_format)) {
			(object->*set)(value);
			return true;
		}
		return false;
	}
}