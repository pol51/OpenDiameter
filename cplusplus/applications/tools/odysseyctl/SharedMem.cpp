// SharedMem.cpp: implementation of the CSharedMem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SharedMem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSharedMem::CSharedMem()
{
	m_Local = NULL;
	m_Peer = NULL;
    m_Size = 0;
	m_Process = NULL;
}

CSharedMem::~CSharedMem()
{
	if (m_Peer != NULL) {
		VirtualFreeEx(m_Process, m_Peer, 0, MEM_RELEASE);
		m_Peer = NULL;
	}
	if (m_Process != NULL) {
		CloseHandle(m_Process);
		m_Process = NULL;
	}
	if (m_Local != NULL) {
		VirtualFree(m_Local, 0, MEM_RELEASE);
		m_Local = NULL;
	}
}

CSharedMem::CSharedMem(HWND hwnd, size_t size)
{
	DWORD procid;

	m_Local = NULL;
	m_Peer = NULL;
	m_Process = NULL;
	m_Size = 0;

	m_Local = VirtualAlloc(NULL, size,
						   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (m_Local == NULL) {
		return;
	}

	GetWindowThreadProcessId(hwnd, &procid);
	m_Process = OpenProcess(PROCESS_VM_OPERATION
							| PROCESS_VM_READ | PROCESS_VM_WRITE,
							FALSE, procid);
	if (m_Process == NULL) {
		VirtualFree(m_Local, 0, MEM_RELEASE);
		m_Local = NULL;
		return;
	}

	m_Peer = VirtualAllocEx(m_Process, NULL, size,
							MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (m_Peer == NULL) {
		VirtualFree(m_Local, 0, MEM_RELEASE);
		m_Local = NULL;
		CloseHandle(m_Process);
		m_Process = NULL;
		return;
	}
	m_Size = size;
}

BOOL CSharedMem::WriteMemory(void)
{
	if (m_Process == NULL  || m_Peer == NULL || m_Local == NULL) {
		return FALSE;
	}
	return (WriteProcessMemory(m_Process, m_Peer, m_Local, m_Size, NULL)) ?
            TRUE : FALSE;
}

BOOL CSharedMem::ReadMemory(void)
{
	if (m_Process == NULL  || m_Peer == NULL || m_Local == NULL) {
		return FALSE;
	}
	return (ReadProcessMemory(m_Process, m_Peer, m_Local, m_Size, NULL)) ?
            TRUE : FALSE;
}
