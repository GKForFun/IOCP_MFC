
// MyIOCPDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "IOCP.h"

// CMyIOCPDlg �Ի���
class CMyIOCPDlg : public CDialogEx
{
// ����
public:
	CMyIOCPDlg(CWnd* pParent = NULL);	// ��׼���캯��
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYIOCP_DIALOG };
#endif

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
	afx_msg void OnBNStart();
	afx_msg void OnBNStop();
	DECLARE_MESSAGE_MAP()
public:
	inline void AddInformation(const CString strInfo)			//�������Ϣ�ؼ������Ϣ������Ϊ�����Ч��ʹ������������
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
