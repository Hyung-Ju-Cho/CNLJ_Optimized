#pragma once

class SPVertex
{
public:
	SPVertex();
	SPVertex(float dist, int vid, int prev_vid);
	~SPVertex();

public:
	float get_dist() const { return m_dist; }
	int get_vid() const { return m_vid; }
	int get_prev_vid() const { return m_prev_vid; }

private:
	float m_dist;
	int m_vid;
	int m_prev_vid;
};


/*
typedef struct _DIST_VTX
{
	float dist;
	int vid;
	int prev_vid;

	_DIST_VTX(float _dist, int _vid, int _prev_vid)
	{
		dist = _dist;
		vid = _vid;
		prev_vid = _prev_vid;
	}

} SPVertex;
*/



// To compare two points
class myComparator
{
public:
	int operator() (SPVertex & lhs, SPVertex & rhs)
	{
		if (lhs.get_dist() < 0)
			exit(-111);
		if (rhs.get_dist() < 0)
			exit(-112);

		return lhs.get_dist() > rhs.get_dist();
	}
};