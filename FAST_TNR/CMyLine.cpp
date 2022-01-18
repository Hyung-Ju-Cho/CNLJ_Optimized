#include "stdafx.h"
#include "CMyLine.h"


CMyLine::CMyLine()
{
}


CMyLine::~CMyLine()
{
}


CMyLine::CMyLine(POINT pt1, POINT pt2)
{
	ASSERT((pt1.x != pt2.x) || (pt1.y != pt2.y));
	this->m_pt1 = pt1;
	this->m_pt2 = pt2;
}


CMyLine::CMyLine(LONG pt1x, LONG pt1y, LONG pt2x, LONG pt2y)
{
	POINT pt1, pt2;

	pt1.x = pt1x;
	pt1.y = pt1y;
	pt2.x = pt2x;
	pt2.y = pt2y;

	ASSERT((pt1.x != pt2.x) || (pt1.y != pt2.y));

	this->m_pt1 = pt1;
	this->m_pt2 = pt2;
}

bool CMyLine::CheckCollinear(CMyLine & line)
{
	POINT start1, end1, start2, end2;

	start1 = this->m_pt1;
	end1 = this->m_pt2;

	start2 = line.m_pt1;
	end2 = line.m_pt2;


	if (start1.x == end1.x && start2.x == end2.x)
	{
		if (start1.x != start2.x) 
			return false; 

		if (min(start2.y, end2.y) <= start1.y && start1.y <= max(start2.y, end2.y))
			return true;

		if (min(start2.y, end2.y) <= end1.y && end1.y <= max(start2.y, end2.y))
			return true;

		if (min(start1.y, end1.y) <= start2.y && start2.y <= max(start1.y, end1.y))
			return true;

		if (min(start1.y, end1.y) <= end2.y && end2.y <= max(start1.y, end1.y))
			return true;

		return false;

	}
	else if (start1.y == end1.y && start2.y == end2.y)
	{
		if (start1.y != start2.y) 
			return false; 

		if (min(start2.x, end2.x) <= start1.x && start1.x <= max(start2.x, end2.x))
			return true;

		if (min(start2.x, end2.x) <= end1.x && end1.x <= max(start2.x, end2.x))
			return true;

		if (min(start1.x, end1.x) <= start2.x && start2.x <= max(start1.x, end1.x))
			return true;

		if (min(start1.x, end1.x) <= end2.x && end2.x <= max(start1.x, end1.x))
			return true;

		return false;

	}
	else
	{
		// We do not have to consider the other cases
		ASSERT(false);
	}

}


// 여기서부터 hyungju hyungju
bool CMyLine::IntersectLine(CMyLine & line)
{
	// [step1] check if two line segments coincide.



	
	POINT start1, end1, start2, end2;

	start1	= this->m_pt1;
	end1	= this->m_pt2;

	start2	= line.m_pt1;
	end2	= line.m_pt2;

	// 정확하게 동작하는지 확인은 못했음... T.T
	double ax = end1.x - start1.x;     // direction of line a
	double ay = end1.y - start1.y;     // ax and ay as above

	double bx = start2.x - end2.x;     // direction of line b, reversed
	double by = start2.y - end2.y;     // really -by and -by as above

	double dx = start2.x - start1.x;   // right-hand side
	double dy = start2.y - start1.y;

	double det = ax * by - ay * bx;

	if (det == 0) 
	{
		return CheckCollinear(line);
	}

	double r = (dx * by - dy * bx) / det;
	double s = (ax * dy - ay * dx) / det;

	if (r < 0 || r > 1 || s < 0 || s > 1)
	{
		return false;
	}
	else
	{
#ifdef _DEBUG
		double x1, y1, x2, y2, x3, y3, x4, y4;

		x1 = start1.x;
		y1 = start1.y;
		x2 = end1.x;
		y2 = end1.y;
		x3 = start2.x;
		y3 = start2.y;
		x4 = end2.x;
		y4 = end2.y;

		double res_x = ((x1*y2 - y1 * x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3 * x4)) / ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
		double res_y = ((x1*y2 - y1 * x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3 * x4)) / ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

		double e = 0.5;
		
		if (min(y1, y2) - e > res_y || res_y > max(y1, y2) + e)
		{
			TRACE(_T("%f %f %f\n"), y1, y2, res_y);
			TRACE(_T("%f %f %f\n"), y1, y2, res_y);
		}
		
		if (min(x3, x4) - e > res_x || res_x > max(x3, x4) + e)
		{
			TRACE(_T("%f %f %f\n"), x3, x4, res_x);
			TRACE(_T("%f %f %f\n"), x3, x4, res_x);
		}


		ASSERT(min(x1, x2) - e <= res_x && res_x <= max(x1, x2) + e);
		ASSERT(min(x3, x4) - e <= res_x && res_x <= max(x3, x4) + e);
		ASSERT(min(y1, y2) - e <= res_y && res_y <= max(y1, y2) + e);
		ASSERT(min(y3, y4) - e <= res_y && res_y <= max(y3, y4) + e);


#endif


		return true;
	}

	
	// return !(r < 0 || r > 1 || s < 0 || s > 1);

}


#if 0
bool intersection(double start1[2], double end1[2], double start2[2], double end2[2])
{
	float ax = end1[0] - start1[0];     // direction of line a
	float ay = end1[1] - start1[1];     // ax and ay as above

	float bx = start2[0] - end2[0];     // direction of line b, reversed
	float by = start2[1] - end2[1];     // really -by and -by as above

	float dx = start2[0] - start1[0];   // right-hand side
	float dy = start2[1] - start1[1];

	float det = ax * by - ay * bx;

	if (det == 0) return false;

	float r = (dx * by - dy * bx) / det;
	float s = (ax * dy - ay * dx) / det;

	return !(r < 0 || r > 1 || s < 0 || s > 1);
}
#endif