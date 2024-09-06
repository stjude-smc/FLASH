#pragma once

#include "stdafx.h"
#include "dcimg2tiff.h"
#include <sstream>



class DcimgFile
{
public:
	DcimgFile(char* filename)
	{
		DCIMG_ERR	err;

		// initialize DCIMG-API
		DCIMG_INIT	initparam;
		memset(&initparam, 0, sizeof(initparam));
		initparam.size = sizeof(initparam);

		err = dcimg_init(&initparam);
		log_dcimg_error("dcimg_init", err);

		// open DCIMG file, retrying a few times in case file is still saving.
		DCIMG_OPEN	openparam;
		memset(&openparam, 0, sizeof(openparam));
		openparam.size = sizeof(openparam);
		openparam.path = filename;

		err = dcimg_open(&openparam);
		log_dcimg_error("dcimg_open", err);

		hdcimg = openparam.hdcimg;

		memset(&frame, 0, sizeof(frame));
		frame.size = sizeof(frame);
	}

	~DcimgFile()
	{
		if (hdcimg)
			dcimg_close(hdcimg);
	}

	// Prevent copying file handles to avoid double close
	// Copy constructor is required to compile but unclear why
	//DcimgFile(const DcimgFile&) = delete;
	//DcimgFile& operator= (const DcimgFile&) = delete;


	// Read a single frame from DCIMG file
	uint16_t* read_frame(const int i)
	{
		frame.iFrame = i;
		DCIMG_ERR err = dcimg_lockframe(hdcimg, &frame);
		log_dcimg_error("dcimg_lockframe", err);

		return static_cast<uint16_t*>(frame.buf);
	}


	// Throws runtime_error if dcimg metadata does not matche inputs
	void dcimg_is_valid(int32 width, int32 height, int32 nFrames)
	{
		DCIMG_ERR err;
		int32 data;

		err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_WIDTH, &data);
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_IMAGE_WIDTH)", err);

		if (data != width) {
			throw std::runtime_error("dcimg width mismatch");
		}

		err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_HEIGHT, &data);
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_IMAGE_HEIGHT)", err);

		if (data != height) {
			throw std::runtime_error("dcimg image height mismatch");
		}

		//Options:DCIMG_PIXELTYPE_NONE, DCIMG_PIXELTYPE_MONO8, DCIMG_PIXELTYPE_MONO16
		err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_PIXELTYPE, &data);
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_IMAGE_HEIGHT)", err);

		if (data != DCIMG_PIXELTYPE_MONO16) {
			throw std::runtime_error("dcimg pixel type mismatch");
		}

		err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_NUMBEROF_FRAME, &data);
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_NUMBEROF_FRAME)", err);

		if (data < nFrames) {
			throw std::runtime_error("dcimg frame number mismatch");
		}
	}


	// Raed all time stamps and verify there are no dropped frames
	bool check_dcimg_timestamps()
	{
		DCIMG_ERR	err;

		// get number of frame
		int32 nFrame;
		err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_NUMBEROF_FRAME, &nFrame);
		log_dcimg_error("dcimg_getparaml(DCIMG_IDPARAML_NUMBEROF_FRAME)", err);

		auto timestamps = std::make_unique<DCIMG_TIMESTAMP[]>(nFrame);
		if (timestamps == NULL)
			throw std::bad_alloc();

		DCIMG_TIMESTAMPBLOCK	block;
		memset(&block, 0, sizeof(block));
		block.hdr.size = sizeof(block);
		block.hdr.iKind = DCIMG_METADATAKIND_TIMESTAMPS;

		block.timestamps = timestamps.get();
		block.timestampmax = nFrame;
		block.timestampsize = sizeof(DCIMG_TIMESTAMP);  //sizeof(*timestamps);

		err = dcimg_copymetadatablock(hdcimg, &block.hdr);
		log_dcimg_error("dcimg_copymetadatablock(DCIMG_TIMESTAMPBLOCK)", err);

		if (block.timestampvalidsize < sizeof(DCIMG_TIMESTAMP))  {
			throw std::runtime_error("dcimg_copymetadatablock returned time stamp smaller than expected.");
		}

		double firstFrameTime = 0;
		bool droppedFrames = false;

		// Detect dropped times by the unusually large time between frames
		for (int i = 1; i < block.timestampcount; i++)
		{
			double current = 1.0e-6 * timestamps[i].microsec + timestamps[i].sec;
			double previous = 1.0e-6 * timestamps[i - 1].microsec + timestamps[i - 1].sec;
			double frameTime = current - previous;

			if (i == 1)
				firstFrameTime = frameTime;
			else if (abs(frameTime - firstFrameTime) / firstFrameTime > 0.5)
			{
				std::cerr << std::fixed << "Dropped frame? Expected=" << firstFrameTime
					<< " vs " << frameTime << "ms." << std::endl;
				droppedFrames = true;
			}
		}

		return droppedFrames;
	}



protected:
	HDCIMG hdcimg = nullptr;  //dcimgapi file handle
	DCIMG_FRAME	frame;


	// Convert dcimgapi errors, if any, to C++ exception
	void log_dcimg_error(const char* fcn, DCIMG_ERR err)
	{
		if (failed(err))
		{
			std::stringstream ss;
			ss << "dcimg error 0x" << std::hex << err << std::dec << " in " << fcn;
			throw std::runtime_error(ss.str());
		}
	}
};




