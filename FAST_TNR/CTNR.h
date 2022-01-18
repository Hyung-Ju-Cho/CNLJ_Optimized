#pragma once

#include <map>
#include "Matrix.h"
#include "CGridCell.h"
#include "SPVertex.h"
#include "MyNode.h"
#include "MyEdge.h"
#include "MyBNode.h"
#include "MySeq.h"
#include "DataObj.h"
#include "DataSeg.h"
#include <assert.h>
//-------------------------------------------------------------------------------------//
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

using namespace std;


// Program to find Dijkstra's shortest path using
// priority_queue in STL
// #include<bits/stdc++.h>

# define INF 0x3f3f3f3f

// iPair ==> Integer Pair
typedef pair<int, float> ifPair;
typedef pair<float, int> fiPair;







namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<int, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
typedef pair<box, unsigned> value;


typedef struct _MYVERTEX
{
	int vid;
	POINT loc;
} MYVERTEX;

typedef struct _MYEDGE
{
	int vid1;
	int vid2;
	float dist;
} MYEDGE;





class cmp_seg_pair_dist
{
public:
	bool operator()(const pair<pair<int, float>, pair<int, float>>& a, const pair<pair<int, float>, pair<int, float>>& b) const 
	{
		if (a.first == b.first && a.second == b.second)
		{
			return false;
		}
		
		if (a.first != b.first)
		{
			if (a.first.first != b.first.first)
				return (a.first.first < b.first.first);
			else
				return (a.first.second < b.first.second);
		}

		if (a.second != b.second)
		{
			if (a.second.first != b.second.first)
				return (a.second.first < b.second.first);
			else
				return (a.second.second < b.second.second);
		}
		
		assert(0);
	}


};


class CTNR
{
public:
	CTNR();
	~CTNR();

	void set_edge_to_tbl(CMyEdge & edge);
	void set_node_to_tbl(CMyNode & node);
	void set_bnode_to_tbl(CMyBNode & bnode);
	void set_seq_to_tbl(CMySeq & seq);
	void set_obj_to_tbl(CObj & obj);
	void set_qry_to_tbl(CQry& qry);

	CQry& get_qry_from_tbl(int qry_id);

	void set_obj_seg_to_tbl(CObjSeg& obj_seg);
	void set_qry_seg_to_tbl(CQrySeg& qry_seg);
	void set_minmaxdist_to_tbl(pair<int, int> & obj_seg_pair, pair<float, float> & min_max_dist);


	CMyEdge & get_edge_from_tbl(int edgeid);
	CMyNode & get_node_from_tbl(int nodeid);

	CMyBNode & get_bnode_from_tbl(int bnodeid);
	CMySeq & get_seq_from_tbl(int seqid);
	CObjSeg & get_obj_seg_from_tbl(int outer_seg_id);
	CQrySeg& get_qry_seg_from_tbl(int qry_seg_id);

	float GetEdgeLen(int nodeid1, int nodeid2);

	void Populate_objects(CString data_fname);
	void Populate_queries(CString query_fname, int min_num_of_fns, int max_num_of_fns);

	void add_obj_to_seq(CObj & obj);
	void add_qry_to_seq(CQry & qry);

	int Get_seqid_from_edgeid(int edgeid);
	float Distance_bnode_to_dobj(CObj & dobj, CMyBNode & node, CMySeq & seq);
	void Group_queries_objects();
	void Group_objects_in_seq(CMySeq & seq);
	void Group_queries_in_seq(CMySeq & seq);


	void Gen_BNode_Seq();
	int MakeSeq(int s_nodeid, int adj_nodeid, multimap<int, int> & mm);
	float MakeSeqLen(vector<int> & v);
	int chk_dup_seq(CMySeq & seq, multimap<int, int> & mm);
	bool is_twin(CMySeq & seq1, CMySeq & seq2);
	int get_edge_id(int s_node, int e_node);

