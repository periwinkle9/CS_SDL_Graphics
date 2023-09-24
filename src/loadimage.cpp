#include "loadimage.h"
#include "rendering.h"
#include "bitmap.h"
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "doukutsu/cstdlib.h"
#include "doukutsu/draw.h"
#include "doukutsu/misc.h"
#include "doukutsu/window.h"

// These functions automatically scale the bitmap
Bitmap loadBitmapFromFile(const char* name)
{
	std::filesystem::path dataPath{csvanilla::gDataPath};
	// Try both .bmp and .pbm
	bool found = false;
	std::filesystem::path filePath;
	for (auto ext : {".png", ".bmp", ".pbm"})
	{
		std::string fileName = std::string(name) + ext;
		filePath = dataPath / fileName;
		// Don't check for (C)Pixel, just check if the file exists
		if (std::filesystem::is_regular_file(filePath))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		csvanilla::ErrorLog(name, 0);
		return Bitmap{};
	}

	Bitmap bitmap{filePath.string().c_str()};
	
	return bitmap.scale(csvanilla::window_magnification);
}

Bitmap loadBitmapFromResource(const char* resourceName)
{
	HRSRC resource = FindResourceA(NULL, resourceName, MAKEINTRESOURCEA(2));
	if (resource == NULL)
		return Bitmap{};
	HGLOBAL resourceHandle = LoadResource(NULL, resource);
	if (resourceHandle == NULL)
		return Bitmap{};
	unsigned char* resourceBytes = (unsigned char*)LockResource(resourceHandle);
	if (resourceBytes == NULL)
		return Bitmap{};

	DWORD resourceSize = SizeofResource(NULL, resource);

	// BITMAP resources are missing the BITMAPFILEHEADER information, so we have to fill it out ourselves
	std::vector<unsigned char> bmpBytes(sizeof(BITMAPFILEHEADER) + resourceSize);
	std::copy_n(resourceBytes, resourceSize, std::begin(bmpBytes) + sizeof(BITMAPFILEHEADER));

	const BITMAPINFOHEADER& bih = *reinterpret_cast<const BITMAPINFOHEADER*>(resourceBytes);
	// Too much hassle to support anything other than uncompressed RGB bitmaps
	if (bih.biCompression != BI_RGB)
		return Bitmap{};
	DWORD imgSize = bih.biSizeImage;
	if (imgSize == 0) // Microsoft allows this to be 0 for uncompressed bitmaps :/
	{
		// Calculate size of bitmap pixels (simpler than calculating size of the color table etc.)
		// Although...maybe calculating the size of the header properly might be more accurate (TODO?)
		DWORD stride = ((bih.biWidth * bih.biBitCount + 31) & ~31u) >> 3; // I'm just gonna trust MSDN that this is correct
		imgSize = std::abs(bih.biHeight) * stride;
	}
	// Fill out BITMAPFILEHEADER
	BITMAPFILEHEADER bfh;
	bfh.bfType = 0x4D42;
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + resourceSize;
	bfh.bfReserved1 = bfh.bfReserved2 = 0;
	bfh.bfOffBits = bfh.bfSize - imgSize;
	std::memcpy(bmpBytes.data(), &bfh, sizeof bfh);

	// Finally, load the bitmap into a standard format
	Bitmap bitmap{bmpBytes.data(), bmpBytes.size()};
	return bitmap.scale(csvanilla::window_magnification);;
}

BOOL MakeSurface_Resource(const char* name, int surf_no)
{
	Bitmap bitmap = loadBitmapFromResource(name);
	if (!bitmap)
		return FALSE;
	if (renderer == nullptr || !renderer->createSurface(surf_no, bitmap.size().first, bitmap.size().second, false))
		return FALSE;
	if (!renderer->copyBitmapOntoSurface(surf_no, bitmap))
		return FALSE;

	auto& surfaceMetadata = csvanilla::surface_metadata[surf_no];
	surfaceMetadata.type = 2;
	surfaceMetadata.width = bitmap.size().first / csvanilla::window_magnification;
	surfaceMetadata.height = bitmap.size().second / csvanilla::window_magnification;
	surfaceMetadata.bSystem = FALSE;
	csvanilla::strcpy(surfaceMetadata.name, name);

	return TRUE;
}

BOOL MakeSurface_File(const char* name, int surf_no)
{
	Bitmap bitmap = loadBitmapFromFile(name);
	if (!bitmap)
	{
		csvanilla::ErrorLog(name, 1);
		return FALSE;
	}

	if (renderer == nullptr || !renderer->createSurface(surf_no, bitmap.size().first, bitmap.size().second, false))
	{
		csvanilla::ErrorLog("create surface", surf_no);
		return FALSE;
	}

	if (!renderer->copyBitmapOntoSurface(surf_no, bitmap))
	{
		renderer->releaseSurface(surf_no);
		return FALSE;
	}

	auto& surfaceMetadata = csvanilla::surface_metadata[surf_no];
	surfaceMetadata.type = 3;
	surfaceMetadata.width = bitmap.size().first / csvanilla::window_magnification;
	surfaceMetadata.height = bitmap.size().second / csvanilla::window_magnification;
	surfaceMetadata.bSystem = FALSE;
	csvanilla::strcpy(surfaceMetadata.name, name);

	return TRUE;
}

BOOL ReloadBitmap_Resource(const char* name, int surf_no)
{
	Bitmap bitmap = loadBitmapFromResource(name);
	if (!bitmap)
		return FALSE;
	if (renderer == nullptr || !renderer->copyBitmapOntoSurface(surf_no, bitmap))
		return FALSE;

	auto& surfaceMetadata = csvanilla::surface_metadata[surf_no];
	surfaceMetadata.type = 2;
	csvanilla::strcpy(surfaceMetadata.name, name);

	return TRUE;
}

BOOL ReloadBitmap_File(const char* name, int surf_no)
{
	Bitmap bitmap = loadBitmapFromFile(name);
	if (!bitmap)
	{
		csvanilla::ErrorLog(name, 1);
		return FALSE;
	}

	if (renderer == nullptr || !renderer->copyBitmapOntoSurface(surf_no, bitmap))
		return FALSE;

	auto& surfaceMetadata = csvanilla::surface_metadata[surf_no];
	surfaceMetadata.type = 3;
	csvanilla::strcpy(surfaceMetadata.name, name);

	return TRUE;
}

BOOL MakeSurface_Generic(int bxsize, int bysize, int surf_no, BOOL bSystem)
{
	// I can't add a BOOL bTarget parameter to this function like CSE2-p can, so I'll have to hardcode it like this :|
	bool isRenderTarget;
	switch (surf_no)
	{
	case 9:
	case 10:
	case 13:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
		isRenderTarget = true;
		break;
	default:
		isRenderTarget = false;
	}

	int mag = csvanilla::window_magnification;
	if (renderer == nullptr || !renderer->createSurface(surf_no, bxsize * mag, bysize * mag, isRenderTarget))
		return FALSE;

	auto& surfaceMetadata = csvanilla::surface_metadata[surf_no];
	surfaceMetadata.type = 1;
	surfaceMetadata.width = bxsize;
	surfaceMetadata.height = bysize;
	surfaceMetadata.bSystem = bSystem;
	csvanilla::strcpy(surfaceMetadata.name, "generic");
	return TRUE;
}
