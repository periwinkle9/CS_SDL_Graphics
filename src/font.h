// Based on CSE2-portable code originally released under the MIT licence.
// See Font.cpp for details.

#pragma once

#include <cstddef>

typedef struct Font Font;

Font* LoadFreeTypeFont(const char *font_filename, std::size_t cell_width, std::size_t cell_height);
void UnloadFont(Font *font);

extern Font* font;
