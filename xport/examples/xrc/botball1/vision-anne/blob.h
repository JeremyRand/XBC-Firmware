#ifndef _BLOB_H
#define _BLOB_H

// TODO
//
// *** Priority 1
//
// *** Priority 2:
//
// *** Priority 3:
//
// Sort blobs according to area
// Compute elongation, major/minor axes (SMoments::GetStats)
//
// *** Priority 4:
//
// Clean up code
// Think about heap management of CBlobs
// Think about heap management of SLinkedSegments
//
// *** Priority 5 (maybe never do):
// 
// Try small and large SMoments structure (small for segment)
// Try more efficient SSegment structure for lastBottom, nextBottom
//
// *** DONE
//
// DONE Make XRC LUT
// DONE Use XRC LUT
// DONE Optimize blob assy
// DONE Start compiling
// DONE Conditionally record segments
// DONE Ask rich about FP, trig
// Take segmented image in (DONE in imageserver.cc, ARW 10/7/04)
// Produce colored segmented image out (DONE in imageserver.cc, ARW 10/7/04)
// Draw blob stats in image out (DONE for centroid, bounding box
//                               in imageserver.cc, ARW 10/7/04)
// Delete segments when deleting blob (DONE, ARW 10/7/04)
// Check to see if we attach to multiple blobs  (DONE, ARW 10/7/04)
// Sort blobs according to area  (DONE, ARW 10/7/04)

#include <stdlib.h>
#include <assert.h>
//#include <memory.h>
#include <math.h>

// Uncomment this for verbose output for testing
//#include <iostream.h>

struct SMomentStats {
  int   area;
  // X is 0 on the left side of the image and increases to the right
  // Y is 0 on the top of the image and increases to the bottom
  float centroidX, centroidY;
  // angle is 0 to PI, in radians.
  // 0 points to the right (positive X)
  // PI/2 points downward (positive Y)
  float angle;
  float majorDiameter;
  float minorDiameter;
};

// Image size is 352x278
// Full-screen blob area is 97856
// Full-screen centroid is 176,139
// sumX, sumY is then 17222656, 13601984; well within 32 bits
struct SMoments {
// Skip major/minor axis computation when this is false
  static bool computeAxes;
  