	void Get_Params_from_File();
	void Draw_grids();
	void Make_map_vertex_to_edge();
	void Make_vertex_info();
	void Make_edge_info();
	void Find_crossing_edges_in_cell();
	void Find_crossing_edges_in_gridcell(CGridCell & gridcell);
	void Find_crossing_edges_in_rect(const RECT & extent, set<int> & crossing_edges);
	void Find_edges_inside_rect(const RECT & extent, set<int> & inside_vertexes);

	void Find_transit_vtxes_for_cell();
	void Find_transit_vtxes_for_cell(CGridCell & cell);
	void Find_transit_vtxes_for_cell(int src_vid, set<int> & outer_vertexes, set<int> & inner_vertexes, map<int, SPVertex> & map_dstvtx_dist);
	void Retrieve_transit_vtxes(const vector<int> & path, const set<int> & inner_vertexes, map<int, float> & transit_vtxes);

	void Pick_crossing_vertexes(const set<int> & crossing_edges, set<int> & crossing_vertexes);


	int Insert_Edges_to_RTree(CStdioFile & vertex_file, bgi::rtree< value, bgi::quadratic<64> > & rtree);

	void Dist_transit_vtxes_to_transit_vtxes();
	void Dist_inside_vtxes_to_transit_vtxes();
	void Dist_inside_vtxes_to_transit_vtxes(CGridCell & cell);

	int get_cell_width();
	int get_cell_height();
	void Read_map_file();
	void _Read_map_file();
	void Read_map_file2();

	// 두 점 사이의 거리를 계산한다.
	float Road_dist(CMyNode & pt1, CMyNode & pt2);
	float Road_dist(pair<CMySeq, float> & x, pair<CMySeq, float> & y);
	float Road_dist(CObjSeg & src_seg, float src_loc, CObjSeg & dst_seg, float dst_loc);

	void ShortestPath(const int src_vid, const set<int> & dst_vtxes, map<int, SPVertex> & map_dstvtx_dist);
	void ShortestPath(const int src_vid, const set<int> & dst_vtxes, map<int, float> & map_dstvtx_dist);
	void ShortestPath(const int src_vid, const set<int> & dst_vtxes, vector<pair<int, float>> & dst_dist);

	void ShortestPath(vector<pair<int, float>> adj[], int V, int src, int dst_vid);
	//void ShortestPath(const int src_vertex, const set<int> & dst_vtxes);
	void ShortestPath(const int src_vid, const int dst_vid);
	vector<int> Retrieve_ShortestPath(const map<int, SPVertex> & map_dstvtx_dist, const int src_vid, const int dst_vid);

	vector<int> Retrieve_ShortestPath(vector<int> & prev_vid, const int src_vid, const int dst_vid);

	void Get_adjacent_vertexes(int vid, vector<pair<int, float>> & result);
	
	float Comp_network_dist(int src_vid, int dst_vid);
	pair<int,int> Get_cell_ID(const POINT & pt);

	float Search_network_dist(int src_vid, int dst_vid);
	float Lookup_network_dist(int src_vid, int dst_vid);
	void Dist_transit_vtxes_to_transit_vtxes(set<int> & all_transit_vtxes);
	void Write_dist_transit_vtxes_to_transit_vtxes(CFile & output);
	void Retrieve_dist_transit_vtxes_to_transit_vtxes(CFile & input);
	void Store_dist_transit_vtxes_to_transit_vtxes();

	void Comp_min_max_dist();
	pair<float, float> Comp_dist_two_segs(CMySeq& qry_seq, CMySeq& obj_seq);
	void MkFN_search(CMySeq & seq, const vector<pair<int, pair<float, float>>> & v);
	void kFN_query(CQrySeg& qry_seg, const float qry_loc, CQry& qry, const vector<pair<int, pair<float, float>>>& sorted_obj_seg_list, const set<int>& candidates, const float offset = 0);
	int find_max_num_of_fns(CMySeq& qry_seq);
	void kFN_search_using_candidates(CMySeq& qry_seq, CQry& tmp_qry_st, CQry& tmp_qry_ed, float qst_loc, float qed_loc);

