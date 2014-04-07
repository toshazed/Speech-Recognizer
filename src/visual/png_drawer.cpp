#include <cstdlib>
#include "png.h"
#include "png_drawer.h"
#include "assert.h"

using namespace std;
using namespace wtm::audio;

namespace wtm {
namespace visual {

/**
 * Draw PNG visualization for the data
 *
 * @see http://www.libpng.org/pub/png/book/
 */
void PngDrawer::drawToFile(const TWavDataPtr wavData, const string& file) {

	uint32_t imgWidth = 10 * 1024;
	uint32_t imgHeight = 255;
	uint32_t xMax = wavData->getDuration();
	uint32_t yMax = wavData->getMaxVal();

	uint32_t bufferSize = imgWidth * imgHeight * sizeof(bool);
	bool* image = (bool*) malloc(bufferSize);
	if (image == NULL) {
		fprintf(stderr, "Could not create image buffer\n");
		return;
	}
	memset(image, 0, bufferSize);

	uint32_t xCurr = 0;
	for (list<raw_t>::iterator yCurr = wavData->getRawData()->begin();
			yCurr != wavData->getRawData()->end(); ++yCurr) {

		// Contractive mapping
		uint32_t x = xCurr * (imgWidth - 1) / xMax;
		assert(x <= xMax);

		uint32_t y = (*yCurr) * (imgHeight - 1) / yMax;
		assert(y <= yMax);

		uint32_t index = x + y * imgWidth;
		assert(index <= bufferSize);

		image[index] = true;
		xCurr++;
	}

	writeImage(file.c_str(), image, imgWidth, imgHeight);
	free(image);
}

int PngDrawer::writeImage(const char* filename, bool* image, uint32_t width, uint32_t height) {
	int code = 0;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep row;

	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
	uint32_t rowSize;
	rowSize = 3 * width * sizeof(png_byte);
	row = (png_bytep) malloc(rowSize);

	// Write image data
	uint32_t x, y;
	for (y = 0; y < height; y++) {
		memset(row, 0, rowSize);

		for (x = 0; x < width; x++) {
			if (image[x + y * width]) {
				row[x] = 255;
				row[x + 1] = 255;
				row[x + 2] = 255;
			}
		}

		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	// Clean up
	finalise:
		if (fp != NULL)
			fclose(fp);
		if (info_ptr != NULL)
			png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		if (png_ptr != NULL)
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		if (row != NULL)
			free(row);

	return code;
}

} // namespace visual
} // namespace wtm
