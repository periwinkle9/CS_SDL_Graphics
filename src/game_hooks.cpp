#include "SDL.h"

#include "rendering.h"
#include "loadimage.h"
#include "font.h"
#include "patch_utils.h"
#include "doukutsu/credits.h"
#include "doukutsu/cstdlib.h"
#include "doukutsu/draw.h"
#include "doukutsu/keybinds.h"
#include "doukutsu/map.h"
#include "doukutsu/organya.h"
#include "doukutsu/tsc.h"
#include "doukutsu/window.h"
#include <Windows.h>
#include <synchapi.h>
#include <ShlObj.h>
#include <string>
#include <cstddef>
#include <filesystem>

// Replaces the CreateMutexA call at 0x412472
bool WINAPI initGraphics(LPSECURITY_ATTRIBUTES atrb, BOOL owner, LPCSTR name)
{
	csvanilla::hMutex = CreateMutexA(atrb, owner, name);
	try
	{
		renderer = new RenderBackend();
	}
	catch (...)
	{
		return false;
	}

	csvanilla::bActive = TRUE; // Fixes rando and other mods using the run-when-unfocused patch
	return true;
}

// Replaces the window initialization code starting at 0x4126F3
bool setupWindow(int conf_display_mode)
{
	if (renderer == nullptr)
		return false;

	bool error = false;

	// Allow for higher scaled windowed resolutions (by overwriting the 16 and 24 bit fullscreen options)
	int mag = conf_display_mode;
	if (conf_display_mode == 0) // Fullscreen
	{
		mag = 4;
		csvanilla::bFullscreen = true;
	}
	csvanilla::windowWidth = 320 * mag;
	csvanilla::windowHeight = 240 * mag;
	if (!renderer->initWindow(csvanilla::windowWidth, csvanilla::windowHeight, conf_display_mode))
		error = true;

	// This code is faithful to the original Config.dat specifications
	/*
	switch (conf_display_mode)
	{
	// Windowed mode
	case 1:
	case 2:
		csvanilla::windowWidth = 320 * conf_display_mode;
		csvanilla::windowHeight = 240 * conf_display_mode;
		if (!renderer->initWindow(csvanilla::windowWidth, csvanilla::windowHeight, conf_display_mode))
			error = true;
		break;
	// Fullscreen
	case 0:
	case 3:
	case 4:
		csvanilla::windowWidth = 640;
		csvanilla::windowHeight = 480;
		if (!renderer->initWindow(csvanilla::windowWidth, csvanilla::windowHeight, 0))
			error = true;
		csvanilla::bFullscreen = true;
		break;
	}
	*/

	if (error)
	{
		delete renderer;
		renderer = nullptr;
	}
	return !error;
}

// Replaces the ReleaseMutex call when exiting from WinMain()
void WINAPI deInit(HANDLE mutex)
{
	ReleaseMutex(mutex);
	delete renderer;
	renderer = nullptr;
}

bool SystemTask();

BOOL Flip_SystemTask(HWND)
{
	constexpr int FrameTime = 20; // 20 ms/frame = 50 frames/s

	static unsigned long timePrev;
	unsigned long timeNow;

	if (renderer == nullptr)
		return FALSE;

	while (true)
	{
		if (!SystemTask())
			return FALSE;

		timeNow = SDL_GetTicks();
		if (timeNow >= timePrev + FrameTime)
			break;

		SDL_Delay(1);
	}

	if (timeNow >= timePrev + 100)
		timePrev = timeNow;
	else
		timePrev += FrameTime;

	renderer->drawScreen();

	if (renderer->restoreSurfaces())
	{
		csvanilla::RestoreStripper();
		csvanilla::RestoreMapName();
		csvanilla::RestoreTextScript();
	}

	return TRUE;
}

