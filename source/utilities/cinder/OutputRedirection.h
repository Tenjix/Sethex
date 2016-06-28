#pragma once

#include <sstream>
#include <iostream>
#include <cinder/app/AppBase.h>

namespace cinder {

	namespace utilities {

		class RedirectionBuffer : public std::stringbuf {

		protected:

			int sync() {
				cinder::app::console() << str();
				cinder::app::console().flush();
				str("");
				return 0;
			}

		};

		void redirectOutputStreams(RedirectionBuffer& redirection_buffer) {
			std::cout.rdbuf(&redirection_buffer);
			std::cerr.rdbuf(&redirection_buffer);
		}

	}

}