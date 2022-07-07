// dcimg2tiff.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

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
	char* dcimgPath3, char* dcimgPath4)		// Layout:	(ch3right == 0)		(ch3right == 0)
											// 			dcimg1	dcimg2		dcimg1	dcimg2
											//		   	dcimg3 (dcimg4)	   (dcimg4) dcimg3
{
	// initializations
	resetCurrentFrame();
	canceled = 0;
	uint32_t jFlip, kFlip;
	uint32_t i, j, k, m;


	// open raw data (DCIMG) files and TIFF file
	FILE* dcimgFile1 = fopen(dcimgPath1, "rb");

	FILE* dcimgFile2 = nullptr;
	if (nChannels>1) dcimgFile2 = fopen(dcimgPath2, "rb");

	FILE* dcimgFile3 = nullptr;
	if (nChannels>2) dcimgFile3 = fopen(dcimgPath3, "rb");

	FILE* dcimgFile4 = nullptr;
	if (nChannels>3) dcimgFile4 = fopen(dcimgPath4, "rb");

	FILE* tiffFile = fopen(tiffPath, "rb+");


	// set file positions to specified offsets
	_fseeki64(tiffFile, tiffOffset, SEEK_SET);

	_fseeki64(dcimgFile1, DCIMG_OFFSET, SEEK_SET);

	if (nChannels>1) _fseeki64(dcimgFile2, DCIMG_OFFSET, SEEK_SET);
	if (nChannels>2) _fseeki64(dcimgFile3, DCIMG_OFFSET, SEEK_SET);
	if (nChannels>3) _fseeki64(dcimgFile4, DCIMG_OFFSET, SEEK_SET);


	// allocate frame buffers for reading and writing
	uint32_t frameLength = sizeof(uint16_t) * frameWidth * frameHeight;
	uint32_t movieWidth = (nChannels > 1) ? 2 : 1;

	uint16_t* readBuffer1;
	readBuffer1 = (uint16_t*)malloc(frameLength);

	uint16_t* readBuffer2 = nullptr;
	if (nChannels>1) readBuffer2 = (uint16_t*)malloc(frameLength);

	uint16_t* readBuffer3 = nullptr;
	uint16_t* readBuffer4 = nullptr;
	if (nChannels>2) {
		readBuffer3 = (uint16_t*)malloc(frameLength);
		readBuffer4 = (uint16_t*)malloc(frameLength);
	}

	uint16_t* writeBuffer1;
	uint16_t* writeBuffer2 = nullptr;
	writeBuffer1 = (uint16_t*)malloc(movieWidth*frameLength);
	if (nChannels>2) writeBuffer2 = (uint16_t*)malloc(2 * frameLength);

	// additional small buffer for frame "gaps" containing missing pixels
	uint16_t* readBufferExt;
	readBufferExt = (uint16_t*)malloc(FRAME_OFFSET);

	// skip frames if required
	if (skipFrames > 0) {
		for (i = 0; i<skipFrames; i++) {
			fread(readBuffer1, 1, frameLength, dcimgFile1);
			fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile1);

			if (nChannels > 1) {
				fread(readBuffer2, 1, frameLength, dcimgFile2);
				fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile2);
			}
			if (nChannels > 2) {
				fread(readBuffer3, 1, frameLength, dcimgFile3);
				fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile3);
			}
			if (nChannels > 3) {
				fread(readBuffer4, 1, frameLength, dcimgFile4);
				fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile4);
			}


			currentFrame++;
		}
	}


	for (i = 0; i<nFrames; i++) {	// loop over frames
		if (canceled) break; // abort if cancelConversion() was executed

		//read single frame from each raw file, taking into account gap between frames
		fread(readBuffer1, 1, frameLength, dcimgFile1);
		fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile1);
		// filling in of missing pixels doesn't work for some reason
		//for (m = 0; m < (2*MISSING_PIXELS); m++) readBuffer1[frameWidth*frameHeight + m] = readBufferExt[MISSING_OFFSET + m];

		if (nChannels > 1)
		{
			fread(readBuffer2, 1, frameLength, dcimgFile2);
			fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile2);
			//for (m = 0; m < (2*MISSING_PIXELS); m++) readBuffer2[frameWidth*frameHeight + m] = readBufferExt[MISSING_OFFSET + m];
		}

		if (nChannels > 2)
		{
			fread(readBuffer3, 1, frameLength, dcimgFile3);
			fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile3);
			//for (m = 0; m < (2*MISSING_PIXELS); m++) readBuffer3[frameWidth*frameHeight + m] = readBufferExt[MISSING_OFFSET + m];
		}

		if (nChannels > 3)
		{
			fread(readBuffer4, 1, frameLength, dcimgFile4);
			fread(readBufferExt, 1, FRAME_OFFSET, dcimgFile4);
			//for (m = 0; m < (2*MISSING_PIXELS); m++) readBuffer4[frameWidth*frameHeight + m] = readBufferExt[MISSING_OFFSET + m];
		}

		// interleave and arrange the channels (row by row)
		for (j = 0; j<frameHeight; j++) {		// loop over rows
			jFlip = frameHeight - (j + 1); 	// replaces j index if vertical flip is active
			for (k = 0; k<frameWidth; k++) {	// loop over columns
				kFlip = frameWidth - (k + 1);	// replaces k index if horizontal flip is active

												// write channels 1 and 2 to buffer
				if (nChannels>1) { // two or more channels
					writeBuffer1[(2 * j)*frameWidth + k] = readBuffer1[(vFlip[0] ? jFlip : j)*frameWidth + (hFlip[0] ? kFlip : k)];
					writeBuffer1[(2 * j + 1)*frameWidth + k] = readBuffer2[(vFlip[1] ? jFlip : j)*frameWidth + (hFlip[1] ? kFlip : k)];
				}
				else { // single channel
					writeBuffer1[j*frameWidth + k] = readBuffer1[(vFlip[0] ? jFlip : j)*frameWidth + (hFlip[0] ? kFlip : k)];
				}

				// write additional channels if nChannels>2
				if (nChannels>2) {
					if (ch3right) { // channel 3 position: bottom right
						writeBuffer2[(2 * j)*frameWidth + k] = (nChannels>3) ? (readBuffer4[(vFlip[3] ? jFlip : j)*frameWidth + (hFlip[3] ? kFlip : k)]) : 0;
						writeBuffer2[(2 * j + 1)*frameWidth + k] = readBuffer3[(vFlip[2] ? jFlip : j)*frameWidth + (hFlip[2] ? kFlip : k)];
					}
					else { // channel 3 position: bottom left
						writeBuffer2[(2 * j)*frameWidth + k] = readBuffer3[(vFlip[2] ? jFlip : j)*frameWidth + (hFlip[2] ? kFlip : k)];
						writeBuffer2[(2 * j + 1)*frameWidth + k] = (nChannels>3) ? (readBuffer4[(vFlip[3] ? jFlip : j)*frameWidth + (hFlip[3] ? kFlip : k)]) : 0;
					}
				}
			}
		}

		// write frame to output file
		fwrite(writeBuffer1, 1, movieWidth*frameLength, tiffFile);
		if (nChannels>2) fwrite(writeBuffer2, 1, 2 * frameLength, tiffFile);

		currentFrame++;			// This can be queried with getCurrentFrame()

								// Note:
								// Initializing currentFrame to -1 (see above) leaves it at nFrames-1
								// after the last frame has been processed. Only after all files have
								// been closed (see below), it is incremented to nFrames, which tells 
								// a caller function monitoring progress via getCurrentFrame() that the
								// conversion has been completed.
	}

	// clean up
	free(writeBuffer1);
	if (nChannels>2) free(writeBuffer2);

	fclose(dcimgFile1);
	if (nChannels>1) fclose(dcimgFile2);
	if (nChannels>2) fclose(dcimgFile3);
	if (nChannels>3) fclose(dcimgFile4);

	free(readBuffer1);
	if (nChannels>1) free(readBuffer2);
	if (nChannels>2) {
		free(readBuffer3);
		free(readBuffer4);
	}
	free(readBufferExt);

	fclose(tiffFile);

	currentFrame++; // final increment to tell caller that conversion is done

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
