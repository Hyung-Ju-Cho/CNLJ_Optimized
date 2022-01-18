#include "stdafx.h"
#include "CGridCell.h"
#include "CTNR.h"
#include <ASSERT.h>

extern CTNR tnr; 
extern CStdioFile debug_out;
extern CFile g_file_dist_transit_vtxes_to_transit_vtxes;

CGridCell::CGridCell()
{
}


CGridCell::~CGridCell()
{
	m_inside_vtxes.clear();
	m_inside_vtxes_to_crossing_vtxes.clear();

	m_crossing_vtxes.clear();
	m_crossing_inner_vtxes.clear();
	m_crossing_outer_vtxes.clear();
	m_crossing_vtxes_to_transit_vtxes.clear();

	m_transit_vtxes.clear();
}


// 여기서 부터 확인하자... hyungju hyungju
void CGridCell::Set_rect(RECT & rect)
{
	this->m_rect = rect;

	this->m_inner_rect.left		= this->m_rect.left   -	(2 * tnr.get_cell_width());
	this->m_inner_rect.right	= this->m_rect.right  +	(2 * tnr.get_cell_width());
	this->m_inner_rect.top		= this->m_rect.top    -	(2 * tnr.get_cell_height());
	this->m_inner_rect.bottom	= this->m_rect.bottom +	(2 * tnr.get_cell_height());

	this->m_outer_rect.left		= this->m_rect.left   -	(4 * tnr.get_cell_width());
	this->m_outer_rect.right	= this->m_rect.right  +	(4 * tnr.get_cell_width());
	this->m_outer_rect.top		= this->m_rect.top    -	(4 * tnr.get_cell_height());
	this->m_outer_rect.bottom	= this->m_rect.bottom +	(4 * tnr.get_cell_height());

	ASSERT(m_rect.left <= m_rect.right && m_rect.top <= m_rect.bottom);
	ASSERT(m_inner_rect.left <= m_inner_rect.right && m_inner_rect.top <= m_inner_rect.bottom);
	ASSERT(m_outer_rect.left <= m_outer_rect.right && m_outer_rect.top <= m_outer_rect.bottom);


}


void CGridCell::Dist_inside_vtxes_to_crossing_vtxes(CTNR *ptnr)
{
	if (m_inside_vtxes.size() > 0 &&  m_crossing_vtxes.size() > 0)
		Dist_src_vtxes_to_dst_vtxes(ptnr, m_inside_vtxes, m_crossing_vtxes, m_inside_vtxes_to_crossing_vtxes);
}


void CGridCell::Dist_crossing_vtxes_to_transit_vtxes(CTNR *ptnr)
{
	if (m_crossing_vtxes.size() > 0 && m_transit_vtxes.size() > 0)
		Dist_src_vtxes_to_dst_vtxes(ptnr, m_crossing_vtxes, m_transit_vtxes, m_crossing_vtxes_to_transit_vtxes);
}


void CGridCell::Dist_src_vtxes_to_dst_vtxes(CTNR *ptnr, const set<int> & src_vtxes, const set<int> & dst_vtxes, map<int, vector<pair<int, float>>> & result)
{

	ASSERT(src_vtxes.size() > 0);
	ASSERT(dst_vtxes.size() > 0);

	for (auto src_vid : src_vtxes)
	{
		vector<pair<int, float>> dstvtx_dist;

		ptnr->ShortestPath(src_vid, dst_vtxes, dstvtx_dist);

		pair<map<int, vector<pair<int, float>>>::iterator, bool> ret;
		ret = result.insert(make_pair(src_vid, dstvtx_dist));
		assert(ret.second == true);

	}
}





void CGridCell::Find_transit_vtxes(CTNR *ptnr)
{
	for (auto src_vid : m_crossing_vtxes)
	{
		map<int, SPVertex> map_dstvtx_dist;

		ptnr->ShortestPath(src_vid, m_crossing_outer_vtxes, map_dstvtx_dist);
		Find_transit_vtxes(ptnr, src_vid, map_dstvtx_dist);
	}
}


void CGridCell::Find_transit_vtxes(CTNR *ptnr, int src_vid, map<int, SPVertex> & map_dstvtx_dist)
{

	map<int, float> transit_vtxes;

	for (auto & dst_vid : m_crossing_outer_vtxes)
	{
		vector<int> path;

		path = std::move(ptnr->Retrieve_ShortestPath(map_dstvtx_dist, src_vid, dst_vid));

		Retrieve_transit_vtxes(path);
	}

	//TRACE(_T("~~~~~~~~~~~~~ %d %d\n"), m_crossing_inner_vtxes.size(), m_transit_vtxes.size());

}

void CGridCell::Retrieve_transit_vtxes(const vector<int> & path)
{
	bool found = false;

	for (auto & vid : path)
	{
		if (m_crossing_inner_vtxes.find(vid) != m_crossing_inner_vtxes.end())
		{
			m_transit_vtxes.insert(vid);
			found = true;
			break;
		}

	}

	ASSERT(found == true);
}
