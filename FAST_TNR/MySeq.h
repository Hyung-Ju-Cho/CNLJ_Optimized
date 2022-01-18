#pragma once

#include <vector>
#include <queue>
#include "MyObj.h"
#include <map>
//#include "DataSeg.h"
//#include "DataObj.h"

using namespace std;

class CMySeq
{
public:
	CMySeq(void);
	~CMySeq(void);


	int seqid;	// seqid
	int s_node;	// 시작노드
	int e_node;	// 끝노드
	vector<int> v_nodes; // seq에 포함된 node 들의 정보

	float seqlen;

	map<int, float> obj_list;			// key = outer_obj_id,	value = distance to base node
	vector<pair<int, float>> qry_list;			// key = outer_obj_id,	value = distance to base node


	int obj_seg_id;	// object segment id
	int qry_seg_id;	// query segment id






};

