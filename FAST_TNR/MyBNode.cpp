#include "StdAfx.h"
#include "MyBNode.h"
#include "MyObj.h"

CMyBNode::CMyBNode(void)
{
//	ASSERT(map_data_obj.size() == 0); // key = objid, value = distance to node
//	ASSERT(map_qry_obj.size () == 0); // key = objid, value = distance to node
}


CMyBNode::~CMyBNode(void)
{
}


#if 0

CMyBNode& CMyBNode::operator=(const CMyBNode &rhs) 
{
	if (this == &rhs) { return *this; }

	this->nodeid			= rhs.nodeid;
	this->observation_dist	= rhs.observation_dist;
	
	// vector<int> v_int;
	// vector<int>::iterator i = v_int.begin();

	// vector< CMyObj > vectobj;
	// vector< CMyObj >::iterator st = vectobj.begin();

	vector< CMyObj >::iterator st = rhs.v_myobj.end();


	/*
	for( vector< RoomUser >::iterator IterPos = RoomUsers.begin();
		IterPos != RoomUsers.end();
		++IterPos )
	{
		cout << "유저코드 : " << IterPos->CharCd << endl;
		cout << "유저레벨 : " << IterPos->Level << endl;
	}
	*/



	// vector<MyObj> list_objects;
	// vector<int> v_bnodes;


	return *this;
}
#endif