#pragma once

#include <vector>
#include <set>
#include <map>
#include "SPVertex.h"

using namespace std;

class CTNR;

class CGridCell
{
public:
	CGridCell();
	~CGridCell();

	void Set_rect(RECT & rect);
	const RECT & Get_rect()			{ return m_rect; };
	const RECT & Get_inner_rect()	{ return m_inner_rect; };
	const RECT & Get_outer_rect()	{ return m_outer_rect; };

	void Find_transit_vtxes(CTNR *ptnr);
	void Find_transit_vtxes(CTNR *ptnr, int src_vid, map<int, SPVertex> & map_dstvtx_dist);

	void Retrieve_transit_vtxes(const vector<int> & path);

	void Dist_inside_vtxes_to_crossing_vtxes(CTNR *ptnr);
	void Dist_crossing_vtxes_to_transit_vtxes(CTNR *ptnr);

	void Dist_src_vtxes_to_dst_vtxes(CTNR *ptnr, const set<int> & src_vtxes, const set<int> & dst_vtxes, map<int, vector<pair<int, float>>> & result);

private:
	RECT m_rect;
	RECT m_inner_rect;
	RECT m_outer_rect;

	static int s_cell_width;
	static int s_cell_height;


public:
	// set<int> m_inside_edges; 
	set<int> m_inside_vtxes;
	
	set<int> m_crossing_vtxes;
	//set<int> m_crossing_edges;
	
	set<int> m_crossing_inner_vtxes;
	set<int> m_crossing_outer_vtxes;

	//map<int, float> m_transit_vtxes;
	set<int> m_transit_vtxes;
	                                     
	map<int, vector<pair<int, float>>> m_inside_vtxes_to_crossing_vtxes;
	map<int, vector<pair<int, float>>> m_crossing_vtxes_to_transit_vtxes;



};