bool SystemTask()
{
	using csvanilla::bActive;

	while (SDL_PollEvent(nullptr) || !bActive)
	{
		SDL_Event event;
		if (!SDL_WaitEvent(&event))
			return false;

		using csvanilla::gKey;
		using namespace csvanilla::Key;

		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				gKey |= KEY_ESCAPE;
				break;
			case SDLK_w:
				gKey |= KEY_MAP;
				break;
			case SDLK_LEFT:
				gKey |= KEY_LEFT;
				break;
			case SDLK_RIGHT:
				gKey |= KEY_RIGHT;
				break;
			case SDLK_UP:
				gKey |= KEY_UP;
				break;
			case SDLK_DOWN:
				gKey |= KEY_DOWN;
				break;
			case SDLK_x:
				gKey |= KEY_X;
				break;
			case SDLK_z:
				gKey |= KEY_Z;
				break;
			case SDLK_s:
				gKey |= KEY_ARMS;
				break;
			case SDLK_a:
				gKey |= KEY_ARMSREV;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				gKey |= KEY_SHIFT;
				break;
			case SDLK_F1:
				gKey |= KEY_F1;
				break;
			case SDLK_F2:
				gKey |= KEY_F2;
				break;
			case SDLK_q:
				gKey |= KEY_ITEM;
				break;
			case SDLK_COMMA:
				gKey |= KEY_ALT_LEFT;
				break;
			case SDLK_PERIOD:
				gKey |= KEY_ALT_DOWN;
				break;
			case SDLK_SLASH:
				gKey |= KEY_ALT_RIGHT;
				break;
			case SDLK_l:
				gKey |= KEY_L;
				break;
			case SDLK_EQUALS:
				gKey |= KEY_PLUS;
				break;
			case SDLK_F5:
				csvanilla::gbUseJoystick = FALSE;
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				gKey &= ~KEY_ESCAPE;
				break;
			case SDLK_w:
				gKey &= ~KEY_MAP;
				break;
			case SDLK_LEFT:
				gKey &= ~KEY_LEFT;
				break;
			case SDLK_RIGHT:
				gKey &= ~KEY_RIGHT;
				break;
			case SDLK_UP:
				gKey &= ~KEY_UP;
				break;
			case SDLK_DOWN:
				gKey &= ~KEY_DOWN;
				break;
			case SDLK_x:
				gKey &= ~KEY_X;
				break;
			case SDLK_z:
				gKey &= ~KEY_Z;
				break;
			case SDLK_s:
				gKey &= ~KEY_ARMS;
				break;
			case SDLK_a:
				gKey &= ~KEY_ARMSREV;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				gKey &= ~KEY_SHIFT;
				break;
			case SDLK_F1:
				gKey &= ~KEY_F1;
				break;
			case SDLK_F2:
				gKey &= ~KEY_F2;
				break;
			case SDLK_q:
				gKey &= ~KEY_ITEM;
				break;
			case SDLK_COMMA:
				gKey &= ~KEY_ALT_LEFT;
				break;
			case SDLK_PERIOD:
				gKey &= ~KEY_ALT_DOWN;
				break;
			case SDLK_SLASH:
				gKey &= ~KEY_ALT_RIGHT;
				break;
			case SDLK_l:
				gKey &= ~KEY_L;
				break;
			case SDLK_EQUALS:
				gKey &= ~KEY_PLUS;
				break;
			}
			break;
		// TODO handle controller connecting/disconnecting if we ever add controller support in the future
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_FOCUS_LOST:
				csvanilla::InactiveWindow();
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				csvanilla::ActiveWindow();
				break;
			}
			break;
		case SDL_QUIT:
			csvanilla::StopOrganyaMusic();
			return false;
		case SDL_RENDER_TARGETS_RESET:
			renderer->markRenderTargetsLost();
			break;
		}
	}

	// TODO run joystick code here if we're going to implement it
	
	return true;
}

void ReleaseSurface(int surf_no)
{
	if (renderer != nullptr)
	{
		renderer->releaseSurface(surf_no);
		csvanilla::memset(&csvanilla::surface_metadata[surf_no], 0, sizeof(csvanilla::SurfaceMetadata));
	}
}

