#include "stdafx.h"
#include "CTNR.h"
#include "CGridCell.h"
#include "CMyLine.h"
#include "SPVertex.h"
#include "DataSeg.h"
#include <functional>
#include <queue>
#include <limits>
#include <assert.h>
#include <set>

extern CStdioFile debug_out;
extern CFile g_file_dist_transit_vtxes_to_transit_vtxes;

static int g_num_seqs = 0; // sequence 개수를 저장한다.


void Print_debug_out(CString & str, bool print)
{
	if (print == true)
	{
		debug_out.WriteString(str);
		debug_out.Flush();
		TRACE(str);
	}
}

bool Inside(const RECT & rect, const POINT & pt)
{
	ASSERT(rect.left <= rect.right);
	ASSERT(rect.top <= rect.bottom);

	bool chk1 = (rect.left <= pt.x && pt.x <= rect.right);
	bool chk2 = (rect.top <= pt.y && pt.y <= rect.bottom);

	return chk1 && chk2;

}

LARGE_INTEGER start_time()
{
	LARGE_INTEGER sPos, freq;
	
	//타이머의 주기를 얻어서 freq에 저장
	QueryPerformanceFrequency(&freq);

	//시작시간 저장
	QueryPerformanceCounter(&sPos);

	return sPos;
}

extern int g_num_road_dist;
extern CTNR tnr;
extern int g_num_kfn_queries;

void elapsed_time(CString & msg, LARGE_INTEGER & sPos, bool print)
{
	LARGE_INTEGER ePos, freq;

	//타이머의 주기를 얻어서 freq에 저장
	QueryPerformanceFrequency(&freq);

	//끝나는 시간
	QueryPerformanceCounter(&ePos);

	//밀리세컨드 출력
	__int64 ms_interval = (ePos.QuadPart - sPos.QuadPart) / (freq.QuadPart / 1000);

	if (print == true)
	{
		CString str;
		str.Format(_T("%s : %.2f s %d %d %d \n"), \
			msg.GetBuffer(), (float)ms_interval / (float)1000, tnr.get_map_qry().size(), tnr.get_map_qry_seg().size(), g_num_kfn_queries);
		Print_debug_out(str, print);
	}

}

CTNR::CTNR()
	:m_grid(0,0)
{

}


CTNR::~CTNR()
{
	debug_out.Close();
}


void CTNR::Draw_grids()
{
	TRACE(_T("test here \n"));
	for (int xgrid = 0; xgrid < this->m_num_X_grids; xgrid++)	
		for (int ygrid = 0; ygrid < this->m_num_Y_grids; ygrid++)	
		{
			RECT rect;
			
			rect.left	= xgrid * this->m_cell_width;
			rect.top	= ygrid * this->m_cell_height; 
			rect.right	= (xgrid + 1 == m_num_X_grids)? this->m_xmax : (xgrid + 1) * this->m_cell_width;
			rect.bottom = (ygrid + 1 == m_num_Y_grids)? this->m_ymax : (ygrid + 1) * this->m_cell_height;

			ASSERT(rect.left <= rect.right);
			ASSERT(rect.top <= rect.bottom);

			CGridCell & gridcell = m_grid[xgrid][ygrid];
			gridcell.Set_rect(rect);
		}

}


void CTNR::Find_crossing_edges_in_rect(const RECT & extent, set<int> & crossing_edges)
{
	map<int, pair<MYVERTEX, MYVERTEX>>::iterator it;

	CMyLine extleft(extent.left, extent.bottom, extent.left, extent.top);
	CMyLine extright(extent.right, extent.bottom, extent.right, extent.top);
	CMyLine exttop(extent.left, extent.top, extent.right, extent.top);
	CMyLine extbottom(extent.left, extent.bottom, extent.right, extent.bottom);

	// rtree에 edge정보를 추가한다.
	point minpt(min(extent.left, extent.right), min(extent.bottom, extent.top));
	point maxpt(max(extent.left, extent.right), max(extent.bottom, extent.top));

	box query_box(minpt, maxpt);

	vector<value> result_intersects;
	//m_rtree.query(bgi::within(query_box), back_inserter(result_s));
	m_rtree.query(bgi::intersects(query_box), back_inserter(result_intersects));

	for (const auto &v : result_intersects) // access by reference to avoid copying
	{
		box b = v.first;
		int eid = v.second;

		map<int, CMyEdge>::iterator it = this->m_mapMyEdge.find(eid);
		ASSERT(it != this->m_mapMyEdge.end());
		ASSERT(it->first == eid);

		map<int, CMyNode>::iterator pos1 = m_mapMyNode.find(it->second.node1);
		map<int, CMyNode>::iterator pos2 = m_mapMyNode.find(it->second.node2);

		ASSERT(pos1 != m_mapMyNode.end());
		ASSERT(pos2 != m_mapMyNode.end());
				
		POINT pt1, pt2;

		pt1.x = pos1->second.x;
		pt1.y = pos1->second.y;

		pt2.x = pos2->second.x;
		pt2.y = pos2->second.y;
		

		CMyLine edgeline(pt1, pt2);

		bool res1, res2, res3, res4;

		res1 = extleft.IntersectLine(edgeline);
		res2 = extright.IntersectLine(edgeline);
		res3 = exttop.IntersectLine(edgeline);
		res4 = extbottom.IntersectLine(edgeline);

		if (res1 || res2 || res3 || res4)
		{
			crossing_edges.insert(eid);
		}
	}
	// 여기서 부터, inside 조건으로 검색하자. 화이팅!! hyungju hyungju
	TRACE(_T("num of crossing edges2 = %d %d\n\n\n"), result_intersects.size(), crossing_edges.size());
	TRACE(_T("test here \n"));


}

void CTNR::Find_edges_inside_rect(const RECT & extent, set<int> & inside_vertexes)
{
	map<int, pair<MYVERTEX, MYVERTEX>>::iterator it;

	CMyLine extleft(extent.left, extent.bottom, extent.left, extent.top);
	CMyLine extright(extent.right, extent.bottom, extent.right, extent.top);
	CMyLine exttop(extent.left, extent.top, extent.right, extent.top);
	CMyLine extbottom(extent.left, extent.bottom, extent.right, extent.bottom);

	// rtree에 edge정보를 추가한다.
	point minpt(min(extent.left, extent.right), min(extent.bottom, extent.top));
	point maxpt(max(extent.left, extent.right), max(extent.bottom, extent.top));

	box query_box(minpt, maxpt);

	//vector<value> result_within;
	//m_rtree.query(bgi::within(query_box), back_inserter(result_within));

	vector<value> result_intersects;
	m_rtree.query(bgi::intersects(query_box), back_inserter(result_intersects));

	for (const auto &v : result_intersects) // access by reference to avoid copying
	{
		box b = v.first;
		int eid = v.second;

		map<int, CMyEdge>::iterator it = this->m_mapMyEdge.find(eid);
		ASSERT(it != this->m_mapMyEdge.end());
		ASSERT(it->first == eid);

		CMyEdge & edge = get_edge_from_tbl(eid);

		CMyNode & node1 = this->get_node_from_tbl(edge.node1);
		CMyNode & node2 = this->get_node_from_tbl(edge.node2);

		POINT pt1, pt2;

		pt1.x = node1.x;
		pt1.y = node1.y;

		pt2.x = node2.x;
		pt2.y = node2.y;

		if (Inside(extent, pt1) == true)
		{
			inside_vertexes.insert(node1.nodeid);
		}

		if (Inside(extent, pt2) == true)
		{
			inside_vertexes.insert(node2.nodeid);
		}
	}
}

void CTNR::Pick_crossing_vertexes(const set<int> & crossing_edges, set<int> & crossing_vertexes)
{
	ASSERT(crossing_vertexes.size() == 0);

	for (auto eid : crossing_edges)
	{
		map<int, CMyEdge>::const_iterator it_edge = this->m_mapMyEdge.find(eid);

		ASSERT(it_edge != this->m_mapMyEdge.end());
		ASSERT(it_edge->first == eid);

		//crossing_vertexes.insert(min(it_edge->second.node1, it_edge->second.node2));

		CMyNode & node1 = get_node_from_tbl(it_edge->second.node1);
		CMyNode & node2 = get_node_from_tbl(it_edge->second.node2);

		if (node1.v_adj_nodes.size() > node2.v_adj_nodes.size())
		{
			crossing_vertexes.insert(it_edge->second.node1);
		}
		else
		{
			crossing_vertexes.insert(it_edge->second.node2);
		}



	}
}



pair<int,int> CTNR::Get_cell_ID(const POINT & pt)
{
	pair<int, int> result;

	int xgrid = pt.x / m_cell_width;
	int ygrid = pt.y / m_cell_height;



	if (xgrid >= this->m_num_X_grids)
	{
		xgrid = this->m_num_X_grids - 1;


	}

	if (ygrid >= this->m_num_Y_grids)
	{
		ygrid = this->m_num_Y_grids - 1;
	}



	CGridCell & cell = m_grid[xgrid][ygrid];

	const RECT & rect = cell.Get_rect();

	ASSERT(Inside(rect, pt) == true);

	return make_pair(xgrid, ygrid);
}

int cntcntcnt = 0;

float CTNR::Comp_network_dist(int src_vid, int dst_vid)
{
	int edgeid = get_edge_id(src_vid, dst_vid);
	float dist;

	if (edgeid != -1)
	{
		ASSERT(m_mapMyEdge.find(edgeid)->second.edgelen > 0);
		return m_mapMyEdge.find(edgeid)->second.edgelen;
	}

	map< pair<int, int>, float>::iterator pos = m_bnode_pair_dist.find(make_pair(min(src_vid, dst_vid), max(src_vid, dst_vid)));
	if (pos != m_bnode_pair_dist.end())
	{
		return pos->second;
	}




	CMyNode & src_node = this->get_node_from_tbl(src_vid);
	CMyNode & dst_node = this->get_node_from_tbl(dst_vid);

	POINT src_pt, dst_pt;
	src_pt.x = src_node.x;
	src_pt.y = src_node.y;

	dst_pt.x = dst_node.x;
	dst_pt.y = dst_node.y;


	pair<int,int> & result = Get_cell_ID(src_pt);

	int xgrid = result.first;
	int ygrid = result.second;
	
	CGridCell & src_cell = m_grid[xgrid][ygrid];

	const RECT & outer_rect = src_cell.Get_outer_rect();


	cntcntcnt++ ;

	if (Inside(outer_rect, dst_pt) == true)
	{
		dist = Search_network_dist(src_vid, dst_vid);
		
	}
	else
	{
		dist = Lookup_network_dist(src_vid, dst_vid);
	}
	

	pair<int, int> key = make_pair(min(src_vid, dst_vid), max(src_vid, dst_vid));

	m_bnode_pair_dist.insert(make_pair(key, dist));

	return dist;


}

