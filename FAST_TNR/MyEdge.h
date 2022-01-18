#pragma once
#include "afx.h"
class CMyEdge 
{
public:
	CMyEdge(void);	
	CMyEdge(int edgeid, int node1, int node2, float edgelen);
	~CMyEdge(void);

	int edgeid;
	int node1;
	int node2;
	float edgelen;
	int seqid;

};

