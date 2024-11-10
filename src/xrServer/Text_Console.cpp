#include "stdafx.h"
#include "Text_Console.h"
#include "../xrEngine/line_editor.h"

char const* const ioc_prompt = ">>> ";
char const* const ch_cursor = "_";

CTextConsole::CTextConsole()
{
	m_pMainWnd    = nullptr;
	m_hConsoleWnd = nullptr;
	m_hLogWnd     = nullptr;
	m_hLogWndFont = nullptr;

	m_bScrollLog  = true;
	m_dwStartLine = 0;

	m_bNeedUpdate      = false;
	m_dwLastUpdateTime = Device.dwTimeGlobal;
	m_last_time        = Device.dwTimeGlobal;
}

CTextConsole::~CTextConsole()
{
	m_pMainWnd = nullptr;
}

//-------------------------------------------------------------------------------------------
LRESULT CALLBACK TextConsole_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void	CTextConsole::CreateConsoleWnd()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	auto Window = (HWND)SDL_GetProperty(SDL_GetWindowProperties(g_AppInfo.Window), "SDL.window.win32.hwnd", nullptr);

	//----------------------------------
	RECT cRc;
	GetClientRect(Window, &cRc);
	INT lX = cRc.left;
	INT lY = cRc.top;
	INT lWidth = cRc.right - cRc.left;
	INT lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	const char*	wndclass ="TEXT_CONSOLE";

	// Register the windows class
	WNDCLASSA wndClass = { 0, TextConsole_WndProc, 0, 0, hInstance,
		nullptr,
		LoadCursor( hInstance, IDC_ARROW ),
		GetStockBrush(GRAY_BRUSH),
		nullptr, wndclass };
	RegisterClassA( &wndClass );

	// Set the window's initial style
	u32 dwWindowStyle = WS_OVERLAPPED | WS_CHILD | WS_VISIBLE;// | WS_CLIPSIBLINGS;// | WS_CLIPCHILDREN;

	// Set the window's initial width
	RECT rc;
	SetRect			( &rc, lX, lY, lWidth, lHeight );
//	AdjustWindowRect( &rc, dwWindowStyle, FALSE );

	// Create the render window
	m_hConsoleWnd = CreateWindowA( wndclass, "XRAY Text Console", dwWindowStyle,
		lX, lY,
		lWidth, lHeight, Window,
		0, hInstance, 0L );
	//---------------------------------------------------------------------------
	R_ASSERT2(m_hConsoleWnd, "Unable to Create TextConsole Window!");
};
//-------------------------------------------------------------------------------------------
LRESULT CALLBACK TextConsole_LogWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void	CTextConsole::CreateLogWnd()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	//----------------------------------
	RECT cRc;
	GetClientRect(m_hConsoleWnd, &cRc);
	INT lX = cRc.left;
	INT lY = cRc.top;
	INT lWidth = cRc.right - cRc.left;
	INT lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	const char*	wndclass ="TEXT_CONSOLE_LOG_WND";

	// Register the windows class
	WNDCLASSA wndClass = { 0, TextConsole_LogWndProc, 0, 0, hInstance,
		nullptr,
		LoadCursor( nullptr, IDC_ARROW ),
		GetStockBrush(BLACK_BRUSH),
		nullptr, wndclass };
	RegisterClassA( &wndClass );

	// Set the window's initial style
	u32 dwWindowStyle = WS_OVERLAPPED | WS_CHILD | WS_VISIBLE;// | WS_CLIPSIBLINGS;
//	u32 dwWindowStyleEx = WS_EX_CLIENTEDGE;

	// Set the window's initial width
	RECT rc;
	SetRect			( &rc, lX, lY, lWidth, lHeight );