void InitTextObject(const char*) // Font name parameter unused here for now - hardcoding Courier New
{
	PWSTR fontFolder;
	if (SHGetKnownFolderPath(FOLDERID_Fonts, 0, NULL, &fontFolder) == S_OK)
	{
		std::filesystem::path fontPath{fontFolder};
#ifdef JAPANESE
		fontPath /= "msgothic.ttc";
#else
		fontPath /= "cour.ttf"; // TODO look into supporting other fonts (like the parameter to this function)
#endif
		std::size_t width, height;
		int mag = csvanilla::window_magnification;
#ifdef JAPANESE
		if (mag == 1)
		{
			width = 6;
			height = 12;
		}
		else
		{
			width = 5 * mag;
			height = 10 * mag;
		}
#else
		width = 5 * mag;
		height = 10 * mag;
#endif
		font = LoadFreeTypeFont(fontPath.string().c_str(), width, height);
	}
	CoTaskMemFree(fontFolder);
}

void PutText2(int x, int y, const char* text, unsigned long color, int surf_no)
{
	if (renderer == nullptr)
		return;

	int mag = csvanilla::window_magnification;
	renderer->drawText(font, surf_no, x * mag, y * mag, color, text);
}

void EndTextObject()
{
	UnloadFont(font);
}

#define VERIFY(addr) if (!patcher::verifyBytes(0x##addr, origBytes##addr, sizeof origBytes##addr)) return false

// Verify that the memory addresses that we're patching contain the original code
// (i.e., haven't been touched by another ASM or DLL hack)
bool verifyIntegrity()
{
	const patcher::byte origBytes412472[] = {0xFF, 0x15, 0xC4, 0xC0, 0x48, 0x00, 0xA3, 0x78, 0xE4, 0x49, 0x00, 0x8B, 0x45, 0x08};
	VERIFY(412472);

	const patcher::byte origBytes4126F3[] = {0x6A, 0x30, 0x6A, 0x00, 0x8D, 0x45, 0xA8, 0x50, 0xE8, 0x30, 0xE6, 0x06, 0x00, 0x83,
		0xC4, 0x0C, 0xC7, 0x45, 0xA8, 0x30, 0x00, 0x00, 0x00, 0xC7, 0x45, 0xB0, 0xA0, 0x2C, 0x41, 0x00, 0x8B, 0x4D, 0x08};
	VERIFY(4126F3);

	const patcher::byte callReleaseMutex[] = {0xFF, 0x15, 0xC8, 0xC0, 0x48, 0x00};
	if (!patcher::verifyBytes(0x412AF3, callReleaseMutex, sizeof callReleaseMutex) ||
		!patcher::verifyBytes(0x412B78, callReleaseMutex, sizeof callReleaseMutex))
		return false;

	const patcher::byte origBytes412B20[] = {0xE8, 0x8B, 0xF3, 0xFF, 0xFF};
	VERIFY(412B20);

	const patcher::byte origBytes412B69[] = {0xE8, 0x52, 0x8B, 0xFF, 0xFF};
	VERIFY(412B69);

	auto verifyFunctionHeader = [](patcher::dword address)
	{
		const patcher::byte header[] = {0x55, 0x8B, 0xEC};
		return patcher::verifyBytes(address, header, sizeof header);
	};

	const patcher::dword replacedFunctions[] = {
		reinterpret_cast<patcher::dword>(csvanilla::Flip_SystemTask),
		reinterpret_cast<patcher::dword>(csvanilla::ReleaseSurface),
		reinterpret_cast<patcher::dword>(csvanilla::MakeSurface_Resource),
		reinterpret_cast<patcher::dword>(csvanilla::MakeSurface_File),
		reinterpret_cast<patcher::dword>(csvanilla::ReloadBitmap_Resource),
		reinterpret_cast<patcher::dword>(csvanilla::ReloadBitmap_File),
		reinterpret_cast<patcher::dword>(csvanilla::MakeSurface_Generic),
		reinterpret_cast<patcher::dword>(csvanilla::BackupSurface),
		reinterpret_cast<patcher::dword>(csvanilla::PutBitmap3),
		reinterpret_cast<patcher::dword>(csvanilla::PutBitmap4),
		reinterpret_cast<patcher::dword>(csvanilla::Surface2Surface),
		reinterpret_cast<patcher::dword>(csvanilla::GetCortBoxColor),
		reinterpret_cast<patcher::dword>(csvanilla::CortBox),
		reinterpret_cast<patcher::dword>(csvanilla::CortBox2),
		reinterpret_cast<patcher::dword>(csvanilla::RestoreSurfaces),
		reinterpret_cast<patcher::dword>(csvanilla::InitTextObject),
		reinterpret_cast<patcher::dword>(csvanilla::PutText2),
		reinterpret_cast<patcher::dword>(csvanilla::EndTextObject),
	};
	for (patcher::dword func : replacedFunctions)
		if (!verifyFunctionHeader(func))
			return false;

	return true;
}