float CTNR::Lookup_network_dist(int src_vid, int dst_vid)
{
	float dist = numeric_limits<float>::infinity();  

	CMyNode & src_node = this->get_node_from_tbl(src_vid);
	CMyNode & dst_node = this->get_node_from_tbl(dst_vid);

	POINT src_pt, dst_pt;
	src_pt.x = src_node.x;
	src_pt.y = src_node.y;

	dst_pt.x = dst_node.x;
	dst_pt.y = dst_node.y;

	pair<int,int> & src_cell_id = Get_cell_ID(src_pt);
	pair<int,int> & dst_cell_id = Get_cell_ID(dst_pt);

	int src_xgrid = src_cell_id.first;
	int src_ygrid = src_cell_id.second;
	
	int dst_xgrid = dst_cell_id.first; 
	int dst_ygrid = dst_cell_id.second;
	
	CGridCell & src_cell = m_grid[src_xgrid][src_ygrid];
	CGridCell & dst_cell = m_grid[dst_xgrid][dst_ygrid];

	

	assert(src_cell.m_transit_vtxes.size() > 0);
	assert(dst_cell.m_transit_vtxes.size() > 0);


	// 여기서 부터 코딩한다. 
	for (auto & src_transit_vtx : src_cell.m_transit_vtxes)
		for (auto & dst_transit_vtx : dst_cell.m_transit_vtxes)
		{
			map<pair<int, int>, float>::iterator pos;
			pos = m_dist_transit_vtxes_to_transit_vtxes.find(make_pair(min(src_transit_vtx, dst_transit_vtx), max(src_transit_vtx, dst_transit_vtx)));

			float dist_transit_vtx_to_transit_vtx = pos->second;
			
			ASSERT(pos != m_dist_transit_vtxes_to_transit_vtxes.end());

			map<int, float>::const_iterator src_pos = src_node.m_map_transit_vtx_dist.find(src_transit_vtx);
			map<int, float>::const_iterator dst_pos = dst_node.m_map_transit_vtx_dist.find(dst_transit_vtx);
			

			ASSERT(src_pos != src_node.m_map_transit_vtx_dist.end());
			ASSERT(dst_pos != dst_node.m_map_transit_vtx_dist.end());

			float tmp_dist = src_pos->second + dist_transit_vtx_to_transit_vtx + dst_pos->second;
			dist = min(dist, tmp_dist);

		}

	ASSERT(dist != numeric_limits<float>::infinity());

	return dist;
}


float CTNR::Search_network_dist(int src_vid, int dst_vid)
{
	set<int> dst_vtxes;

	dst_vtxes.insert(dst_vid);

	map<int, SPVertex> map_dstvtx_dist;
	
	ShortestPath(src_vid, dst_vtxes, map_dstvtx_dist);

	map<int, SPVertex>::iterator pos = map_dstvtx_dist.find(dst_vid);
	ASSERT(pos != map_dstvtx_dist.end());

	return pos->second.get_dist();

}

void CTNR::Dist_transit_vtxes_to_transit_vtxes(set<int> & all_transit_vtxes)
{
	///* -------------------------------------- */
	//CString str;
	//str.Format(_T("** start Dist_transit_vtxes_to_transit_vtxes %d \n"), all_transit_vtxes.size());
	//Print_debug_out(str);
	///* -------------------------------------- */

	for (set<int>::iterator src_itr = all_transit_vtxes.begin(); src_itr != all_transit_vtxes.end(); src_itr++)
	{
		set<int> dst_vtxes;
		map<int, float> map_dstvtx_dist;

		int src_vid = *src_itr;

		for (set<int>::iterator dst_itr = src_itr; dst_itr != all_transit_vtxes.end(); dst_itr++)
		{
			if (*dst_itr > src_vid) { dst_vtxes.insert(*dst_itr); }
		}

		/* -------------------------------------- */
		//str.Format(_T("** before ShortestPath %d %d %d \n"), src_vid, dst_vtxes.size(), map_dstvtx_dist.size());
		//Print_debug_out(str);
		/* -------------------------------------- */

		ShortestPath(src_vid, dst_vtxes, map_dstvtx_dist);

		/* -------------------------------------- */
		//str.Format(_T("** after ShortestPath %d %d %d \n"), src_vid, dst_vtxes.size(), map_dstvtx_dist.size());
		//Print_debug_out(str);
		/* -------------------------------------- */


		for (auto dst_vtx : dst_vtxes)
		{
			map<int, float>::iterator it = map_dstvtx_dist.find(dst_vtx);
			ASSERT(it != map_dstvtx_dist.end());

			int dst_vid = it->first;
			int dist = (int)it->second;

			g_file_dist_transit_vtxes_to_transit_vtxes.Write(&src_vid, sizeof(int));
			g_file_dist_transit_vtxes_to_transit_vtxes.Write(&dst_vid, sizeof(int));
			g_file_dist_transit_vtxes_to_transit_vtxes.Write(&dist, sizeof(int));


		}
		g_file_dist_transit_vtxes_to_transit_vtxes.Flush();
		

		map_dstvtx_dist.clear();
		dst_vtxes.clear();
	}
}


void CTNR::Retrieve_dist_transit_vtxes_to_transit_vtxes(CFile & input)
{
	int src_vtx;
	int dst_vtx;
	int dist;

	while (true)
	{
		if (input.Read(&src_vtx, sizeof(int)) != sizeof(int))
			break;

		if (input.Read(&dst_vtx, sizeof(int)) != sizeof(int))
			break;

		if (input.Read(&dist, sizeof(int)) != sizeof(int))
			break;


		ASSERT(src_vtx <= dst_vtx);

		pair<map<pair<int, int>, float>::iterator, bool> ret;
		ret = m_dist_transit_vtxes_to_transit_vtxes.insert(make_pair(make_pair(min(src_vtx, dst_vtx), max(src_vtx, dst_vtx)), (float)dist));

		ASSERT(ret.second == true);

	}

	////////////////////////////////////////////////////////
	//CString str;
	//str.Format(_T("** m_dist_transit_vtxes_to_transit_vtxes.size() = %d \n"), m_dist_transit_vtxes_to_transit_vtxes.size());
	//Print_debug_out(str);
	//exit(-89);
	////////////////////////////////////////////////////////



}



void CTNR::Write_dist_transit_vtxes_to_transit_vtxes(CFile & output)
{
	int src_vtx;
	int dst_vtx;
	int dist;

	for (auto & x : m_dist_transit_vtxes_to_transit_vtxes)
	{
		src_vtx = x.first.first;
		dst_vtx = x.first.second;
		dist = x.second;

		ASSERT(src_vtx <= dst_vtx);

		// 쓰는 순서에 유의하시오.
		output.Write(&src_vtx, sizeof(int));
		output.Write(&dst_vtx, sizeof(int));
		output.Write(&dist, sizeof(int));
	}

	output.Flush();
	output.Flush();
	output.Flush();

	m_dist_transit_vtxes_to_transit_vtxes.clear();
}


void CTNR::Dist_inside_vtxes_to_transit_vtxes(CGridCell & cell)
{
	for (auto & inside_vtx : cell.m_inside_vtxes)
	{
		set<int> & transit_vtxes = cell.m_transit_vtxes;
		CMyNode & node = get_node_from_tbl(inside_vtx);

		node.m_map_transit_vtx_dist.clear();

		ShortestPath(inside_vtx, transit_vtxes, node.m_map_transit_vtx_dist);

		set_node_to_tbl(node);

	}
}


void CTNR::Dist_inside_vtxes_to_transit_vtxes()
{
	// 디버깅을 위하여 임시로 코멘트 처리했음
	for (int xgrid = 0; xgrid < this->m_num_X_grids; xgrid++)
		for (int ygrid = 0; ygrid < this->m_num_Y_grids; ygrid++)
		{
			CGridCell & cell = m_grid[xgrid][ygrid];
			Dist_inside_vtxes_to_transit_vtxes(cell);
		}
}

void CTNR::Store_dist_transit_vtxes_to_transit_vtxes()
{

	set<int> all_transit_vtxes;

	// 디버깅을 위하여 임시로 코멘트 처리했음
	for (int xgrid = 0; xgrid < this->m_num_X_grids; xgrid++)
	{
		for (int ygrid = 0; ygrid < this->m_num_Y_grids; ygrid++)
		{
			CGridCell & cell = m_grid[xgrid][ygrid];

			all_transit_vtxes.insert(cell.m_transit_vtxes.begin(), cell.m_transit_vtxes.end());


		}
	}


	m_grid.clear();





	Dist_transit_vtxes_to_transit_vtxes(all_transit_vtxes);



	all_transit_vtxes.clear();

}

void CTNR::Dist_transit_vtxes_to_transit_vtxes()
{
	if (m_read_dist_transit_vtxes == true)
	{
		Retrieve_dist_transit_vtxes_to_transit_vtxes(g_file_dist_transit_vtxes_to_transit_vtxes);
		g_file_dist_transit_vtxes_to_transit_vtxes.Close();
	}
	else
	{
		Store_dist_transit_vtxes_to_transit_vtxes();
		g_file_dist_transit_vtxes_to_transit_vtxes.Close();
		exit(0);
	}
	
}


// 여기서 부터 hyungju hyungju 2018년 9월 17일
// 거리를 정확하게 계산하는지 확인한다. hyungju hyungju
void CTNR::Find_transit_vtxes_for_cell()
{
// 디버깅을 위하여 임시로 코멘트 처리했음
	for (int xgrid = 0; xgrid < this->m_num_X_grids; xgrid++)
		for (int ygrid = 0; ygrid < this->m_num_Y_grids; ygrid++)
		{

			CGridCell & cell = m_grid[xgrid][ygrid];
			cell.Find_transit_vtxes(this);


			// TRACE(_T("~~~~~~~~~~~~~ %d %d %d %d\n"), \
			//	cell.m_crossing_vtxes.size(), cell.m_crossing_inner_vtxes.size(), cell.m_crossing_outer_vtxes.size(), cell.m_transit_vtxes.size());

			cell.Dist_inside_vtxes_to_crossing_vtxes(this);
			cell.Dist_crossing_vtxes_to_transit_vtxes(this);


			/* -------------------------------------- */
			//CString str;
			//str.Format(_T("******* 	Find_transit_vtxes_for_cell %d %d \n"), xgrid, ygrid);
			//Print_debug_out(str);
			/* -------------------------------------- */
			////////////////////////////////////////////////////////
			//CString tt;
			//tt.Format(_T("*** %d %d \n"), xgrid, ygrid);
			//OutputDebugString(tt);
			////////////////////////////////////////////////////////




		}

}

void CTNR::Find_crossing_edges_in_cell()
{
	set<int> crossing_edges;
	set<int> crossing_inner_edges;
	set<int> crossing_outer_edges;

	// 디버깅을 위하여 임시로 코멘트 처리했음
	for (int xgrid = 0; xgrid < this->m_num_X_grids; xgrid++)
		for (int ygrid = 0; ygrid < this->m_num_Y_grids; ygrid++)
		{
			CGridCell & cell = m_grid[xgrid][ygrid]; 
			
			crossing_edges.clear();
			crossing_inner_edges.clear();
			crossing_outer_edges.clear();

			ASSERT(crossing_edges.empty() == true);
			ASSERT(crossing_inner_edges.empty() == true);
			ASSERT(crossing_outer_edges.empty() == true);

			Find_crossing_edges_in_rect(cell.Get_rect(), crossing_edges);
			Pick_crossing_vertexes(crossing_edges, cell.m_crossing_vtxes);

			Find_crossing_edges_in_rect(cell.Get_inner_rect(), crossing_inner_edges);
			Pick_crossing_vertexes(crossing_inner_edges, cell.m_crossing_inner_vtxes);

			Find_crossing_edges_in_rect(cell.Get_outer_rect(), crossing_outer_edges);
			Pick_crossing_vertexes(crossing_outer_edges, cell.m_crossing_outer_vtxes);
			
			Find_edges_inside_rect(cell.Get_rect(), cell.m_inside_vtxes);
			//Pick_inside_vertexes(inside_edges, cell.m_inside_vtxes);

			

			//Find_edges_inside_rect(const RECT & extent, set<int> & inside_edges);

		}
}



