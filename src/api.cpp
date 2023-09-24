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
	SDL_Texture* Get_SDL_Texture(int surfaceID)
	{
		SDL_Texture* ret = nullptr;
		if (renderer != nullptr)
		{
			if (surfaceID == RenderBackend::FramebufferID)
				ret = renderer->framebuffer.texture;
			else if (surfaceID >= 0 && surfaceID < RenderBackend::NumSurfaces)
				ret = renderer->surf[surfaceID].texture;
		}
		return ret;
	}
}
