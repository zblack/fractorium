#pragma once

#include "EmberCommonPch.h"

#define PNG_COMMENT_MAX 8

/// <summary>
/// Write a PPM file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <returns>True if success, else false</returns>
static bool WritePpm(const char* filename, unsigned char* image, size_t width, size_t height)
{
	bool b = false;
	size_t size = width * height * 3;
	FILE* file;

	if (fopen_s(&file, filename, "wb") == 0)
	{
		fprintf_s(file, "P6\n");
		fprintf_s(file, "%d %d\n255\n", width, height);

		b = (size == fwrite(image, 1, size, file));
		fclose(file);
	}

	return b;
}

/// <summary>
/// Write a JPEG file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="quality">The quality to use</param>
/// <param name="enableComments">True to embed comments, else false</param>
/// <param name="comments">The comment string to embed</param>
/// <param name="id">Id of the author</param>
/// <param name="url">Url of the author</param>
/// <param name="nick">Nickname of the author</param>
/// <returns>True if success, else false</returns>
static bool WriteJpeg(const char* filename, unsigned char* image, size_t width, size_t height, int quality, bool enableComments, EmberImageComments& comments, string id, string url, string nick)
{
	bool b = false;
	FILE* file;

	if (fopen_s(&file, filename, "wb") == 0)
	{
		size_t i;
		jpeg_error_mgr jerr;
		jpeg_compress_struct info;
		char nickString[64], urlString[128], idString[128];
		char bvString[64], niString[64], rtString[64];
		char genomeString[65536], verString[64];
   
		//Create the mandatory comment strings.
		snprintf_s(genomeString, 65536, "flam3_genome: %s", comments.m_Genome.c_str());
		snprintf_s(bvString, 64, "flam3_error_rate: %s", comments.m_Badvals.c_str());
		snprintf_s(niString, 64, "flam3_samples: %s", comments.m_NumIters);
		snprintf_s(rtString, 64, "flam3_time: %s", comments.m_Runtime.c_str());
		snprintf_s(verString, 64, "flam3_version: %s", EmberVersion());

		info.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&info);
		jpeg_stdio_dest(&info, file);
		info.in_color_space = JCS_RGB;
		info.input_components = 3;
		info.image_width = (JDIMENSION)width;
		info.image_height = (JDIMENSION)height;
		jpeg_set_defaults(&info);
		jpeg_set_quality(&info, quality, TRUE);
		jpeg_start_compress(&info, TRUE);
	
		//Write comments to jpeg.
		if (enableComments)
		{
			jpeg_write_marker(&info, JPEG_COM, (unsigned char*)verString, (int)strlen(verString));

			if (nick != "")
			{
				snprintf_s(nickString, 64, "flam3_nickname: %s", nick.c_str());
				jpeg_write_marker(&info, JPEG_COM, (unsigned char*)nickString, (int)strlen(nickString));
			}

			if (url != "")
			{
				snprintf_s(urlString, 128, "flam3_url: %s", url.c_str());
				jpeg_write_marker(&info, JPEG_COM, (unsigned char*)urlString, (int)strlen(urlString));
			}
		
			if (id != "")
			{
				snprintf_s(idString, 128, "flam3_id: %s", id.c_str());
				jpeg_write_marker(&info, JPEG_COM, (unsigned char*)idString, (int)strlen(idString));
			}

			jpeg_write_marker(&info, JPEG_COM, (unsigned char*)bvString, (int)strlen(bvString));
			jpeg_write_marker(&info, JPEG_COM, (unsigned char*)niString, (int)strlen(niString));
			jpeg_write_marker(&info, JPEG_COM, (unsigned char*)rtString, (int)strlen(rtString));
			jpeg_write_marker(&info, JPEG_COM, (unsigned char*)genomeString, (int)strlen(genomeString));
		}

		for (i = 0; i < height; i++)
		{
			JSAMPROW row_pointer[1];
			row_pointer[0] = (unsigned char*)image + (3 * width * i);
			jpeg_write_scanlines(&info, row_pointer, 1);
		}

		jpeg_finish_compress(&info);
		jpeg_destroy_compress(&info);
		fclose(file);
		b = true;
	}

	return b;
}