  int area; // number of pixels
  int sumX; // sum of pixel x coords
  int sumY; // sum of pixel y coords
  // XX, XY, YY used for major/minor axis calculation
  long long sumXX; // sum of x^2 for each pixel
  long long sumYY; // sum of y^2 for each pixel
  long long sumXY; // sum of x*y for each pixel
  void Add(const SMoments &moments) {
    area += moments.area;
    sumX += moments.sumX;
    sumY += moments.sumY;
    if (computeAxes) {
      sumXX += moments.sumXX;
      sumYY += moments.sumYY;
      sumXY += moments.sumXY;
    }
  }
  void GetStats(SMomentStats &stats) {
    stats.area= area;
    stats.centroidX = (float)sumX / (float)area;
    stats.centroidY = (float)sumY / (float)area;

    if (computeAxes) {
      // Find the eigenvalues and eigenvectors for the 2x2 covariance matrix:
      //
      // | sum((x-|x|)^2)        sum((x-|x|)*(y-|y|)) |
      // | sum((x-|x|)*(y-|y|))  sum((y-|y|)^2)       |
      
      // Values= 0.5 * ((sumXX+sumYY) +- sqrt((sumXX+sumYY)^2-4(sumXXsumYY-sumXY^2)))
      // .5 * (xx+yy) +- sqrt(xx^2+2xxyy+yy^2-4xxyy+4xy^2)
      // .5 * (xx+yy) +- sqrt(xx^2-2xxyy+yy^2 + 4xy^2)

      // sum((x-|x|)^2) =
      // sum(x^2) - 2sum(x|x|) + sum(|x|^2) =
      // sum(x^2) - 2|x|sum(x) + n|x|^2 =
      // sumXX - 2*centroidX*sumX + centroidX*sumX =
      // sumXX - centroidX*sumX

      // sum((x-|x|)*(y-|y|))=
      // sum(xy) - sum(x|y|) - sum(y|x|) + sum(|x||y|) =
      // sum(xy) - |y|sum(x) - |x|sum(y) + n|x||y| =
      // sumXY - centroidY*sumX - centroidX*sumY + sumX * centroidY =
      // sumXY - centroidX*sumY
      
      float xx= sumXX - stats.centroidX*sumX;
      float xyTimes2= 2*(sumXY - stats.centroidX*sumY);
      float yy= sumYY - stats.centroidY*sumY;
      float xxMinusyy = xx-yy;
      float xxPlusyy = xx+yy;
      float sq = sqrt(xxMinusyy * xxMinusyy + xyTimes2*xyTimes2);
      float eigMaxTimes2= xxPlusyy+sq;
      float eigMinTimes2= xxPlusyy-sq;
      stats.angle= 0.5*atan2(xyTimes2, xxMinusyy);
      //float aspect= sqrt(eigMin/eigMax);
      //stats.majorDiameter= sqrt(area/aspect);
      //stats.minorDiameter= sqrt(area*aspect);
      //
      // sqrt(eigenvalue/area) is the standard deviation
      // Draw the ellipse with radius of twice the standard deviation,
      // which is a diameter of 4 times, which is 16x inside the sqrt
      
      stats.majorDiameter= sqrt(8.0*eigMaxTimes2/area);
      stats.minorDiameter= sqrt(8.0*eigMinTimes2/area);
    }
  }
  void Reset() {
    area= sumX= sumY= sumXX= sumYY= sumXY= 0;
  }
  bool operator==(const SMoments &rhs) const {
    if (area != rhs.area) return 0;
    if (sumX != rhs.sumX) return 0;
    if (sumY != rhs.sumY) return 0;
    if (computeAxes) {
      if (sumXX != rhs.sumXX) return 0;
      if (sumYY != rhs.sumYY) return 0;
      if (sumXY != rhs.sumXY) return 0;
    }
    return 1;
  }
};

struct SSegment {
  unsigned char  model    : 3 ; // which color channel
  unsigned short row      : 9 ;
  unsigned short startCol : 10; // inclusive
  unsigned short endCol   : 10; // inclusive

  const static short invalid_row= 0x1ff;

  // Sum 0^2 + 1^2 + 2^2 + ... + n^2 is (2n^3 + 3n^2 + n) / 6
  // Sum (a+1)^2 + (a+2)^2 ... b^2 is (2(b^3-a^3) + 3(b^2-a^2) + (b-a)) / 6
  //
  // Sum 0+1+2+3+...+n is (n^2 + n)/2
  // Sum (a+1) + (a+2) ... b is (b^2-a^2 + b-a)/2

  void GetMoments(SMoments &moments) const {
    int s= startCol - 1;
    int s2= s*s;
    int e= endCol;
    int e2= e*e;
    int y= row;
    
    moments.area  = (e-s);
    moments.sumX = ( (e2-s2) + (e-s) ) / 2;
    moments.sumY = (e-s) * y;

    if (SMoments::computeAxes) {
      int e3= e2*e;
      int s3= s2*s;
      moments.sumXY= moments.sumX*y;
      moments.sumXX= (2*(e3-s3) + 3*(e2-s2) + (e-s)) / 6;
      moments.sumYY= moments.sumY*y;
    }
  }
  
  void GetMomentsTest(SMoments &moments) const {
    moments.Reset();
    int y= row;
    for (int x= startCol; x <= endCol; x++) {
      moments.area++;
      moments.sumX += x;
      moments.sumY += y;
      if (SMoments::computeAxes) {
        moments.sumXY += x*y;
        moments.sumXX += x*x;
        moments.sumYY += y*y;
      }
    }
  }
};

