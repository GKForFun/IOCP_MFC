#include "stdafx.h"
#include "IOCP.h"
#include "MyIOCPDlg.h"
#include "resource.h"

CIOCP::CIOCP():m_Port(),ThreadFlag(true),m_ShutdownEvent(NULL),m_Count(0)
{
}

CIOCP::~CIOCP()
{
}

/*
function :  工作线程
parameter:  LPTHREAD_PARAM：
				m_IoCP   (指向CIOCP类指针)
				threadID (进程ID)
retValue :  线程状态(DWORD)
*/
DWORD WINAPI CIOCP::_WorkThread(LPVOID lpParam)
{
	LPTHREAD_PARAM threadParam = (LPTHREAD_PARAM)lpParam;
	DWORD bytesRecv;
	DWORD flags;
	CIOCP* m_IoCP = threadParam->m_IoCP;
	DWORD bytesTransferred;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_DATA perIOdata;
	CString time;
	CTime tm;

	m_IoCP->_ShowMessage("启动工作线程: %d", threadParam->mThreadId);
	while (WAIT_OBJECT_0 != WaitForSingleObject(m_IoCP->m_ShutdownEvent, 0))
	{
		if (GetQueuedCompletionStatus(m_IoCP->m_CompletionPort, &bytesTransferred, (LPDWORD)&perHandleData, (LPOVERLAPPED*)&perIOdata, INFINITE) == 0)
		{
			m_IoCP->_ShowMessage( "完成端口状态获取失败：%d",GetLastError());
			return -1;
		}
		if (bytesTransferred == 0)
		{
			m_IoCP->_ShowMessage("关闭Socket %d", perHandleData->socket);
			m_IoCP->m_Dlg->SetDlgItemInt(IDC_COUNT, --m_IoCP->m_Count);
			if (closesocket(perHandleData->socket) == SOCKET_ERROR)
			{
				m_IoCP->_ShowMessage("关闭失败!:%d ", WSAGetLastError());
				return -1;
			}
			GlobalFree(perHandleData);
			GlobalFree(perIOdata);
			continue;
		}
		if (perIOdata->bytesRecv == 0)
		{
			perIOdata->bytesRecv = bytesTransferred;
		}
		tm = CTime::GetCurrentTime();
		time = tm.Format("%Y-%m-%d %H:%M:%S");
		m_IoCP->_ShowMessage("%s 客户端%d:%s", time, perHandleData->socket, perIOdata->dataBuff);
		perIOdata->bytesRecv = 0;
		send(perHandleData->socket, perIOdata->dataBuff, sizeof(perIOdata->dataBuff), 0);
		flags = 0;
		ZeroMemory(&(perIOdata->overLapped), sizeof(OVERLAPPED));
		perIOdata->wsaBuff.len = MAXBYTE;
		perIOdata->wsaBuff.buf = perIOdata->dataBuff;
		int retValue = WSARecv(perHandleData->socket, &(perIOdata->wsaBuff), 1, &bytesRecv, &flags, &(perIOdata->overLapped), NULL);
		if (retValue == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				m_IoCP->_ShowMessage("获取失败: %d", WSAGetLastError());
				return -1;
			}
		}
	}
	return 0;
}

