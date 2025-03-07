﻿#pragma once

#if WITH_EDITOR
#include "Logging/Logger.h"

/////////////////////////////////////////////////////////////////////////////
// COutputList 창

class COutputList : public CListBox
{
// 생성입니다.
public:
	COutputList() noexcept;

// 구현입니다.
public:
	virtual ~COutputList();
	void CopyStringToClipboard(const CString& InString);

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

class COutputWnd : public CDockablePane
{
// 생성입니다.
public:
	COutputWnd() noexcept;

	void UpdateFonts();

	COutputList& GetOutputDebug() { return m_wndOutputDebug; }

// 특성입니다.
protected:
	CMFCTabCtrl	m_wndTabs;

	COutputList m_wndOutputBuild;
	COutputList m_wndOutputDebug;
	COutputList m_wndOutputFind;

protected:

	void FillBuildWindow();
	void FillDebugWindow();
	void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// 구현입니다.
public:
	virtual ~COutputWnd();

protected:
	void OnLog(ELogVerbosity InLogVerbosity, FStringView InMessage);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#endif // #if WITH_EDITOR