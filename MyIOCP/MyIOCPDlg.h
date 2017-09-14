
// MyIOCPDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "IOCP.h"

// CMyIOCPDlg 对话框
class CMyIOCPDlg : public CDialogEx
{
// 构造
public:
	CMyIOCPDlg(CWnd* pParent = NULL);	// 标准构造函数
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYIOCP_DIALOG };
#endif

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
	afx_msg void OnBNStart();
	afx_msg void OnBNStop();
	DECLARE_MESSAGE_MAP()
public:
	inline void AddInformation(const CString strInfo)			//向输出信息控件添加信息函数（为了提高效率使用内联函数）
	{
		m_ShowMsg.AddString(strInfo);
	}
	inline void AddClient(const CString strInfo)
	{
		m_ClientList.AddString(strInfo);
	}
private:
	void Init();

public:
	CListBox m_ClientList;
	CListBox m_ShowMsg;
private:
	CIOCP m_IoCP;
public:
	afx_msg void OnLbnSelchangeClientList();
};
