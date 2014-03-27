
// MusicDlg.h : 头文件
//

#pragma once
#include "mscomm1.h"
#include "afxwin.h"
#include "Kalman.h"
#include <iostream> 
#include <fstream> 
using namespace std; 
// CMusicDlg 对话框
class CMusicDlg : public CDialogEx
{
// 构造
public:
	CMusicDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MUSIC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMscomm1 m_mscomm;
	CString m_Edit_X;
	CString m_Edit_Y;
	CString m_Edit_Z;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit3();
	DECLARE_EVENTSINK_MAP()
	void OnCommMscomm1();
	afx_msg void OnBnClickedButton3();
	Kalman Kalman_x,Kalman_y;
	CString Gyo_X;
	CString Gyo_Y;
	CString Gyo_Z;
	CString Angle_X;
	CString Angle_Y;
	CString Angle_Z;
	CString Angular_X;
	CString Angular_Y;
	CString Angular_Z;
	CString Number;
	CString NowWay;
};
