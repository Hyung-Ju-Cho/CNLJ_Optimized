#include "StdAfx.h"
#include "MySeq.h"
#include <ASSERT.h>

CMySeq::CMySeq(void)
{
	ASSERT(this->obj_list.size() == 0);
}



CMySeq::~CMySeq(void)
{	
	this->obj_list.clear(); // key = outer_obj_id, value = distance from base node to outer obj
}



