#include "uiaccess.h"
#include <TLHelp32.h>
#include <tchar.h>

static DWORD DuplicateWinloginToken(const DWORD dwSessionId, const DWORD dwDesiredAccess, const PHANDLE phToken)
{
    DWORD dwErr;
    PRIVILEGE_SET ps;

    ps.PrivilegeCount = 1;
    ps.Control = PRIVILEGE_SET_ALL_NECESSARY;

    if (LookupPrivilegeValue(nullptr, SE_TCB_NAME, &ps.Privilege[0].Luid))
    {
        if (const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); INVALID_HANDLE_VALUE != hSnapshot)
        {
            BOOL bFound = FALSE;
            PROCESSENTRY32 pe;

            pe.dwSize = sizeof (pe);
            dwErr = ERROR_NOT_FOUND;

            for (BOOL bCont = Process32First(hSnapshot, &pe); bCont; bCont = Process32Next(hSnapshot, &pe))
            {

                if (0 != _tcsicmp(pe.szExeFile, TEXT("winlogon.exe")))
                {
                    continue;
                }

                if (const HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID))
                {
                    HANDLE hToken;
                    DWORD dwRetLen, sid;

                    if (OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken))
                    {
                        BOOL fTcb;

                        if (PrivilegeCheck(hToken, &ps, &fTcb) && fTcb)
                        {
                            if (GetTokenInformation(hToken, TokenSessionId, &sid, sizeof (sid), &dwRetLen) && sid == dwSessionId)
                            {
                                bFound = TRUE;
                                if (DuplicateTokenEx(hToken, dwDesiredAccess, nullptr, SecurityImpersonation, TokenImpersonation, phToken))
                                {
                                    dwErr = ERROR_SUCCESS;
                                }
                                else
                                {
                                    dwErr = GetLastError();
                                }
                            }
                        }
                        CloseHandle(hToken);
                    }
                    CloseHandle(hProcess);
                }

                if (bFound) break;
            }

            CloseHandle(hSnapshot);
        }
        else
        {
            dwErr = GetLastError();
        }
    }
    else
    {
        dwErr = GetLastError();
    }


    return dwErr;
}

static DWORD CreateUIAccessToken(const PHANDLE phToken)
{
    DWORD dwErr;
    HANDLE hTokenSelf;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hTokenSelf))
    {
        DWORD dwSessionId, dwRetLen;

        if (GetTokenInformation(hTokenSelf, TokenSessionId, &dwSessionId, sizeof (dwSessionId), &dwRetLen))
        {
            HANDLE hTokenSystem;

            dwErr = DuplicateWinloginToken(dwSessionId, TOKEN_IMPERSONATE, &hTokenSystem);
            if (ERROR_SUCCESS == dwErr)
            {
                if (SetThreadToken(nullptr, hTokenSystem))
                {
                    if (DuplicateTokenEx(hTokenSelf, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT, NULL, SecurityAnonymous, TokenPrimary, phToken))
                    {
                        BOOL bUIAccess = TRUE;

                        if (!SetTokenInformation(*phToken, TokenUIAccess, &bUIAccess, sizeof (bUIAccess)))
                        {
                            dwErr = GetLastError();
                            CloseHandle(*phToken);
                        }
                    }
                    else
                    {
                        dwErr = GetLastError();
                    }
                    RevertToSelf();
                }
                else
                {
                    dwErr = GetLastError();
                }
                CloseHandle(hTokenSystem);
            }
        }
        else
        {
            dwErr = GetLastError();
        }

        CloseHandle(hTokenSelf);
    }
    else
    {
        dwErr = GetLastError();
    }

    return dwErr;
}

static BOOL CheckForUIAccess(DWORD* pdwErr, BOOL* pfUIAccess)
{
    BOOL result = FALSE;
    HANDLE hToken;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        DWORD dwRetLen;

        if (GetTokenInformation(hToken, TokenUIAccess, pfUIAccess, sizeof (*pfUIAccess), &dwRetLen))
        {
            result = TRUE;
        }
        else
        {
            *pdwErr = GetLastError();
        }
        CloseHandle(hToken);
    }
    else
    {
        *pdwErr = GetLastError();
    }

    return result;
}

DWORD PrepareForUIAccess()
{
    DWORD dwErr;
    HANDLE hTokenUIAccess;
    BOOL fUIAccess;

    if (CheckForUIAccess(&dwErr, &fUIAccess))
    {
        if (fUIAccess)
        {
            dwErr = ERROR_SUCCESS;
        }
        else
        {
            dwErr = CreateUIAccessToken(&hTokenUIAccess);
            if (ERROR_SUCCESS == dwErr)
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                GetStartupInfo(&si);
                if (CreateProcessAsUser(hTokenUIAccess, nullptr, GetCommandLine(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    ExitProcess(0);
                }

                dwErr = GetLastError();
                CloseHandle(hTokenUIAccess);
            }
        }
    }

    return dwErr;
}