void CTNR::Get_Params_from_File()
{
	CStdioFile params_file;
	if (!params_file.Open(_T("my_input.txt"), CFile::modeRead | CFile::typeText))
	{
		TRACE(_T("Unable to open file\n"));
	}

	CString strline;
	params_file.ReadString(strline);


	strline.Trim();

	//TRACE(strline + _T("\n"));

	int nTokenPos = 0;
	CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
	CString tokens[20];
	int numTokens = 0;

	tokens[numTokens++] = strToken;

	// 문자열을 token으로 분할한다.
	while (!strToken.IsEmpty())
	{
		//TRACE(_T("%d %s\n"), numTokens, strToken);



		strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		strToken.Trim();
		tokens[numTokens++] = strToken;
	}

	// token을 정수로 변환한다.

	this->m_srand_seed = _ttoi(tokens[0]);
	this->m_num_X_grids = _ttoi(tokens[1]);
	this->m_num_Y_grids = _ttoi(tokens[2]);
	this->m_node_fname = tokens[3];
	this->m_edge_fname = tokens[4];
	this->m_xmax = _ttoi(tokens[5]);
	this->m_ymax = _ttoi(tokens[6]);
	this->m_fn_dist_transit_vtxes = tokens[7].IsEmpty() ? _T("") : tokens[7];

	
	// query 파일과 data 파일을 읽어들인다.
	while (params_file.ReadString(strline))
	{
		int nTokenPos = 0;
		CString query_fname, data_fname;

		int min_num_of_fns, max_num_of_fns;

		min_num_of_fns = _ttoi(strline.Tokenize(_T(" \t\n"), nTokenPos));
		max_num_of_fns = _ttoi(strline.Tokenize(_T(" \t\n"), nTokenPos));
		
		query_fname = strline.Tokenize(_T(" \t\n"), nTokenPos);
		data_fname = strline.Tokenize(_T(" \t\n"), nTokenPos);

		this->m_min_num_of_fns.push_back(min_num_of_fns);
		this->m_max_num_of_fns.push_back(max_num_of_fns);

		this->m_query_fname.push_back(query_fname);
		this->m_data_fname.push_back(data_fname);
	}



	this->m_cell_width = this->m_xmax / this->m_num_X_grids;
	this->m_cell_height = this->m_ymax / this->m_num_Y_grids;
	this->m_grid.resize(m_num_X_grids, m_num_Y_grids);

	ASSERT(m_cell_width >= 1);
	ASSERT(m_cell_height >= 1);

	params_file.Close();

	srand(this->m_srand_seed);


	if (m_fn_dist_transit_vtxes.IsEmpty())
	{
		CFileException e;
		CString pszFileName;
		pszFileName.Format(_T("%s_%s_%dX%d_%d.bin"), m_node_fname, m_edge_fname, m_num_X_grids, m_num_Y_grids, rand() % 1000);
		pszFileName.Replace(_T(".txt"), _T(""));
		if (!g_file_dist_transit_vtxes_to_transit_vtxes.Open(pszFileName, CFile::modeCreate | CFile::modeWrite | CFile::osWriteThrough, &e))
		{
			TRACE(_T("File could not be opened %d\n"), e.m_cause);
		}

		m_read_dist_transit_vtxes = false;
	}
	else
	{

		CFileException e;
		if (!g_file_dist_transit_vtxes_to_transit_vtxes.Open(m_fn_dist_transit_vtxes, CFile::modeRead, &e))
		{
			TRACE(_T("File could not be opened %d\n"), e.m_cause);
		}

		m_read_dist_transit_vtxes = true;
	}

	CString FileName;
	CTime t = CTime::GetCurrentTime();
	FileName.Format(_T("CNLJ_Optimized_%s_%s_%d_%d.txt"), m_query_fname[0].GetBuffer(), m_data_fname[0].GetBuffer(), rand() % 1000, t.GetSecond() + t.GetMinute() + t.GetHour());
	FileName.Replace(_T(".txt_"), _T("_"));

	if (!debug_out.Open(FileName, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::osWriteThrough))
	{
		TRACE(_T("Unable to open file\n"));
	}

}



vector<CString>& CTNR::get_data_fname() 
{
	return this->m_data_fname;
}

vector<CString>& CTNR::get_query_fname() 
{
	return this->m_query_fname;
}


vector<int>& CTNR::get_min_num_of_fns()
{
	return this->m_min_num_of_fns;
}

vector<int>& CTNR::get_max_num_of_fns()
{
	return this->m_max_num_of_fns;
}



// Program to print BFS traversal from a given 
// source vertex. BFS(int s) traverses vertices  
// reachable from s. 
#include<iostream> 
#include <list> 

using namespace std;

// This class represents a directed graph using 
// adjacency list representation 
class Graph
{
	int V;    // No. of vertices 

	// Pointer to an array containing adjacency 
	// lists 
	list<int> *adj;
public:
	Graph(int V);  // Constructor 

	// function to add an edge to graph 
	void addEdge(int v, int w);

	// prints BFS traversal from a given source s 
	void BFS(int s);
};

Graph::Graph(int V)
{
	this->V = V;
	adj = new list<int>[V];
}

void Graph::addEdge(int v, int w)
{
	adj[v].push_back(w); // Add w to v’s list. 
}


int xx = 0, yy = 0;

void Graph::BFS(int s)
{
	// Mark all the vertices as not visited 
	bool *visited = new bool[V];
	for (int i = 0; i < V; i++)
		visited[i] = false;

	// Create a queue for BFS 
	list<int> queue;

	// Mark the current node as visited and enqueue it 
	visited[s] = true;
	queue.push_back(s);

	// 'i' will be used to get all adjacent 
	// vertices of a vertex 
	list<int>::iterator i;

	while (!queue.empty())
	{
		// Dequeue a vertex from queue and print it 
		s = queue.front();
		cout << s << " ";
		queue.pop_front();

		// Get all adjacent vertices of the dequeued 
		// vertex s. If a adjacent has not been visited,  
		// then mark it visited and enqueue it 
		for (i = adj[s].begin(); i != adj[s].end(); ++i)
		{
			if (!visited[*i])
			{
				visited[*i] = true;
				queue.push_back(*i);
			}
		}
	}



	for (int i = 0; i < V; i++)
	{
		if (visited[i] == true)
		{
			xx++;
		}
		if (visited[i] == false)
		{
			yy++;
		}

	}

	cout << xx << endl;
	cout << yy << endl;

}



void CTNR::Read_map_file2()
{



	// node 정보 읽어 오기
	CStdioFile node_file, edge_file;

	if (node_file.Open(this->m_node_fname, CFile::modeRead) == 0)
	{
		ASSERT(0);
	}

	CString strline;
	CString strTok;

	this->m_num_nodes = 0;
	this->m_num_edges = 0;

	set<int> visited_nodes;

	while (node_file.ReadString(strline))
	{
		this->m_num_nodes++;

		strline.Trim();

		int nTokenPos = 0;
		CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		CString tokens[5];
		int numTokens = 0;

		tokens[numTokens++] = strToken;

		// 문자열을 token으로 분할한다.
		while (!strToken.IsEmpty())
		{
			strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
			strToken.Trim();
			tokens[numTokens++] = strToken;
		}

		CMyNode node;

		node.nodeid = _ttoi(tokens[0]);
		node.x = _ttoi(tokens[1]);
		node.y = _ttoi(tokens[2]);

		set_node_to_tbl(node);



	}

	Graph g(this->m_num_nodes);


	if (edge_file.Open(this->m_edge_fname, CFile::modeRead) == 0)
	{
		ASSERT(0);
	}

	int edge_cnt = 0;

	while (edge_file.ReadString(strline))
	{
		this->m_num_edges++;

		strline.Trim();

		int nTokenPos = 0;
		CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		CString tokens[5];
		int numTokens = 0;

		tokens[numTokens++] = strToken;

		// 문자열을 token으로 분할한다.
		while (!strToken.IsEmpty())
		{
			strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
			strToken.Trim();
			tokens[numTokens++] = strToken;
		}

		CMyEdge edge;

		edge.edgeid = _ttoi(tokens[0]);

		edge.node1 = min(_ttoi(tokens[1]), _ttoi(tokens[2]));
		edge.node2 = max(_ttoi(tokens[1]), _ttoi(tokens[2]));

		edge.edgelen = GetEdgeLen(edge.node1, edge.node2);

		// m_mapMyEdge[edge.edgeid] = edge;
		set_edge_to_tbl(edge);

		// hjcho here

		CMyNode & node1 = get_node_from_tbl(edge.node1);
		CMyNode & node2 = get_node_from_tbl(edge.node2);

		node1.v_adj_nodes.push_back(node2.nodeid);
		node2.v_adj_nodes.push_back(node1.nodeid);


		set_node_to_tbl(node1);
		set_node_to_tbl(node2);


		// Create a graph given in the above diagram 

		g.addEdge(node1.nodeid, node2.nodeid);
		g.addEdge(node2.nodeid, node1.nodeid);



		visited_nodes.insert(node1.nodeid);
		visited_nodes.insert(node2.nodeid);

	}


	cout << "Following is Breadth First Traversal "
		<< "(starting from vertex 2) \n";
	g.BFS(9);

	



	node_file.Close();
	edge_file.Close();

	exit(-100);
}

void CTNR::Read_map_file()
{
	// node 정보 읽어 오기
	CStdioFile node_file, edge_file;

	if (node_file.Open(this->m_node_fname, CFile::modeRead) == 0)
	{
		ASSERT(0);
	}

	CString strline;
	CString strTok;

	this->m_num_nodes = 0;
	this->m_num_edges = 0;

	while (node_file.ReadString(strline))
	{
		this->m_num_nodes++;

		strline.Trim();

		int nTokenPos = 0;
		CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		CString tokens[5];
		int numTokens = 0;

		tokens[numTokens++] = strToken;

		// 문자열을 token으로 분할한다.
		while (!strToken.IsEmpty())
		{
			strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
			strToken.Trim();
			tokens[numTokens++] = strToken;
		}

		CMyNode node;

		node.nodeid = _ttoi(tokens[0]);
		node.x = _ttoi(tokens[1]);
		node.y = _ttoi(tokens[2]);

		set_node_to_tbl(node);

	}

	if (edge_file.Open(this->m_edge_fname, CFile::modeRead) == 0)
	{
		ASSERT(0);
	}

	int edge_cnt = 0;

	while (edge_file.ReadString(strline))
	{
		this->m_num_edges++;

		strline.Trim();

		int nTokenPos = 0;
		CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		CString tokens[5];
		int numTokens = 0;

		tokens[numTokens++] = strToken;

		// 문자열을 token으로 분할한다.
		while (!strToken.IsEmpty())
		{
			strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
			strToken.Trim();
			tokens[numTokens++] = strToken;
		}

		CMyEdge edge;

		edge.edgeid = _ttoi(tokens[0]);

		edge.node1 = min(_ttoi(tokens[1]), _ttoi(tokens[2]));
		edge.node2 = max(_ttoi(tokens[1]), _ttoi(tokens[2]));

		edge.edgelen = GetEdgeLen(edge.node1, edge.node2);

		// m_mapMyEdge[edge.edgeid] = edge;
		set_edge_to_tbl(edge);

		// hjcho here

		CMyNode & node1 = get_node_from_tbl(edge.node1);
		CMyNode & node2 = get_node_from_tbl(edge.node2);

		node1.v_adj_nodes.push_back(node2.nodeid);
		node2.v_adj_nodes.push_back(node1.nodeid);


		set_node_to_tbl(node1);
		set_node_to_tbl(node2);

		pair<int, int> key = make_pair(min(node1.nodeid, node2.nodeid), max(node1.nodeid, node2.nodeid));
		this->m_mapNode2Edge[key] = edge.edgeid;


		// rtree에 edge정보를 추가한다.
		point minpt(min(node1.x, node2.x), min(node1.y, node2.y));
		point maxpt(max(node1.x, node2.x), max(node1.y, node2.y));

		box edge_box(minpt, maxpt);
		value e = make_pair(edge_box, edge.edgeid);
		m_rtree.insert(e);



	}



	node_file.Close();
	edge_file.Close();
}


