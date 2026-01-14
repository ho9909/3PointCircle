// 3PointCircleDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "3PointCircle.h"
#include "3PointCircleDlg.h"
#include "afxdialogex.h"
#include <cmath>
#include <ctime>
#include <cstdlib>

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
	ON_WM_ERASEBKGND() // 배경 지우기
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_RESET, &CMy3PointCircleDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_RANDOM, &CMy3PointCircleDlg::OnBnClickedBtnRandom)
	ON_MESSAGE(WM_UPDATE_RANDOM_DONE, &CMy3PointCircleDlg::OnUpdateRandomDone)
	ON_MESSAGE(WM_UPDATE_RANDOM_MOVE, &CMy3PointCircleDlg::OnUpdateRandomMove)

	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMy3PointCircleDlg 메시지 처리기

BOOL CMy3PointCircleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정

	std::srand((unsigned int)std::time(nullptr));

	return TRUE;
}

BOOL CMy3PointCircleDlg::OnEraseBkgnd(CDC* pDC) { 
	//return CDialogEx::OnEraseBkgnd(pDC);
	return TRUE; 
}

// 테두리 그리기 (Midpoint Circle Algorithm)
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

// 내부 채우기 (Scanline Fill Algorithm)
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

// 두께 처리 (Thickness Logic)
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
		// 더블 버퍼링 로직 시작
		CPaintDC dc(this); // 실제 화면 DC

		CRect rcClient;
		GetClientRect(&rcClient);

		// 2. 가상 화면(메모리 DC) 만들기
		CDC memDC;
		memDC.CreateCompatibleDC(&dc); // 화면과 호환되는 메모리 DC 생성

		// 3. 도화지(비트맵) 만들기
		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
		CBitmap* pOldBitmap = memDC.SelectObject(&bitmap); // 메모리 DC에 도화지 끼우기

		// 4. 가상 화면을 흰색으로 지우기 (배경색 칠하기)
		// OnEraseBkgnd를 막았으니 여기서 우리가 직접 지워줘야 함
		memDC.FillSolidRect(&rcClient, RGB(255, 255, 255));

		// 5. 그림 그리기

		// 점 그리기
		for (int i = 0; i < m_iClickCount; i++) {
			DrawFilledCircle(&memDC, m_ptClicks[i], m_iPointRadius, RGB(0, 0, 0));
		}

		// 외접원 그리기
		if (m_iClickCount == 3) {
			CPoint center;
			double radius = 0;

			// GetCircumCircle은 Day 6 버전(표준함수)을 쓰거나 기존 것 유지
			if (GetCircumCircle(m_ptClicks[0], m_ptClicks[1], m_ptClicks[2], center, radius)) {
				DrawThickCircle(&memDC, center, (int)radius, m_iLineThickness);

				// CString str;
				// str.Format(_T("..."));
				// memDC.TextOut(10, 10, str); 
			}
		}

		// 6. 완성된 그림을 실제 화면으로 고속 복사 (BitBlt)
		dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

		// 7. 자원 해제 (사용했던 비트맵 복구)
		memDC.SelectObject(pOldBitmap);

		// memDC, bitmap 등은 함수가 끝나면 자동으로 소멸됨
	}
}