/*
function :  监听线程
parameter:  LPTHREAD_PARAM：
				m_IoCP（指向CIOCP类指针）
				threadID(0)
retValue :  线程状态(DWORD)
*/
DWORD WINAPI CIOCP::_MonitorThread(LPVOID lpParam)
{
	LPTHREAD_PARAM thread = (LPTHREAD_PARAM)lpParam;
	CIOCP* m_IoCP = thread->m_IoCP;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_DATA perIOdata;
	DWORD flags;
	DWORD bytesRecv;
	bool retValue;
	sockaddr_in localAddr;
	sockaddr_in clientAddr;
	SOCKET* servSock = &m_IoCP->servSock;
	SOCKET clientSock = INVALID_SOCKET;
	int clientAddrLen = sizeof(clientAddr);
	CString clientInfo,time,temp;
	CTime tm;


	(*servSock) = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if ((*servSock) == INVALID_SOCKET)
	{
		m_IoCP->_ShowMessage("监听Socket创建失败：%d",WSAGetLastError());
		return -1;
	}
	m_IoCP->m_Port = m_IoCP->m_Dlg->GetDlgItemInt(IDC_PORT);
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(m_IoCP->m_Port);
	localAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind((*servSock), (PSOCKADDR)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
	{
		m_IoCP->_ShowMessage("绑定监听Socket失败：%d", WSAGetLastError());
		return -1;
	}
	if (listen((*servSock), SOMAXCONN) == SOCKET_ERROR)
	{
		m_IoCP->_ShowMessage("监听失败：", WSAGetLastError());
		return -1;
	}
	m_IoCP->_ShowMessage("开始监听");
//----------------------------------------------------------------------------------------------------
	while (WAIT_OBJECT_0 != WaitForSingleObject(m_IoCP->m_ShutdownEvent, 0))
	{
		clientSock = WSAAccept((*servSock), (PSOCKADDR)&clientAddr, &clientAddrLen, NULL, 0);			//接收连接请求
		if (clientSock == INVALID_SOCKET)
		{
			m_IoCP->_ShowMessage("客户端Socket接收失败");
			continue;
		}
		m_IoCP->_ShowMessage("客户端: %d 已连接",clientSock);
		m_IoCP->m_Dlg->SetDlgItemInt(IDC_COUNT, ++m_IoCP->m_Count);
		tm = CTime::GetCurrentTime();
		time = tm.Format("%Y-%m-%d %H:%M:%S ");
		temp.Format("%d", htons(clientAddr.sin_port));
		clientInfo = time+inet_ntoa(clientAddr.sin_addr)+":"+temp;
		m_IoCP->_ShowClientList(clientInfo);
		perHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
		perHandleData->socket = clientSock;
		retValue=CreateIoCompletionPort((HANDLE)clientSock, m_IoCP->m_CompletionPort, (DWORD)perHandleData, 0);
		if (retValue == NULL)
		{
			m_IoCP->_ShowMessage("绑定完成端口失败：%d", GetLastError());
			continue;
		}
		perIOdata = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
		ZeroMemory(&(perIOdata->overLapped), sizeof(OVERLAPPED));
		perIOdata->bytesRecv = 0;
		perIOdata->wsaBuff.len = MAXBYTE;
		perIOdata->wsaBuff.buf = perIOdata->dataBuff;
		flags = 0;
		WSARecv(clientSock, &(perIOdata->wsaBuff), 1, &bytesRecv, &flags, &(perIOdata->overLapped), NULL);
	}
	return 0;
}

/*
function :  启动函数
parameter:  无
retValue :  启动结果
*/
bool CIOCP::Start()
{
	InitializeCriticalSection(&cs);
	if (InitIOCP() == false)
	{
		_ShowMessage("初始化失败！");
		return false;
	}
	m_ShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	return true;
}

/*
function :  关闭函数
parameter:  无
retValue :  结果
*/
void CIOCP::Stop()
{
	servSock = INVALID_SOCKET;
	ThreadFlag = false;
	closesocket(servSock);
	SetEvent(m_ShutdownEvent);
	_ShowMessage("关闭服务器");
}
/*
function :  初始化完成端口
parameter:  无
retValue :  创建结果(bool)
*/
bool CIOCP::InitIOCP()
{
	DWORD threadID;
	SYSTEM_INFO systemInfo;

	if ((m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		_ShowMessage("完成端口创建失败! ");
		return false;
	}

	GetSystemInfo(&systemInfo);
	for (int i = 0; i < systemInfo.dwNumberOfProcessors * 2; i++)
	{
		HANDLE threadHandle;
		LPTHREAD_PARAM thread = new THREAD_PARAM;
		thread->mThreadId = i;
		thread->m_IoCP = this;
		if ((threadHandle = CreateThread(NULL, 0, _WorkThread, thread, 0, &threadID)) == NULL)
		{
			_ShowMessage("线程创建失败!");
			return false;
		}
		CloseHandle(threadHandle);
	}

	HANDLE ListenHandle;
	LPTHREAD_PARAM mThread = new THREAD_PARAM;
	mThread->m_IoCP = this;
	mThread->mThreadId = 0;
	ListenHandle = CreateThread(NULL, 0, _MonitorThread, mThread, 0, 0);
	return true;
}

/*
function :  显示消息
parameter:  需要显示的字符串(const CString),
			变长参数
retValue :  无
*/
void CIOCP::_ShowMessage(const CString szFormat, ...)
{
	CString   strMessage;
	va_list   arglist;
	va_start(arglist, szFormat);
	strMessage.FormatV(szFormat, arglist);
	va_end(arglist);
	CMyIOCPDlg* pMain = (CMyIOCPDlg*)m_Dlg;
	if (m_Dlg != NULL)
	{
		EnterCriticalSection(&cs);
		pMain->AddInformation(strMessage);
		TRACE(strMessage + _T("\n"));
		LeaveCriticalSection(&cs);
	}
}

/*
function :  显示用户信息
parameter:  包含用户信息的字符串
retValue :  无
*/
void CIOCP::_ShowClientList(const CString clientInfo, ...)
{
	CString   strMessage;
	va_list   arglist;
	va_start(arglist, clientInfo);
	strMessage.FormatV(clientInfo, arglist);
	va_end(arglist);
	CMyIOCPDlg* pMain = (CMyIOCPDlg*)m_Dlg;
	if (m_Dlg != NULL)
	{
		EnterCriticalSection(&cs);
		pMain->AddClient(strMessage);
		TRACE(strMessage + _T("\n"));
		LeaveCriticalSection(&cs);
	}
}

CString CIOCP::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = { 0 };
	gethostname(hostname, MAX_PATH);
	hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// 将IP地址转化成字符串形式
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strIP = CString(inet_ntoa(inAddr));

	return m_strIP;
}
