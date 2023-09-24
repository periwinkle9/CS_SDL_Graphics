#pragma once

// Functions to be exported from the DLL so that other programs can access the SDL rendering facilities
// that this DLL mod enables

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

extern "C" {
	// Retrieves the SDL_Window object for this instance. Returns null if the rendering backend hasn't been initialized yet.
	SDL_Window* Get_SDL_Window();

	// Retrieves the SDL_Renderer object for this instance. Returns null if the rendering backend hasn't been initialized yet.
	SDL_Renderer* Get_SDL_Renderer();

	// Retrieves the SDL_Texture object for the given surface ID.
	// Returns null if the rendering backend hasn't been initialized yet, or if the specified surfaceID is out of bounds.
	// The surface IDs are mapped in the same way as vanilla CS. You can also pass -1 to retrieve the main framebuffer object.
	// Use SDL_QueryTexture to retrieve the properties of the texture (e.g., width and height).
	SDL_Texture* Get_SDL_Texture(int surfaceID);
}
