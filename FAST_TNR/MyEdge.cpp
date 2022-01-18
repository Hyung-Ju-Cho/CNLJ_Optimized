#include "StdAfx.h"
#include "MyEdge.h"


CMyEdge::CMyEdge(void)
{
}


CMyEdge::CMyEdge(int edgeid, int node1, int node2, float edgelen)
{
	this->edgeid	= edgeid;
	this->node1		= node1;
	this->node2		= node2;
	this->edgelen	= edgelen;

}



CMyEdge::~CMyEdge(void)
{
}
