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
function :  �����߳�
parameter:  LPTHREAD_PARAM��
				m_IoCP   (ָ��CIOCP��ָ��)
				threadID (����ID)
retValue :  �߳�״̬(DWORD)
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

	m_IoCP->_ShowMessage("���������߳�: %d", threadParam->mThreadId);
	while (WAIT_OBJECT_0 != WaitForSingleObject(m_IoCP->m_ShutdownEvent, 0))
	{
		if (GetQueuedCompletionStatus(m_IoCP->m_CompletionPort, &bytesTransferred, (LPDWORD)&perHandleData, (LPOVERLAPPED*)&perIOdata, INFINITE) == 0)
		{
			m_IoCP->_ShowMessage( "��ɶ˿�״̬��ȡʧ�ܣ�%d",GetLastError());
			return -1;
		}
		if (bytesTransferred == 0)
		{
			m_IoCP->_ShowMessage("�ر�Socket %d", perHandleData->socket);
			m_IoCP->m_Dlg->SetDlgItemInt(IDC_COUNT, --m_IoCP->m_Count);
			if (closesocket(perHandleData->socket) == SOCKET_ERROR)
			{
				m_IoCP->_ShowMessage("�ر�ʧ��!:%d ", WSAGetLastError());
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
		m_IoCP->_ShowMessage("%s �ͻ���%d:%s", time, perHandleData->socket, perIOdata->dataBuff);
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
				m_IoCP->_ShowMessage("��ȡʧ��: %d", WSAGetLastError());
				return -1;
			}
		}
	}
	return 0;
}

/*
function :  �����߳�
parameter:  LPTHREAD_PARAM��
				m_IoCP��ָ��CIOCP��ָ�룩
				threadID(0)
retValue :  �߳�״̬(DWORD)
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
		m_IoCP->_ShowMessage("����Socket����ʧ�ܣ�%d",WSAGetLastError());
		return -1;
	}
	m_IoCP->m_Port = m_IoCP->m_Dlg->GetDlgItemInt(IDC_PORT);
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(m_IoCP->m_Port);
	localAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind((*servSock), (PSOCKADDR)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
	{
		m_IoCP->_ShowMessage("�󶨼���Socketʧ�ܣ�%d", WSAGetLastError());
		return -1;
	}
	if (listen((*servSock), SOMAXCONN) == SOCKET_ERROR)
	{
		m_IoCP->_ShowMessage("����ʧ�ܣ�", WSAGetLastError());
		return -1;
	}
	m_IoCP->_ShowMessage("��ʼ����");
//----------------------------------------------------------------------------------------------------
	while (WAIT_OBJECT_0 != WaitForSingleObject(m_IoCP->m_ShutdownEvent, 0))
	{
		clientSock = WSAAccept((*servSock), (PSOCKADDR)&clientAddr, &clientAddrLen, NULL, 0);			//������������
		if (clientSock == INVALID_SOCKET)
		{
			m_IoCP->_ShowMessage("�ͻ���Socket����ʧ��");
			continue;
		}
		m_IoCP->_ShowMessage("�ͻ���: %d ������",clientSock);
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
			m_IoCP->_ShowMessage("����ɶ˿�ʧ�ܣ�%d", GetLastError());
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
function :  ��������
parameter:  ��
retValue :  �������
*/
bool CIOCP::Start()
{
	InitializeCriticalSection(&cs);
	if (InitIOCP() == false)
	{
		_ShowMessage("��ʼ��ʧ�ܣ�");
		return false;
	}
	m_ShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	return true;
}

/*
function :  �رպ���
parameter:  ��
retValue :  ���
*/
void CIOCP::Stop()
{
	servSock = INVALID_SOCKET;
	ThreadFlag = false;
	closesocket(servSock);
	SetEvent(m_ShutdownEvent);
	_ShowMessage("�رշ�����");
}
/*
function :  ��ʼ����ɶ˿�
parameter:  ��
retValue :  �������(bool)
*/
bool CIOCP::InitIOCP()
{
	DWORD threadID;
	SYSTEM_INFO systemInfo;

	if ((m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		_ShowMessage("��ɶ˿ڴ���ʧ��! ");
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
			_ShowMessage("�̴߳���ʧ��!");
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
function :  ��ʾ��Ϣ
parameter:  ��Ҫ��ʾ���ַ���(const CString),
			�䳤����
retValue :  ��
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
function :  ��ʾ�û���Ϣ
parameter:  �����û���Ϣ���ַ���
retValue :  ��
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
	// ��ñ���������
	char hostname[MAX_PATH] = { 0 };
	gethostname(hostname, MAX_PATH);
	hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strIP = CString(inet_ntoa(inAddr));

	return m_strIP;
}
