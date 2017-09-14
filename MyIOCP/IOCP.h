#pragma once
#define MAXBUFF 8192
#define DEFAULT_PORT 27015
#define DEFAULT_IP "127.0.0.1"
//每次IO操作结构体
typedef struct _PER_IO_DATA
{
	OVERLAPPED overLapped;
	WSABUF wsaBuff;
	char dataBuff[MAXBUFF];
	DWORD bytesRecv;
	_PER_IO_DATA()											//初始化结构体
	{
		ZeroMemory((char*)&overLapped, sizeof(overLapped));
		ZeroMemory(dataBuff, MAXBUFF);
		wsaBuff.len = MAXBUFF;
		wsaBuff.buf = dataBuff;
	}
}PER_IO_DATA, *LPPER_IO_DATA;

//每个客户端接收后的Socket参数结构体
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
	static DWORD WINAPI _WorkThread(LPVOID lpParam);		//工作线程
	static DWORD WINAPI _MonitorThread(LPVOID lpParam);		//监听线程
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
	CMyIOCPDlg* m_Dlg;										//用于操作窗口类
	int m_Port;												//端口号
	int m_Count;
	HANDLE m_CompletionPort;								//完成端口
	CRITICAL_SECTION cs;									//临界区
	SOCKET servSock;
	HANDLE m_ShutdownEvent;
	bool ThreadFlag;
	CString m_strIP;
};

//线程传入参数
typedef struct _THREAD_PARAM
{
	CIOCP * m_IoCP;
	int mThreadId;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_DATA perIOdata;
}THREAD_PARAM,*LPTHREAD_PARAM;

