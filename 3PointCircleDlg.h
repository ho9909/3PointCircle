
// 3PointCircleDlg.h: 헤더 파일
//

#pragma once
#include "afxwin.h"
#include <atomic>

#define WM_UPDATE_RANDOM_MOVE (WM_USER + 1)

struct RandomMovePayload { CPoint pts[3]; };


// CMy3PointCircleDlg 대화 상자
class CMy3PointCircleDlg : public CDialogEx
{
// 생성입니다.
public:
	CMy3PointCircleDlg(CWnd* pParent = nullptr);	// 표준 생성자

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MY3POINTCIRCLE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원



protected:
	HICON m_hIcon;

	CPoint m_ptClicks[3];       // 클릭한 3개의 좌표
	int m_iClickCount;          // 현재 클릭된 횟수 (0~3)

	int m_iPointRadius;         // 클릭 지점 원의 반지름 (사용자 입력)
	int m_iLineThickness;       // 정원 가장자리 두께 (사용자 입력)

	bool m_bIsDragging;        // 드래그 중인지 여부
	int m_iDragPointIndex;     // 드래그 중인 점의 인덱스 (0, 1, 2)
	//bool m_bThreadRunning;
	std::atomic<bool> m_bThreadRunning;
	std::atomic<bool> m_bStopFlag;

	//화면 영역 캐싱 변수 및 함수
	CRect m_rcDraw;      // 그림이 그려질 안전한 영역
	CRect GetDrawRect(); // 컨트롤을 피해서 그리기 영역을 계산하는 함수

	static UINT RandomMoveThread(LPVOID pParam); //스레드 함수


	//void DrawCustomCircle(CDC* pDC, CPoint center, double radius, int thickness, bool bFill);
	bool GetCircumCircle(CPoint p1, CPoint p2, CPoint p3, CPoint& outCenter, double& outRadius);

	// 그리기 알고리즘 (성능 개선 및 품질 향상)
	void DrawFilledCircle(CDC* pDC, CPoint center, int radius, COLORREF color);    // Scanline (속도 빠름)
	void DrawMidpointCircle(CDC* pDC, CPoint center, int radius, COLORREF color);  // Midpoint (테두리 정교함)
	void DrawThickCircle(CDC* pDC, CPoint center, int radius, int thickness);      // 두께 처리
	


	void UpdateCoordUI();
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnReset();   // [초기화] 버튼
	afx_msg void OnBnClickedBtnRandom();  // [랜덤 이동] 버튼
	afx_msg LRESULT OnUpdateRandomMove(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
