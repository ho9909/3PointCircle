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
	, m_iPointRadius(10)      
	, m_iLineThickness(2)     
	, m_bThreadRunning(false)
	, m_bStopFlag(false)
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
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정

	return TRUE;
}

void CMy3PointCircleDlg::DrawCustomCircle(CDC* pDC, CPoint center, double radius, int thickness, bool bFill)
{
	CPen pen(PS_SOLID, thickness, RGB(0, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&pen);

	if (bFill)
	{
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

			// 계산된 점을 기준으로 대칭되는 4개의 가로선을 그어 내부를 채우기
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
			DrawCustomCircle(&dc, m_ptClicks[i], m_iPointRadius, 1, true);
		}

		// 외접원 그리기
		if (m_iClickCount == 3) {
			CPoint center;
			double radius = 0;
			if (GetCircumCircle(m_ptClicks[0], m_ptClicks[1], m_ptClicks[2], center, radius)) {

				DrawCustomCircle(&dc, center, radius, m_iLineThickness, false);

				CString str;
				str.Format(_T("Center(%d,%d) R:%.1f"), center.x, center.y, radius);
				GetDlgItem(IDC_STATIC_COORD)->SetWindowText(str);
			}
		}
	}
}

HCURSOR CMy3PointCircleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMy3PointCircleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	UpdateData(TRUE); // 입력창 값 가져오기

	if (m_iClickCount == 3) {
		for (int i = 0; i < 3; i++) {
			double dist = sqrt(pow(point.x - m_ptClicks[i].x, 2) + pow(point.y - m_ptClicks[i].y, 2));
			if (dist <= m_iPointRadius + 5) {
				m_iDragPointIndex = i;
				m_bIsDragging = true;
				SetCapture();
				return;
			}
		}
	}
		
	if (m_iClickCount < 3)
	{
		m_ptClicks[m_iClickCount] = point;
		m_iClickCount++;
		Invalidate(); // 화면 갱신 요청
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CMy3PointCircleDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bIsDragging) {
		m_bIsDragging = false;
		m_iDragPointIndex = -1;
		ReleaseCapture();
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CMy3PointCircleDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsDragging && m_iDragPointIndex != -1) {
		m_ptClicks[m_iDragPointIndex] = point;
		Invalidate();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CMy3PointCircleDlg::OnBnClickedBtnReset()
{
	m_iClickCount = 0;
	m_bIsDragging = false;
	m_iDragPointIndex = -1;
	m_bThreadRunning = false;
	GetDlgItem(IDC_STATIC_COORD)->SetWindowText(_T("초기화"));
	Invalidate();

}

void CMy3PointCircleDlg::OnBnClickedBtnRandom()
{
	if (m_iClickCount < 3)return;
	if (m_bThreadRunning)return;

	m_bThreadRunning = true;
	AfxBeginThread(RandomMoveThread, this); // 스레드
}

UINT CMy3PointCircleDlg::RandomMoveThread(LPVOID pParam) {
	CMy3PointCircleDlg* pDlg = (CMy3PointCircleDlg*)pParam;
	CRect rect;
	pDlg->GetClientRect(&rect);

	for (int i = 0; i < 10; i++) {
		if (!pDlg->m_bStopFlag.load()) {
			break;
		}
		RandomMovePayload* pData = new RandomMovePayload();

		for (int j = 0; j < 3; j++) {
			pDlg->m_ptClicks[j].x = rand() % (rect.Width() - 50) + 25;
			pDlg->m_ptClicks[j].y = rand() % (rect.Height() - 50) + 25;
		}
		::PostMessage(pDlg->m_hWnd, WM_UPDATE_RANDOM_MOVE, 0, (LPARAM)pData);
		Sleep(500);
	}
	pDlg->m_bThreadRunning = false;
	return 0;
}

LRESULT CMy3PointCircleDlg::OnUpdateRandomMove(WPARAM wParam, LPARAM lParam) {
	RandomMovePayload* pData = (RandomMovePayload*)lParam;
	if (!pData) return 0;
	if (m_bStopFlag.load()) { delete pData; return 0; }

	for (int i = 0; i < 3; i++) m_ptClicks[i] = pData->pts[i];
	delete pData;
	Invalidate();
	return 0;
}