struct SLinkedSegment {
  SSegment segment;
  SLinkedSegment *next;
  SLinkedSegment(const SSegment &segmentInit) :
    segment(segmentInit), next(NULL) {}
};

class CBlob {
  // These are at the beginning for fast inclusion checking
public:
  CBlob *next;            // next ptr for linked list

  // Bottom of blob, which is the surface we'll attach more segments to
  // If bottom of blob contains multiple segments, this is the smallest
  // segment containing the multiple segments
  SSegment lastBottom;

  // Next bottom of blob, currently under construction
  SSegment nextBottom;
  
  // Bounding box, inclusive.  nextBottom.row contains the "bottom"
  short left, top, right;

  void getBBox(short &leftRet, short &topRet,
               short &rightRet, short &bottomRet) {
    leftRet= left;
    topRet= top;
    rightRet= right;
    bottomRet= lastBottom.row;
  }
  
  // Segments which compose the blob
  // Only recorded if CBlob::recordSegments is true
  // firstSegment points to first segment in linked list
  SLinkedSegment *firstSegment;
  // lastSegmentPtr points to the next pointer field _inside_ the
  // last element of the linked list.  This is the field you would
  // modify in order to append to the end of the list.  Therefore
  // **lastSegmentPtr should always equal to NULL.
  // When the list is empty, lastSegmentPtr actually doesn't point inside
  // a SLinkedSegment structure at all but instead at the firstSegment
  // field above, which in turn is NULL.
  SLinkedSegment **lastSegmentPtr;

  SMoments moments;

  static bool recordSegments;
  // Set to true for testing code only.  Very slow!
  static bool testMoments;

  CBlob() {
    // Setup pointers
    firstSegment= NULL;
    lastSegmentPtr= &firstSegment;

    // Reset blob data
    Reset();
  }

  ~CBlob() {
    // Free segments, if any
    Reset();
  }

  // Clear blob data and free segments, if any
  void Reset() {
    // Clear blob data
    moments.Reset();

    // Empty bounds
    right = -1;
    left = top = 0x7fff;
    lastBottom.row = lastBottom.invalid_row;
    nextBottom.row = nextBottom.invalid_row;

    // Delete segments if any
    SLinkedSegment *tmp;
    while(firstSegment!=NULL) {
      tmp = firstSegment;
      firstSegment = tmp->next;
      delete tmp;
    }
    lastSegmentPtr= &firstSegment;
  }
    
  void NewRow() {
    if (nextBottom.row != nextBottom.invalid_row) {
      lastBottom= nextBottom;
      nextBottom.row= nextBottom.invalid_row;
    }
  }
  
  void Add(const SSegment &segment) {
    // Enlarge bounding box if necessary
    UpdateBoundingBox(segment.startCol, segment.row, segment.endCol);

    // Update next attachment "surface" at bottom of blob
    if (nextBottom.row == nextBottom.invalid_row) {
      // New row.
      nextBottom= segment;
    } else {
      // Same row.  Add to right side of nextBottom.
      nextBottom.endCol= segment.endCol;
    }
    
    SMoments segmentMoments;
    segment.GetMoments(segmentMoments);
    moments.Add(segmentMoments);

    if (testMoments) {
      SMoments test;
      segment.GetMomentsTest(test);
      assert(test == segmentMoments);
    }
    if (recordSegments) {
      // Add segment to the _end_ of the linked list
      *lastSegmentPtr= new SLinkedSegment(segment);
      lastSegmentPtr= &((*lastSegmentPtr)->next);
    }
  }