	pair<float, float> CTNR::Comp_min_max_dist(CMySeq & x, CMySeq & y);

	float Update_result(int src_oid, int dst_oid, float dist);
	void store_seg_pair_dist(pair<int, float> & seg1, pair<int, float> & seg2, float dist);
	float retrieve_seg_pair_dist(pair<int, float> & seg1, pair<int, float> & seg2);
	int Get_num_adj_segs(CMyBNode & bnode);

	void store_bnode_pair_dist(int bnode1, int bnode2, float dist);
	float retrieve_bnode_pair_dist(int bnode1, int bnode2);
	void Comp_bnode_pair_dist();

	int get_map_obj_seg_size() { return m_map_obj_seg.size(); }

	void print_qry_result();
	

	map<int, CQrySeg>& get_map_qry_seg() { return m_map_qry_seg; }
	map<int, CQry>& get_map_qry() { return m_map_qry; }

	vector<CString>& get_data_fname();
	vector<CString>& get_query_fname();

	vector<int>& get_min_num_of_fns();
	vector<int>& get_max_num_of_fns();

	float get_max_span(CMyBNode& bnode);

	void clear_objects();

private:
	int m_srand_seed;		// srand seed 값
	vector<int> m_min_num_of_fns;	// 최소 fns 개수
	vector<int> m_max_num_of_fns;	// 최대 fns 개수


	int m_num_X_grids;	// 가로를 구성하는 그리드 갯수
	int m_num_Y_grids;	// 세로를 구성하는 그리드 갯수
	CString m_node_fname;	// vertex 파일 이름
	CString m_edge_fname;	// edge 파일 이름
	int m_xmax;					// x축 최대 길이
	int m_ymax;					// y축 최대 길이
	vector<CString> m_data_fname;		// data file name
	vector<CString> m_query_fname;		// query file name

	CString m_fn_dist_transit_vtxes;
	bool m_read_dist_transit_vtxes; // true: 파일에서 데이터를 읽는다. false: 데이터를 생성하여 파일에 저장한다. 

	int m_cell_width;
	int m_cell_height;
	matrix <CGridCell> m_grid;

	// create the rtree using default constructor
	bgi::rtree< value, bgi::quadratic<64> > m_rtree;


	map<int, MYEDGE> m_edge_to_vertex_map;	// key = edge id, value = pair of two vertices that consist of the edge
	multimap<int, int> m_vertex_to_edge_map;				// key = vertex id, value = edge id


	map<int, CMyBNode>	m_mapMyBNode;
	map<int, CMySeq>	m_mapMySeq;
	map<int, CMyNode>	m_mapMyNode;
	map<int, CMyEdge>	m_mapMyEdge;

	map<pair<int, int>, float> m_dist_transit_vtxes_to_transit_vtxes;
	map<pair<int, int>, int> m_mapNode2Edge;
	map<int, CObj> m_map_obj;
	map<int, CObjSeg> m_map_obj_seg;


	map<int, CQry> m_map_qry;	// query 객체에 대한 정보를 저장한다. 
	map<int, CQrySeg> m_map_qry_seg;

	map<int, CQry> m_map_bnode_qry_result;	// query 객체에 대한 정보를 저장한다. 




	// map<pair<int,int>, pair<float,float>> m_mapMinMaxDist;

	// map<int, pair<int,float>> m_result;

	map< pair<pair<int, float>, pair<int, float>>, float, cmp_seg_pair_dist> m_seg_pair_dist;
	// map< pair<pair<int, float>, pair<int, float>>, float> m_seg_pair_dist;
	map< pair<int,int>, float> m_bnode_pair_dist;


	int m_num_nodes;		// number of nodes
	int m_num_edges;		// number of edges

	friend class cmp_seg_pair_dist;
};


extern void Print_debug_out(CString & str, bool print = false);