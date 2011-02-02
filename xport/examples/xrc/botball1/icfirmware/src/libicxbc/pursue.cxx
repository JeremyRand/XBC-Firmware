#include "pursue.h"
#include "textdisp.h"

const short CPursue::m_calibLUT[] = {
         470,
         463,
         455,
         447,
         440,
         432,
         424,
         416,
         409,
         401,
         393,
         385,
         378,
         370,
         362,
         354,
         346,
         339,
         331,
         323,
         315,
         307,
         300,
         292,
         284,
         276,
         268,
         260,
         253,
         245,
         237,
         229,
         221,
         213,
         205,
         197,
         190,
         182,
         174,
         166,
         158,
         150,
         142,
         134,
         126,
         118,
         110,
         102,
          94,
          86,
          79,
          71,
          63,
          55,
          47,
          39,
          31,
          23,
          15,
           7,
          -1,
          -9,
         -17,
         -25,
         -33,
         -42,
         -50,
         -58,
         -66,
         -74,
         -82,
         -90,
         -98,
        -106,
        -114,
        -122,
        -130,
        -138,
        -147,
        -155,
        -163,
        -171,
        -179,
        -187,
        -195,
        -203,
        -212,
        -220,
        -228,
        -236,
        -244,
        -252,
        -261,
        -269,
        -277,
        -285,
        -293,
        -302,
        -310,
        -318,
        -326,
        -335,
        -343,
        -351,
        -359,
        -368,
        -376,
        -384,
        -392,
        -401,
        -409,
        -417,
        -426,
        -434,
        -442,
        -450,
        -459,
        -467,
        -475,
        -484,
        -492,
        -500,
        -509,
        -517,
        -525,
        -534,
        -542,
        -551,
        -559,
        -567,
        -576,
        -584,
        -593,
        -601,
        -609,
        -618,
        -626,
        -635,
        -643,
        -652,
        -660,
        -669,
        -677,
        -685,
        -694,
        -702,
        -711,
        -719,
        -728,
        -736,
        -745,
        -753,
        -762,
        -770,
        -779,
        -788,
        -796,
        -805,
        -813,
        -822,
        -830,
        -839,
        -847,
        -856,
        -865,
        -873,
        -882,
        -890,
        -899,
        -908,
        -916,
        -925,
        -934,
        -942,
        -951,
        -959,
        -968,
        -977,
        -985,
        -994,
       -1003,
       -1012,
       -1020,
       -1029,
       -1038,
       -1046,
       -1055,
       -1064,
       -1073,
       -1081,
       -1090,
       -1099,
       -1108,
       -1116,
       -1125,
       -1134,
       -1143,
       -1151,
       -1160,
       -1169,
       -1178,
       -1187,
       -1195,
       -1204,
       -1213,
       -1222,
       -1231,
       -1240,
       -1248,
       -1257,
       -1266,
       -1275,
       -1284,
       -1293,
       -1302,
       -1311,
       -1319,
       -1328,
       -1337,
       -1346,
       -1355,
       -1364,
       -1373,
       -1382,
       -1391,
       -1400,
       -1409,
       -1418,
       -1427,
       -1436,
       -1445,
       -1454,
       -1463,
       -1472,
       -1481,
       -1490,
       -1499,
       -1508,
       -1517,
       -1526,
       -1535,
       -1544,
       -1553,
       -1562,
       -1571,
       -1580,
       -1589,
       -1598,
       -1608,
       -1617,
       -1626,
       -1635,
       -1644,
       -1653,
       -1662,
       -1671,
       -1681,
       -1690,
       -1699,
       -1708,
       -1717,
       -1726,
       -1736,
       -1745,
       -1754,
       -1763,
       -1773,
       -1782,
       -1791,
       -1800,
       -1809,
       -1819,
       -1828,
       -1837,
       -1846,
       -1856,
       -1865,
       -1874,
       -1884,
       -1893,
       -1902,
       -1912,
       -1921,
       -1930,
       -1940,
       -1949,
       -1958,
       -1968
	   };


CPursue::CPursue(CDiffBase *pdb, CVision *pVision) :
	m_heading(3000, 0, 0)
	{
	m_pdb = pdb;
	m_pVision = pVision;
	m_measurements = 43;
	}

int CPursue::Find(unsigned char model)
	{
	m_model = model;
	CBlob *blob;
	CBlob *blobs[DV_MODELS];
	SMomentStats stats;

	while(1)
		{
		m_pVision->GetBlobs(blobs);
		blob = blobs[m_model];
		if (blob)
			{
			blob->moments.GetStats(stats);
			if (stats.area>DP_MIN_AREA)
				{
				blob->moments.GetStats(stats);
				m_x = (int)stats.centroidX; 
				m_y = (int)stats.centroidY;
				return 0;
				}
			}
		}
	}

