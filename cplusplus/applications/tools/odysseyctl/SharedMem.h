// SharedMem.h: interface for the CSharedMem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREDMEM_H__92154662_68F3_4325_9D3D_FAD4CA4E8171__INCLUDED_)
#define AFX_SHAREDMEM_H__92154662_68F3_4325_9D3D_FAD4CA4E8171__INCLUDED_

#include "stdafx.h"

class CSharedMem  
{
public:
	CSharedMem();
	CSharedMem(HWND, size_t);
	virtual ~CSharedMem();

	BOOL WriteMemory(void);
	BOOL ReadMemory(void);

    VOID *Local() {
        return m_Local;
    }
    void *Peer() {
        return m_Peer;
    }

private:
	void  *m_Local;	// virtual address from local process
	void  *m_Peer;	// virtual address from peer proccess
	size_t m_Size;
	HANDLE m_Process;
};

#endif // !defined(AFX_SHAREDMEM_H__92154662_68F3_4325_9D3D_FAD4CA4E8171__INCLUDED_)
