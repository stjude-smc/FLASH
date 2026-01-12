
#include "stdafx.h"
#include "dcimg2tiff.h"



#ifdef _WINDLL
std::ofstream r2tlog(LOGFILE);
#else
std::ostream& r2tlog = std::cout;
#endif



template <class T>
int raw2tiff_impl(char* tiffPath, uint32_t frameWidth, uint32_t frameHeight, uint32_t nFrames,
	uint32_t skipFrames, uint32_t nChannels, uint32_t* vFlip, uint32_t* hFlip, uint32_t ch3right,
	int64_t tiffOffset, uint32_t raw_offset, uint32_t frame_gap_bytes, std::vector<char*> pathlist)
{

	const int movieWidth = (nChannels > 1) ? 2 : 1;
	const int movieHeight = (nChannels > 2) ? 2 : 1;
	const int frameInputBytes = sizeof(T) * frameWidth * frameHeight + frame_gap_bytes;

	// Load raw data input files
	std::vector<std::ifstream> rawfile;
	for (int i=0; i<nChannels; ++i)
		rawfile.emplace_back(pathlist[i], std::ios::binary);

	for (auto& file : rawfile) {
		const uint64_t offset = raw_offset + skipFrames * frameInputBytes;

		file.seekg(offset, std::ios::beg);
		if (!file)
			return FG_ERROR_INVALID_INPUT;
	}

	// Open pre-formed TIFF file without destroying contents for writing frame data
	std::fstream tiffFile(tiffPath, std::ios::binary | std::ios::in | std::ios::out);
	if (!tiffFile)
		return FG_ERROR_INVALID_OUTPUT;

	tiffFile.seekp(tiffOffset, std::ios::beg);


	// NOTE: writeBuffer initialization to zero is important for unused channels (3-color).
	std::vector<T> readBuffer(frameWidth * frameHeight, 0);
	std::vector<T> writeBuffer(frameWidth * frameHeight * movieWidth * movieHeight, 0);
	auto itrIn = readBuffer.begin();

	for (int i = 0; i < nFrames; ++i)  // loop over frames
	{
		// Copy frame data from dcimg file to TIFF frame.
		for (int ch = 0; ch < nChannels; ++ch)
		{
			//r2tlog << "Read frame #" << i << ", ch " << ch << ": " << readBuffer.size() << " pixels.\n";

			rawfile[ch].read(reinterpret_cast<char*>(readBuffer.data()), readBuffer.size() * sizeof(T));
			if (!rawfile[ch])
				return FG_ERROR_INVALID_INPUT;
			rawfile[ch].seekg(frame_gap_bytes, std::ios::cur);  //fixed gap between images

			int rowOffset = (ch >= 2) * movieWidth * frameWidth * frameHeight;

			// place 3rd channel on the right side of the bottom row.
			int movieCol = ch % 2;
			if (nChannels==3 && ch == 2 && ch3right) movieCol = 1;

			// Copy frame data row by row, flipping if needed.
			for (int j = 0; j < frameHeight; j++)
			{
				auto itrOut = writeBuffer.begin() + ((movieWidth * j + movieCol) * frameWidth + rowOffset);

				if (vFlip[ch])
					itrIn = readBuffer.begin() + (frameHeight - (j + 1)) * frameWidth; //reverses row order
				else
					itrIn = readBuffer.begin() + j * frameWidth;

				if (hFlip[ch])
					std::reverse_copy(itrIn, itrIn + frameWidth, itrOut);
				else
					std::copy(itrIn, itrIn + frameWidth, itrOut);
			}
		} //for each input file

		//r2tlog << "Write frame #" << i << ": " << writeBuffer.size() << " pixels.\n";
		tiffFile.write(reinterpret_cast<char*>(writeBuffer.data()), writeBuffer.size() * sizeof(T));
		if (!tiffFile)
			return FG_ERROR_INVALID_OUTPUT;
		++currentFrame;

	} //for each frame

	return 0;
}



// Similar to dcimg2tiff but reads raw frame data from PVCam driver (photometrics).
//--------------------------------------------------------------------------------
extern "C" uint32_t raw2tiff(
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
	uint32_t bytesPerSample,				// number of bits per pixel (8 or 16).
	uint32_t raw_offset,                    // byte offset to image data in raw file
	uint32_t raw_frame_gap,					// byte gap between frames in raw file
	char* dcimgPath1, char* dcimgPath2, 	// raw image stack file paths
	char* dcimgPath3, char* dcimgPath4)		// Layout:	(ch3right == 0)		(ch3right == 1)
	// 			dcimg1	dcimg2		dcimg1	dcimg2
	//		   	dcimg3 (dcimg4)	   (dcimg4) dcimg3
{
	const int movieWidth = (nChannels > 1) ? 2 : 1;
	const int movieHeight = (nChannels > 2) ? 2 : 1;
	auto startTime = std::chrono::system_clock::now();
	int result;

	currentFrame = 0;

	std::vector<char*> pathlist{ dcimgPath1, dcimgPath2, dcimgPath3, dcimgPath4 };

	r2tlog << "START: tiffPath=" << tiffPath << ", dcimgPath1=" << dcimgPath1 << " frameWidth=" << frameWidth
		<< ", frameHeight=" << frameHeight << ", nFrames=" << nFrames << ", skipFrames=" << skipFrames
		<< ", nChannels=" << nChannels << ", ch3right=" << ch3right << ", tiffOffset=" << tiffOffset
		<< ", Raw offset=" << raw_offset << ", Frame gap=" << raw_frame_gap << std::endl;

	// Load raw frame, combine, and save to tif file.
	if (bytesPerSample == 2)
		result = raw2tiff_impl<uint16_t>(tiffPath, frameWidth, frameHeight, nFrames,
			skipFrames, nChannels, vFlip, hFlip, ch3right, tiffOffset, raw_offset, raw_frame_gap, pathlist);
	else
		result = raw2tiff_impl<uint8_t>(tiffPath, frameWidth, frameHeight, nFrames, 
			skipFrames, nChannels, vFlip, hFlip, ch3right, tiffOffset, raw_offset, raw_frame_gap, pathlist);

	currentFrame++; // final increment tells caller that conversion is done

	double outputBytes = static_cast<double>(nFrames) * movieWidth * movieHeight * frameWidth * frameHeight * bytesPerSample;
	std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - startTime;
	double mbs = outputBytes / elapsed.count() / (1024.0 * 1024.0 * 1024.0);
	r2tlog << std::fixed;
	r2tlog.precision(2);
	r2tlog << "FINISHED after " << elapsed.count() << " seconds (" << mbs << " GB/s).\n";

	return result;
}

