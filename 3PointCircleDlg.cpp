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
	, m_bIsDragging(false)
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

// [추가 1] 테두리 그리기 (Midpoint Circle Algorithm)
// 정수 연산만 사용하여 속도가 빠르고 픽셀이 정교함
void CMy3PointCircleDlg::DrawMidpointCircle(CDC* pDC, CPoint center, int radius, COLORREF color)
{
	int x = 0;
	int y = radius;
	int p = 1 - radius;

	while (x <= y) {
		// 8방향 대칭 점 찍기 (SetPixelV가 SetPixel보다 조금 더 빠름)
		pDC->SetPixelV(center.x + x, center.y + y, color);
		pDC->SetPixelV(center.x - x, center.y + y, color);
		pDC->SetPixelV(center.x + x, center.y - y, color);
		pDC->SetPixelV(center.x - x, center.y - y, color);
		pDC->SetPixelV(center.x + y, center.y + x, color);
		pDC->SetPixelV(center.x - y, center.y + x, color);
		pDC->SetPixelV(center.x + y, center.y - x, color);
		pDC->SetPixelV(center.x - y, center.y - x, color);

		x++;
		if (p < 0) {
			p += 2 * x + 1;
		}
		else {
			y--;
			p += 2 * (x - y) + 1;
		}
	}
}

// [추가 2] 내부 채우기 (Scanline Fill Algorithm)
// 점을 하나씩 찍는 대신, 가로선(LineTo)을 그어 속도를 극대화함
void CMy3PointCircleDlg::DrawFilledCircle(CDC* pDC, CPoint center, int radius, COLORREF color)
{
	CPen pen(PS_SOLID, 1, color);
	CPen* pOldPen = pDC->SelectObject(&pen);

	int x = 0;
	int y = radius;
	int p = 1 - radius;

	while (x <= y) {
		// 좌우 대칭 점을 잇는 가로선을 그음
		pDC->MoveTo(center.x - x, center.y + y); pDC->LineTo(center.x + x, center.y + y);
		pDC->MoveTo(center.x - x, center.y - y); pDC->LineTo(center.x + x, center.y - y);
		pDC->MoveTo(center.x - y, center.y + x); pDC->LineTo(center.x + y, center.y + x);
		pDC->MoveTo(center.x - y, center.y - x); pDC->LineTo(center.x + y, center.y - x);

		x++;
		if (p < 0) {
			p += 2 * x + 1;
		}
		else {
			y--;
			p += 2 * (x - y) + 1;
		}
	}
	pDC->SelectObject(pOldPen);
}

// [추가 3] 두께 처리 (Thickness Logic)
// 입력된 두께만큼 반지름을 조절하며 여러 번 그림
void CMy3PointCircleDlg::DrawThickCircle(CDC* pDC, CPoint center, int radius, int thickness)
{
	// 중심 기준으로 안팎으로 퍼지게 계산
	int start = -(thickness - 1) / 2;
	for (int k = 0; k < thickness; ++k) {
		int r = radius + start + k;
		if (r > 0) {
			DrawMidpointCircle(pDC, center, r, RGB(0, 0, 0));
		}
	}
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

void CMy3PointCircleDlg::UpdateCoordUI()
{
	CString strVal, strTemp;
	if (m_iClickCount >= 1) { strTemp.Format(_T("P1(%d, %d)  "), m_ptClicks[0].x, m_ptClicks[0].y); strVal += strTemp; }
	if (m_iClickCount >= 2) { strTemp.Format(_T("P2(%d, %d)  "), m_ptClicks[1].x, m_ptClicks[1].y); strVal += strTemp; }
	if (m_iClickCount >= 3) { strTemp.Format(_T("P3(%d, %d)\r\n"), m_ptClicks[2].x, m_ptClicks[2].y); strVal += strTemp; }
	if (m_iClickCount == 3) {
		CPoint center; double radius;
		if (GetCircumCircle(m_ptClicks[0], m_ptClicks[1], m_ptClicks[2], center, radius)) {
			strTemp.Format(_T("Center(%d, %d)  Radius: %.1f"), center.x, center.y, radius); strVal += strTemp;
		}
		else strVal += _T("일직선입니다 (원 불가)");
	}
	GetDlgItem(IDC_STATIC_COORD)->SetWindowText(strVal);
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
		return;
	}
	else
	{
		CPaintDC dc(this);

		// [변경] 점 그리기 -> DrawFilledCircle 사용 (속도 빠름)
		for (int i = 0; i < m_iClickCount; i++) {
			DrawFilledCircle(&dc, m_ptClicks[i], m_iPointRadius, RGB(0, 0, 0));
		}

		// [변경] 외접원 그리기 -> DrawThickCircle 사용 (정교한 두께)
		if (m_iClickCount == 3) {
			CPoint center;
			double radius = 0;
			// GetCircumCircle은 Day 6 최종본의 fabs, sqrt 적용된 버전 사용
			if (GetCircumCircle(m_ptClicks[0], m_ptClicks[1], m_ptClicks[2], center, radius)) {

				// 여기서 새로운 함수 호출
				DrawThickCircle(&dc, center, (int)radius, m_iLineThickness);

				// 좌표 텍스트 갱신은 별도 함수(UpdateCoordUI)나 기존 방식 유지
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