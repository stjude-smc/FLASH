#include "stdafx.h"
#include "dcimg2tiff.h"

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

	try
	{
		for (int rep = 0; rep < repeats; ++rep)
		{
			result = dcimg2tiff( "E:\\test\\Stack005.tif", 1152, 1152, 1000, 1, 1, hflip, vflip, 0, 16400384, "E:\\test\\Stack005_Cy2.dcimg", 0, 0, 0 );
			//result = raw2tiff("E:\\test\\test.tif", frameWidth, frameHeight, nFrames, 1, nChannels, vflip, hflip, 0, 16400384, bytesPerSample, "E:\\test\\test_Cy3.raw", "E:\\test\\test_Cy5.raw", 0, 0);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cout << "Other error" << std::endl;
		return -1;
	}

	return result;
}