// Called upon DLL initialization
bool applySDLPatches()
{
	// Verify all bytes that we're overwriting, before doing anything
	if (!verifyIntegrity())
		return false;

	// Initialize SDL at start of WinMain()
	const patcher::byte initGraphicsReturnValueChk[] = {
		0x84, 0xC0,                         // test al, al
		0x0F, 0x84, 0xF2, 0x06, 0x00, 0x00, // jz 412B71 ; ReleaseMutex(), end of WinMain
		0x90
	};
	patcher::writeCall(0x412472, initGraphics);
	patcher::patchBytes(0x412477, initGraphicsReturnValueChk, sizeof initGraphicsReturnValueChk);

	// Redirect window and DirectDraw initialization calls to our code
	const patcher::byte initWindow[] = {
		0xFF, 0xB5, 0x74, 0xFF, 0xFF, 0xFF, // push dword ptr [ebp-8c] ; conf.display_mode
		0xE8, 0x00, 0x00, 0x00, 0x00,       // call (our code)
		0x59,                               // pop ecx
		0x84, 0xC0,                         // test al, al
		0x0F, 0x84, 0x6A, 0x04, 0x00, 0x00, // jz 412b71 ; ReleaseMutex(), end of WinMain
		0xA1, 0x58, 0xE4, 0x49, 0x00,       // mov eax, dword ptr [49e458] ; ghWnd
		0x89, 0x45, 0xE8,                   // mov dword ptr [ebp-18], eax ; hWnd
		0xE9, 0x1D, 0x03, 0x00, 0x00        // jmp 412a31 ; jump past vanilla windows initialization code
	};
	patcher::patchBytes(0x4126F3, initWindow, sizeof initWindow);
	patcher::writeCall(0x4126F9, setupWindow);

	// nop out EndDirectDraw call (this is handled by deInit later)
	patcher::writeNOPs(0x412B69, 5);

	// Release all resources on exit
	patcher::writeCall(0x412AF3, deInit);
	patcher::writeNOPs(0x412AF8, 1);

	patcher::writeCall(0x412B78, deInit);
	patcher::writeNOPs(0x412B7D, 1);

	// Replace drawing functions
	patcher::replaceFunction(csvanilla::Flip_SystemTask, Flip_SystemTask);
	patcher::replaceFunction(csvanilla::ReleaseSurface, ReleaseSurface);
	patcher::replaceFunction(csvanilla::MakeSurface_Resource, MakeSurface_Resource);
	patcher::replaceFunction(csvanilla::MakeSurface_File, MakeSurface_File);
	patcher::replaceFunction(csvanilla::ReloadBitmap_Resource, ReloadBitmap_Resource);
	patcher::replaceFunction(csvanilla::ReloadBitmap_File, ReloadBitmap_File);
	patcher::replaceFunction(csvanilla::MakeSurface_Generic, MakeSurface_Generic);
	patcher::replaceFunction(csvanilla::BackupSurface, BackupSurface);
	patcher::replaceFunction(csvanilla::PutBitmap3, PutBitmap3);
	patcher::replaceFunction(csvanilla::PutBitmap4, PutBitmap4);
	patcher::replaceFunction(csvanilla::Surface2Surface, Surface2Surface);
	patcher::replaceFunction(csvanilla::GetCortBoxColor, GetCortBoxColor);
	patcher::replaceFunction(csvanilla::CortBox, CortBox);
	patcher::replaceFunction(csvanilla::CortBox2, CortBox2);
	patcher::replaceFunction(csvanilla::RestoreSurfaces, RestoreSurfaces);
	patcher::replaceFunction(csvanilla::InitTextObject, InitTextObject);
	patcher::replaceFunction(csvanilla::PutText2, PutText2);
	patcher::replaceFunction(csvanilla::EndTextObject, EndTextObject);

	return true;
}
