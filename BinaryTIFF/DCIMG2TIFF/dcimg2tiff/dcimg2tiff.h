#pragma once
#ifdef DCIMG2TIFF_EXPORTS
#define DCIMG2TIFFDLL_API __declspec(dllexport) 
#else
#define DCIMG2TIFFDLL_API __declspec(dllimport) 
#endif

#define DCIMG_OFFSET	864		// Hamamatsu DCIMG header size was 912
#define FRAME_OFFSET	32		// bytes between frames was 16
#define MISSING_OFFSET	12		// byte offset to missing pixels between frames
#define MISSING_PIXELS	4		// number of missing pixels

extern "C" DCIMG2TIFFDLL_API uint64_t getCurrentFrame();

extern "C" DCIMG2TIFFDLL_API void resetCurrentFrame();

extern "C" DCIMG2TIFFDLL_API void cancelConversion();

extern "C" DCIMG2TIFFDLL_API uint32_t dcimg2tiff(
	char* tiffPath, 						// TIFF file (prepared by BinaryTIFF.lvproj)
	uint32_t frameWidth,					// frame width (single channel)
	uint32_t frameHeight, 					// frame height (single channel)
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

extern "C" DCIMG2TIFFDLL_API void writeDummyFile(char* filePath, int fileLength);