#pragma once
#include <utility>
#include "DataObj.h"
using namespace std;


class CObjSeg
{
public:
	CObjSeg(void);
	~CObjSeg(void);

	int id;
	pair<int, float> dist_start_pair;
	pair<int, float> dist_end_pair;

private:
};


class CQrySeg : public CObjSeg
{
public:
	CQrySeg(void);
	~CQrySeg(void);

	// int id;
	// pair<int, float> dist_start_pair;
	// pair<int, float> dist_end_pair;

private:
};

