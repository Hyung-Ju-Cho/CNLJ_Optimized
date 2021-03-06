
// FAST_TNRDoc.cpp: CFASTTNRDoc 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "FAST_TNR.h"
#endif

#include "FAST_TNRDoc.h"
#include "CTNR.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFASTTNRDoc

IMPLEMENT_DYNCREATE(CFASTTNRDoc, CDocument)

BEGIN_MESSAGE_MAP(CFASTTNRDoc, CDocument)
END_MESSAGE_MAP()


CStdioFile debug_out;
CFile g_file_dist_transit_vtxes_to_transit_vtxes;

// CFASTTNRDoc 생성/소멸

extern LARGE_INTEGER start_time();
extern void elapsed_time(CString & msg, LARGE_INTEGER & sPos, bool print = false);
extern void mytrace(const char* format, ...);


CTNR tnr;

CFASTTNRDoc::CFASTTNRDoc() noexcept
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.




	   	  

	MyMain();

}

CFASTTNRDoc::~CFASTTNRDoc()
{
}

BOOL CFASTTNRDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}





void CFASTTNRDoc::MyMain()
{
	//LARGE_INTEGER s1;
	//s1 = start_time();

	tnr.Get_Params_from_File();

	tnr.Read_map_file();

	///* -------------------------------------- */
	//CString str;
	//str.Format(_T("** tnr.Read_map_file() \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */



	tnr.Gen_BNode_Seq();

	///* -------------------------------------- */
	//str.Format(_T("** tnr.Gen_BNode_Seq() \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */









	tnr.Draw_grids();

	///* -------------------------------------- */
	//str.Format(_T("** 	tnr.Draw_grids() \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */



	tnr.Find_crossing_edges_in_cell();


	///* -------------------------------------- */
	//str.Format(_T("** 	tnr.Find_crossing_edges_in_cell() \n"));
	//Print_debug_out(str);
	/* -------------------------------------- */





	tnr.Find_transit_vtxes_for_cell();

	///* -------------------------------------- */
	//str.Format(_T("** 	tnr.Find_transit_vtxes_for_cell() \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */



	///* -------------------------------------- */
	//str.Format(_T("** before Dist_inside_vtxes_to_transit_vtxes \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */

	tnr.Dist_inside_vtxes_to_transit_vtxes();
	
	///* -------------------------------------- */
	//str.Format(_T("** after Dist_inside_vtxes_to_transit_vtxes \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */




	///* -------------------------------------- */
	//str.Format(_T("** before Dist_transit_vtxes_to_transit_vtxes \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */
	
	tnr.Dist_transit_vtxes_to_transit_vtxes();
	
	///* -------------------------------------- */
	//str.Format(_T("** after Dist_transit_vtxes_to_transit_vtxes \n"));
	//Print_debug_out(str);
	///* -------------------------------------- */


	Answer_queries();

	exit(0);
}

void CFASTTNRDoc::Answer_queries()
{
	LARGE_INTEGER s1;
	s1 = start_time(); 
	extern int g_num_kfn_queries;
	extern set<int> g_qry_nodes;

	vector<CString>& query_fnames = tnr.get_query_fname();
	vector<CString>& data_fnames = tnr.get_data_fname();
	
	vector<int>& min_num_of_fns = tnr.get_min_num_of_fns();
	vector<int>& max_num_of_fns = tnr.get_max_num_of_fns();

	

	for (int i = 0; i < query_fnames.size(); i++)
	{		
		g_num_kfn_queries = 0;
		g_qry_nodes.clear();
		
			
		// queries를 배치하고, grouping시킨다. 여기서 부터 확인하자. hyungju hyungju
		tnr.Populate_queries(query_fnames[i], min_num_of_fns[i], max_num_of_fns[i]);

		// objects를 배치하고, grouping시킨다. 여기서 부터 확인하자. hyungju hyungju
		tnr.Populate_objects(data_fnames[i]);

		tnr.Group_queries_objects();


		CString msg = _T("before");
		elapsed_time(msg, s1);
		s1 = start_time();

		//--------------------------------------------------------------------------------//
		// 여기서부터 디버깅합니다. hyungju hyungju (2019년 11월 24일)
		//--------------------------------------------------------------------------------//
		tnr.Comp_min_max_dist();

		// 삭제하면 안 된다.
		msg = _T("tnr.Comp_min_max_dist()");
		elapsed_time(msg, s1, true);

		tnr.clear_objects();
	}	

}


// CFASTTNRDoc serialization

void CFASTTNRDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}



// CFASTTNRDoc 진단

#ifdef _DEBUG
void CFASTTNRDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFASTTNRDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFASTTNRDoc 명령
