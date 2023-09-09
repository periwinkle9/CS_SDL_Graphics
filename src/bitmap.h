#pragma once

#include <utility>

class Bitmap
{
	unsigned char* bitmapData;
	int width;
	int height;

	void setTransparency();
public:
	Bitmap();
	Bitmap(const unsigned char* bytes, unsigned size) { loadFromMemory(bytes, size); }
	explicit Bitmap(const char* fileName) { loadFromFile(fileName); }
	~Bitmap();
	Bitmap(const Bitmap&) = delete;
	Bitmap(Bitmap&&) noexcept;
	Bitmap& operator=(const Bitmap&) = delete;
	Bitmap& operator=(Bitmap&&) noexcept;

	bool loadFromMemory(const unsigned char* bytes, unsigned size);
	bool loadFromFile(const char* fileName);

	unsigned char* data() const { return bitmapData; }
	std::pair<int, int> size() const { return std::make_pair(width, height); }

	Bitmap scale(int mag) const;

	explicit operator bool() const { return bitmapData != nullptr; }
};
