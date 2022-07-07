// dcimg2tiff_debug.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	char* tiffPath = "C:\\tempData\\Stack.tif";
	uint32_t frameWidth = 1024;
	uint32_t frameHeight = 478;
	uint32_t nFrames = 2000;
	uint32_t skipFrames = 1;
	uint32_t nChannels = 2;
	uint32_t vFlip[] = { 0,0 };
	uint32_t hFlip[] = { 0,0 };
	uint32_t ch3right = 0;
	int64_t tiffOffset = 0;
	char* dcimgPath1 = "C:\\tempData\\Stack_Cy3.dcimg";
	char* dcimgPath2 = "C:\\tempData\\Stack_Cy5.dcimg";
	char* dcimgPath3 = "";
	char* dcimgPath4 = "";

	FILE* newFile = fopen(tiffPath, "wb");
	fclose(newFile);

	dcimg2tiff(tiffPath, frameWidth, frameHeight, nFrames, skipFrames, nChannels, vFlip, hFlip, ch3right, tiffOffset, dcimgPath1, dcimgPath2, dcimgPath3, dcimgPath4);
    return 0;
}


//int main()
//{
//	uint64_t counter = thread_test();
//	printf("%u", (unsigned int)counter);
//	system("pause");
//	return 0;
//}
