#pragma once
class CMyLine
{
public:
	CMyLine();
	~CMyLine();
	CMyLine(POINT pt1, POINT pt2);
	CMyLine(LONG pt1x, LONG pt1y, LONG pt2x, LONG pt2y);

	bool IntersectLine(CMyLine & line);
	bool CheckCollinear(CMyLine & line);

public:
	POINT m_pt1;
	POINT m_pt2;
};

