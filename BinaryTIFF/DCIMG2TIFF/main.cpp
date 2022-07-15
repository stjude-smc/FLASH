#include "stdafx.h"



int main()
{
	uint32_t flip[] = { 0,0,0,0 };
	return dcimg2tiff( "E:\\test.tif.tmp", 1024, 1024, 1000, 1, 2, flip, flip, 0, 16400384, "E:\\test_Cy5.dcimg", "E:\\test_Cy7.dcimg", 0, 0 );

		/*
		char* tiffPath, 						// TIFF file (prepared by BinaryTIFF.lvproj)
		uint32_t frameWidth,					// frame width in pixels (single channel)
		uint32_t frameHeight, 					// frame height in pixels (single channel)
		uint32_t nFrames, 						// number of frames (excluding skipFrames)
		uint32_t skipFrames,					// number of frames to skip from beginning of movie
		uint32_t nChannels,						// number of channels (1 to 4)
		uint32_t* vFlip,						// length = nChannels. 0 - don't flip; 1 - flip upside down
		uint32_t* hFlip,						// length = nChannels. 0 - don't flip; 1 - flip left/right
		uint32_t ch3right,						// 3rd channel is placed on right if != 0, on left if == 0
												// (applies only if nChannels > 2)
		int64_t tiffOffset,						// offset to TIFF data block
		char* dcimgPath1, char* dcimgPath2, 	// raw image files (Hamamatsu DCIMG format, unsigned 16bit int)
		char* dcimgPath3, char* dcimgPath4);		// Layout:	(ch3right == 0)		(ch3right == 0)
												// 			dcimg1	dcimg2		dcimg1	dcimg2
												//		   	dcimg3 (dcimg4)	   (dcimg4) dcimg3
		*/
}