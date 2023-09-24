#include "api.h"
#include "rendering.h"

extern "C" {
	SDL_Window* Get_SDL_Window()
	{
		if (renderer != nullptr)
			return renderer->window;
		else
			return nullptr;
	}
	SDL_Renderer* Get_SDL_Renderer()
	{
		if (renderer != nullptr)
			return renderer->renderer;
		else
			return nullptr;
	}
	void* Get_Surface(int surfaceID)
	{
		if (renderer != nullptr)
		{
			if (surfaceID == RenderBackend::FramebufferID)
				return &renderer->framebuffer;
			else if (surfaceID >= 0 && surfaceID < RenderBackend::NumSurfaces)
				return &renderer->surf[surfaceID];
		}
		return nullptr;
	}
}