HCURSOR CMy3PointCircleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMy3PointCircleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bThreadRunning.load()) return;

	UpdateData(TRUE);
	if (m_iClickCount == 3) {
		for (int i = 0; i < 3; i++) {
			double dx = point.x - m_ptClicks[i].x;
			double dy = point.y - m_ptClicks[i].y;
			if (std::sqrt(dx * dx + dy * dy) <= m_iPointRadius + 5) {
				m_iDragIndex = i;
				m_bIsDragging = true;
				SetCapture();
				return;
			}
		}
		return;
	}
	if (m_iClickCount < 3) {
		m_ptClicks[m_iClickCount] = point;
		m_iClickCount++;
		UpdateCoordUI();

		CRect r = GetDrawRect();
		InvalidateRect(&r, FALSE);
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CMy3PointCircleDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bIsDragging) {
		m_bIsDragging = false;
		m_iDragIndex = -1;
		ReleaseCapture();
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CMy3PointCircleDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsDragging && (nFlags & MK_LBUTTON) && m_iDragIndex != -1) {
		CRect r = GetDrawRect();

		int margin = m_iPointRadius;
		point.x = max(r.left + margin, min(r.right - 1 - margin, point.x));
		point.y = max(r.top + margin, min(r.bottom - 1 - margin, point.y));

		m_ptClicks[m_iDragIndex] = point;
		UpdateCoordUI();
		InvalidateRect(&r, FALSE);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CMy3PointCircleDlg::OnBnClickedBtnReset()
{
	if (GetCapture() == this) ReleaseCapture();

	m_bStopFlag.store(true);
	KillTimer(TIMER_THREAD_CLEANUP);

	if (m_pRandomThread) {
		DWORD r = WaitForSingleObject(m_pRandomThread->m_hThread, 0);
		if (r == WAIT_OBJECT_0) {
			delete m_pRandomThread;
			m_pRandomThread = nullptr;
		}
		else {
			SetTimer(TIMER_THREAD_CLEANUP, 50, nullptr);
		}
	}

	m_bThreadRunning.store(false);
	GetDlgItem(IDC_BTN_RANDOM)->EnableWindow(m_pRandomThread == nullptr);

	m_iClickCount = 0;
	m_bIsDragging = false;
	m_iDragIndex = -1;
	UpdateCoordUI();
	Invalidate(FALSE);
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

	// 스레드 종료 시 무조건 실행되어야 할 일을 람다로 정의
	// (함수가 길어지거나 리턴 지점이 많아질 때 실수를 방지함)
	auto PostDone = [&]() {
		if (::IsWindow(pDlg->m_hWnd))
			::PostMessage(pDlg->m_hWnd, WM_UPDATE_RANDOM_DONE, 0, 0);
		};

	CRect rect = pDlg->GetDrawRect();

	// 안전장치로 그릴 공간이 너무 작으면 즉시 종료 (버그 방지)
	if (rect.Width() < 10 || rect.Height() < 10) {
		PostDone(); // 버튼 복구 신호 보내고 종료
		return 0;
	}

	int margin = pDlg->m_iPointRadius + 2;
	int xMin = rect.left + margin;
	int xMax = rect.right - margin;

	// 만약 마진 때문에 범위가 꼬이면 보정
	if (xMax <= xMin) { xMin = rect.left; xMax = rect.right; }

	for (int i = 0; i < 10; i++) {
		if (pDlg->m_bStopFlag.load()) break;

		RandomMovePayload* pData = new RandomMovePayload();
		for (int j = 0; j < 3; j++) {
			pData->pts[j].x = rand() % (max(1, xMax - xMin)) + xMin;
			// Y좌표도 동일한 방식으로 안전하게 계산 (생략 가능하나 넣으면 좋음)
			pData->pts[j].y = rand() % (rect.Height() - margin * 2) + rect.top + margin;
		}

		// 윈도우가 살아있을 때만 보냄
		if (::IsWindow(pDlg->m_hWnd)) {
			::PostMessage(pDlg->m_hWnd, WM_UPDATE_RANDOM_MOVE, 0, (LPARAM)pData);
		}
		else {
			delete pData; // 윈도우 없으면 그냥 삭제 (누수 방지)
			break;
		}

		// Sleep 쪼개기
		for (int t = 0; t < 50; t++) {
			if (pDlg->m_bStopFlag.load()) break;
			Sleep(10);
		}
	}

	PostDone(); // 정상 종료 시에도 호출
	return 0;
}

LRESULT CMy3PointCircleDlg::OnUpdateRandomMove(WPARAM wParam, LPARAM lParam) {
	RandomMovePayload* pData = (RandomMovePayload*)lParam;
	if (!pData) return 0;

	//정지 신호가 켜져있거나 스레드가 끝난 상태면 데이터 무시
	// (리셋 버튼 누른 직후에 날아온 유령 데이터 방지)
	if (m_bStopFlag.load() || !m_bThreadRunning.load()) {
		delete pData; // 메모리는 해제하고 적용은 안 함
		return 0;
	}

	for (int i = 0; i < 3; i++) {
		m_ptClicks[i] = pData->pts[i];
	}
	delete pData;
	UpdateCoordUI();

	// Day 4에서 만든 스마트 갱신
	CRect r = GetDrawRect();
	InvalidateRect(&r, FALSE);

	return 0;
}

CRect CMy3PointCircleDlg::GetDrawRect()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	// 1. 피해 다녀야 할 컨트롤들의 ID 목록
	// (리소스 편집기에서 만든 ID들과 일치해야 함)
	int ctrlIDs[] = {
		IDC_BTN_RESET, IDC_BTN_RANDOM,
		IDC_EDIT_RADIUS, IDC_EDIT_THICKNESS,
		IDC_STATIC_COORD
	};

	CRect rcControlsUnion(0, 0, 0, 0); // 컨트롤들이 차지하는 전체 영역
	bool bFirst = true;

	// 2. 모든 컨트롤의 영역을 합침 (Union)
	for (int id : ctrlIDs) {
		CWnd* pWnd = GetDlgItem(id);
		if (pWnd && pWnd->GetSafeHwnd()) {
			CRect rcCtrl;
			pWnd->GetWindowRect(&rcCtrl);
			ScreenToClient(&rcCtrl); // 화면 좌표 -> 클라이언트 좌표 변환

			if (bFirst) {
				rcControlsUnion = rcCtrl;
				bFirst = false;
			}
			else {
				rcControlsUnion.UnionRect(&rcControlsUnion, &rcCtrl);
			}
		}
	}

	// 컨트롤이 하나도 없으면 전체 영역 반환
	if (bFirst) return rcClient;

	// 3. 그리기 영역 결정 (컨트롤 영역을 제외한 빈 공간)
	// 여기서는 컨트롤들이 '아래쪽'이나 '위쪽'에 몰려있다고 가정하고
	// 가장 넓은 세로 공간을 선택하는 로직

	// 후보 1: 컨트롤들의 위쪽 공간
	CRect rcTop = rcClient;
	rcTop.bottom = max(rcClient.top, rcControlsUnion.top - 10); // 10px 여유

	// 후보 2: 컨트롤들의 아래쪽 공간
	CRect rcBottom = rcClient;
	rcBottom.top = min(rcClient.bottom, rcControlsUnion.bottom + 10);

	// 더 넓은 쪽을 그리기 영역으로 선택
	if (rcTop.Height() >= rcBottom.Height()) {
		return rcTop;
	}
	else {
		return rcBottom;
	}
}