int CPursue::FindSlice(unsigned char model)
	{
	m_model = model;
	CBlob *blob;
	CBlob *blobs[DV_MODELS];
	SMomentStats stats;

	//while(1)
	//	{
		m_pVision->GetBlobs(blobs);
		blob = blobs[m_model];
		if (blob)
			{
			blob->moments.GetStats(stats);
			if (stats.area>DP_MIN_AREA)
				{
				blob->moments.GetStats(stats);
				m_x = (int)stats.centroidX; 
				m_y = (int)stats.centroidY;
				return 0;
				}
			}
	//	}
	
	return(1);
	}
	
int CPursue::FindSliceIC(CBlob *blob, SMomentStats *stats)
	{
	//m_model = model;
	//CBlob *blob;
	//CBlob *blobs[DV_MODELS];
	//SMomentStats stats;

	//while(1)
	//	{
		//m_pVision->GetBlobs(blobs);
		//blob = blobs[m_model];
		if (blob)
			{
			//blob->moments.GetStats(stats);
			if (stats->area>DP_MIN_AREA)
				{
				//blob->moments.GetStats(stats);
				m_x = (int)stats->centroidX; 
				m_y = (int)stats->centroidY;
				return 0;
				}
			}
	//	}
	
	return(1);
	}


int CPursue::Pursue(int velocity)
	{
	int hVel;
	int pos0, pos;
	bool res;
	CBlob *blob;
	CBlob *blobs[DV_MODELS];
	SMomentStats stats;

	m_measurements = 0;
	pos0 = m_pdb->GetPosition(0);
	while(1)
		{
		m_pVision->GetBlobs(blobs);
		pos = m_pdb->GetPosition(0)-pos0;
		blob = blobs[m_model];

		hVel = 0;
		res = Track(blob);
		if (res)
			{
			hVel = m_heading.Compensate(170-m_x);
			printf("%d %d\n", m_y, pos);
			if (m_measurements<DP_MAX_MEASUREMENTS)
				{
				m_trans[m_measurements] = pos;
				m_centroid[m_measurements++] = m_y;
				}
			}

		m_pdb->MoveVelocity(0, velocity, 5000);
		m_pdb->MoveVelocity(1, hVel, 20000);
		m_pdb->Execute();

		if (!res) 
			return m_measurements>25 ? 0 : -1;
		}
	}
	
int CPursue::PursueSlice(int velocity)
	{
	int hVel;
	int pos0, pos;
	bool res;
	CBlob *blob;
	CBlob *blobs[DV_MODELS];
	SMomentStats stats;

	m_measurements = 0;
	pos0 = m_pdb->GetPosition(0);
	//while(1)
	//	{
		m_pVision->GetBlobs(blobs);
		pos = m_pdb->GetPosition(0)-pos0;
		blob = blobs[m_model];

		hVel = 0;
		res = Track(blob);
		if (res)
			{
			hVel = m_heading.Compensate(170-m_x);
			printf("%d %d\n", m_y, pos);
			if (m_measurements<DP_MAX_MEASUREMENTS)
				{
				m_trans[m_measurements] = pos;
				m_centroid[m_measurements++] = m_y;
				}
			}

		m_pdb->MoveVelocity(0, velocity, 5000);
		m_pdb->MoveVelocity(1, hVel, 20000);
		m_pdb->Execute();

		if (!res) 
			return m_measurements>25 ? 0 : -1;
	//	}
	
	return(1);
	}	

int CPursue::Estimate(int *px, int *py)
	{
	int i;
	long long A11=0, A12=0, A22=0;
	long long b1=0, b2=0;
	long long d, x, y;
	short slope;
	long long slope2;

	for (i=0; i<m_measurements; i++)
		{
		// only evaluate measurements at the bottom of image
		if (m_centroid[i]>=220 && m_centroid[i]<=270)
			{
			slope = m_calibLUT[m_centroid[i]];
			slope2 = slope*slope;
			A11 += slope2;
			A12 += -slope;
			A22 ++;
			b1 += slope2*m_trans[i];
			b2 += -slope*m_trans[i];
			}
		}
	d = -A11*A22 + A12*A12;

	if (px)
		{
		x = (-A22*b1 + A12*b2)/d;
		*px = (int)x;
		}
	if (py)
		{
		y = ((A12*b1 - A11*b2)/d)>>12;
		*py = (int)y;
		}
	return 0;
	}	

bool CPursue::Track(CBlob *blob)
	{
	SMomentStats stats;
	int x, y, dx, dy;

	while(blob)
		{
		blob->moments.GetStats(stats);
		if (stats.area>DP_MIN_AREA)
			{
			x = (int)stats.centroidX; 
			y = (int)stats.centroidY;
			
			dx = x - m_x;
			dx = dx<0 ? -dx : dx;
			dy = y - m_y;
			dy = dy<0 ? -dy : dy;
			
			if (dx<=DP_MAX_TRACK && dy<=DP_MAX_TRACK)
				{
				printf("dx %d dy %d\n", dx, dy);
				m_x = x;
				m_y = y;
				return true;
				}
			}
		blob = blob->next;
		}
	return false;
	}

