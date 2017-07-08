#include <windows.h>
#include <tlhelp32.h>
#include "PSAPI.H"

#include "ToolFunction.h"

#pragma comment(lib, "PSAPI.LIB")

typedef ULONG (WINAPI *PFNNtUnmapViewOfSection)( IN HANDLE ProcessHandle,IN PVOID BaseAddress );

BOOL WINAPI UnmapViewOfModule (DWORD dwProcessId, DWORD dwBaseAddr)
{
	MODULEENTRY32 me32 = {0};
	
	me32.dwSize = sizeof(MODULEENTRY32);

    HMODULE hModule = GetModuleHandle ( TEXT("ntdll.dll") ) ;
    if ( hModule == NULL )
        hModule = LoadLibrary ( TEXT("ntdll.dll") ) ;

    PFNNtUnmapViewOfSection pfnNtUnmapViewOfSection = (PFNNtUnmapViewOfSection)GetProcAddress ( hModule, "NtUnmapViewOfSection" ) ;
    
    HANDLE hProcess = OpenProcess ( PROCESS_ALL_ACCESS, TRUE, dwProcessId ) ;
	HANDLE hModuleSnap   =  CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwProcessId);

	if( hModuleSnap == INVALID_HANDLE_VALUE ) 
	{ 
		
		return FALSE; 
	}

	
	if( !Module32First( hModuleSnap, &me32 ) ) 
	{ 
		
		CloseHandle( hModuleSnap );
		return FALSE; 
	}

	do 
	{
		if (!(dwBaseAddr ^ (DWORD)me32.modBaseAddr))
		{
			break;
		}
	} while (Module32Next(hModuleSnap, &me32));


    ULONG    ret = pfnNtUnmapViewOfSection ( hProcess, me32.modBaseAddr) ;
    CloseHandle ( hProcess ) ;
	CloseHandle(hModuleSnap);
    return ret ? FALSE : TRUE ;
}

BOOL WINAPI EnableDebugPriv(LPCTSTR name)//����Ȩ����
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;
    //�򿪽������ƻ�
    if(!OpenProcessToken(GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
			&hToken))
    {
        //  OpenProcessToken Error
            return FALSE;
    }
    //��ý��̱���ΨһID
    if(!LookupPrivilegeValue(NULL, name, &luid))
    {
        //  LookupPrivivlegeValue Error;
		return FALSE;
    }

    tp.PrivilegeCount=1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tp.Privileges[0].Luid=luid;
    
	//����Ȩ��  
	if(!AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
         // AdjustTokenPrivileges Error
        return FALSE;
    }
    
	return TRUE;
}

BOOL WINAPI InjectIt(LPCTSTR DllPath, const DWORD dwRemoteProcessld)//ע��������
{
    HANDLE hrp = NULL;

    if((hrp=OpenProcess(PROCESS_CREATE_THREAD|//����Զ�̴����߳�
				PROCESS_VM_OPERATION|//����Զ��VM����
			   PROCESS_VM_WRITE,//����Զ��VMд
				FALSE,dwRemoteProcessld)) == NULL)
    {
        // OpenProcess Error
        return FALSE;
    }

    LPTSTR psLibFileRemote = NULL;

    //ʹ��VirtualAllocEx������Զ�̽��̵��ڴ��ַ�ռ����DLL�ļ�������
    psLibFileRemote=(LPTSTR)VirtualAllocEx(hrp, NULL, lstrlen(DllPath)+1,
								  MEM_COMMIT, PAGE_READWRITE);

    if(psLibFileRemote == NULL)
    {
        // VirtualAllocEx Error
        return FALSE;
    }

    //ʹ��WriteProcessMemory������DLL��·�������Ƶ�Զ�̵��ڴ�ռ�
    if(WriteProcessMemory(hrp, psLibFileRemote, (void *)DllPath, lstrlen(DllPath)+1, NULL) == 0)
    {
        // WriteProcessMemory Error
        return FALSE;
    }

    //����LoadLibraryA����ڵ�ַ
    PTHREAD_START_ROUTINE pfnStartAddr=(PTHREAD_START_ROUTINE)
        GetProcAddress(GetModuleHandle(TEXT("Kernel32")),"LoadLibraryA");

    if(pfnStartAddr == NULL)
    {
        // GetProcAddress Error!
        return FALSE;
    }
    //pfnStartAddr��ַ����LoadLibraryA����ڵ�ַ


    HANDLE hrt = NULL;

    if((hrt = CreateRemoteThread(hrp,
        NULL,
        0,
        pfnStartAddr,
        psLibFileRemote,
        0,
        NULL)) == NULL)
    {
        // CreateRemote Error
        return FALSE;
    }
    return TRUE;
}

DWORD WINAPI getpid(LPSTR pn)//�õ�����pid
{
    BOOL b = FALSE;
    HANDLE hnd = NULL;
    PROCESSENTRY32 pe = {0};

    //�õ����̿���
    hnd=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    pe.dwSize=sizeof(pe);
    b=Process32First(hnd, &pe);

    while(b)
    {
        if(stricmp(pn,pe.szExeFile)==0)
            return pe.th32ProcessID;

        b=Process32Next(hnd,&pe);
    }

	return (-1);
}
