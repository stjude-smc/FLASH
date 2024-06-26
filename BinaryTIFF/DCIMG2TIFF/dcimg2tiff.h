#pragma once
#ifdef DCIMG2TIFF_EXPORTS
#define DCIMG2TIFFDLL_API __declspec(dllexport) 
#else
#define DCIMG2TIFFDLL_API __declspec(dllimport) 
#endif

// Link to DCIMG API. From https://dcam-api.com/sdk-downloads/
// This should be installed in your compiler's include path.
// It is not included with the source code because of license restrictions.
// We used version 17.4.5275 to compile the dll distributed with the software.
#include "dcimgapi.h"
#pragma comment(lib,"dcimgapi.lib")


extern "C" DCIMG2TIFFDLL_API uint64_t getCurrentFrame();

extern "C" DCIMG2TIFFDLL_API void resetCurrentFrame();

extern "C" DCIMG2TIFFDLL_API void cancelConversion();

extern "C" DCIMG2TIFFDLL_API uint32_t dcimg2tiff(
	char* tiffPath, 						// TIFF file (prepared by BinaryTIFF.lvproj)
	uint32_t _frameWidth,					// frame width (single channel)
	uint32_t _frameHeight, 					// frame height (single channel)
	uint32_t nFrames, 						// number of frames
	uint32_t skipFrames,					// number of frames to skip from beginning of movie
	uint32_t nChannels,						// number of channels (2 to 4)
	uint32_t* vFlip,						// length = nChannels. 0 - don't flip; 1 - flip upside down
	uint32_t* hFlip,						// length = nChannels. 0 - don't flip; 1 - flip left/right
	uint32_t ch3right,						// 3rd channel is placed on right if != 0, on left if == 0
											// (applies only if nChannels > 2)
	int64_t tiffOffset,						// offset to TIFF data block
	char* dcimgPath1, char* dcimgPath2, 	// raw image files (Hamamatsu DCIMG format, unsigned 16bit int)
	char* dcimgPath3, char* dcimgPath4);	// (dcimgPath3/4 optional)
											// Layout:	(ch3right == 0)		(ch3right == 0)
											// 			dcimg1	dcimg2		dcimg1	dcimg2
											//		   	dcimg3 (dcimg4)	   (dcimg4) dcimg3

extern "C" DCIMG2TIFFDLL_API uint32_t raw2tiff(
	char* tiffPath, 						// TIFF file (prepared by BinaryTIFF.lvproj)
	uint32_t _frameWidth,					// frame width (single channel)
	uint32_t _frameHeight, 					// frame height (single channel)
	uint32_t nFrames, 						// number of frames
	uint32_t skipFrames,					// number of frames to skip from beginning of movie
	uint32_t nChannels,						// number of channels (2 to 4)
	uint32_t* vFlip,						// length = nChannels. 0 - don't flip; 1 - flip upside down
	uint32_t* hFlip,						// length = nChannels. 0 - don't flip; 1 - flip left/right
	uint32_t ch3right,						// 3rd channel is placed on right if != 0, on left if == 0
	// (applies only if nChannels > 2)
	int64_t tiffOffset,						// offset to TIFF data block
	uint32_t bytesPerSample,				// number of bits per pixel (8 or 16).
	char* dcimgPath1, char* dcimgPath2, 	// raw image stack file paths
	char* dcimgPath3, char* dcimgPath4);	// (dcimgPath3/4 optional)
// Layout:	(ch3right == 0)		(ch3right == 0)
// 			dcimg1	dcimg2		dcimg1	dcimg2
//		   	dcimg3 (dcimg4)	   (dcimg4) dcimg3

extern "C" DCIMG2TIFFDLL_API void writeDummyFile(char* filePath, int fileLength);