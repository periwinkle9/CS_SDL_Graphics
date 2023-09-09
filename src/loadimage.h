#pragma once

#include "bitmap.h"

Bitmap loadBitmapFromFile(const char* name);
Bitmap loadBitmapFromResource(const char* resourceName);

// Replacement vanilla functions
int MakeSurface_Resource(const char* name, int surf_no);
int MakeSurface_File(const char* name, int surf_no);
int ReloadBitmap_Resource(const char* name, int surf_no);
int ReloadBitmap_File(const char* name, int surf_no);
int MakeSurface_Generic(int bxsize, int bysize, int surf_no, int bSystem);
