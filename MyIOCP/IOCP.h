#pragma once
#define MAXBUFF 8192
#define DEFAULT_PORT 27015
#define DEFAULT_IP "127.0.0.1"
//ÿ��IO�����ṹ��
typedef struct _PER_IO_DATA
{
	OVERLAPPED overLapped;
	WSABUF wsaBuff;
	char dataBuff[MAXBUFF];
	DWORD bytesRecv;
	_PER_IO_DATA()											//��ʼ���ṹ��
	{
		ZeroMemory((char*)&overLapped, sizeof(overLapped));
		ZeroMemory(dataBuff, MAXBUFF);
		wsaBuff.len = MAXBUFF;
		wsaBuff.buf = dataBuff;
	}
}PER_IO_DATA, *LPPER_IO_DATA;

//ÿ���ͻ��˽��պ��Socket�����ṹ��
typedef struct _PER_HANDLE_DATA
{
	SOCKET socket;
	_PER_HANDLE_DATA()
	{
		socket = INVALID_SOCKET;
	}
	~_PER_HANDLE_DATA()
	{
		if (socket != INVALID_SOCKET)
		{
			closesocket(socket);
		}
	}

}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

class CMyIOCPDlg;

class CIOCP
{
public:
	CIOCP();
	~CIOCP();
	static DWORD WINAPI _WorkThread(LPVOID lpParam);		//�����߳�
	static DWORD WINAPI _MonitorThread(LPVOID lpParam);		//�����߳�
	CString GetLocalIP();
	bool Start();
	void Stop();
	bool InitIOCP();
	inline void SetParent(CMyIOCPDlg *pWnd)
	{
		m_Dlg = pWnd;
	}
	void _ShowMessage(const CString szFormat, ...);
	void _ShowClientList(const CString clientInfo,...);
private:
	CMyIOCPDlg* m_Dlg;										//���ڲ���������
	int m_Port;												//�˿ں�
	int m_Count;
	HANDLE m_CompletionPort;								//��ɶ˿�
	CRITICAL_SECTION cs;									//�ٽ���
	SOCKET servSock;
	HANDLE m_ShutdownEvent;
	bool ThreadFlag;
	CString m_strIP;
};

//�̴߳������
typedef struct _THREAD_PARAM
{
	CIOCP * m_IoCP;
	int mThreadId;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_DATA perIOdata;
}THREAD_PARAM,*LPTHREAD_PARAM;