//	AdjustWindowRect( &rc, dwWindowStyle, FALSE );

	// Create the render window
	m_hLogWnd = CreateWindowA(wndclass, "XRAY Text Console Log", dwWindowStyle,
		lX, lY,
		lWidth, lHeight, m_hConsoleWnd,
		0, hInstance, 0L );
	//---------------------------------------------------------------------------
	R_ASSERT2(m_hLogWnd, "Unable to Create TextConsole Window!");
	//---------------------------------------------------------------------------
	ShowWindow(m_hLogWnd, SW_SHOW); 
	UpdateWindow(m_hLogWnd);
	//-----------------------------------------------
	LOGFONTA lf; 
	lf.lfHeight = -12; 
	lf.lfWidth = 0;
	lf.lfEscapement = 0; 
	lf.lfOrientation = 0; 
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = 0; 
	lf.lfUnderline = 0; 
	lf.lfStrikeOut = 0; 
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_STRING_PRECIS;
	lf.lfClipPrecision = CLIP_STROKE_PRECIS;	
	lf.lfQuality = DRAFT_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	xr_sprintf(lf.lfFaceName,sizeof(lf.lfFaceName),"");

	m_hLogWndFont = CreateFontIndirectA(&lf);
	R_ASSERT2(m_hLogWndFont, "Unable to Create Font for Log Window");
	//------------------------------------------------
	m_hDC_LogWnd = GetDC(m_hLogWnd);
	R_ASSERT2(m_hDC_LogWnd, "Unable to Get DC for Log Window!");
	//------------------------------------------------
	m_hDC_LogWnd_BackBuffer = CreateCompatibleDC(m_hDC_LogWnd);
	R_ASSERT2(m_hDC_LogWnd_BackBuffer, "Unable to Create Compatible DC for Log Window!");
	//------------------------------------------------
	GetClientRect(m_hLogWnd, &cRc);
	lWidth = cRc.right - cRc.left;
	lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	m_hBB_BM = CreateCompatibleBitmap(m_hDC_LogWnd, lWidth, lHeight);
	R_ASSERT2(m_hBB_BM, "Unable to Create Compatible Bitmap for Log Window!");
	//------------------------------------------------
	m_hOld_BM = (HBITMAP)SelectObject(m_hDC_LogWnd_BackBuffer, m_hBB_BM);
	//------------------------------------------------
	m_hPrevFont = (HFONT)SelectObject(m_hDC_LogWnd_BackBuffer, m_hLogWndFont);
	//------------------------------------------------
	SetTextColor(m_hDC_LogWnd_BackBuffer, RGB(255, 255, 255));
	SetBkColor(m_hDC_LogWnd_BackBuffer, RGB(1, 1, 1));
	//------------------------------------------------
	m_hBackGroundBrush = GetStockBrush(BLACK_BRUSH);
}

void CTextConsole::Initialize()
{
	inherited::Initialize();
	
	m_pMainWnd         = &g_AppInfo.Window;
	m_dwLastUpdateTime = Device.dwTimeGlobal;
	m_last_time        = Device.dwTimeGlobal;

	CreateConsoleWnd();
	CreateLogWnd();

	ShowWindow( m_hConsoleWnd, SW_SHOW );
	UpdateWindow( m_hConsoleWnd );	

	m_server_info.ResetData();
}

void CTextConsole::Destroy()
{
	inherited::Destroy();	

	SelectObject( m_hDC_LogWnd_BackBuffer, m_hPrevFont );
	SelectObject( m_hDC_LogWnd_BackBuffer, m_hOld_BM );

	if ( m_hBB_BM )           DeleteObject( m_hBB_BM );
	if ( m_hOld_BM )          DeleteObject( m_hOld_BM );
	if ( m_hLogWndFont )      DeleteObject( m_hLogWndFont );
	if ( m_hPrevFont )        DeleteObject( m_hPrevFont );
	if ( m_hBackGroundBrush ) DeleteObject( m_hBackGroundBrush );

	ReleaseDC( m_hLogWnd, m_hDC_LogWnd_BackBuffer );
	ReleaseDC( m_hLogWnd, m_hDC_LogWnd );

	DestroyWindow( m_hLogWnd );
	DestroyWindow( m_hConsoleWnd );
}

void CTextConsole::OnRender() {} //disable CConsole::OnRender()

void CTextConsole::OnPaint()
{
	RECT wRC;
	PAINTSTRUCT ps;

	BeginPaint(m_hLogWnd, &ps);

	if (Device.dwFrame % 2)
	{
		SDL_GetWindowSize(g_AppInfo.Window, (int*)&wRC.right, (int*)&wRC.bottom);
		DrawLog(m_hDC_LogWnd_BackBuffer, &wRC);
	}
	else
	{
		wRC = ps.rcPaint;
	}

	BitBlt(m_hDC_LogWnd,
		wRC.left, wRC.top,
		wRC.right - wRC.left, wRC.bottom - wRC.top,
		m_hDC_LogWnd_BackBuffer,
		wRC.left, wRC.top,
		SRCCOPY);

	EndPaint(m_hLogWnd, &ps);
}

