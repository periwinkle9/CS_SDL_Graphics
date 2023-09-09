#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <ddraw.h>

static HMODULE getRealDDrawDLL()
{
	static HMODULE realDDrawDLL = NULL;

	if (realDDrawDLL == NULL)
	{
		// Get path to ddraw.dll in system32 folder
		TCHAR ddrawPath[MAX_PATH];
		if (GetSystemDirectory(ddrawPath, MAX_PATH) && PathAppend(ddrawPath, TEXT("ddraw.dll")))
			realDDrawDLL = LoadLibrary(ddrawPath);
	}

	return realDDrawDLL;
}

#define GET_REAL_ADDRESS(name) using Func = decltype(&Real_##name); \
static Func addr = nullptr; \
if (!addr) \
	addr = (Func)GetProcAddress(getRealDDrawDLL(), #name)

extern "C" {

	// Guessing at some of these function signatures (they're probably wrong but hopefully equivalent)

	void WINAPI Real_AcquireDDThreadLock()
	{
		GET_REAL_ADDRESS(AcquireDDThreadLock);
		return addr();
	}

	HRESULT WINAPI Real_CompleteCreateSysmemSurface(LPVOID arg)
	{
		GET_REAL_ADDRESS(CompleteCreateSysmemSurface);
		return addr(arg);
	}

	HRESULT WINAPI Real_D3DParseUnknownCommand(LPVOID arg1, LPVOID* arg2)
	{
		GET_REAL_ADDRESS(D3DParseUnknownCommand);
		return addr(arg1, arg2);
	}

	HRESULT WINAPI Real_DDGetAttachedSurfaceLcl(LPVOID arg1, LPVOID arg2, LPVOID arg3)
	{
		GET_REAL_ADDRESS(DDGetAttachedSurfaceLcl);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_DDInternalLock(LPVOID arg1, LPVOID arg2)
	{
		GET_REAL_ADDRESS(DDInternalLock);
		return addr(arg1, arg2);
	}

	HRESULT WINAPI Real_DDInternalUnlock(LPVOID arg)
	{
		GET_REAL_ADDRESS(DDInternalUnlock);
		return addr(arg);
	}

	HRESULT WINAPI Real_DSoundHelp(HWND arg1, HLOCAL arg2, HLOCAL arg3)
	{
		GET_REAL_ADDRESS(DSoundHelp);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_DirectDrawCreate(LPGUID arg1, LPDIRECTDRAW arg2, LPUNKNOWN arg3)
	{
		GET_REAL_ADDRESS(DirectDrawCreate);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_DirectDrawCreateClipper(DWORD arg1, LPDIRECTDRAWCLIPPER arg2, LPUNKNOWN arg3)
	{
		GET_REAL_ADDRESS(DirectDrawCreateClipper);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_DirectDrawCreateEx(LPGUID arg1, LPVOID* arg2, REFIID arg3, LPUNKNOWN* arg4)
	{
		GET_REAL_ADDRESS(DirectDrawCreateEx);
		return addr(arg1, arg2, arg3, arg4);
	}

	HRESULT WINAPI Real_DirectDrawEnumerateA(LPDDENUMCALLBACKA arg1, LPVOID arg2)
	{
		GET_REAL_ADDRESS(DirectDrawEnumerateA);
		return addr(arg1, arg2);
	}

	HRESULT WINAPI Real_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA arg1, LPVOID arg2, DWORD arg3)
	{
		GET_REAL_ADDRESS(DirectDrawEnumerateExA);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW arg1, LPVOID arg2, DWORD arg3)
	{
		GET_REAL_ADDRESS(DirectDrawEnumerateExW);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_DirectDrawEnumerateW(LPDDENUMCALLBACKW arg1, LPVOID arg2)
	{
		GET_REAL_ADDRESS(DirectDrawEnumerateW);
		return addr(arg1, arg2);
	}

	HRESULT WINAPI Real_DllCanUnloadNow()
	{
		GET_REAL_ADDRESS(DllCanUnloadNow);
		return addr();
	}

	HRESULT WINAPI Real_DllGetClassObject(REFCLSID arg1, REFIID arg2, LPVOID* arg3)
	{
		GET_REAL_ADDRESS(DllGetClassObject);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_GetDDSurfaceLocal(LPVOID arg1, DWORD arg2, LPVOID arg3)
	{
		GET_REAL_ADDRESS(GetDDSurfaceLocal);
		return addr(arg1, arg2, arg3);
	}

	using idkwhatthisis = int(*)(int, int, int, int, int, int);
	idkwhatthisis WINAPI Real_GetOLEThunkData(int arg)
	{
		GET_REAL_ADDRESS(GetOLEThunkData);
		return addr(arg);
	}

	HRESULT WINAPI Real_GetSurfaceFromDC(LPVOID arg1, LPVOID arg2, LPVOID arg3)
	{
		GET_REAL_ADDRESS(GetSurfaceFromDC);
		return addr(arg1, arg2, arg3);
	}

	HRESULT WINAPI Real_RegisterSpecialCase(LPVOID arg1, LPVOID arg2, LPVOID arg3, LPVOID arg4)
	{
		GET_REAL_ADDRESS(RegisterSpecialCase);
		return addr(arg1, arg2, arg3, arg4);
	}

	void WINAPI Real_ReleaseDDThreadLock()
	{
		GET_REAL_ADDRESS(ReleaseDDThreadLock);
		return addr();
	}

	HRESULT WINAPI Real_SetAppCompatData(DWORD arg1, DWORD arg2)
	{
		GET_REAL_ADDRESS(SetAppCompatData);
		return addr(arg1, arg2);
	}

} // extern "C"
