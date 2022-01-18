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

	

	// 노드에서 인접한 시퀀스 위에 있는 객체들을 저장한다. 
	// map<int,CMyObjInfo> map_data_obj; // key = objid, value = distance to node
	// map<int,CMyObjInfo> map_qry_obj;  // key = objid, value = distance to node
	

	// map<int,int> map_data_obj; // key = objid, value = distance to node
	

	// CMyBNode& operator=(const CMyBNode &rhs) ;


};