float CTNR::GetEdgeLen(int nodeid1, int nodeid2)
{
	float length = 0;

	CMyNode & node1 = get_node_from_tbl(nodeid1);
	CMyNode & node2 = get_node_from_tbl(nodeid2);


	return (int)sqrt(pow(node1.x - node2.x, 2) + pow(node1.y - node2.y, 2));


}




#ifdef _DEBUG
const int g_V = 435666;
vector<pair<int, float>> g_adj[g_V];
#endif




#if 0
float CTNR::Road_dist(POINT & pt1, POINT & pt2)
{
	float dist = (float)sqrt(pow(pt1.x - pt2.x, 2) + pow(pt1.y - pt2.y, 2));

	ASSERT(dist >= 0);

	return (int)dist;	
}
#endif

float CTNR::Road_dist(CMyNode & pt1, CMyNode & pt2)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	g_num_road_dist++;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int edgeid = get_edge_id(pt1.nodeid, pt2.nodeid);

	if (edgeid != -1)
	{
		return m_mapMyEdge.find(edgeid)->second.edgelen;
	}

	return Comp_network_dist(pt1.nodeid, pt2.nodeid);
}




int CTNR::get_cell_width()
{ 
	return m_cell_width;
}

int CTNR::get_cell_height()
{ 
	return m_cell_height; 
}

void CTNR::Get_adjacent_vertexes(int vid, vector<pair<int, float>> & result)
{
	map<int, CMyNode>::const_iterator pos = this->m_mapMyNode.find(vid);

	ASSERT(pos->second.v_adj_nodes.size() > 0);

	for (auto adj_vid : pos->second.v_adj_nodes)
	{
		map<int, CMyEdge>::const_iterator edge_it = this->m_mapMyEdge.find(get_edge_id(vid, adj_vid));
		result.push_back(make_pair(adj_vid, edge_it->second.edgelen));
	}

}



vector<int> CTNR::Retrieve_ShortestPath(const map<int, SPVertex> & map_dstvtx_dist, const int src_vid, const int dst_vid)
{
	vector<int> path;

	int cur_vid = dst_vid;

	do
	{
		path.push_back(cur_vid);
		map<int, SPVertex>::const_iterator it = map_dstvtx_dist.find(cur_vid);

		ASSERT(it != map_dstvtx_dist.end());
		
		cur_vid = it->second.get_prev_vid();
		

	} while (cur_vid != -1);

	return path;
}


vector<int> CTNR::Retrieve_ShortestPath(vector<int> & prev_vid, const int src_vid, const int dst_vid)
{
	vector<int> path;

	int cur_vid = dst_vid;

	do
	{
		path.push_back(cur_vid);
		cur_vid = prev_vid[cur_vid];


	} while (cur_vid != -1);

	return path;
}

// A가 B를 포함하는지 검사한다.
bool Contains(const set<int> & A, const set<int> & B)
{
	for (auto & x : B)
	{
		if (A.find(x) == A.end())
		{
			return false;
		}

	}

	return true;
}

// A가 B를 포함하는지 검사한다.
bool Intersects(const set<int> & A, const set<int> & B)
{
	for (auto & x : B)
	{
		if (A.find(x) != A.end())
		{
			return true;
		}

	}

	return false;
}


// 코드를 검증해 보자. hyungju hyungju 여기서부터...
void CTNR::ShortestPath(const int src_vid, const set<int> & dst_vtxes, map<int, SPVertex> & map_dstvtx_dist)
{
	set<int> visited;

	priority_queue< SPVertex, vector <SPVertex>, myComparator > pq;

	SPVertex src_vtx(0, src_vid, -1);
	map_dstvtx_dist.insert(make_pair(src_vid, src_vtx));

	pq.push(src_vtx);

	while (!pq.empty())
	{
		SPVertex u = pq.top();
		pq.pop();


		if (visited.find(u.get_vid()) != visited.end())  {	continue; }





		// 코드를 검증해 보자. hyungju hyungju 여기서부터... 
		if (Contains(visited, dst_vtxes) == true)
		{
			break;
		}

		vector<pair<int, float>> result;
		Get_adjacent_vertexes(u.get_vid(), result);

		// 인접한 vertex 리스트를 만들어야 한다. 
		for (pair<int, float> & adj_vtx : result)
		{
			int adj_vid = adj_vtx.first;
			float dist = adj_vtx.second + u.get_dist();

			map<int, SPVertex>::iterator spv_it = map_dstvtx_dist.find(adj_vid);

			if (visited.find(adj_vid) == visited.end())
			{
				SPVertex adj_vtx(dist, adj_vid, u.get_vid());
				pq.push(adj_vtx);

				if (spv_it == map_dstvtx_dist.end())
				{
					map_dstvtx_dist.insert(make_pair(adj_vid, adj_vtx));
				}
				else if (dist < spv_it->second.get_dist())
				{
					map_dstvtx_dist[adj_vid] = adj_vtx;
				}

			}

		}

		// 경로가 발견된 vtx를 추가한다. 
		visited.insert(u.get_vid());

		result.clear();
	}

	visited.clear();
	
	// 임시로 코멘트 처리함
	while (pq.empty() == false)
	{
		pq.pop();
	}	
	
}


void CTNR::ShortestPath(const int src_vid, const set<int> & dst_vtxes, vector<pair<int, float>> & dst_dist)
{


	priority_queue< SPVertex, vector <SPVertex>, myComparator > pq;

	SPVertex src_vtx(0, src_vid, -1);

	set<int> visited;

	pq.push(src_vtx);

	while (!pq.empty())
	{
		SPVertex u = pq.top();
		pq.pop();

		if (visited.find(u.get_vid()) != visited.end()) { continue; }
			   

		if (dst_vtxes.find(u.get_vid()) != dst_vtxes.end())
		{
			dst_dist.push_back(make_pair(u.get_vid(), u.get_dist())) ;
		}

		// 코드를 검증해 보자. hyungju hyungju 여기서부터... 
		if (dst_dist.size() >= dst_vtxes.size())
		{
			break;
		}

		vector<pair<int, float>> result;
		Get_adjacent_vertexes(u.get_vid(), result);

		// 인접한 vertex 리스트를 만들어야 한다. 
		for (pair<int, float> & adj_vtx : result)
		{
			int adj_vid = adj_vtx.first;
			float dist = adj_vtx.second + u.get_dist();

			if (visited.find(adj_vid) == visited.end())
			{
				SPVertex adj_vtx(dist, adj_vid, u.get_vid());
				pq.push(adj_vtx);
			}
		}

		visited.insert(u.get_vid());
		result.clear();

	}

	visited.clear();

	while (pq.empty() == false)
	{
		pq.pop();
	}




}


void CTNR::ShortestPath(const int src_vid, const set<int> & dst_vtxes, map<int, float> & map_dstvtx_dist)
{

	priority_queue< SPVertex, vector <SPVertex>, myComparator > pq;

	SPVertex src_vtx(0, src_vid, -1);

	set<int> visited;

	pq.push(src_vtx);

	while (!pq.empty())
	{
		SPVertex u = pq.top();
		pq.pop();

		if (visited.find(u.get_vid()) != visited.end()) { continue; }


		
		
		if (dst_vtxes.find(u.get_vid()) != dst_vtxes.end())
		{
			map_dstvtx_dist.insert(make_pair(u.get_vid(), u.get_dist()));
		}
			   		 
		// 코드를 검증해 보자. hyungju hyungju 여기서부터... 
		if (map_dstvtx_dist.size() >= dst_vtxes.size())
		{
			break;
		}

		vector<pair<int, float>> result;
		Get_adjacent_vertexes(u.get_vid(), result);

		// 인접한 vertex 리스트를 만들어야 한다. 
		for (pair<int, float> & adj_vtx : result)
		{
			int adj_vid = adj_vtx.first;
			float dist = adj_vtx.second + u.get_dist();

			if (visited.find(adj_vid) == visited.end())
			{
				SPVertex adj_vtx(dist, adj_vid, u.get_vid());
				pq.push(adj_vtx);
			}
		}

		visited.insert(u.get_vid());
		result.clear();
		
	}

	visited.clear();

	while (pq.empty() == false)
	{
		pq.pop();
	}




}





// 검증용 shortest path 알고리즘
void CTNR::ShortestPath(const int src_vid, const int dst_vid)
{

	priority_queue< SPVertex, vector <SPVertex>, myComparator > pq;

	map<int, SPVertex> map_dstvtx_dist;

	SPVertex src_vtx(0, src_vid , -1);
	map_dstvtx_dist.insert(make_pair(src_vid, src_vtx));
	
	pq.push(src_vtx);

	set<int> visited;

	/* Looping till priority queue becomes empty (or all
	distances are not finalized) */
	while (!pq.empty())
	{

		SPVertex u = pq.top();
		pq.pop();


		// 테스트를 위하여 코드를 추가함
		if (u.get_vid() == dst_vid)
		{
			break;
		}


		if (visited.find(u.get_vid()) != visited.end()) { continue; }


		vector<pair<int, float>> result;
		Get_adjacent_vertexes(u.get_vid(), result);

		// 인접한 vertex 리스트를 만들어야 한다. hyungju hyungju 여기서부터...
		for (pair<int, float> & adj_vtx : result)
		{
			int adj_vid = adj_vtx.first;
			float dist = adj_vtx.second + u.get_dist();

			map<int, SPVertex>::iterator spv_it = map_dstvtx_dist.find(adj_vid);

			if (spv_it == map_dstvtx_dist.end())
			{
				SPVertex adj_vtx(dist, adj_vid, u.get_vid());

				map_dstvtx_dist.insert(make_pair(adj_vid, adj_vtx));
				pq.push(adj_vtx);
			}
			else if(dist < spv_it->second.get_dist())
			{
				SPVertex adj_vtx(dist, adj_vid, u.get_vid());

				map_dstvtx_dist[adj_vid] = adj_vtx;
				pq.push(adj_vtx);
			}

		}

		visited.insert(u.get_vid());
		result.clear();
	}

	visited.clear();

	while (pq.empty() == false)
	{
		pq.pop();
	}

	TRACE(_T("################ %d \t\t %f\n"), dst_vid, map_dstvtx_dist[dst_vid].get_dist());

	Retrieve_ShortestPath(map_dstvtx_dist, src_vid, dst_vid);


}


// Prints shortest paths from src to all other vertices
void CTNR::ShortestPath(vector<pair<int, float>> adj[], int V, int src, int dst_vid)
{
	// Create a priority queue to store vertices that
	// are being preprocessed. This is weird syntax in C++.
	// Refer below link for details of this syntax
	// http://geeksquiz.com/implement-min-heap-using-stl/
	priority_queue< fiPair, vector <fiPair>, greater<fiPair> > pq;

	// Create a vector for distances and initialize all
	// distances as infinite (INF)
	vector<float> dist(V, (float)INF); // distance, prev_vid
	vector<int> prev_vid(V, -1);

	// Insert source itself in priority queue and initialize
	// its distance as 0.
	pq.push(make_pair((float)0, src));
	dist[src] = 0;
	prev_vid[src] = -1;

	/* Looping till priority queue becomes empty (or all
	distances are not finalized) */
	while (!pq.empty())
	{
		// The first vertex in pair is the minimum distance
		// vertex, extract it from priority queue.
		// vertex label is stored in second of pair (it
		// has to be done this way to keep the vertices
		// sorted distance (distance must be first item
		// in pair)
		int u = pq.top().second;
		pq.pop();

		// 테스트를 위하여 코드를 추가함
		if (u == dst_vid)
		{
			break;
		}


		// Get all adjacent of u. 
		for (auto x : adj[u])
		{
			// Get vertex label and weight of current adjacent
			// of u.
			int v = x.first;
			float weight = x.second;

			// If there is shorted path to v through u.
			if (dist[v] > dist[u] + weight)
			{
				// Updating distance of v
				dist[v] = dist[u] + weight;
				pq.push(make_pair(dist[v], v));

				prev_vid[v] = u;
			}
		}
	}

	TRACE(_T("@@@@@@@@@@@@@@@@@ %d \t\t %f\n"), dst_vid, dist[dst_vid]);

	while (pq.empty() == false)
	{
		pq.pop();
	}


	Retrieve_ShortestPath(prev_vid, src, dst_vid);


}









