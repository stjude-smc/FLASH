// dcimg2tiff.cpp : Defines the exported functions for the DLL application.
// 
// FIXME: wrap dcimg library functions in a class to support RIAA.

#include "stdafx.h"
#include "dcimg2tiff.h"


// definitions for hamamatsu functions
HDCIMG dcimgcon_init_open(const char* filename);
bool dcimg_is_valid(HDCIMG hdcimg, int32 width, int32 height, int32 nFrames);
bool check_dcimg_timestamps(HDCIMG hdcimg);


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


#ifdef _WINDLL
std::ofstream fglog(LOGFILE);
#else
std::ostream& fglog = std::cout;
#endif

void log_dcimg_error(const char* fcn, DCIMG_ERR err)
{
	fglog << " *** " << fcn << " error: 0x" << std::hex << err << std::dec << std::endl;
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
	fglog << "START: tiffPath=" << tiffPath << ", dcimgPath1=" << dcimgPath1 << ", frameWidth=" << frameWidth
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
	std::vector<HDCIMG> hdcimg(nChannels, nullptr);

	for (int i = 0; i < nChannels; ++i)
	{
		hdcimg[i] = dcimgcon_init_open(dcimgPaths[i]);
		if (hdcimg[i] == nullptr)
		{
			fglog << "Error opening dcimg input file: " << dcimgPaths[i] << std::endl;
			return FG_ERROR_INVALID_INPUT;
		}

		if (!dcimg_is_valid(hdcimg[i], frameWidth, frameHeight, nFrames+skipFrames))
		{
			fglog << "Mismatched dcimg metadata parameters: " << dcimgPaths[i] << std::endl;
			return FG_ERROR_INVALID_INPUT;
		}

		// Verify frame timestamps have no gaps (dropped frames)
		if (!check_dcimg_timestamps(hdcimg[i]))
			return FG_ERROR_DROPPED_FRAME;

		// FIXME: close files already open, if any.
	}
	
	// Open pre-formed TIFF file without destroying contents for writing frame data
	std::fstream tiffFile(tiffPath, std::ios::binary | std::ios::in | std::ios::out);
	if (!tiffFile)
	{
		fglog << "Error opening tif output file: " << tiffPath << std::endl;
		for (auto& file : hdcimg)
			dcimg_close(file);
		return FG_ERROR_INVALID_OUTPUT;
	}
	tiffFile.seekp(tiffOffset, std::ios::beg);


	DCIMG_FRAME	frame;
	memset(&frame, 0, sizeof(frame));
	frame.size = sizeof(frame);

	for (int i=skipFrames; i<nFrames+skipFrames; ++i)
	{
		frame.iFrame = i;

		for (int ch=0; ch<nChannels; ++ch)
		{
			// Read frame data from DCIMG file via API
			err = dcimg_lockframe(hdcimg[ch], &frame);
			if (failed(err))  {
				fglog << "dcimg_lockframe error " << std::hex << err << std::dec << " frame #" << i << std::endl;
				for (auto& file : hdcimg)
					dcimg_close(file);
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
					itrIn = static_cast<uint16_t*>(frame.buf) + (frameHeight - (j + 1)) * frameWidth; //reverses row order
				else
					itrIn = static_cast<uint16_t*>(frame.buf) + j * frameWidth;

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

	// clean up
	for (auto & file : hdcimg)
		dcimg_close(file);

	currentFrame++; // final increment to tell caller that conversion is done

	std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - startTime;
	double outputBytes = static_cast<double>(nFrames) * movieWidth * movieHeight * frameWidth * frameHeight * sizeof(uint16_t);
	double mbs = outputBytes / elapsed.count() / (1024.0 * 1024.0 * 1024.0);
	fglog << std::fixed;
	fglog.precision(2);
	fglog << "FINISHED after " << elapsed.count() << " seconds (" << mbs << " GB/s).\n\n" << std::endl;

	return 0;
}




// ----------------------------------------------------------------
// These files are from common_dcimg.cpp

/**
 @brief open file and get DCIMG handle
 @param filename:	DCIMG file name
 @return DCIMG handle
 */
HDCIMG dcimgcon_init_open(const char* filename)
{
	DCIMG_ERR	err;

	// initialize DCIMG-API
	DCIMG_INIT	initparam;
	memset(&initparam, 0, sizeof(initparam));
	initparam.size = sizeof(initparam);

	err = dcimg_init(&initparam);
	if (failed(err)) {
		log_dcimg_error("dcimg_init", err);
		return nullptr;
	}

	// open DCIMG file, retrying a few times in case file is still saving.
	DCIMG_OPEN	openparam;
	memset(&openparam, 0, sizeof(openparam));
	openparam.size = sizeof(openparam);
	openparam.path = filename;

	err = dcimg_open(&openparam);
	if (failed(err)) {
		log_dcimg_error("dcimg_open", err);
		return nullptr;
	}

	return openparam.hdcimg;
}


// Return false if dcimg file doesn't match input parameters.
// FIXME: would be nice to give a log message if failed.
bool dcimg_is_valid(HDCIMG hdcimg, int32 width, int32 height, int32 nFrames)
{
	DCIMG_ERR err;
	int32 data;

	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_WIDTH, &data);
	if (failed(err)) {
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_IMAGE_WIDTH)", err);
		return false;
	}
	else if (data != width) {
		fglog << "dcimg width mismatch: " << data << " =/= " << width << std::endl;
		return false;
	}

	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_HEIGHT, &data);
	if (failed(err)) {
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_IMAGE_HEIGHT)", err);
		return false;
	}
	else if (data != height) {
		fglog << "dcimg height mismatch: " << data << " =/= " << height << std::endl;
		return false;
	}

	//Options:DCIMG_PIXELTYPE_NONE, DCIMG_PIXELTYPE_MONO8, DCIMG_PIXELTYPE_MONO16
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_PIXELTYPE, &data);
	if (failed(err)) {
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_IMAGE_HEIGHT)", err);
		return false;
	}
	else if (data != DCIMG_PIXELTYPE_MONO16) {
		fglog << "dcimg pixel type mismatch: " << data << " =/= " << DCIMG_PIXELTYPE_MONO16 << std::endl;
		return false;
	}

	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_NUMBEROF_FRAME, &data);
	if (failed(err)) {
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_NUMBEROF_FRAME)", err);
		return false;
	}
	else if (data < nFrames) {
		fglog << "dcimg frame number mismatch: " << data << " =/= " << nFrames << std::endl;
		return false;
	}

	return true;
}


