#pragma once
#include "afx.h"
#include "MyObj.h"
#include "DataObj.h"  

#include <vector>  
#include <map>  



using namespace std;  

class CMyBNode
{
public:
	CMyBNode(void);
	~CMyBNode(void);

	int nodeid;

	vector<int> v_adj_seq;

	

	// ��忡�� ������ ������ ���� �ִ� ��ü���� �����Ѵ�. 
	// map<int,CMyObjInfo> map_data_obj; // key = objid, value = distance to node
	// map<int,CMyObjInfo> map_qry_obj;  // key = objid, value = distance to node
	

	// map<int,int> map_data_obj; // key = objid, value = distance to node
	

	// CMyBNode& operator=(const CMyBNode &rhs) ;


};

