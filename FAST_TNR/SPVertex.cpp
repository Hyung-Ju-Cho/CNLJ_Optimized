#include "stdafx.h"
#include "SPVertex.h"

SPVertex::SPVertex()
{
}


SPVertex::SPVertex(float dist, int vid, int prev_vid)
: m_dist(dist), m_vid(vid), m_prev_vid(prev_vid)
{

}

SPVertex::~SPVertex()
{

}