  // This takes futileResister and assimilates it into this blob
  //
  // Takes advantage of the fact that we are always assembling top to
  // bottom, left to right.
  //
  // Be sure to call like so:
  // leftblob.Assimilate(rightblob);
  //
  // This lets us assume two things:
  // 1) The assimilated blob contains no segments on the current row
  // 2) The assimilated blob lastBottom surface is to the right
  //    of this blob's lastBottom surface
  void Assimilate(CBlob &futileResister) {
    moments.Add(futileResister.moments);
    UpdateBoundingBox(futileResister.left,
                      futileResister.top,
                      futileResister.right);
    // Update lastBottom
    if (futileResister.lastBottom.endCol > lastBottom.endCol) {
      lastBottom.endCol= futileResister.lastBottom.endCol;
    }
    
    if (recordSegments) {
      // Take segments from futileResister, append on end
      *lastSegmentPtr= futileResister.firstSegment;
      lastSegmentPtr= futileResister.lastSegmentPtr;
      futileResister.firstSegment= NULL;
      futileResister.lastSegmentPtr= &futileResister.firstSegment;
      // Futile resister is left with no segments
    }
  }

  // Only updates left, top, and right.  bottom is updated 
  // by UpdateAttachmentSurface below
  void UpdateBoundingBox(int newLeft, int newTop, int newRight) {
    if (newLeft  < left ) left = newLeft;
    if (newTop   < top  ) top  = newTop;
    if (newRight > right) right= newRight;
  }
};

// Strategy for using CBlobAssembler:
//
// Make one CBlobAssembler for each color channel.
// CBlobAssembler ignores the model index, so you need to be sure to
// only pass the correct segments to each CBlobAssembler.
//
// At the beginning of a frame, call Reset() on each assembler
// As segments appear, call Add(segment)
// At the end of a frame, call EndFrame() on each assembler
// Get blobs from finishedBlobs.  Blobs will remain valid until
//    the next call to Reset(), at which point they will be deleted.
//
// To get statistics for a blob, do the following:
//  SMomentStats stats;
//  blob->moments.GetStats(stats);
// (See imageserver.cc: draw_blob() for an example)

class CBlobAssembler {
  short currentRow;
  
  // Active blobs, in left to right order
  // (Active means we are still potentially adding segments)
  CBlob *activeBlobs;

  // Current candidate for adding a segment to.  This is a member
  // of activeBlobs, and scans left to right as we search the active blobs.
  CBlob *currentBlob;
  
  // Pointer to pointer to current candidate, which is actually the pointer
  // to the "next" field inside the previous candidate, or a pointer to
  // the activeBlobs field of this object if the current candidate is the
  // first element of the activeBlobs list.  Used for inserting and
  // deleting blobs.
  CBlob **previousBlobPtr;

public:
  // Blobs we're no longer adding to
  CBlob *finishedBlobs;
  short maxRowDelta;
  static bool keepFinishedSorted;

public:
  CBlobAssembler() {
    activeBlobs= currentBlob= finishedBlobs= NULL;
    previousBlobPtr= &activeBlobs;
    currentRow=-1;
    maxRowDelta=1;
  }

  ~CBlobAssembler() {
    // Flush any active blobs into finished blobs
    EndFrame();
    // Free any finished blobs
    Reset();
  }

  // Call prior to starting a frame
  // Deletes any previously created blobs
  void Reset() {
    assert(!activeBlobs);
    while (finishedBlobs) {
      CBlob *tmp= finishedBlobs->next;
      delete finishedBlobs;
      finishedBlobs= tmp;
    }
  }


  // Manage currentBlob
  //
  // We always want to guarantee that both currentBlob
  // and currentBlob->next have had NewRow() called, and have
  // been validated to remain on the active list.  We could just
  // do this for all activeBlobs at the beginning of each row,
  // but it's less work to only do it on demand as segments come in
  // since it might allow us to skip blobs for a given row
  // if there are no segments which might overlap.
  
  // BlobNewRow:
  //
  // Tell blob there is a new row of data, and confirm that the
  // blob should still be on the active list by seeing if too many
  // rows have elapsed since the last segment was added.
  //
  // If blob should no longer be on the active list, remove it and
  // place on the finished list, and skip to the next blob.
  //
  // Call this either zero or one time per blob per row, never more.
  //
  // Pass in the pointer to the "next" field pointing to the blob, so
  // we can delete the blob from the linked list if it's not valid.
  
