#pragma once
#include "afx.h"
//#include "DataObj.h"

#include <vector>
#include <map>


using namespace std;

class CMyNode
{
public:
	CMyNode(void);
	~CMyNode(void);

	CMyNode(int x, int y);

	int x;
	int y;
	int nodeid;


	vector<int> v_adj_nodes;


	map<int, float>	m_map_transit_vtx_dist;
	
};

