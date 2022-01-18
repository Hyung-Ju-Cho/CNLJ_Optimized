#pragma once

#include <map>
using namespace std;

class CMyObj
{
public:
	CMyObj(void);
	~CMyObj(void);

	int type;
	int id;
	
	int prev_x;	// 이전 x 위치
	int prev_y;	// 이전 y 위치
	int cur_x;	// 현재 x 위치
	int cur_y;	// 현재 y 위치
		
	int cur_edgeid;		// 현재 seqid
	int prev_edgeid;	// 이전 seqid

	int s_node;
	int e_node;

	// map<int, int> related_qries;	// key = qryid, value = qryid
};