void CMy3PointCircleDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_THREAD_CLEANUP) {
		// 스레드가 존재하는지 확인
		if (m_pRandomThread) {
			// 죽었는지 확인 (기다리지 않음, 0초 체크)
			DWORD result = WaitForSingleObject(m_pRandomThread->m_hThread, 0);

			if (result == WAIT_OBJECT_0) {
				// 죽었으면 메모리 해제
				delete m_pRandomThread;
				m_pRandomThread = nullptr;
				m_bThreadRunning = false;
				m_bStopFlag.store(false); // 플래그 초기화

				KillTimer(TIMER_THREAD_CLEANUP); // 임무 완료했으니 타이머 종료

				// 랜덤 버튼 다시 활성화 (필요하다면)
				// GetDlgItem(IDC_BTN_RANDOM)->EnableWindow(TRUE);
			}
		}
		else {
			// 스레드 객체가 없으면 타이머도 필요 없음
			KillTimer(TIMER_THREAD_CLEANUP);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CMy3PointCircleDlg::OnUpdateRandomDone(WPARAM wParam, LPARAM lParam)
{
	// 스레드가 정상 종료되었음을 표시
	// 실제 메모리 해제는 여기서 바로 Wait를 걸거나,
	// 다음번 실행 때 처리하도록 둘 수 있음.

	// 여기서는 간단하게 "실행 중 아님" 상태로 변경
	m_bThreadRunning = false;
	m_bStopFlag.store(false);

	// 스레드 핸들 정리는 Reset이나 다음 Random 실행 시, 
	// 혹은 Timer를 잠깐 돌려서 처리할 수도 있음.

	// 더 완벽하게 하려면 여기서도 타이머를 켜서 delete m_pRandomThread를 수행하게 하면 됨.
	SetTimer(TIMER_THREAD_CLEANUP, 50, NULL);

	return 0;
}


void CMy3PointCircleDlg::OnDestroy()
{
	// 1. 스레드에게 "멈춰" 신호 보냄
	m_bStopFlag.store(true);

	// 2. 타이머가 돌고 있다면 끔
	KillTimer(TIMER_THREAD_CLEANUP);

	// 3. 스레드가 완전히 죽을 때까지 기다림 (동기화)
	// 창이 닫히는 중이므로 UI 멈춤(Freezing) 걱정 없이 INFINITE로 기다려도 됨
	if (m_pRandomThread) {
		WaitForSingleObject(m_pRandomThread->m_hThread, INFINITE);
		delete m_pRandomThread;
		m_pRandomThread = nullptr;
	}

	//메시지 큐 청소
	// 스레드는 데이터를 보냈는데(PostMessage), 아직 처리가 안 돼서
	// 공중에 떠 있는 메시지(Payload)가 있을 수 있음. 이걸 안 지우면 메모리 누수!
	MSG msg;
	while (::PeekMessage(&msg, m_hWnd, WM_UPDATE_RANDOM_MOVE, WM_UPDATE_RANDOM_MOVE, PM_REMOVE)) {
		// 메시지 큐에서 꺼내서, 안에 들어있는 Payload를 강제로 삭제
		RandomMovePayload* pData = (RandomMovePayload*)msg.lParam;
		if (pData) delete pData;
	}

	CDialogEx::OnDestroy();
}