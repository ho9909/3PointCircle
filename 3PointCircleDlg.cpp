// 3PointCircleDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "3PointCircle.h"
#include "3PointCircleDlg.h"
#include "afxdialogex.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMy3PointCircleDlg 대화 상자
const double PI = 3.14159265358979323846;


CMy3PointCircleDlg::CMy3PointCircleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MY3POINTCIRCLE_DIALOG, pParent)
	, m_iPointRadius(10)      // 변수명 확인: m_iPointRadius
	, m_iLineThickness(2)     // 변수명 확인: m_iLineThickness
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_iClickCount = 0;
}

void CMy3PointCircleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_RADIUS, m_iPointRadius);
	DDX_Text(pDX, IDC_EDIT_THICKNESS, m_iLineThickness);
}

BEGIN_MESSAGE_MAP(CMy3PointCircleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_RESET, &CMy3PointCircleDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_RANDOM, &CMy3PointCircleDlg::OnBnClickedBtnRandom)
END_MESSAGE_MAP()


// CMy3PointCircleDlg 메시지 처리기

BOOL CMy3PointCircleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	return TRUE;
}

void CMy3PointCircleDlg::DrawCustomCircle(CDC* pDC, CPoint center, double radius, int thickness, bool bFill)
{
	CPen pen(PS_SOLID, thickness, RGB(0, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&pen);

	if (bFill)
	{
		// [업그레이드된 로직] 중간점 원 알고리즘을 이용한 채우기
		// 원의 대칭성을 이용하여 1/8만 계산하고 나머지는 대칭으로 그립니다.
		int x = 0;
		int y = (int)radius;
		int p = 1 - (int)radius; // 초기 결정 파라미터

		// 중심선 그리기
		pDC->MoveTo(center.x - y, center.y);
		pDC->LineTo(center.x + y, center.y);

		while (x < y)
		{
			x++;
			if (p < 0)
			{
				p += 2 * x + 1;
			}
			else
			{
				y--;
				p += 2 * (x - y) + 1;
			}

			// 계산된 점을 기준으로 대칭되는 4개의 가로선을 그어 내부를 채웁니다.
			// 상단, 하단 부분 채우기
			pDC->MoveTo(center.x - x, center.y + y);
			pDC->LineTo(center.x + x, center.y + y);
			pDC->MoveTo(center.x - x, center.y - y);
			pDC->LineTo(center.x + x, center.y - y);

			// 좌측, 우측 부분 채우기
			pDC->MoveTo(center.x - y, center.y + x);
			pDC->LineTo(center.x + y, center.y + x);
			pDC->MoveTo(center.x - y, center.y - x);
			pDC->LineTo(center.x + y, center.y - x);
		}
	}
	else
	{
		// 테두리만 그리는 경우 (큰 원) -> 기존 방식 유지하되 품질 향상
		int segments = (int)(radius * 2.0);
		if (segments < 60) segments = 60; // 최소 세그먼트 수를 늘려 더 부드럽게

		double angleStep = 2 * PI / segments;

		pDC->MoveTo((int)(center.x + radius), center.y);

		for (int i = 1; i <= segments; ++i) {
			double angle = i * angleStep;
			pDC->LineTo((int)(center.x + radius * cos(angle)), (int)(center.y + radius * sin(angle)));
		}
	}

	pDC->SelectObject(pOldPen);
}

// [수정 포인트 1] double outRadius -> double& outRadius (참조자 & 추가 필수!)
bool CMy3PointCircleDlg::GetCircumCircle(CPoint p1, CPoint p2, CPoint p3, CPoint& outCenter, double& outRadius) {
	double x1 = p1.x; double y1 = p1.y;
	double x2 = p2.x; double y2 = p2.y;
	double x3 = p3.x; double y3 = p3.y;

	double distance = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

	if (abs(distance) < 0.01) return false;

	double centerX = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / distance;
	double centerY = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / distance;

	outCenter.x = (long)centerX;
	outCenter.y = (long)centerY;
	outRadius = sqrt(pow(centerX - x1, 2) + pow(centerY - y1, 2));

	return true;
}


void CMy3PointCircleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);

		// 점 그리기
		for (int i = 0; i < m_iClickCount; i++) {
			// [수정 포인트 2] m_iRadius -> m_iPointRadius (변수명 일치)
			DrawCustomCircle(&dc, m_ptClicks[i], m_iPointRadius, 1, true);
		}

		// 외접원 그리기
		if (m_iClickCount == 3) {
			CPoint center;
			double radius = 0;
			if (GetCircumCircle(m_ptClicks[0], m_ptClicks[1], m_ptClicks[2], center, radius)) {
				// [수정 포인트 2] m_iThickness -> m_iLineThickness (변수명 일치)
				DrawCustomCircle(&dc, center, radius, m_iLineThickness, false);

				// 좌표값 출력
				CString str;
				str.Format(_T("Center(%d,%d) R:%.1f"), center.x, center.y, radius);
				GetDlgItem(IDC_STATIC_COORD)->SetWindowText(str);
			}
		}
		// [수정 포인트 3] CDialogEx::OnPaint() 삭제함 (화면 덮어쓰기 방지)
	}
}

HCURSOR CMy3PointCircleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMy3PointCircleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	UpdateData(TRUE); // 입력창 값 가져오기

	if (m_iClickCount < 3)
	{
		m_ptClicks[m_iClickCount] = point;
		m_iClickCount++;
		Invalidate(); // 화면 갱신 요청
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

// 아래는 3일차에 구현할 빈 함수들 (그대로 둠)
void CMy3PointCircleDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CMy3PointCircleDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialogEx::OnMouseMove(nFlags, point);
}

void CMy3PointCircleDlg::OnBnClickedBtnReset()
{
}

void CMy3PointCircleDlg::OnBnClickedBtnRandom()
{
}