void CTNR::clear_objects()
{
	m_bnode_pair_dist.clear();
	m_map_qry.clear();
	m_map_obj.clear();

	m_map_qry_seg.clear(); 
	m_map_obj_seg.clear();

	m_map_bnode_qry_result.clear();

	for (auto& x : m_mapMySeq)
	{
		CMySeq& seq = x.second;
		
		seq.qry_list.clear();
		seq.obj_list.clear();
	}
	
}


/*
void CTNR::set_minmaxdist_to_tbl(pair<int, int> & qry_seg_obj_seg_pair, pair<float,float> & min_max_dist)
{
	m_mapMinMaxDist[qry_seg_obj_seg_pair] = min_max_dist;
}
*/

void CTNR::set_edge_to_tbl(CMyEdge & edge)
{
	m_mapMyEdge[edge.edgeid] = edge;
}

void CTNR::set_node_to_tbl(CMyNode & node)
{
	m_mapMyNode[node.nodeid] = node;
}

void CTNR::set_bnode_to_tbl(CMyBNode & bnode)
{
	m_mapMyBNode[bnode.nodeid] = bnode;
}

void CTNR::set_seq_to_tbl(CMySeq & seq)
{
	m_mapMySeq[seq.seqid] = seq;
}

void CTNR::set_obj_to_tbl(CObj & obj)
{
	m_map_obj[obj.id] = obj;
}

void CTNR::set_qry_to_tbl(CQry & qry)
{
	m_map_qry[qry.id] = qry;
}

CQry & CTNR::get_qry_from_tbl(int qry_id)
{
	return m_map_qry.find(qry_id)->second;
}

void CTNR::set_obj_seg_to_tbl(CObjSeg & obj_seg)
{
	m_map_obj_seg[obj_seg.id] = obj_seg;
}

void CTNR::set_qry_seg_to_tbl(CQrySeg& qry_seg)
{
	m_map_qry_seg[qry_seg.id] = qry_seg;
}

CMyEdge & CTNR::get_edge_from_tbl(int edgeid)
{
	return m_mapMyEdge.find(edgeid)->second;
}

CMyNode & CTNR::get_node_from_tbl(int nodeid)
{
	return m_mapMyNode.find(nodeid)->second;
}

CObjSeg & CTNR::get_obj_seg_from_tbl(int outer_seg_id)
{
	return m_map_obj_seg.find(outer_seg_id)->second;
}

CQrySeg & CTNR::get_qry_seg_from_tbl(int qry_seg_id)
{
	return m_map_qry_seg.find(qry_seg_id)->second;
}


CMyBNode & CTNR::get_bnode_from_tbl(int bnodeid)
{
	return m_mapMyBNode.find(bnodeid)->second;
}

CMySeq & CTNR::get_seq_from_tbl(int seqid)
{
	return m_mapMySeq.find(seqid)->second;
}


bool is_found(vector<int> &v, int elem)
{
	bool found = false;

	for (int i = 0; i < v.size(); i++)
	{
		if (v[i] == elem)
		{
			found = true;
			break;
		}
	}

	return found;
}

void CTNR::Gen_BNode_Seq()
{
	map<int, CMyNode>::iterator st = m_mapMyNode.begin();
	map<int, CMyNode>::iterator ed = m_mapMyNode.end();

	// boundary snode 정보와 sequence 정보 완성함
	for (map<int, CMyNode>::iterator ii = st; ii != ed; ++ii)
	{
		CMyNode node = ii->second;

		// 2가 아닌 노드는 boundary node가 아니다.
		if (node.v_adj_nodes.size() != 2)
		{
			CMyBNode bnode;
			bnode.nodeid = node.nodeid;


			ASSERT(bnode.v_adj_seq.size() == 0);


			ASSERT(bnode.nodeid == node.nodeid);
			set_bnode_to_tbl(bnode);

		}

	}

	multimap<int, int> mm; // key : small node id, value : seqid

	float sum_seq_len = 0;
	


	for (map<int, CMyNode>::iterator ii = st; ii != ed; ++ii)
	{

		CMyNode node = ii->second;

		// intersection node 일 경우에...
		if (node.v_adj_nodes.size() != 2)
		{
			vector<int> v_visited;

			CMyBNode & bnode = get_bnode_from_tbl(node.nodeid);
			ASSERT(bnode.nodeid == node.nodeid);


			for (int i = 0; i < node.v_adj_nodes.size(); i++)
			{
				int seqid;

				seqid = MakeSeq(node.nodeid, node.v_adj_nodes[i], mm);

				if (is_found(bnode.v_adj_seq, seqid) == true)
				{
					// TRACE("hello, world1\n");
				}
				else
				{
					bnode.v_adj_seq.push_back(seqid);
				}

				sum_seq_len += get_seq_from_tbl(seqid).seqlen;
			}

			set_bnode_to_tbl(bnode);


		}
	}

	TRACE("g_num_seqs = %d\n", g_num_seqs);

	// verify_it();

}

float CTNR::MakeSeqLen(vector<int> & v)
{
	float dist = 0;
	CMyNode node1, node2;

	for (int i = 0; i < v.size() - 1; i++)
	{
		dist += GetEdgeLen(v[i], v[i + 1]);
	}

	return dist;

}


bool CTNR::is_twin(CMySeq & seq1, CMySeq & seq2)
{

	if (min(seq1.s_node, seq1.e_node) != min(seq2.s_node, seq2.e_node))
		return false;

	if (max(seq1.s_node, seq1.e_node) != max(seq2.s_node, seq2.e_node))
		return false;

	if (seq1.v_nodes.size() != seq2.v_nodes.size())
		return false;



	int i = 0;
	int len = seq2.v_nodes.size();

	while (i < len)
	{
		if ((seq1.v_nodes[i] != seq2.v_nodes[len - (i + 1)]) && (seq1.v_nodes[i] != seq2.v_nodes[i]))
		{
			return false;
		}
		i++;
	}

	return true;
}


int CTNR::chk_dup_seq(CMySeq & seq, multimap<int, int> & mm)
{
	pair<multimap<int, int>::iterator, multimap<int, int>::iterator > iter_pair;

	iter_pair = mm.equal_range(min(seq.s_node, seq.e_node));

	multimap<int, int>::iterator iter;
	for (iter = iter_pair.first; iter != iter_pair.second; iter++)
	{
		// CMySeq existing_seq = m_mapMySeq[iter->second];
		CMySeq & existing_seq = get_seq_from_tbl(iter->second);

		if (is_twin(seq, existing_seq) == true)
		{
			return existing_seq.seqid;
		}
	}


	return (-1);
}

int CTNR::get_edge_id(int s_node, int e_node)
{
	pair<int, int> key = make_pair(min(s_node, e_node), max(s_node, e_node));

	map< pair<int, int>, int>::iterator it;
	it = this->m_mapNode2Edge.find(key);

	/*-------------------------------------------------------------------*/
	//if (it == m_mapNode2Edge.end())
	//{
	//	CString str;
	//	str.Format(_T("%d %d  \n"), s_node, e_node);
	//	Print_debug_out(str);
	//	exit(-11);
	//
	//}
	/*-------------------------------------------------------------------*/


	return (it != this->m_mapNode2Edge.end()) ? it->second : (-1);

}


int CTNR::MakeSeq(int s_nodeid, int adj_nodeid, multimap<int, int> & mm)
{
	CMySeq seq;
	vector<int> v_visited;


	seq.s_node = s_nodeid;

	// TRACE(_T("g_num_seqs : %d\n"), g_num_seqs);

	v_visited.push_back(s_nodeid); // 자신의 nodeid 를 넣는다.
	v_visited.push_back(adj_nodeid); // 자신의 인접 노드를 넣는다.

	CMyNode cur_adj_node = get_node_from_tbl(adj_nodeid); // 값을 복사해야 한다.

	while (1)
	{
		if (cur_adj_node.v_adj_nodes.size() == 2)
		{

			if (is_found(v_visited, cur_adj_node.v_adj_nodes[0]) == false)
			{
				ASSERT(is_found(v_visited, cur_adj_node.v_adj_nodes[1]) == true);
				
				// intermediate node를 추가한다.
				v_visited.push_back(cur_adj_node.v_adj_nodes[0]);
				cur_adj_node = get_node_from_tbl(cur_adj_node.v_adj_nodes[0]);
			}
			else if (is_found(v_visited, cur_adj_node.v_adj_nodes[1]) == false)
			{
				ASSERT(is_found(v_visited, cur_adj_node.v_adj_nodes[0]) == true);

				// intermediate node를 추가한다.
				v_visited.push_back(cur_adj_node.v_adj_nodes[1]);
				cur_adj_node = get_node_from_tbl(cur_adj_node.v_adj_nodes[1]);
			}
			else
			{
				// 시작 노드를 추가한다. 
				v_visited.push_back(s_nodeid);
				break;
			}
		}
		else
		{
			break;
		}
	}

	seq.e_node = v_visited.back();
	seq.v_nodes = v_visited;
	seq.seqlen = MakeSeqLen(seq.v_nodes);

	// TRACE("seqlen : %f\n", seq.seqlen);

	// ASSERT(seq.seqlen > 0);

	CMyNode & node1 = get_node_from_tbl(seq.s_node);
	CMyNode & node2 = get_node_from_tbl(seq.e_node);

	ASSERT(node1.v_adj_nodes.size() != 2);
	ASSERT(node2.v_adj_nodes.size() != 2);
	//------------------------------------------------------------//

	// 기존에 중복되는 seq가 있는지 검사한다. 
	int res = chk_dup_seq(seq, mm);


	if (res >= 0)	// 중복이 발견되면, 발견된 seqid를 저장하고, map에 추가하지 않는다.  
	{
		seq.seqid = res;
	}
	else // 중복이 발견 안 되면, g_num_seqs를 증가시키고, map에 추가한다. 
	{
		ASSERT(seq.s_node <= seq.e_node);

		seq.seqid = g_num_seqs++;

		set_seq_to_tbl(seq);

		mm.insert(pair<int, int>(min(seq.s_node, seq.e_node), seq.seqid));
	}




	// edge가 속하는 sequence 정보를 추가한다.
	for (int i = 0; i < seq.v_nodes.size() - 1; i++)
	{
		int edgeid = get_edge_id(seq.v_nodes[i], seq.v_nodes[i + 1]);

		CMyEdge & edge = get_edge_from_tbl(edgeid);
		edge.seqid = seq.seqid; // edge가 속하는 seqid 를 저장한다. 
		set_edge_to_tbl(edge);


	}

	return seq.seqid;

}





