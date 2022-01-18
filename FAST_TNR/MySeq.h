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
	int s_node;	// ���۳��
	int e_node;	// �����
	vector<int> v_nodes; // seq�� ���Ե� node ���� ����

	float seqlen;

	map<int, float> obj_list;			// key = outer_obj_id,	value = distance to base node
	vector<pair<int, float>> qry_list;			// key = outer_obj_id,	value = distance to base node


	int obj_seg_id;	// object segment id
	int qry_seg_id;	// query segment id






};

