#include "bitmap.h"

#define STBI_ONLY_BMP
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdint>

Bitmap::Bitmap() : bitmapData(nullptr), width(0), height(0)
{}

Bitmap::Bitmap(Bitmap&& other) noexcept : bitmapData(other.bitmapData), width(other.width), height(other.height)
{
	other.bitmapData = nullptr;
}

Bitmap& Bitmap::operator=(Bitmap&& other) noexcept
{
	bitmapData = other.bitmapData;
	width = other.width;
	height = other.height;
	other.bitmapData = nullptr;
	return *this;
}

Bitmap::~Bitmap()
{
	stbi_image_free(bitmapData);
}

bool Bitmap::loadFromMemory(const unsigned char* bytes, unsigned size)
{
	int actualChannelCount;
	bitmapData = stbi_load_from_memory(bytes, size, &width, &height, &actualChannelCount, 4);
	if (bitmapData != nullptr && actualChannelCount == 3)
		setTransparency();
	return bitmapData != nullptr;
}

bool Bitmap::loadFromFile(const char* fileName)
{
	int actualChannelCount;
	bitmapData = stbi_load(fileName, &width, &height, &actualChannelCount, 4);
	if (bitmapData != nullptr && actualChannelCount == 3)
		setTransparency();
	return bitmapData != nullptr;
}

void Bitmap::setTransparency()
{
	std::uint32_t* pixel = reinterpret_cast<std::uint32_t*>(bitmapData);
	for (int i = 0; i < width * height; ++i)
	{
		// Set alpha of black pixels to 0
		if ((*pixel & 0xFFFFFF) == 0)
			*pixel = 0;
		++pixel;
	}
}

Bitmap Bitmap::scale(int mag) const
{
	if (bitmapData == nullptr)
		return Bitmap{};

	unsigned char* upscaledImgData = static_cast<unsigned char*>(malloc(width * height * mag * mag * 4));
	if (upscaledImgData == nullptr)
	{
		// I would throw an exception but I don't feel like handling those properly, so let's just return an empty image
		return Bitmap{};
	}

	const unsigned char* srcPtr = bitmapData;
	const unsigned UpscaledRowLength = width * mag * 4;
	
	for (int y = 0; y < height; ++y)
	{
		unsigned char* destRow = &upscaledImgData[y * mag * UpscaledRowLength];

		unsigned char* destPtr = destRow;
		for (int x = 0; x < width; ++x)
		{
			for (int i = 0; i < mag; ++i)
				for (int j = 0; j < 4; ++j)
					*destPtr++ = srcPtr[j];
			srcPtr += 4;
		}

		for (int i = 1; i < mag; ++i)
			memcpy(destRow + i * UpscaledRowLength, destRow, UpscaledRowLength);
	}

	Bitmap upscaled;
	upscaled.bitmapData = upscaledImgData;
	upscaled.width = width * mag;
	upscaled.height = height * mag;
	return upscaled;
}