void CTNR::Populate_objects(CString data_fname)
{
	CStdioFile fp;

	CFileException ex;

	if (!fp.Open(data_fname, CFile::modeRead | CFile::shareDenyNone, &ex))
	{
		TCHAR   szCause[255];
		
		ex.GetErrorMessage(szCause, 255);

		ASSERT(0);
	}

	CString strline;
	CString strTok;

	CTime prev_time, cur_time;
	//int cntcnt = 0;

	while (fp.ReadString(strline))
	{
		strline.Trim();

		int nTokenPos = 0;
		CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		CString tokens[5];
		int numTokens = 0;

		tokens[numTokens++] = strToken;

		// 문자열을 token으로 분할한다.
		while (!strToken.IsEmpty())
		{
			strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
			strToken.Trim();
			tokens[numTokens++] = strToken;
		}




		CObj obj;



		obj.id = _ttoi(tokens[0]);
		obj.cur_x = _ttoi(tokens[1]);
		obj.cur_y = _ttoi(tokens[2]);
		obj.cur_edgeid = _ttoi(tokens[3]);




		this->set_obj_to_tbl(obj);
		this->add_obj_to_seq(obj);



		// data object 를 tbl에 넣는다.		

	}

	fp.Close();

}


void CTNR::Populate_queries(CString query_fname, int min_num_of_fns, int max_num_of_fns)
{
	CStdioFile fp;

	CFileException ex;

	if (!fp.Open(query_fname, CFile::modeRead | CFile::shareDenyNone, &ex))
	{
		TCHAR   szCause[255];

		ex.GetErrorMessage(szCause, 255);

		assert(0);
	}

	CString strline;
	CString strTok;

	CTime prev_time, cur_time;
	//int cntcnt = 0;

	while (fp.ReadString(strline))
	{
		strline.Trim();

		int nTokenPos = 0;
		CString strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
		CString tokens[5];
		int numTokens = 0;

		tokens[numTokens++] = strToken;

		// 문자열을 token으로 분할한다.
		while (!strToken.IsEmpty())
		{
			strToken = strline.Tokenize(_T(" \t\n"), nTokenPos);
			strToken.Trim();
			tokens[numTokens++] = strToken;
		}




		CQry qry;



		qry.id = _ttoi(tokens[0]);
		qry.cur_x = _ttoi(tokens[1]);
		qry.cur_y = _ttoi(tokens[2]);
		qry.cur_edgeid = _ttoi(tokens[3]);
		qry.num_of_fns = min_num_of_fns + (rand() % (max_num_of_fns - min_num_of_fns + 1));	// 임시로 최대 32로 지정함. 추후에 수정 가능함

		assert(min_num_of_fns <= qry.num_of_fns && qry.num_of_fns <= max_num_of_fns);


		this->set_qry_to_tbl(qry);
		this->add_qry_to_seq(qry);


		// qry object 를 tbl에 넣는다.		

	}

	fp.Close();

}


int CTNR::Get_seqid_from_edgeid(int edgeid)
{
	CMyEdge & edge = get_edge_from_tbl(edgeid);
	return edge.seqid;
}


void CTNR::add_obj_to_seq(CObj & obj)
{
	// to do list
	int seqid = Get_seqid_from_edgeid(obj.cur_edgeid);
	CMySeq & obj_seq = get_seq_from_tbl(seqid);

	float len = Distance_bnode_to_dobj(obj, get_bnode_from_tbl(obj_seq.s_node), obj_seq);

	obj_seq.obj_list[obj.id] = len;
	set_seq_to_tbl(obj_seq);
}


void CTNR::add_qry_to_seq(CQry & qry)
{
	// to do list
	int seqid = Get_seqid_from_edgeid(qry.cur_edgeid);
	CMySeq & qry_seq = get_seq_from_tbl(seqid);

	float len = Distance_bnode_to_dobj(qry, get_bnode_from_tbl(qry_seq.s_node), qry_seq);

	qry_seq.qry_list.push_back(make_pair(qry.id, len));
	set_seq_to_tbl(qry_seq);
}




float get_dist(float x1, float y1, float x2, float y2)
{
	float dist = ((float)pow((x1 - x2), 2) + (float)pow((y1 - y2), 2));

	return (float)sqrt(dist);
}

float CTNR::Distance_bnode_to_dobj(CObj & dobj, CMyBNode & node, CMySeq & seq)
{
#ifdef _DEBUG
	if (seq.s_node != node.nodeid)
	{
		ASSERT(seq.s_node == node.nodeid);
	}
#endif

	float len = 0.0;
	map<int, CMyNode>::iterator node_it;

	if (seq.s_node == node.nodeid)
	{

		for (int i = 0; i < seq.v_nodes.size() - 1; i++)
		{
			int edgeid = get_edge_id(seq.v_nodes[i], seq.v_nodes[i + 1]);

			CMyNode & node1 = get_node_from_tbl(seq.v_nodes[i]);
			CMyNode & node2 = get_node_from_tbl(seq.v_nodes[i + 1]);

			if (edgeid == dobj.cur_edgeid)
			{
				len += get_dist((float)dobj.cur_x, (float)dobj.cur_y, (float)node1.x, (float)node1.y);
				break;
			}
			else
			{
				// len += get_dist((float)node1.x, (float)node1.y, (float)node2.x, (float)node2.y);
				len += GetEdgeLen(node1.nodeid, node2.nodeid);

			}

		}

	}
	else
	{
		for (int j = seq.v_nodes.size() - 1; j > 0; j--)
		{
			int edgeid = get_edge_id(seq.v_nodes[j], seq.v_nodes[j - 1]);

			CMyNode & node1 = get_node_from_tbl(seq.v_nodes[j]);
			CMyNode & node2 = get_node_from_tbl(seq.v_nodes[j - 1]);

			if (edgeid == dobj.cur_edgeid)
			{
				len += get_dist((float)dobj.cur_x, (float)dobj.cur_y, (float)node1.x, (float)node1.y);
				break;
			}
			else
			{
				// len += get_dist((float)node1->x, (float)node1->y, (float)node2->x, (float)node2->y);
				len += GetEdgeLen(node1.nodeid, node2.nodeid);
			}
		}

	}

	return min(len, seq.seqlen);
}


void CTNR::Group_queries_objects()
{
	for (auto & x : m_mapMySeq)
	{
		CMySeq & seq = x.second;

		// hyungju hyungju
		Group_queries_in_seq(seq);
		Group_objects_in_seq(seq);
		
		this->set_seq_to_tbl(seq);



	}



}


void CTNR::Group_queries_in_seq(CMySeq& qry_seq)
{
	qry_seq.qry_seg_id = (qry_seq.qry_list.size() == 0) ? -1 : qry_seq.seqid;

	if (qry_seq.qry_list.size() == 0)
	{
		return;
	}

	assert(qry_seq.qry_list.size() > 0);

	CQrySeg qry_seg;

	qry_seg.id = qry_seq.seqid;
	qry_seg.dist_start_pair.second = 0;
	qry_seg.dist_end_pair.second = qry_seq.seqlen;

	/*
	for (vector<pair<int, float>>::iterator qry_it = qry_seq.qry_list.begin(); qry_it != qry_seq.qry_list.end(); qry_it++)
	{
		if (qry_it->second <= qry_seg.dist_start_pair.second)
		{
			qry_seg.dist_start_pair.first = qry_it->first;
			qry_seg.dist_start_pair.second = qry_it->second;
		}

		if (qry_it->second >= qry_seg.dist_end_pair.second)
		{
			qry_seg.dist_end_pair.first = qry_it->first;
			qry_seg.dist_end_pair.second = qry_it->second;
		}

		assert(qry_seg.dist_start_pair.second <= qry_seg.dist_end_pair.second);

	}
	*/

	this->set_qry_seg_to_tbl(qry_seg);

}



void CTNR::Group_objects_in_seq(CMySeq & obj_seq)
{
	obj_seq.obj_seg_id = (obj_seq.obj_list.size() == 0) ? -1 : obj_seq.seqid;

	if (obj_seq.obj_list.size() == 0) 
	{ 
		return; 
	}

	ASSERT(obj_seq.obj_list.size() > 0);

	CObjSeg obj_seg;

	obj_seg.id = obj_seq.seqid;
	obj_seg.dist_start_pair.second = 0;
	obj_seg.dist_end_pair.second = obj_seq.seqlen;

	/*
	for (map<int, float>::iterator obj_it = obj_seq.obj_list.begin(); obj_it != obj_seq.obj_list.end(); obj_it++)
	{
		if (obj_it->second <= obj_seg.dist_start_pair.second)
		{
			obj_seg.dist_start_pair.first = obj_it->first;
			obj_seg.dist_start_pair.second = obj_it->second;
		}

		if (obj_it->second >= obj_seg.dist_end_pair.second)
		{
			obj_seg.dist_end_pair.first = obj_it->first;
			obj_seg.dist_end_pair.second = obj_it->second;
		}

		ASSERT(obj_seg.dist_start_pair.second <= obj_seg.dist_end_pair.second);

	}
	*/

	this->set_obj_seg_to_tbl(obj_seg);

}

pair<float,float> Loc_star(float dist1, float dist2, float seglen, const pair<float,float> & segment)
{

	float star;
	float maxdist;

	if ((dist1 + seglen) == dist2)
	{
		star = segment.second;
		maxdist = dist2;
	}
	else if ((dist2 + seglen) == dist1)
	{
		star = segment.first;
		maxdist = dist1;
	}
	else
	{
		star = segment.first + (dist2 + seglen - dist1) / 2;
		maxdist = (dist2 + seglen + dist1) / 2;
	}

	// 값을 보정한다.
	star = (star < segment.first)? segment.first : star;
	star = (star > segment.second)? segment.second : star;

	maxdist = max(max(dist1, dist2), maxdist);

	return make_pair(star, maxdist);
}


float CTNR::retrieve_seg_pair_dist(pair<int, float> & qry_seg, pair<int, float> & obj_seg)
{
	pair<pair<int, float>, pair<int, float>> key;

	key = make_pair(qry_seg, obj_seg);

	map< pair<pair<int, float>, pair<int, float>>, float>::iterator pos = m_seg_pair_dist.find(key);

	assert(pos != m_seg_pair_dist.end());

	return pos->second;
}


