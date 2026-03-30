#pragma once
#include "tx/math.h"
#include "tx/grid.h"
#include "tx/graphics.h"
#include "tx/map.h"
#include "tx/json.h"
#include "tx/resource.h"
#include <bits/stdc++.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace std {
using bitSpan = span<tx::u8>;
}

namespace tx {

namespace RenderEngine {

const std::fs::path shaderPath = tx::getExeDir() / "assets/shaders";

std::string readShaderSource(const std::string& filename) {
	return readWholeFileText(shaderPath / filename);
}






} // namespace RenderEngine


Coord getWindowSize(GLFWwindow* window) {
	int x, y;
	glfwGetWindowSize(window, &x, &y);
	return Coord{ x, y };
}







template <class T, void (T::*Callback)(GLFWwindow*, int, int, int, int)>
void glfwSetKeyCallback(GLFWwindow* window, T* _this) {
	glfwSetWindowUserPointer(window, _this);

	glfwSetKeyCallback(
	    window,
	    [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		    T* obj = static_cast<T*>(glfwGetWindowUserPointer(window));
		    if (obj) {
			    (obj->*Callback)(window, key, scancode, action, mods);
		    }
	    });
}
inline void glEnableTransparent() {
	gl::enable(gl::enums::BLEND);
	gl::blendFunc(gl::enums::SRC_ALPHA, gl::enums::ONE_MINUS_SRC_ALPHA);
}
inline void glDisableDepthTest() {
	gl::disable(gl::enums::DEPTH_TEST);
}
inline void glBasicSettings() {
	glEnableTransparent();
	glDisableDepthTest();
}


enum class CoordDirection : u32 {
	Right = 0,
	Left = 1,
	Top = 2,
	Bottom = 3,
	TopRight = 4,
	TopLeft = 5,
	BottomLeft = 6,
	BottomRight = 7,
	None = 255
};
constexpr tx::Coord dirToCoord(CoordDirection dir) {
	return tx::_8wayIncrement[static_cast<int>(dir)];
}

// BMP Loader *****************************************************************************************************************
class BMPFile {
public:
	// headers **********************************************************
	// ================= BMP FILE HEADER =================
	// Fixed size: 14 bytes
	// Purpose: describes the FILE, not the image itself

	// Signature of the file.
	// Must be 'BM' (0x4D42, little-endian).
	// This is how you know the file is a BMP at all.
	uint16_t bfType;

	// Total size of the BMP file in bytes.
	// Includes headers, palettes, padding, and pixel data.
	// Rarely needed in loaders, mostly informational.
	uint32_t bfSize;

	// Reserved by Microsoft.
	// Must be zero.
	// You ignore these, but non-zero usually means malformed file.
	uint16_t bfReserved1;
	uint16_t bfReserved2;

	// Offset (in bytes from beginning of file) to the FIRST pixel.
	// This is the most important field in the file header.
	// You MUST seek to this location before reading pixel data.
	// Everything between the start of the file and bfOffBits is "metadata".
	uint32_t bfOffBits;


	// ================= DIB HEADER =================
	// Variable size (at least 12 bytes, usually 40)
	// Purpose: describes the IMAGE layout

	// Size of the DIB header in bytes.
	// Determines which DIB format is used:
	//  40  = BITMAPINFOHEADER (classic, what you want)
	// 108  = BITMAPV4HEADER
	// 124  = BITMAPV5HEADER
	// Never assume 40 — always read this.
	uint32_t dibSize;

	// Raw bytes of the DIB header AFTER dibSize.
	// Size = dibSize - 4.
	// Stored raw so:
	//  - you support future BMP variants
	//  - you don’t need different structs per version
	// You selectively parse fields you care about.
	std::vector<uint8_t> dibData;


	// ========== COMMONLY USED IMAGE FIELDS ==========
	// These exist starting from BITMAPINFOHEADER (dibSize >= 40)
	// Offsets are RELATIVE TO dibData (not file start)

