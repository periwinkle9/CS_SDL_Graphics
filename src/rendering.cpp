#include "rendering.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "bitmap.h"
#include "font.h"
#include "loadimage.h"
#include <Windows.h>

#include "doukutsu/draw.h"
#include "doukutsu/misc.h"
#include "doukutsu/window.h"

#include <string>
#include <stdexcept>

RenderBackend* renderer;

RenderBackend::RenderBackend() : window(nullptr), renderer(nullptr), framebuffer{}, surf{}, glyph_atlas(nullptr)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::string errorMessage = std::string("Failed to initialize SDL video subsystem: ") + SDL_GetError();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), nullptr);
		SDL_Quit();
		throw std::runtime_error("SDL initialization failure");
	}
}

RenderBackend::~RenderBackend()
{
	deinit();
}

void RenderBackend::deinit()
{
	for (int i = 0; i < NumSurfaces; ++i)
	{
		SDL_DestroyTexture(surf[i].texture);
		surf[i].texture = nullptr;
	}
	SDL_DestroyTexture(framebuffer.texture);
	framebuffer.texture = nullptr;
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}

#ifdef JAPANESE
unsigned short ShiftJISToUTF32(const unsigned char* string, std::size_t* bytes_read);
std::string convertWindowNameUTF8()
{
	std::string windowNameSJIS{csvanilla::lpWindowName};
	const unsigned char* currentPos = reinterpret_cast<const unsigned char*>(windowNameSJIS.data());
	const unsigned char* const endPos = currentPos + windowNameSJIS.size();
	std::string converted;
	while (currentPos < endPos)
	{
		std::size_t numBytesRead;
		unsigned short unicodeVal = ShiftJISToUTF32(currentPos, &numBytesRead);
		currentPos += numBytesRead;
		// Convert to UTF-8
		if (unicodeVal < 0x80)
			converted.push_back(static_cast<char>(unicodeVal));
		else if (unicodeVal < 0x800)
		{
			converted.push_back(static_cast<char>(0xC0 | (unicodeVal >> 6)));
			converted.push_back(static_cast<char>(0x80 | (unicodeVal & 0x3F)));
		}
		else
		{
			converted.push_back(static_cast<char>(0xE0 | (unicodeVal >> 12)));
			converted.push_back(static_cast<char>(0x80 | ((unicodeVal & 0xFFF) >> 6)));
			converted.push_back(static_cast<char>(0x80 | (unicodeVal & 0x3F)));
		}
	}
	return converted;
}
#endif

bool RenderBackend::initWindow(int width, int height, int magnification)
{
	ZeroMemory(csvanilla::surface_metadata, sizeof csvanilla::surface_metadata);

	if (magnification > 0)
	{
		csvanilla::window_magnification = magnification;
		csvanilla::is_fullscreen = false;
	}
	else
	{
		csvanilla::window_magnification = 4;
		csvanilla::is_fullscreen = true;
	}

	const char* windowTitle = csvanilla::lpWindowName;
#ifdef JAPANESE
	std::string windowTitleConverted = convertWindowNameUTF8();
	windowTitle = windowTitleConverted.c_str();
#endif
	window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	if (window == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error: CreateWindow", SDL_GetError(), nullptr);
		return false;
	}
	
	if (csvanilla::is_fullscreen)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (renderer == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error: CreateRenderer", SDL_GetError(), window);
		return false;
	}

	framebuffer.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
	if (framebuffer.texture == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error: CreateTexture", SDL_GetError(), window);
		return false;
	}

	SDL_SetTextureBlendMode(framebuffer.texture, SDL_BLENDMODE_NONE);
	framebuffer.width = width;
	framebuffer.height = height;

	if (csvanilla::is_fullscreen)
		SDL_ShowCursor(SDL_DISABLE);

	// TODO figure out how to load the cursor?

	// Get native window handle (for the other things that the game needs it for)
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(window, &info) == SDL_TRUE)
		csvanilla::ghWnd = info.info.win.window;

	if (csvanilla::IsKeyFile("fps"))
		csvanilla::bFPS = true;

	return true;
}

