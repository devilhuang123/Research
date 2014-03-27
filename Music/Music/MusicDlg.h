
// MusicDlg.h : ͷ�ļ�
//

#pragma once
#include "mscomm1.h"
#include "afxwin.h"
#include "Kalman.h"
#include <iostream> 
#include <fstream> 
using namespace std; 
// CMusicDlg �Ի���
class CMusicDlg : public CDialogEx
{
// ����
public:
	CMusicDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MUSIC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
