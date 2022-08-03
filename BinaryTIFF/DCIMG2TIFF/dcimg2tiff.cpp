// dcimg2tiff.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <time.h>


#define LOGFILE "C:\\temp\\dcimg2tiff.log"


// definitions for hamamatsu functions
BOOL get_image_information(HDCIMG hdcimg, int32& width, int32& height, int32& rowbytes, int32& pixeltype);
HDCIMG dcimgcon_init_open(const char* filename);


// Global variables
uint64_t currentFrame;
uint8_t canceled;

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

// cancel dcimg2tiff() operation
extern "C" void cancelConversion()
{
	canceled = 1;
}


void dcimgcon_show_dcimgerr(DCIMG_ERR errid, const char* apiname, const char *fmt=0, ...)
{
	FILE *fp = fopen(LOGFILE, "a");

	fprintf(fp, "FAILED: (DCIMG_ERR)0x%08x @ %s", errid, apiname);

	if (fmt != NULL)
	{
		fprintf(fp," : ");

		va_list arg;
		va_start(arg, fmt);
		vfprintf(fp, fmt, arg);
		va_end(arg);
	}

	fprintf(fp, "\n");
	fclose(fp);
	return;
}

void debugPrintf(const char *fmt, ...)
{
	va_list arg;
	FILE *fp = fopen(LOGFILE, "a");

	va_start(arg, fmt);
	vfprintf(fp, fmt, arg);
	va_end(arg);

	fprintf(fp, "\n");
	fclose(fp);
	return;
}




// dcimg2tiff combines the data from 1 to 4 Hamamatsu HD recorder files (DCIMG raw image format)
// into the data block of a BigTIFF file whose header and IFDs have been created 
// using the BinaryTIFF LabView project. Unsigned 16bit files only.
//
extern "C" uint32_t dcimg2tiff(
	char* tiffPath, 						// TIFF file (prepared by BinaryTIFF.lvproj)
	uint32_t _frameWidth,					// frame width in pixels (single channel)
	uint32_t _frameHeight, 					// frame height in pixels (single channel)
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
	// initializations
	remove(LOGFILE);
	//clock_t startTime = clock();
	//debugPrintf("START: %s", dcimgPath1);
	resetCurrentFrame();
	canceled = 0;
	uint32_t jFlip, kFlip;
	uint32_t i, j, k; // , m;


	// Load DCIMG files using the Hamamatsu API
	DCIMG_ERR	err;
	HDCIMG hdcimg[4];

	//for( i=0; i<nChannels; ++i )
	//	dcimgcon_init_open(dcimgPath[i]);

	hdcimg[0] = dcimgcon_init_open(dcimgPath1);
	if (nChannels > 1) hdcimg[1] = dcimgcon_init_open(dcimgPath2);
	if (nChannels > 2) hdcimg[2] = dcimgcon_init_open(dcimgPath3);
	if (nChannels > 3) hdcimg[3] = dcimgcon_init_open(dcimgPath4);

	if (hdcimg[0] == NULL)
		cancelConversion();


	FILE* tiffFile = fopen(tiffPath, "rb+");
	// set file positions to specified offsets
	_fseeki64(tiffFile, tiffOffset, SEEK_SET);


	// Prepare API frame buffer

	// Verify input arguments match the dcimg file
	// FIXME: we assume these parameters all match (they should)
	int32 frameWidth, frameHeight, rowbytes, pixeltype;
	if ( !get_image_information(hdcimg[0], frameWidth, frameHeight, rowbytes, pixeltype) )
		cancelConversion();

	DCIMG_FRAME	frame;
	memset(&frame, 0, sizeof(frame));
	frame.size = sizeof(frame);
	//frame.width = frameWidth;
	//frame.height = frameHeight;
	//frame.rowbytes = rowbytes;  //seems these are only used by dcimg_copyframe


	// allocate frame buffers for reading and writing
	uint32_t pxSize = sizeof(uint16_t);
	uint32_t frameLength = pxSize * frameWidth * frameHeight;
	uint32_t movieWidth = (nChannels > 1) ? 2 : 1;
	uint32_t movieHeight = (nChannels > 2) ? 2 : 1;

	uint16_t* newbuf;  //pointer to dcimg_api frame buffer.
	uint16_t* writeBuffer = (uint16_t*)malloc(movieWidth*movieHeight*frameLength);


	for (i=skipFrames; i<nFrames+skipFrames; ++i)  // loop over frames
	{
		if (canceled) break; // abort if cancelConversion() was executed
		frame.iFrame = i;


		// Special case: write directly to disk if single channel and no flipping (very fast)
		if (nChannels == 1 && !hFlip[0] && !vFlip[0]) {
			//debugPrintf("Writing frame #%d: %d, %dx%d (%d bytes).", i, frame.size, width, height, rowbytes);
			err = dcimg_lockframe(hdcimg[0], &frame);
			if (failed(err)) {
				cancelConversion();
				dcimgcon_show_dcimgerr(err, "dcimg_copyframe()", "frame #%d", i);
				break;
			}
			fwrite(frame.buf, 1, frameLength, tiffFile);
			currentFrame = i - 1;
			continue;
		}


		// Copy frame data from dcimg file to TIFF frame.
		for (uint32_t ch=0; ch<nChannels; ++ch)
		{
			// Read frame data from DCIMG file via API
			//debugPrintf("Writing frame #%d, ch %d: %d, %dx%d (%d bytes).", i, ch, frame.size, width, height, rowbytes);
			err = dcimg_lockframe(hdcimg[ch], &frame);
			if (failed(err))  {
				cancelConversion();
				dcimgcon_show_dcimgerr(err, "dcimg_lockframe()", "frame #%d", i);
				break;
			}
			newbuf = (uint16_t*)frame.buf;
			uint32_t movieCol = ch % 2;
			uint32_t rowOffset = (ch >= 2) * movieWidth * frameWidth * frameHeight;

			// place 3rd channel on the right side of the bottom row.
			if (ch>=2 && ch3right)  movieCol = !movieCol;

			// Fill in zeros for missing field with 3-color.
			if (ch==2 && nChannels == 3)
				memset( &writeBuffer[rowOffset], 0, frameLength );

			// No horizontal flipping: copy row by row
			if (!hFlip[ch] && !vFlip[ch]) {
				for (j = 0; j < frameHeight; j++) {		// loop over rows
					memcpy(&writeBuffer[(movieWidth * j + movieCol)*frameWidth + rowOffset], &newbuf[j*frameWidth], pxSize*frameWidth);
				}
			}
			else if (!hFlip[ch] && vFlip[ch]) {
				for (j = 0; j < frameHeight; j++) {		// loop over rows
					jFlip = frameHeight - (j + 1); 	// replaces j index if vertical flip is active
					memcpy(&writeBuffer[(movieWidth * j + movieCol)*frameWidth + rowOffset], &newbuf[jFlip*frameWidth], pxSize*frameWidth);
				}
			}

			// General case: horizontal flipping requires byte-by-byte copying.
			else if (hFlip[ch]) {
				for (j = 0; j < frameHeight; j++) {		// loop over rows
					jFlip = (vFlip[ch] ? frameHeight - (j + 1) : j); 	// replaces j index if vertical flip is active
					for (k = 0; k < frameWidth; k++) {	// loop over columns
						kFlip = frameWidth - (k + 1);	// replaces k index if horizontal flip is active
						writeBuffer[(movieWidth * j + movieCol)*frameWidth + rowOffset + k] = newbuf[jFlip*frameWidth + kFlip];
					}
				}
			}
		} //for each dcimg file

		// write frame to output file
		fwrite(writeBuffer, 1, movieWidth*movieHeight*frameLength, tiffFile);

		currentFrame=i-1;			// This can be queried with getCurrentFrame()

								// Note:
								// Initializing currentFrame to -1 (see above) leaves it at nFrames-1
								// after the last frame has been processed. Only after all files have
								// been closed (see below), it is incremented to nFrames, which tells 
								// a caller function monitoring progress via getCurrentFrame() that the
								// conversion has been completed.
	}

	// clean up
	free(writeBuffer);
	for (uint32_t ch=0; ch<nChannels; ++ch)
		dcimg_close(hdcimg[ch]);
	fclose(tiffFile);

	currentFrame++; // final increment to tell caller that conversion is done
	//debugPrintf("FINISHED after %.1f seconds.", ((double)(clock()-startTime))/CLOCKS_PER_SEC );

	if (canceled) return -1;
	else return 0;
}


