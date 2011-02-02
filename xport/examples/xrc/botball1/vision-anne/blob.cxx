#include "blob.h"

bool CBlob::recordSegments= false;
// Set to true for testing code only.  Very slow!
bool CBlob::testMoments= false;
// Skip major/minor axis computation when this is false
bool SMoments::computeAxes= false;

