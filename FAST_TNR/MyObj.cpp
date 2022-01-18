#include "StdAfx.h"
#include "MyObj.h"


CMyObj::CMyObj(void)
{
	this->type	= -1;
	this->id	= -1;
	
	this->prev_x			= -1;	// 이전 x 위치
	this->prev_y			= -1;	// 이전 y 위치
	this->cur_x			= -1;	// 현재 x 위치
	this->cur_y			= -1;	// 현재 y 위치
		
	this->cur_edgeid		= -1;	// 현재 seqid
	this->prev_edgeid	= -1;	// 이전 seqid

	this->s_node		= -1;
	this->e_node		= -1;

	// this->related_qries.clear();	// key = qryid, value = qryid

}


CMyObj::~CMyObj(void)
{
}
