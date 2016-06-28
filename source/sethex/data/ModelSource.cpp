#include "ModelSource.h"

using namespace cinder::geom;

namespace sethex {

	uint8 ModelSource::getAttribDims(Attrib attrib) const {
		switch (attrib) {
			case POSITION: return positions.empty() ? 0 : 3;
			case NORMAL: return normals.empty() ? 0 : 3;
			case COLOR: return colors.empty() ? 0 : 3;
			case TEX_COORD_0: return texinates.empty() ? 0 : 2;
			default: return 0;
		}
	}

	AttribSet ModelSource::getAvailableAttribs() const {
		AttribSet attribs;
		if (!positions.empty())attribs.insert(POSITION);
		if (!normals.empty()) attribs.insert(NORMAL);
		if (!colors.empty())attribs.insert(COLOR);
		if (!texinates.empty()) attribs.insert(TEX_COORD_0);
		return attribs;
	}

	void ModelSource::loadInto(Target* target, const AttribSet& requestedAttribs) const {

	}

}