void CTextConsole::DrawLog(HDC hDC, RECT* pRect)
{
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);

	RECT wRC = *pRect;
	FillRect(hDC, &wRC, m_hBackGroundBrush);

	int Width = wRC.right - wRC.left;
	int Height = wRC.bottom - wRC.top;
	wRC = *pRect;
	int y_top_max = (int)(0.32f * Height);

	//---------------------------------------------------------------------------------
	LPCSTR s_edt = ec().str_edit();
	LPCSTR s_cur = ec().str_before_cursor();

	u32 cur_len = xr_strlen(s_cur) + xr_strlen(ch_cursor) + 1;
	PSTR buf = (PSTR)_alloca(cur_len * sizeof(char));
	xr_strcpy(buf, cur_len, s_cur);
	xr_strcat(buf, cur_len, ch_cursor);
	buf[cur_len - 1] = 0;

	u32 cur0_len = xr_strlen(s_cur);

	int xb = 25;

	SetTextColor(hDC, RGB(255, 255, 255));
	TextOutA(hDC, xb, Height - tm.tmHeight - 1, buf, cur_len - 1);
	buf[cur0_len] = 0;

	SetTextColor(hDC, RGB(0, 0, 0));
	TextOutA(hDC, xb, Height - tm.tmHeight - 1, buf, cur0_len);


	SetTextColor(hDC, RGB(255, 255, 255));
	TextOutA(hDC, 0, Height - tm.tmHeight - 3, ioc_prompt, xr_strlen(ioc_prompt)); // ">>> "

	SetTextColor(hDC, (COLORREF)bgr2rgb(get_mark_color(mark11)));
	TextOutA(hDC, xb, Height - tm.tmHeight - 3, s_edt, xr_strlen(s_edt));

	SetTextColor(hDC, RGB(205, 205, 225));
	u32 log_line = (u32)m_log_history.GetSize() - 1;
	string16 q, q2;
	_itoa(log_line, q, 10);
	xr_strcpy(q2, sizeof(q2), "[");
	xr_strcat(q2, sizeof(q2), q);
	xr_strcat(q2, sizeof(q2), "]");
	u32 qn = xr_strlen(q2);

	TextOutA(hDC, Width - 8 * qn, Height - tm.tmHeight - tm.tmHeight, q2, qn);
	{
		xrCriticalSectionGuard guardLog(&m_log_history_guard);
		u32 log_line = m_log_history.GetSize();

		int ypos = Height - tm.tmHeight - tm.tmHeight;
		for (u32 i = scroll_delta; i < log_line; ++i)
		{
			const shared_str& logLine = m_log_history.GetLooped(m_log_history.GetTail() - i);

			if (!logLine.size()) {
				continue;
			}

			ypos -= tm.tmHeight;
			if (ypos < y_top_max)
			{
				break;
			}

			LPCSTR ls = logLine.c_str();

			Console_mark cm = (Console_mark)ls[0];
			COLORREF     c2 = (COLORREF)bgr2rgb(get_mark_color(cm));
			SetTextColor(hDC, c2);
			u8 b = (is_mark(cm)) ? 2 : 0;
			LPCSTR pOut = ls + b;

			BOOL res = TextOutA(hDC, 10, ypos, pOut, xr_strlen(pOut));
			if (!res)
			{
				R_ASSERT2(0, "TextOut(..) return nullptr");
			}
		}


		if (g_pGameLevel && (Device.dwTimeGlobal - m_last_time > 500))
		{
			m_last_time = Device.dwTimeGlobal;

			m_server_info.ResetData();
			g_pGameLevel->GetLevelInfo(&m_server_info);
		}

		ypos = 5;
		for (u32 i = 0; i < m_server_info.Size(); ++i)
		{
			SetTextColor(hDC, m_server_info[i].color);
			TextOutA(hDC, 10, ypos, m_server_info[i].name, xr_strlen(m_server_info[i].name));

			ypos += tm.tmHeight;
			if (ypos > y_top_max)
			{
				break;
			}
		}
	}
}

void CTextConsole::OnFrame()
{
	inherited::OnFrame();
	InvalidateRect(m_hConsoleWnd, nullptr, FALSE);
}
