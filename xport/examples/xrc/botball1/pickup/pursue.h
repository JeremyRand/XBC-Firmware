#ifndef _PURSUE_H
#define _PURSUE_H

#include "vision.h"
#include "diffbase.h"
#include "pid.h"

#define DP_MAX_MEASUREMENTS		0x200
#define DP_MIN_AREA				200
#define DP_MAX_TRACK			25

class CPursue
	{
public:
	CPursue(CDiffBase *pdb, CVision *pVision);
	int Find(unsigned char model);
	int Pursue(int velocity);
	int Estimate(int *px, int *py);
	bool Track(CBlob *blob);

	// camera calibration table
	static const short m_calibLUT[];

	// measurments
	int m_trans[DP_MAX_MEASUREMENTS];
	int m_centroid[DP_MAX_MEASUREMENTS];

	Cpid m_heading;
	CVision *m_pVision;
	CDiffBase *m_pdb;
	unsigned char m_model;
	int m_measurements;
	int m_x, m_y;
	};

#endif
