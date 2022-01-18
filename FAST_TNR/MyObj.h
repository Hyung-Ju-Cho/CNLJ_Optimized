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
	
	int prev_x;	// ���� x ��ġ
	int prev_y;	// ���� y ��ġ
	int cur_x;	// ���� x ��ġ
	int cur_y;	// ���� y ��ġ
		
	int cur_edgeid;		// ���� seqid
	int prev_edgeid;	// ���� seqid

	int s_node;
	int e_node;

	// map<int, int> related_qries;	// key = qryid, value = qryid
};