void RenderBackend::drawScreen()
{
	SDL_SetRenderTarget(renderer, nullptr);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer.texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

bool RenderBackend::createSurface(int surf_no, int width, int height, bool isRenderTarget)
{
	// Don't allow overwriting an existing surface
	if (surf_no >= NumSurfaces || surf_no < 0 || surf[surf_no].texture != nullptr)
		return false;

	Surface& surface = surf[surf_no];
	surface.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
		isRenderTarget ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STATIC,
		width, height);
	if (surface.texture == nullptr)
		return false;

	surface.width = width;
	surface.height = height;
	surface.render_target = isRenderTarget;
	surface.lost = false;

	return true;
}

bool RenderBackend::copyBitmapOntoSurface(int surf_no, const Bitmap& bitmap)
{
	if (surf_no < 0 || surf_no >= NumSurfaces || surf[surf_no].texture == nullptr)
		return false;

	int width = bitmap.size().first, height = bitmap.size().second;
	const SDL_Rect rect = {0, 0, width, height};
	return SDL_UpdateTexture(surf[surf_no].texture, &rect, bitmap.data(), width * 4) == 0;
}

void RenderBackend::releaseSurface(int surf_no)
{
	if (surf_no >= 0 && surf_no < NumSurfaces && surf[surf_no].texture != nullptr)
	{
		Surface& surface = surf[surf_no];
		SDL_DestroyTexture(surface.texture);
		surface = Surface{}; // Zero out the fields just to be safe
	}
}

void RenderBackend::markRenderTargetsLost()
{
	for (int i = 0; i < NumSurfaces; ++i)
	{
		if (surf[i].texture != nullptr && surf[i].render_target)
			surf[i].lost = true;
	}
}

int RenderBackend::restoreSurfaces()
{
	if (framebuffer.texture == nullptr)
		return 0;

	int numSurfacesRegenerated = 0;
	if (framebuffer.lost)
	{
		framebuffer.lost = false; // I swear this is literally what CSE2 does, essentially
		++numSurfacesRegenerated;
	}

	for (int s = 0; s < NumSurfaces; ++s)
	{
		if (surf[s].lost)
		{
			surf[s].lost = false;
			++numSurfacesRegenerated;

			const auto& surfaceMetadata = csvanilla::surface_metadata[s];
			if (!surfaceMetadata.bSystem)
			{
				switch (surfaceMetadata.type)
				{
				case 1:
				{
					RECT rect = {0, 0, static_cast<long>(surfaceMetadata.width), static_cast<long>(surfaceMetadata.height)};
					CortBox2(&rect, 0, s);
					break;
				}
				case 2:
				{
					Bitmap bitmap = loadBitmapFromResource(surfaceMetadata.name);
					copyBitmapOntoSurface(s, bitmap);
					break;
				}
				case 3:
				{
					Bitmap bitmap = loadBitmapFromFile(surfaceMetadata.name);
					copyBitmapOntoSurface(s, bitmap);
					break;
				}
				}
			}
		}
	}
	
	return numSurfacesRegenerated;
}

namespace
{
SDL_Rect RECT2SDL_Rect(const RECT& rect)
{
	return SDL_Rect{rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top};
}
RECT& operator*=(RECT& rect, int mag)
{
	rect.left *= mag;
	rect.top *= mag;
	rect.right *= mag;
	rect.bottom *= mag;
	return rect;
}
RECT operator*(RECT rect, int mag)
{
	return rect *= mag;
}

void PutBitmap(const RECT* rcView, int x, int y, const RECT* rect, int surf_no, bool useColorKey)
{
	if (renderer == nullptr)
		return;

	RECT rcWork = *rect;
	if (x + rect->right - rect->left > rcView->right)
		rcWork.right -= (x + rect->right - rect->left) - rcView->right;
	if (x < rcView->left)
	{
		rcWork.left += rcView->left - x;
		x = rcView->left;
	}
	if (y + rect->bottom - rect->top > rcView->bottom)
		rcWork.bottom -= (y + rect->bottom - rect->top) - rcView->bottom;
	if (y < rcView->top)
	{
		rcWork.top += rcView->top - y;
		y = rcView->top;
	}

	int mag = csvanilla::window_magnification;
	rcWork *= mag;

	renderer->blit(surf_no, rcWork, RenderBackend::FramebufferID, x * mag, y * mag, useColorKey);
}
}