pair<float, float> CTNR::Comp_min_max_dist(CMySeq& qry_seq, CMySeq& obj_seq)
{
	CQrySeg& x_qry_seg = get_qry_seg_from_tbl(qry_seq.seqid);
	CObjSeg& y_obj_seg = get_obj_seg_from_tbl(obj_seq.seqid);

	float x_s, x_e, y_s, y_e;

	x_s = x_qry_seg.dist_start_pair.second;
	x_e = x_qry_seg.dist_end_pair.second;
	y_s = y_obj_seg.dist_start_pair.second;
	y_e = y_obj_seg.dist_end_pair.second;

	//------------------------------------------------------------------------------------------------------//
	// 코드 수정한다. 
	//------------------------------------------------------------------------------------------------------//

	float mindist, maxdist;
	float dist_xs_ys, dist_xe_ys, dist_xs_ye, dist_xe_ye;

	//dist_xs_ys = Road_dist(make_pair(qry_seq, x_s), make_pair(obj_seq, y_s));
	//dist_xe_ys = Road_dist(make_pair(qry_seq, x_e), make_pair(obj_seq, y_s));
	//dist_xs_ye = Road_dist(make_pair(qry_seq, x_s), make_pair(obj_seq, y_e));
	//dist_xe_ye = Road_dist(make_pair(qry_seq, x_e), make_pair(obj_seq, y_e));

	dist_xs_ys = Comp_network_dist(qry_seq.s_node, obj_seq.s_node);
	dist_xe_ys = Comp_network_dist(qry_seq.e_node, obj_seq.s_node);
	dist_xs_ye = Comp_network_dist(qry_seq.s_node, obj_seq.e_node);
	dist_xe_ye = Comp_network_dist(qry_seq.e_node, obj_seq.e_node);


	//-----------------------------------------------------------------------
	//{
	//	CString tt;
	//	tt.Format(_T("---- %f , %f , %f , %f \n"), dist_xs_ys, dist_xe_ys, dist_xs_ye, dist_xe_ye);
	//	OutputDebugString(tt);
	//}
	//-----------------------------------------------------------------------

	if ((qry_seq.seqid == obj_seq.seqid) && ((x_s <= y_s && y_s <= x_e) || (x_s <= y_e && y_e <= x_e) || (y_s <= x_s && x_s <= y_e) || (y_s <= x_e && x_e <= y_e)))
	{
		mindist = 0;
	}
	else
	{
		mindist = min(min(dist_xs_ys, dist_xe_ys), min(dist_xs_ye, dist_xe_ye));
	}

	pair<float, float> ys_star = Loc_star(dist_xs_ys, dist_xe_ys, (x_e - x_s), make_pair(min(x_s, x_e), max(x_s, x_e)));
	pair<float, float> ye_star = Loc_star(dist_xs_ye, dist_xe_ye, (x_e - x_s), make_pair(min(x_s, x_e), max(x_s, x_e)));


	float ys_star_loc, ye_star_loc;
	ys_star_loc = ys_star.first;
	ye_star_loc = ye_star.first;

	if (ys_star_loc == ye_star_loc)
	{

		float dist_ys_ysstar, dist_ye_ysstar;

		dist_ys_ysstar = Road_dist(make_pair(obj_seq, y_s), make_pair(qry_seq, ys_star_loc));
		dist_ye_ysstar = Road_dist(make_pair(obj_seq, y_e), make_pair(qry_seq, ys_star_loc));

		pair<float, float> ys_star_star = Loc_star(dist_ys_ysstar, dist_ye_ysstar, abs(y_e - y_s), make_pair(min(y_s, y_e), max(y_s, y_e)));

		// maxdist = ys_star_star.second;
		maxdist = max(ys_star_star.second, max(max(dist_xs_ys, dist_xe_ys), max(dist_xs_ye, dist_xe_ye)));

		return make_pair(mindist, maxdist);
	}
	else if (ys_star_loc != ye_star_loc)
	{
		float dist_ys_ysstar, dist_ye_ysstar, dist_ys_yestar, dist_ye_yestar;

		dist_ys_ysstar = Road_dist(make_pair(obj_seq, y_s), make_pair(qry_seq, ys_star_loc));
		dist_ye_ysstar = Road_dist(make_pair(obj_seq, y_e), make_pair(qry_seq, ys_star_loc));

		dist_ys_yestar = Road_dist(make_pair(obj_seq, y_s), make_pair(qry_seq, ye_star_loc));
		dist_ye_yestar = Road_dist(make_pair(obj_seq, y_e), make_pair(qry_seq, ye_star_loc));

		pair<float, float> ys_star_star = Loc_star(dist_ys_ysstar, dist_ye_ysstar, abs(y_e - y_s), make_pair(min(y_s, y_e), max(y_s, y_e)));
		pair<float, float> ye_star_star = Loc_star(dist_ys_yestar, dist_ye_yestar, abs(y_e - y_s), make_pair(min(y_s, y_e), max(y_s, y_e)));

		// maxdist = max(ys_star_star.second, ye_star_star.second);

		maxdist = max(max(ys_star_star.second, ye_star_star.second), max(max(dist_xs_ys, dist_xe_ys), max(dist_xs_ye, dist_xe_ye)));



		return make_pair(mindist, maxdist);

	}

}


float CTNR::Road_dist(pair<CMySeq, float> & x, pair<CMySeq, float> & y)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	g_num_road_dist++;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_snode, x_enode, y_snode, y_enode;

	x_snode = x.first.s_node;
	x_enode = x.first.e_node;
	y_snode = y.first.s_node;
	y_enode = y.first.e_node;

	float x_seqlen, y_seqlen;
	x_seqlen = x.first.seqlen;
	y_seqlen = y.first.seqlen;

	float x_loc, y_loc;
	x_loc = x.second;
	y_loc = y.second;


	float dist1, dist2, dist3, dist4;

	if (x.first.seqid != y.first.seqid)
	{
		float tmp_dist_xs_ys = retrieve_bnode_pair_dist(x_snode, y_snode);
		float tmp_dist_xs_ye = retrieve_bnode_pair_dist(x_snode, y_enode);
		float tmp_dist_xe_ys = retrieve_bnode_pair_dist(x_enode, y_snode);
		float tmp_dist_xe_ye = retrieve_bnode_pair_dist(x_enode, y_enode);
		
		float dist_xs_ys = (tmp_dist_xs_ys < 0) ? Comp_network_dist(x_snode, y_snode) : tmp_dist_xs_ys;
		float dist_xs_ye = (tmp_dist_xs_ye < 0) ? Comp_network_dist(x_snode, y_enode) : tmp_dist_xs_ye;
		float dist_xe_ys = (tmp_dist_xe_ys < 0) ? Comp_network_dist(x_enode, y_snode) : tmp_dist_xe_ys;
		float dist_xe_ye = (tmp_dist_xe_ye < 0) ? Comp_network_dist(x_enode, y_enode) : tmp_dist_xe_ye;


		dist1 = dist_xs_ys + x_loc + y_loc;
		dist2 = dist_xs_ye + x_loc + (y_seqlen - y_loc);
		dist3 = dist_xe_ys + (x_seqlen - x_loc) + y_loc;
		dist4 = dist_xe_ye + (x_seqlen - x_loc) + (y_seqlen - y_loc);

	}
	else
	{
		float tmp_dist_xs_ye = retrieve_bnode_pair_dist(x_snode, y_enode);
		float dist_xs_ye = (tmp_dist_xs_ye < 0) ? Comp_network_dist(x_snode, y_enode) : tmp_dist_xs_ye;

		dist1 = abs(x_loc - y_loc);
		dist2 = dist_xs_ye + x_loc + (y_seqlen - y_loc);
		dist3 = dist_xs_ye + y_loc + (x_seqlen - x_loc);
		dist4 = numeric_limits<float>::infinity();
	}

	ASSERT(dist1 > 0);
	ASSERT(dist2 > 0);
	ASSERT(dist3 > 0);
	ASSERT(dist4 > 0);

	return min(min(dist1, dist2), min(dist3, dist4));




}


pair<float,float> CTNR::Comp_dist_two_segs(CMySeq & qry_seq, CMySeq & obj_seq)
{
	ASSERT(qry_seq.qry_list.size() > 0 && obj_seq.obj_list.size() > 0);

	pair<float,float> min_max_dist;

	min_max_dist = Comp_min_max_dist(qry_seq, obj_seq);

	// set_minmaxdist_to_tbl(make_pair(qry_seq.qry_seg_id, obj_seq.obj_seg_id), min_max_dist);

	return min_max_dist;
}

// Compares two intervals according to staring times. 
bool compare_maxdist(const pair<int, pair<float, float>> & elem1, const pair<int, pair<float, float>> & elem2)
{
	float maxdist1, maxdist2;
	float mindist1, mindist2;

	mindist1 = elem1.second.first;
	maxdist1 = elem1.second.second;

	mindist2 = elem2.second.first;
	maxdist2 = elem2.second.second;

	if (maxdist1 != maxdist2)
	{
		return (maxdist1 > maxdist2);
	}
	else
	{
		return (mindist1 > mindist2);
	}

	
}



int g_num_road_dist = 0;

float CTNR::Road_dist(CObjSeg & src_seg, float src_loc, CObjSeg & dst_seg, float dst_loc)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	g_num_road_dist++;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int src_segid, dst_segid;

	src_segid = src_seg.id;
	dst_segid = dst_seg.id;

	float src_seg_st = src_seg.dist_start_pair.second;
	float src_seg_ed = src_seg.dist_end_pair.second;
	float dst_seg_st = dst_seg.dist_start_pair.second;;
	float dst_seg_ed = dst_seg.dist_end_pair.second;;

	////////////////////////////////////////////////////////
	if (src_seg_st > src_loc || src_loc > src_seg_ed || dst_seg_st > dst_loc || dst_loc > dst_seg_ed)
	{
		assert(0);
	}
	////////////////////////////////////////////////////////




  CMySeq& src_seq = get_seq_from_tbl(src_segid);
  CMySeq& dst_seq = get_seq_from_tbl(dst_segid);
  
  
  float dist_st_st = Comp_network_dist(src_seq.s_node, dst_seq.s_node);
  float dist_st_ed = Comp_network_dist(src_seq.s_node, dst_seq.e_node);
  float dist_ed_st = Comp_network_dist(src_seq.e_node, dst_seq.s_node);
  float dist_ed_ed = Comp_network_dist(src_seq.e_node, dst_seq.e_node);
  
  float dist1 = dist_st_st + abs(src_loc - src_seg_st) + abs(dst_loc - dst_seg_st);
  float dist2 = dist_st_ed + abs(src_loc - src_seg_st) + abs(dst_loc - dst_seg_ed);
  float dist3 = dist_ed_st + abs(src_loc - src_seg_ed) + abs(dst_loc - dst_seg_st);
  float dist4 = dist_ed_ed + abs(src_loc - src_seg_ed) + abs(dst_loc - dst_seg_ed);
  
  return min(min(dist1, dist2), min(dist3, dist4));

}



bool Contains(const vector<pair<int, float>>& res, const set<int>& candidates)
{
	if (res.size() < candidates.size())
		return false;

	set<int> res_set;

	for (auto& aa : res)
	{
		int obj_id = aa.first;
		res_set.insert(obj_id);
	}

	assert(res.size() == res_set.size());


	for (auto & bb : candidates)
	{
		int can_obj_id = bb;

		if (res_set.find(can_obj_id) == res_set.end())
			return false;
	}

	return true;
}

int g_num_kfn_queries = 0;
set<int> g_qry_nodes;

void CTNR::kFN_query
(CQrySeg & qry_seg, const float qry_loc, CQry & qry, const vector<pair<int, pair<float, float>>>& sorted_obj_seg_list, const set<int> & candidates, const float offset)
{
	// 결과 집합을 초기화한다. 
	qry.get_result().clear();

	assert(qry.get_result().size() == 0);


	float prundist = -1;
	// int qry_id = q_id_loc.first;
	// float qry_loc = q_id_loc.second;

	for (auto& item : sorted_obj_seg_list)
	{
		int obj_segid = item.first;
		const pair<float, float>& min_max_dist = item.second;



		// 후보 집합이 없으면, prundist를 이용한다. 
		if (candidates.size() == 0)
		{
			
			if (prundist > min_max_dist.second + offset) 
			{ 
				assert(qry.get_result().size() >= qry.num_of_fns);
				return; 
			}
		}
		else
		{
			if(Contains(qry.get_result(), candidates) == true)
			{ 
				return; 
			}
		}		

		CObjSeg& obj_seg = get_obj_seg_from_tbl(obj_segid);
		CMySeq& obj_seq = get_seq_from_tbl(obj_segid);

		for (auto& obj : obj_seq.obj_list)
		{

			int obj_id = obj.first;
			float obj_loc = obj.second;

			float obj_dist, kth_dist;


			// k개의 farthest objects를 저장할 수 있도록 코드 수정이 필요함 hyungju hyungju
			obj_dist = Road_dist(qry_seg, qry_loc, obj_seg, obj_loc);

			//{//------------------------------------------------------------------------------------------
			//	if (qry.id == 556 && qry.get_result().size() == 10)
			//	{
			//		CString tt;
			//		tt.Format(_T("---- %d , %d , %f \n"), qry.id, obj_id, obj_dist);
			//		OutputDebugString(tt);
			//	}
			//}//------------------------------------------------------------------------------------------

			kth_dist = qry.update_result(obj_id, obj_dist, candidates, offset);





			prundist = (prundist < 0) ? kth_dist : min(prundist, kth_dist);


		}
	}
}


