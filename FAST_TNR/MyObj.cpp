#include "StdAfx.h"
#include "MyObj.h"


CMyObj::CMyObj(void)
{
	this->type	= -1;
	this->id	= -1;
	
	this->prev_x			= -1;	// ���� x ��ġ
	this->prev_y			= -1;	// ���� y ��ġ
	this->cur_x			= -1;	// ���� x ��ġ
	this->cur_y			= -1;	// ���� y ��ġ
		
	this->cur_edgeid		= -1;	// ���� seqid
	this->prev_edgeid	= -1;	// ���� seqid

	this->s_node		= -1;
	this->e_node		= -1;

	// this->related_qries.clear();	// key = qryid, value = qryid

}


CMyObj::~CMyObj(void)
{
}
