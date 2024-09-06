// dcimg2tiff.cpp : Defines the exported functions for the DLL application.
// 
// FIXME: redirect cout and cerr to log file instead of creating a new ofstream object.


#include "stdafx.h"
#include "dcimg2tiff.h"
#include "DcimgFile.h"


// Global variables
uint64_t currentFrame;

// query progress of dcimg2tiff()
extern "C" uint64_t getCurrentFrame()
{
	return currentFrame;
}

// reset current frame to -1
extern "C" void resetCurrentFrame()
{
	currentFrame = -1;
}



// dcimg2tiff combines the data from 1 to 4 Hamamatsu HD recorder files (DCIMG raw image format)
// into the data block of a BigTIFF file whose header and IFDs have been created 
// using the BinaryTIFF LabView project. Unsigned 16bit files only.
//
extern "C" uint32_t dcimg2tiff(
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
	char* dcimgPath3, char* dcimgPath4)		// Layout:	(ch3right == 0)		(ch3right == 1)
											// 			dcimg1	dcimg2		dcimg1	dcimg2
											//		   	dcimg3 (dcimg4)	   (dcimg4) dcimg3
{
	// Redirect console output to log file
#ifdef _WINDLL
	std::ofstream fglog(LOGFILE);
	std::cout.rdbuf(fglog.rdbuf());
	std::cerr.rdbuf(fglog.rdbuf());
#endif

	std::cout << "START: tiffPath=" << tiffPath << ", dcimgPath1=" << dcimgPath1 << ", frameWidth=" << frameWidth
		  << ", frameHeight=" << frameHeight << ", nFrames=" << nFrames << ", skipFrames=" << skipFrames
		  << ", nChannels=" << nChannels << ", ch3right=" << ch3right << ", tiffOffset=" << tiffOffset << std::endl;
	auto startTime = std::chrono::system_clock::now();

	currentFrame = 0;
	int movieWidth = (nChannels > 1) ? 2 : 1;
	int movieHeight = (nChannels > 2) ? 2 : 1;
	std::vector<uint16_t> writeBuffer(movieWidth * movieHeight * frameWidth * frameHeight, 0);

	// Open image data inputs (dcimg files)
	DCIMG_ERR	err;
	std::vector<char*> dcimgPaths{ dcimgPath1, dcimgPath2, dcimgPath3, dcimgPath4 };
	std::vector<DcimgFile> hdcimg;
	bool droppedFrames = false;

	for (int i = 0; i < nChannels; ++i)
	{
		try {
			hdcimg.emplace_back(dcimgPaths[i]);
			hdcimg[i].dcimg_is_valid(frameWidth, frameHeight, nFrames + skipFrames);
			droppedFrames = hdcimg[i].check_dcimg_timestamps();
			if (droppedFrames)
				std::cout << "Dropped frames detected in dcimg file #" << i+1 << std::endl;
		}
		catch (std::runtime_error& err) {
			std::cout << "Error opening dcimg input file #" << i+1 << ": " << err.what() << std::endl << std::endl;
			return FG_ERROR_INVALID_INPUT;
		}
	}
	
	// Open pre-formed TIFF file without destroying contents for writing frame data
	std::fstream tiffFile(tiffPath, std::ios::binary | std::ios::in | std::ios::out);
	if (!tiffFile)
	{
		std::cout << "Error opening tif output file: " << tiffPath << std::endl << std::endl;
		return FG_ERROR_INVALID_OUTPUT;
	}
	tiffFile.seekp(tiffOffset, std::ios::beg);

	
	for (int i=skipFrames; i<nFrames+skipFrames; ++i)
	{
		for (int ch=0; ch<nChannels; ++ch)
		{
			uint16_t* data;
			try {
				data = hdcimg[ch].read_frame(i);
			}
			catch (std::runtime_error& err) {
				std::cout << "Error reading frame " << i << " from dcimg file #" << ch << ": " << err.what() << std::endl << std::endl;
				return FG_ERROR_INVALID_INPUT;
			}

			int movieCol = ch % 2;
			int rowOffset = (ch >= 2) * movieWidth * frameWidth * frameHeight;

			// place 3rd channel on the right side of the bottom row.
			if (nChannels==3 && ch==2 && ch3right) movieCol = 1;

			// Copy frame data row by row, flipping if needed.
			for (int j = 0; j < frameHeight; j++)
			{
				uint16_t* itrIn;
				auto itrOut = writeBuffer.begin() + ((movieWidth * j + movieCol) * frameWidth + rowOffset);

				if (vFlip[ch])
					itrIn = data + (frameHeight - (j + 1)) * frameWidth; //reverses row order
				else
					itrIn = data + j * frameWidth;

				if (hFlip[ch])
					std::reverse_copy(itrIn, itrIn + frameWidth, itrOut);
				else
					std::copy(itrIn, itrIn + frameWidth, itrOut);
			}
		} //for each dcimg file

		tiffFile.write(reinterpret_cast<char*>(writeBuffer.data()), writeBuffer.size()*sizeof(uint16_t));
		if (!tiffFile)
			return FG_ERROR_INVALID_OUTPUT;

		currentFrame++;
	}

	currentFrame++; // final increment to tell caller that conversion is done

	std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - startTime;
	double outputBytes = static_cast<double>(nFrames) * movieWidth * movieHeight * frameWidth * frameHeight * sizeof(uint16_t);
	double mbs = outputBytes / elapsed.count() / (1024.0 * 1024.0 * 1024.0);
	std::cout << std::fixed;
	std::cout.precision(2);
	std::cout << "FINISHED after " << elapsed.count() << " seconds (" << mbs << " GB/s).\n\n" << std::endl;

	if (droppedFrames)
		return FG_ERROR_DROPPED_FRAME;
	else
		return 0;
}
