
// 3PointCircleDlg.h: 헤더 파일
//

#pragma once
#include "afxwin.h"

#define WM_UPDATE_RANDOM_MOVE (WM_USER + 1)

// CMy3PointCircleDlg 대화 상자
class CMy3PointCircleDlg : public CDialogEx
{
// 생성입니다.
public:
	CMy3PointCircleDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MY3POINTCIRCLE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	CPoint m_ptClicks[3];       // 클릭한 3개의 좌표
	int m_iClickCount;          // 현재 클릭된 횟수 (0~3)

	int m_iRadius;
	int m_iThickness;


	int m_iPointRadius;         // 클릭 지점 원의 반지름 (사용자 입력)
	int m_iLineThickness;       // 정원 가장자리 두께 (사용자 입력)

	bool GetCircumCircle(CPoint p1, CPoint p2, CPoint p3, CPoint& outCenter, double& outRadius);
	void DrawCustomCircle(CDC* pDC, CPoint center, double radius, int thickness, bool bFill);

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnReset();   // [초기화] 버튼
	afx_msg void OnBnClickedBtnRandom();  // [랜덤 이동] 버튼
	DECLARE_MESSAGE_MAP()
};