// 여기서부터 집중적으로 하자.... hyungju hyungju
void CTNR::kFN_search_using_candidates(CMySeq& qry_seq, CQry& tmp_qry_st, CQry& tmp_qry_ed, float qst_loc, float qed_loc)
{
	for (auto& q_id_loc : qry_seq.qry_list)
	{
		int q_id = q_id_loc.first;
		float q_loc = q_id_loc.second;

		CQry& qry = get_qry_from_tbl(q_id);


		assert(qry.get_result().size() == 0);

		
		

		// 메쏘드로 변경하자. 코드가 길다. hyungju hyungju

		map<int, float> map_qry_st_result;
		map<int, float> map_qry_ed_result;
		map<int, int> map_obj_id;

		vector<pair<int, float>>& v_st = tmp_qry_st.get_result();
		vector<pair<int, float>>& v_ed = tmp_qry_ed.get_result();

		
		for (auto& x : v_st)
		{
			map_qry_st_result.insert(x);
			map_obj_id.insert(make_pair(x.first, x.first));
		}
		for (auto& x : v_ed)
		{
			map_qry_ed_result.insert(x);
			map_obj_id.insert(make_pair(x.first, x.first));
		}
		
		for (auto& x : map_obj_id)
		{
			int obj_id = x.first;
			map<int, float>::iterator it_st = map_qry_st_result.find(obj_id);
			map<int, float>::iterator it_ed = map_qry_ed_result.find(obj_id);

			if (it_st != map_qry_st_result.end() && it_ed != map_qry_ed_result.end())
			{
				float dist_qst_obj = it_st->second;
				float dist_qed_obj = it_ed->second;

				float st_dist = abs(q_loc - qst_loc) + dist_qst_obj;
				float ed_dist = abs(q_loc - qed_loc) + dist_qed_obj;

				float obj_dist = min(st_dist, ed_dist);

				qry.update_result(obj_id, obj_dist);
			}

		}
		// qry.update_result();
	}
	

}

void Add_res_to_candidates(vector<pair<int, float>>& res, set<int>& candidates, int num_of_fns)
{
	int cnt = 0;

	for (auto& item : res)
	{
		int obj_id = item.first;
		candidates.insert(obj_id);
	}
	assert(num_of_fns <= candidates.size());
}

float CTNR::get_max_span(CMyBNode& bnode)
{
	float max_span = -1;

	for (auto seqid : bnode.v_adj_seq)
	{
		map<int, CQrySeg>::const_iterator itr = m_map_qry_seg.find(seqid);
		if (itr != m_map_qry_seg.end())
		{
			CMySeq& seq = get_seq_from_tbl(seqid);

			max_span = max(seq.seqlen, max_span);	

		}

	}

	return max_span;
}



// 여기서부터 집중적으로 하자.... hyungju hyungju
void CTNR::MkFN_search(CMySeq & qry_seq, const vector<pair<int, pair<float, float>>> & sorted_obj_seg_list)
{
	float prundist = -1;

	CQrySeg & qry_seg = get_qry_seg_from_tbl(qry_seq.qry_seg_id);



	////////////////////////////////////////////////////////
	// 같은 qry segment에 qry중에서 num_of_fns 이 가장 큰 경우를 찾는다.
	// int max_num_of_fns = this-> find_max_num_of_fns(qry_seq);
	int num_of_fns = this->m_max_num_of_fns.front();
	
	assert(m_max_num_of_fns.front() == m_min_num_of_fns.front());

	float qst_loc = 0;
	float qed_loc = qry_seq.seqlen;

	CQry tmp_qry_st; // get_qry_from_tbl(-1);
	CQry tmp_qry_ed; // get_qry_from_tbl(-1);


	tmp_qry_st.num_of_fns = tmp_qry_ed.num_of_fns = num_of_fns; // 임시로 코멘트 처리함

	// ------------------------------------------------------------------------------------------
	// 질의 결과를 저장하고, 재사용한다. 
	bool additional_query = false;

	map<int, CQry>::iterator itr_snode = m_map_bnode_qry_result.find(qry_seq.s_node);
	map<int, CQry>::iterator itr_enode = m_map_bnode_qry_result.find(qry_seq.e_node);

	// kFN_query를 수정함 prundist_offset추가함... 중요... hyungju
	std::set<int> candidates;

	CMyBNode& s_bnode = get_bnode_from_tbl(qry_seq.s_node);
	CMyBNode& e_bnode = get_bnode_from_tbl(qry_seq.e_node);

	if (itr_snode != m_map_bnode_qry_result.end())
	{
		tmp_qry_st = (CQry)itr_snode->second;
	}
	else
	{
		float max_span_st = get_max_span(s_bnode);

		// candidates 집합이 공집합이다.
		kFN_query(qry_seg, 0, tmp_qry_st, sorted_obj_seg_list, candidates, max_span_st);
		m_map_bnode_qry_result.insert(make_pair(qry_seq.s_node, tmp_qry_st));
		g_num_kfn_queries ++;
		additional_query = true;
	}
	Add_res_to_candidates(tmp_qry_st.get_result(), candidates, num_of_fns);


	if (itr_enode != m_map_bnode_qry_result.end())
	{
		tmp_qry_ed = (CQry)itr_enode->second;
	}
	else
	{
		float max_span_ed = get_max_span(e_bnode);

		// candidates 집합이 공집합이다.
		kFN_query(qry_seg, qry_seq.seqlen, tmp_qry_ed, sorted_obj_seg_list, candidates, max_span_ed);
		m_map_bnode_qry_result.insert(make_pair(qry_seq.e_node, tmp_qry_ed));
		g_num_kfn_queries++;
		additional_query = true;
	}
	Add_res_to_candidates(tmp_qry_ed.get_result(), candidates, num_of_fns);

	//if (additional_query = true)
	//{
	//	float max_span_st = get_max_span(s_bnode);
	//	kFN_query(qry_seg, 0, tmp_qry_st, sorted_obj_seg_list, candidates, max_span_st);
	//}
		
	kFN_search_using_candidates(qry_seq, tmp_qry_st, tmp_qry_ed, qst_loc, qed_loc);

	// 결과를 출력하는 코드를 추가하자. hyungju hyungju
}




int CTNR::Get_num_adj_segs(CMyBNode & bnode)
{
	int num = 0;

	for (auto & x : bnode.v_adj_seq)
	{
		CMySeq & adj_seq = get_seq_from_tbl(x);

		if (adj_seq.obj_seg_id == adj_seq.seqid) {	num++;	}

	}

	return num;
}


void CTNR::Comp_bnode_pair_dist()
{
	int num_adj_seg1, num_adj_seg2;

	for (map<int, CMyBNode>::iterator pos1 = m_mapMyBNode.begin(); pos1 != m_mapMyBNode.end(); pos1++)
	{
		int bnodeid1 = pos1->first;
		num_adj_seg1 = Get_num_adj_segs(pos1->second);

		if (num_adj_seg1 == 0) { continue; }

		for (map<int, CMyBNode>::iterator pos2 = pos1; pos2 != m_mapMyBNode.end(); pos2++)
		{
			int bnodeid2 = pos2->first;
			num_adj_seg2 = Get_num_adj_segs(pos2->second);

			if ((num_adj_seg2 == 0) || (num_adj_seg1 == 1 && num_adj_seg2 == 1) || (pos1 == pos2) )
			{
				continue;
			}

			pair<int, int> key = make_pair( min(bnodeid1, bnodeid2), max(bnodeid1, bnodeid2) );
			float dist = Road_dist(get_node_from_tbl(bnodeid1), get_node_from_tbl(bnodeid2));
			store_bnode_pair_dist(bnodeid1, bnodeid2, dist);

		}

	}


	//CString str;
	//str.Format(_T("m_bnode_pair_dist.size(): %d  \n"), m_bnode_pair_dist.size());
	//Print_debug_out(str);


}

int cntcnt = 0;

float CTNR::retrieve_bnode_pair_dist(int bnode1, int bnode2)
{


	if (bnode1 == bnode2)
	{
		return 0;
	}
	

	pair<int, int> key;

	key = (bnode1 < bnode2) ? make_pair(bnode1, bnode2) : make_pair(bnode2, bnode1);

	map< pair<int, int>, float>::iterator pos = m_bnode_pair_dist.find(key);

	if (pos == m_bnode_pair_dist.end())
	{
		return (-1);
	}
	cntcnt++;
	return pos->second;



}

void CTNR::store_bnode_pair_dist(int bnodeid1, int bnodeid2, float dist)
{
	if (bnodeid1 == bnodeid2)
		return;

	pair<int, int> key = make_pair(min(bnodeid1, bnodeid2), max(bnodeid1, bnodeid2));

	m_bnode_pair_dist.insert(make_pair(key, dist));
}



void CTNR::Comp_min_max_dist()
{


	Comp_bnode_pair_dist();
	cntcntcnt = 0;

	// m_mapMinMaxDist.clear();

	for (auto& qry_segment : this->m_map_qry_seg)
	{
		int qry_segid, obj_segid;

		vector<pair<int, pair<float, float>>> v;

		qry_segid = qry_segment.first;
		CMySeq& qry_seq = get_seq_from_tbl(qry_segid);

		//---------------------------------------------------------------------------------------//
		CMyBNode& s_bnode = get_bnode_from_tbl(qry_seq.s_node);
		CMyBNode& e_bnode = get_bnode_from_tbl(qry_seq.e_node);

		map<int, CQry>::iterator itr_snode = m_map_bnode_qry_result.find(s_bnode.nodeid);
		map<int, CQry>::iterator itr_enode = m_map_bnode_qry_result.find(e_bnode.nodeid);

		if (itr_snode != m_map_bnode_qry_result.end() && itr_enode != m_map_bnode_qry_result.end())
		{
			kFN_search_using_candidates(qry_seq, (CQry)itr_snode->second, (CQry)itr_enode->second, 0, qry_seq.seqlen);
			continue;
		}
		//---------------------------------------------------------------------------------------//


		for (auto& obj_segment : this->m_map_obj_seg)
		{
			//----------------------------------------------------------------------------------
			//if (obj_segment.first != 4616)
			//	continue;
			//----------------------------------------------------------------------------------
			obj_segid = obj_segment.first;

			pair<float, float> min_max_dist = Comp_dist_two_segs(qry_seq, get_seq_from_tbl(obj_segid));
			pair<int, pair<float, float>> elem = make_pair(obj_segid, min_max_dist);
			v.push_back(elem);


			assert(min_max_dist.first <= min_max_dist.second);


		}

		vector<pair<int, pair<float, float>>>& sorted_v = v;

		////////////////////////////////////////////////////////
		//CString tt;
		//tt.Format(_T("** before v.size() = %d , sorted_v.size() = %d \n"), v.size(), sorted_v.size());
		//OutputDebugString(tt);
		////////////////////////////////////////////////////////		

		sort(sorted_v.begin(), sorted_v.end(), compare_maxdist);
		MkFN_search(qry_seq, sorted_v);
	}



	// 결과를 출력한다. hyungju hyungju
	// print_qry_result();
}


void CTNR::print_qry_result()
{
	for (auto& q : m_map_qry)
	{
		CString dbg_str;

		vector<pair<int, float>>& result = q.second.get_result();

		int q_id = q.first;



		dbg_str.Format(_T("%d ( %d )\n"), q_id, q.second.num_of_fns);
		Print_debug_out(dbg_str, true);


		for (auto& obj : result)
		{
			int obj_id = obj.first;
			float dist_q_obj = obj.second;

			dbg_str.Format(_T("%d %f \n"), obj_id, dist_q_obj);
			Print_debug_out(dbg_str, true);
		}
		dbg_str.Format(_T("-------------\n\n"));
		Print_debug_out(dbg_str, true);
	}

}