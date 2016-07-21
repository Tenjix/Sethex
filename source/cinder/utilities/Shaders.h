#pragma once

#include <sstream>
#include <string>
#include <regex>

namespace shader {

	// inserts "addition" into "shader" at the next line after "#version ..."
	template <class Text>
	void insert(std::string& shader, Text&& text) {
		std::smatch match;
		if (regex_search(shader, match, std::regex("#version \\d+[^\\n]*\\n"))) {
			shader.insert(match.position() + match.length(), std::forward<Text>(text));
		}
	}

	// inserts "lines" into "shader" at the next line after "#version ..."
	// (adds a linebreak after each inserted line and before the first one)
	template <class... Types>
	void insert_lines(std::string& shader, Types&&... lines) {
		std::ostringstream stream;
		stream << '\n';
		auto list = { 0, (static_cast<void>(stream << std::forward<Types>(lines) << '\n'), 0) ... };
		insert(shader, stream.str());
	}

	// inserts "#define ..." statements for the passed "constants" into "shader" at the next line after "#version ..."
	// (adds a linebreak after each inserted statement and before the first one)
	template <class... Types>
	void define(std::string& shader, Types&&... constants) {
		std::ostringstream stream;
		auto list = { 0, (static_cast<void>(stream << "#define " << std::forward<Types>(constants) << '\n'), 0) ... };
		insert(shader, stream.str());
	}

	// inserts "#undef ..." statements for the passed "constants" into "shader" at the next line after "#version ..."
	// (adds a linebreak after each inserted statement and before the first one)
	template <class... Types>
	void undefine(std::string& shader, Types&&... constants) {
		std::ostringstream stream;
		auto list = { 0, (static_cast<void>(stream << "#undef " << std::forward<Types>(constants) << '\n'), 0) ... };
		insert(shader, stream.str());
	}

}