auto RenderBackend::getSurface(int id) -> Surface&
{
	return id == FramebufferID ? framebuffer : surf[id];
}
void RenderBackend::blit(int srcSurfaceID, const RECT& rect, int destSurfaceID, int x, int y, bool useColorKey)
{
	SDL_Rect sourceRect = RECT2SDL_Rect(rect);
	if (sourceRect.w <= 0 || sourceRect.h <= 0)
		return; // Don't draw invalid RECT
	SDL_Rect destinationRect = {x, y, sourceRect.w, sourceRect.h};

	Surface& sourceSurface = getSurface(srcSurfaceID);
	Surface& destSurface = getSurface(destSurfaceID);
	if (sourceSurface.texture == nullptr || destSurface.texture == nullptr)
		return;

	SDL_SetTextureBlendMode(sourceSurface.texture, useColorKey ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(renderer, destSurface.texture);
	SDL_RenderCopy(renderer, sourceSurface.texture, &sourceRect, &destinationRect);
}
void RenderBackend::colorFill(int surfaceID, const RECT& rect, unsigned long color)
{
	SDL_Rect convertedRect = RECT2SDL_Rect(rect);
	if (convertedRect.w < 0 || convertedRect.h < 0)
		return; // Don't draw invalid RECT
	
	Surface& surface = getSurface(surfaceID);
	if (surface.texture == nullptr)
		return;

	Uint8 alpha = SDL_ALPHA_OPAQUE;
	// Color key
	if ((color & 0x00FFFFFFu) == 0)
		alpha = SDL_ALPHA_TRANSPARENT;

	Uint8 red = color & 0xFF, green = (color >> 8) & 0xFF, blue = (color >> 16) & 0xFF;
	SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
	SDL_SetRenderTarget(renderer, surface.texture);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(renderer, &convertedRect);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void BackupSurface(int surf_no, const RECT* rect)
{
	if (renderer == nullptr)
		return;

	RECT rcSet = *rect * csvanilla::window_magnification;
	renderer->blit(RenderBackend::FramebufferID, rcSet, surf_no, rcSet.left, rcSet.top, false);
}

void PutBitmap3(const RECT* rcView, int x, int y, const RECT* rect, int surf_no)
{
	PutBitmap(rcView, x, y, rect, surf_no, true);
}
void PutBitmap4(const RECT* rcView, int x, int y, const RECT* rect, int surf_no)
{
	PutBitmap(rcView, x, y, rect, surf_no, false);
}

void Surface2Surface(int x, int y, const RECT* rect, int to, int from)
{
	if (renderer == nullptr)
		return;

	int mag = csvanilla::window_magnification;
	RECT rcWork = *rect * mag;
	renderer->blit(from, rcWork, to, x * mag, y * mag, true);
}

unsigned long GetCortBoxColor(unsigned long col)
{
	// No conversion needed :)
	return col;
}

void CortBox(const RECT* rect, unsigned long col)
{
	CortBox2(rect, col, RenderBackend::FramebufferID);
}
void CortBox2(const RECT* rect, unsigned long col, int surf_no)
{
	if (renderer == nullptr)
		return;

	RECT rcSet = *rect * csvanilla::window_magnification;
	if (surf_no >= 0)
		csvanilla::surface_metadata[surf_no].type = 1;
	
	renderer->colorFill(surf_no, rcSet, col);
}

int RestoreSurfaces()
{
	if (renderer != nullptr)
		return renderer->restoreSurfaces();
	else
		return 0;
}