/// <summary>
/// Write a PNG file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="bytesPerChannel">Bytes per channel, 1 or 2.</param>
/// <param name="enableComments">True to embed comments, else false</param>
/// <param name="comments">The comment string to embed</param>
/// <param name="id">Id of the author</param>
/// <param name="url">Url of the author</param>
/// <param name="nick">Nickname of the author</param>
/// <returns>True if success, else false</returns>
static bool WritePng(const char* filename, unsigned char* image, size_t width, size_t height, size_t bytesPerChannel, bool enableComments, EmberImageComments& comments, string id, string url, string nick)
{
	bool b = false;
	FILE* file;

	if (fopen_s(&file, filename, "wb") == 0)
	{
		png_structp  png_ptr;
		png_infop    info_ptr;
		png_text     text[PNG_COMMENT_MAX];
		size_t i;
		unsigned short testbe = 1;
		vector<unsigned char*> rows(height);

		text[0].compression = PNG_TEXT_COMPRESSION_NONE;
		text[0].key = "flam3_version";
		text[0].text = (png_charp)EmberVersion();

		text[1].compression = PNG_TEXT_COMPRESSION_NONE;
		text[1].key = "flam3_nickname";
		text[1].text = (png_charp)nick.c_str();

		text[2].compression = PNG_TEXT_COMPRESSION_NONE;
		text[2].key = "flam3_url";
		text[2].text = (png_charp)url.c_str();
  
		text[3].compression = PNG_TEXT_COMPRESSION_NONE;
		text[3].key = "flam3_id";
		text[3].text = (png_charp)id.c_str();

		text[4].compression = PNG_TEXT_COMPRESSION_NONE;
		text[4].key = "flam3_error_rate";
		text[4].text = (png_charp)comments.m_Badvals.c_str();

		text[5].compression = PNG_TEXT_COMPRESSION_NONE;
		text[5].key = "flam3_samples";
		text[5].text = (png_charp)comments.m_NumIters.c_str();

		text[6].compression = PNG_TEXT_COMPRESSION_NONE;
		text[6].key = "flam3_time";
		text[6].text = (png_charp)comments.m_Runtime.c_str();

		text[7].compression = PNG_TEXT_COMPRESSION_zTXt;
		text[7].key = "flam3_genome";
		text[7].text = (png_charp)comments.m_Genome.c_str();

		for (i = 0; i < height; i++)
			rows[i] = (unsigned char*)image + i * width * 4 * bytesPerChannel;
	  
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		info_ptr = png_create_info_struct(png_ptr);

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			fclose(file);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			perror("writing file");
			return false;
		}

		png_init_io(png_ptr, file);

		png_set_IHDR(png_ptr, info_ptr, (png_uint_32)width, (png_uint_32)height, 8 * (png_uint_32)bytesPerChannel,
			PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE,
			PNG_FILTER_TYPE_BASE);
		   
		if (enableComments == 1)
			png_set_text(png_ptr, info_ptr, text, PNG_COMMENT_MAX);

		png_write_info(png_ptr, info_ptr);

		//Must set this after png_write_info().
		if (bytesPerChannel == 2 && testbe != htons(testbe))
		{
			png_set_swap(png_ptr);
		}

		png_write_image(png_ptr, (png_bytepp) rows.data());
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(file);
		b = true;
	}

	return b;
}

/// <summary>
/// Convert an RGB buffer to BGR for usage with BMP.
/// </summary>
/// <param name="buffer">The buffer to convert</param>
/// <param name="width">The width.</param>
/// <param name="height">The height.</param>
/// <param name="newSize">The size of the new buffer created</param>
/// <returns>The converted buffer if successful, else NULL.</returns>
static BYTE* ConvertRGBToBMPBuffer(BYTE* buffer, size_t width, size_t height, size_t& newSize)
{
	if (NULL == buffer || width == 0 || height == 0)
		return NULL;
		
	size_t padding = 0;
	size_t scanlinebytes = width * 3;
	while ((scanlinebytes + padding ) % 4 != 0) 
		padding++;

	size_t psw = scanlinebytes + padding;

	newSize = height * psw;
	BYTE* newBuf = new BYTE[newSize];

	if (newBuf)
	{
		memset (newBuf, 0, newSize);

		size_t bufpos = 0;
		size_t newpos = 0;

		for (size_t y = 0; y < height; y++)
		{
			for (size_t x = 0; x < 3 * width; x += 3)
			{
				bufpos = y * 3 * width + x;     // position in original buffer
				newpos = (height - y - 1) * psw + x; // position in padded buffer
				newBuf[newpos] = buffer[bufpos+2];       // swap r and b
				newBuf[newpos + 1] = buffer[bufpos + 1]; // g stays
				newBuf[newpos + 2] = buffer[bufpos];     // swap b and r

				//No swap.
				//newBuf[newpos] = buffer[bufpos];
				//newBuf[newpos + 1] = buffer[bufpos + 1];
				//newBuf[newpos + 2] = buffer[bufpos + 2];
			}
		}

		return newBuf;
	}

	return NULL;
}

/// <summary>
/// Save a Bmp file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="paddedSize">Padded size, greater than or equal to total image size.</param>
/// <returns>True if success, else false</returns>
static bool SaveBmp(const char* filename, BYTE* image, size_t width, size_t height, size_t paddedSize)
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	DWORD bwritten;
	HANDLE file;
	memset (&bmfh, 0, sizeof (BITMAPFILEHEADER));
	memset (&info, 0, sizeof (BITMAPINFOHEADER));

	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (DWORD)paddedSize;
	bmfh.bfOffBits = 0x36;

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = (LONG)width;
	info.biHeight = (LONG)height;
	info.biPlanes = 1;	
	info.biBitCount = 24;
	info.biCompression = BI_RGB;	
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0x0ec4;  
	info.biYPelsPerMeter = 0x0ec4;     
	info.biClrUsed = 0;	
	info.biClrImportant = 0; 

	if ((file = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == NULL)
	{
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, &bmfh, sizeof (BITMAPFILEHEADER), &bwritten, NULL) == false)
	{	
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, &info, sizeof(BITMAPINFOHEADER), &bwritten, NULL) == false)
	{	
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, image, (DWORD)paddedSize, &bwritten, NULL) == false)
	{	
		CloseHandle(file);
		return false;
	}

	CloseHandle(file);
	return true;
}

/// <summary>
/// Convert a buffer from RGB to BGR and write a Bmp file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <returns>True if success, else false</returns>
static bool WriteBmp(const char* filename, unsigned char* image, size_t width, size_t height)
{
	bool b = false;
	size_t newSize;
	auto_ptr<BYTE> bgrBuf(ConvertRGBToBMPBuffer(image, width, height, newSize));

	if (bgrBuf.get())
		b = SaveBmp(filename, bgrBuf.get(), width, height, newSize);

	return b;
}
