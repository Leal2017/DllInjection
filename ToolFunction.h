#ifndef _TOOLFUNC_H_
#define _TOOLFUNC_H_

#include <windows.h>

BOOL WINAPI UnmapViewOfModule (DWORD dwProcessId, DWORD dwBaseAddr);
BOOL WINAPI EnableDebugPriv(LPCTSTR name);//����Ȩ����
BOOL WINAPI InjectIt(LPCTSTR DllPath, const DWORD dwRemoteProcessld);//ע��������
DWORD WINAPI getpid(LPSTR pn);//�õ�����pid

#endif