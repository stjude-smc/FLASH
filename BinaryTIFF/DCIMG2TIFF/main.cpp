#include "stdafx.h"
#include <chrono>
#include <iostream>

/*
Use this entry point to test the code.
We assume the files below have been created already by Flash Gordon.
Need to configure it to not delete the intermediate files after conversion.
*/



int main()
{
	constexpr int repeats = 3;
	constexpr int frameWidth = 1600;
	constexpr int frameHeight = 1600;
	constexpr int nFrames = 1000;
	constexpr int nChannels = 2;
	uint32_t bytesPerSample = 2;
	uint32_t vflip[] = { 0,1,0,0 };
	uint32_t hflip[] = { 1,0,0,0 };

	int result;
	double totalTime = 0;
	static auto start = std::chrono::system_clock::now();

	try
	{
		for (int rep = 0; rep < repeats; ++rep)
		{
			//return dcimg2tiff( "E:\\test.tif.tmp", 1024, 1024, 1000, 1, 2, hflip, vflip, 0, 16400384, "E:\\test_Cy5.dcimg", "E:\\test_Cy7.dcimg", 0, 0 );
			result = raw2tiff("E:\\test\\test.tif", frameWidth, frameHeight, nFrames, 1, nChannels, vflip, hflip, 0, 16400384, bytesPerSample, "E:\\test\\test_Cy3.raw", "E:\\test\\test_Cy5.raw", 0, 0);
		}
	}
	catch (...)
	{
		std::cout << "Error" << std::endl;
		return -1;
	}

	std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
	double gbs = static_cast<double>(frameWidth)*frameHeight*nChannels*bytesPerSample*nFrames*repeats / elapsed.count() / (1024.0*1024.0*1024.0);
	std::cout << std::endl << "Mean elapsed time: " << elapsed.count() / repeats << "s (" << gbs << " GiB/s)" << std::endl;

	return result;
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