#include "MEPFragment.h"

#include <string>

using namespace na62;
namespace na62 {
namespace l0 {

MEPFragment::MEPFragment():
		rawData(nullptr), sourceID_(0), sourceSubID_(0)  {
}

MEPFragment::~MEPFragment() {
}

/*
 * The sourceID in the header of this MEP event
 */
uint_fast8_t MEPFragment::getSourceID() const {
	return sourceID_;
}

/*
 * The sourceSubID in the header of this MEP event
 */
uint_fast8_t MEPFragment::getSourceSubID() const {
	return sourceSubID_;
}

/*
 * The internally used number corresponding to the sourceID of this MEP event
 */
//uint_fast8_t MEPFragment::getSourceIDNum() const {
//	return SourceIDManager::sourceIDToNum(sourceID_);
//}

} /* namespace l0 */
} /* namespace na62 */