// Raed all time stamps and verify there are no dropped frames
bool check_dcimg_timestamps(HDCIMG hdcimg)
{
	DCIMG_ERR	err;

	// get number of frame
	int32 nFrame;
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_NUMBEROF_FRAME, &nFrame);
	if (failed(err))
	{
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_NUMBEROF_FRAME)", err);
		return false;
	}

	auto timestamps = std::make_unique<DCIMG_TIMESTAMP[]>(nFrame);
	if (timestamps == NULL)
		return false;

	DCIMG_TIMESTAMPBLOCK	block;
	memset(&block, 0, sizeof(block));
	block.hdr.size = sizeof(block);
	block.hdr.iKind = DCIMG_METADATAKIND_TIMESTAMPS;

	block.timestamps = timestamps.get();
	block.timestampmax = nFrame;
	block.timestampsize = sizeof(DCIMG_TIMESTAMP);  //sizeof(*timestamps);

	err = dcimg_copymetadatablock(hdcimg, &block.hdr);
	if (failed(err))
	{
		log_dcimg_error("dcimg_copymetadatablock(DCIMG_TIMESTAMPBLOCK)", err);
		return false;
	}
	else
	{
		//fglog << "TIME CHECK: ";
		//fglog.precision(3);
		if (block.timestampvalidsize < sizeof(DCIMG_TIMESTAMP))
		{
			fglog << "dcimg_copymetadatablock(DCIMG_TIMESTAMPBLOCK) returns unknown time stamp that size is " <<
					block.timestampvalidsize << " bytes.This is smaller than expected." << std::endl;
			return false;
		}
		else
		{
			double firstFrameTime = 0;

			// Detect dropped times by the unusually large time between frames
			for (int i = 1; i < block.timestampcount; i++)
			{
				double current = 1.0e-6 * timestamps[i].microsec + timestamps[i].sec;
				double previous = 1.0e-6 * timestamps[i-1].microsec + timestamps[i-1].sec;
				double frameTime = current - previous;
				//fglog << frameTime << " ";

				if (i == 1)
					firstFrameTime = frameTime;
				else if (abs(frameTime - firstFrameTime) / firstFrameTime > 0.5)
				{
					fglog.precision(2);
					fglog << std::fixed << "Dropped frame? Expected=" << firstFrameTime
						<< " vs " << frameTime << "ms." << std::endl;
					//return false;
				}
			}
		}
		//fglog << std::endl;
	}

	return true;
}