  void BlobNewRow(CBlob **ptr) {
    while (*ptr) {
      CBlob *blob= *ptr;
      blob->NewRow();
      if (currentRow - blob->lastBottom.row > maxRowDelta) {
        // Too many rows have elapsed.  Move it to the finished list.
        *ptr= blob->next;
        blob->next= finishedBlobs;
        finishedBlobs= blob;
      } else {
        // Blob is valid
        return;
      }
    }
  }
  
  void RewindCurrent() {
    BlobNewRow(&activeBlobs);
    previousBlobPtr= &activeBlobs;
    currentBlob= *previousBlobPtr;

    if (currentBlob) BlobNewRow(&currentBlob->next);
  }
  
  void AdvanceCurrent() {
    previousBlobPtr= &(currentBlob->next);
    currentBlob= *previousBlobPtr;
    if (currentBlob) BlobNewRow(&currentBlob->next);
  }
  
  // Call once for each segment in the color channel
  void Add(const SSegment &segment) {
    if (segment.row != currentRow) {
      // Start new row
      currentRow= segment.row;
      RewindCurrent();
    }
    
    // Try to link this to a previous blob
    while (currentBlob) {
      if (segment.startCol > currentBlob->lastBottom.endCol) {
        // Doesn't connect.  Keep searching more blobs to the right.
        AdvanceCurrent();
      } else {
        if (segment.endCol < currentBlob->lastBottom.startCol) {
          // Doesn't connect to any blob.  Stop searching.
          break;
        } else {
          // Found a blob to connect to
          currentBlob->Add(segment);
          // Check to see if we attach to multiple blobs
          while(currentBlob->next &&
                segment.endCol >= currentBlob->next->lastBottom.startCol) {
            // Can merge the current blob with the next one,
            // assimilate the next one and delete it.
            
            // Uncomment this for verbose output for testing
            // cout << "Merging blobs:" << endl
            //     << " curr: bottom=" << currentBlob->bottom
            //     << ", " << currentBlob->lastBottom.startCol
            //     << " to " << currentBlob->lastBottom.endCol
            //     << ", area " << currentBlob->moments.area << endl
            //     << " next: bottom=" << currentBlob->next->bottom
            //     << ", " << currentBlob->next->lastBottom.startCol
            //     << " to " << currentBlob->next->lastBottom.endCol
            //     << ", area " << currentBlob->next->moments.area << endl;
         
            CBlob *futileResister = currentBlob->next;
            // Cut it out of the list
            currentBlob->next = futileResister->next;
            // Assimilate it's segments and moments
            currentBlob->Assimilate(*(futileResister));

            // Uncomment this for verbose output for testing
            // cout << " NEW curr: bottom=" << currentBlob->bottom
            //     << ", " << currentBlob->lastBottom.startCol
            //     << " to " << currentBlob->lastBottom.endCol
            //     << ", area " << currentBlob->moments.area << endl;

            // Delete it
            delete futileResister;

            BlobNewRow(&currentBlob->next);
          }
          return;
        }
      }
    }
    
    // Could not attach to previous blob, insert new one before currentBlob
    CBlob *newBlob= new CBlob();
    newBlob->next= currentBlob;
    *previousBlobPtr= newBlob;
    previousBlobPtr= &newBlob->next;
    newBlob->Add(segment);
  }

  // Call at end of frame
  // Moves all active blobs to finished list
  // TODO: sort finished in descending area
  void EndFrame() {
    while (activeBlobs) {
      activeBlobs->NewRow();
      CBlob *tmp= activeBlobs->next;
      activeBlobs->next= finishedBlobs;
      finishedBlobs= activeBlobs;
      activeBlobs= tmp;
    }
  }
};

#endif // _BLOB_H