// write dummy binary file for testing
extern "C" void writeDummyFile(char* filePath, int fileLengthGB)
{
	uint8_t* writeBuffer;
	int sizeGB = 1024 * 1024 * 1024;
	int i;

	writeBuffer = (uint8_t *) malloc(sizeGB);
	for (i = 0; i<sizeGB; i++) {
		writeBuffer[i] = 42;
	}

	FILE* dummyFile = fopen(filePath, "wb");
	for (i = 0; i<fileLengthGB; i++) {
		fwrite(writeBuffer, 1, sizeGB, dummyFile);
	}
	fclose(dummyFile);
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
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_init()");
		return NULL;
	}

	// open DCIMG file
	DCIMG_OPEN	openparam;
	memset(&openparam, 0, sizeof(openparam));
	openparam.size = sizeof(openparam);
	openparam.path = filename;

	err = dcimg_open(&openparam);
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_open", "file name is %s", filename);
		return NULL;
	}

	return openparam.hdcimg;
}



/** This is copied from access_recorded_image.cpp
 @brief	get image information
 @param	hdcimg:		DCIMG handle
 @param width:		image width
 @param height:		image height
 @param rowbytes:	image rowbytes
 @param pixeltype:	DCIMG_PIXELTYPE value
 @return	result to get information
 */
BOOL get_image_information(HDCIMG hdcimg, int32& width, int32& height, int32& rowbytes, int32& pixeltype)
{
	DCIMG_ERR err;

	int32 nWidth;
	// get width
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_WIDTH, &nWidth);
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_getparaml(DCIMG_IDPARAML_IMAGE_WIDTH)");
		return FALSE;
	}

	int32 nHeight;
	// get height
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_HEIGHT, &nHeight);
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_getparaml(DCIMG_IDPARAML_IMAGE_HEIGHT)");
		return FALSE;
	}

	int32 nRowbytes;
	// get row bytes
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_ROWBYTES, &nRowbytes);
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_getparaml(DCIMG_IDPARAML_IMAGE_ROWBYTES)");
		return FALSE;
	}

	int32 nPixeltype;
	// get pixel type
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_IMAGE_PIXELTYPE, &nPixeltype);
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_getparaml(DCIMG_IDPARAML_IMAGE_PIXELTYPE)");
		return FALSE;
	}

	/* DCIMG_IDPARAML_NUMBEROF_TOTALFRAME = all sessions?
	int32 input_nFrames;
	// get pixel type
	err = dcimg_getparaml(hdcimg, DCIMG_IDPARAML_NUMBEROF_FRAME, &input_nFrames);
	if (failed(err))
	{
		dcimgcon_show_dcimgerr(err, "dcimg_getparaml(DCIMG_IDPARAML_NUMBEROF_FRAME)");
		return FALSE;
	}
	*/

	width = nWidth;
	height = nHeight;
	rowbytes = nRowbytes;
	pixeltype = nPixeltype;
	//nFrames = input_nFrames;

	return TRUE;
}