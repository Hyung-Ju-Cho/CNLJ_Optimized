#pragma once

#include <vector>
#include <map>
#include <utility>
#include <set>

using namespace std;


class CObj
{
public:
	CObj(void);
	~CObj(void);

	int id;
	int cur_x;
	int cur_y;
	int cur_edgeid;
	// int num_of_fns;

	// 여기서 부터 ... 
	// void add_ans_obj(std::pair<int, float> & new_inner_obj);
	// map<int, float> & get_ans_obj_list() { return ans_obj_list; }

	

// private: // 삭제하자. hyungju hyungju (0114)------------------------
//	map<int, float> ans_obj_list;

};


class CQry : public CObj
{
public:
	CQry(void);
	~CQry(void);

	// int id;
	// int cur_x;
	// int cur_y;
	// int cur_edgeid;
	int num_of_fns;


	// 여기서 부터 ... 
	// void add_ans_obj(std::pair<int, float>& new_inner_obj);
	// map<int, float>& get_ans_obj_list() { return ans_obj_list; }

	float update_result(const int new_obj_id, const float new_obj_dist); 
  
	float update_result(const int new_obj_id, const float new_obj_dist, const set<int>& candidates, const float offset);

	void copy_result(vector<pair<int, float>>& obj_list);

	vector<pair<int, float>> & get_result();


private:
	vector<pair<int, float>> ans_obj_list;
};


class MyPoint
{
public:
	double x;
	double y;

	MyPoint(); 
	MyPoint(double xx, double yy);
	MyPoint(const MyPoint & rhs);
	MyPoint & operator= (const MyPoint & rhs);
	

	bool operator==(const MyPoint &p) const
	{
		return x == p.x && y == p.y;
	};
};