	// Image width in pixels.
	// Positive value only.
	// This defines how many pixels per row.
	int32_t width = 0;

	// Image height in pixels.
	// IMPORTANT SIGN MEANING:
	//   > 0 : image is bottom-up (row 0 = bottom row)
	//   < 0 : image is top-down  (row 0 = top row)
	// Magnitude is the pixel height.
	int32_t height = 0;

	// Must be 1.
	// Historical artifact from old hardware.
	// If not 1, the file is invalid.
	uint16_t planes = 0;

	// Bits per pixel.
	// Common values:
	//   24 = BGR (no alpha)
	//   32 = BGRA
	//   8  = paletted
	// This tells you how many BYTES each pixel occupies.
	uint16_t bitCount = 0;

	// Compression method.
	// Common values:
	//   0 = BI_RGB (no compression) ← what you want
	//   1 = BI_RLE8
	//   2 = BI_RLE4
	//   3 = BI_BITFIELDS
	// Anything non-zero usually complicates loading.
	uint32_t compression = 0;


	bool valid = 1;

	BMPFile(const std::filesystem::path& fp) {
		this->ifs = std::ifstream{ fp };
		if (!ifs) {
			valid = 0;
			return;
		}

		// --- FILE HEADER
		ifs.read(reinterpret_cast<char*>(&bfType), 2);
		ifs.read(reinterpret_cast<char*>(&bfSize), 4);
		ifs.read(reinterpret_cast<char*>(&bfReserved1), 2);
		ifs.read(reinterpret_cast<char*>(&bfReserved2), 2);
		ifs.read(reinterpret_cast<char*>(&bfOffBits), 4);

		if (bfType != 0x4D42) {
			valid = 0;
			return;
		} // "BM"

		// --- DIB HEADER SIZE
		ifs.read(reinterpret_cast<char*>(&dibSize), 4);
		if (dibSize < 4) {
			valid = 0;
			return;
		}

		// --- DIB HEADER RAW DATA
		dibData.resize(dibSize - 4);
		ifs.read(reinterpret_cast<char*>(dibData.data()), dibData.size());

		// --- Parse common fields if BITMAPINFOHEADER or larger
		if (dibSize >= 40) {
			std::memcpy(&width, dibData.data() + 0, 4);
			std::memcpy(&height, dibData.data() + 4, 4);
			std::memcpy(&planes, dibData.data() + 8, 2);
			std::memcpy(&bitCount, dibData.data() + 10, 2);
			std::memcpy(&compression, dibData.data() + 12, 4);
		}

		// --- Skip anything between headers and pixels
		ifs.seekg(bfOffBits, std::ios::beg);
	}


	std::ifstream& getIfs() {
		return ifs;
	}

private:
	std::ifstream ifs;
};

using RGBMap = tx::GridSystem<tx::RGB>;
RGBMap readBMP(const std::fs::path& fp) {
	BMPFile file{ fp };

	RGBMap map{ file.width, file.height };

	std::ifstream& ifs = file.getIfs();

	const int width = file.width;
	const int height = std::abs(file.height);
	const bool topDown = file.height < 0;

	const int rowStride = ((width * 3 + 3) / 4) * 4;
	std::vector<uint8_t> row(rowStride);

	for (int bmpY = 0; bmpY < height; ++bmpY) {
		int y = topDown ? (height - 1 - bmpY) : bmpY;

		ifs.read(reinterpret_cast<char*>(row.data()), rowStride);

		for (int x = 0; x < width; ++x) {
			uint8_t b = row[x * 3 + 0];
			uint8_t g = row[x * 3 + 1];
			uint8_t r = row[x * 3 + 2];

			map.set(x, y, tx::RGB(r, g, b));
		}
	}
	//map.foreach([](const tx::RGB& in){ cout << in; });
	return map;
}

} // namespace tx