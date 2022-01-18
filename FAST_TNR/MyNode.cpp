#include "StdAfx.h"
#include "MyNode.h"

CMyNode::CMyNode(void)
{
	// this->num_adj_nodes = 0;
}

CMyNode::~CMyNode(void)
{
}

CMyNode::CMyNode(int x, int y)
{
	this->x				= x;
	this->y				= y;
	// this->num_adj_nodes = 0;
}