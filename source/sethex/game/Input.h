#pragma once

#include <sethex/Common.h>

namespace sethex {

	struct Mouse {
		unsigned2 position;
	};

	struct Input {
		Mouse mouse;
	};

}
