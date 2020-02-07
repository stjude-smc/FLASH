// ti2wrap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ti2wrap.h"

// Open device connection
EXTERN TI2WRAP_API int32_t ti2_open() {
	const lx_int32 iDeviceIndex = 0;
	lx_uint64 uiConnectedAccessoryMask = 0;
	const lx_uint32 uiErrMsgMaxSize = 255;
	lx_wchar pwszErrMsg[256] = { 0 };
		
	lx_result err = MIC_Open(iDeviceIndex, uiConnectedAccessoryMask, uiErrMsgMaxSize, pwszErrMsg);
	return (int32_t)err;
}

// Close device connection if open
EXTERN TI2WRAP_API int32_t ti2_close() {
	lx_result err = MIC_Close();
	return (int32_t)err;
}

// Get filter cube position
EXTERN TI2WRAP_API int32_t ti2_getFilter(int32_t* filterPos) {
	MIC_Data sData;
	sData.uiDataUsageMask = MIC_DATA_MASK_TURRET1POS;

	lx_result err = MIC_DataGet(sData);
	*filterPos = (int32_t)sData.iTURRET1POS;
	return (int32_t)err;
}

// Set filter cube position
EXTERN TI2WRAP_API int32_t ti2_setFilter(const int32_t filterPos) {
	MIC_Data sDataIn, sDataOut;
	sDataIn.uiDataUsageMask = MIC_DATA_MASK_TURRET1POS;
	sDataIn.iTURRET1POS = (lx_int32)filterPos;
		
	lx_result err = MIC_DataSet(sDataIn, sDataOut, TRUE);
	return (int32_t)err;
}

// Get Z position
EXTERN TI2WRAP_API int32_t ti2_getZPos(double* zPos_um) {
	MIC_Data sData;
	sData.uiDataUsageMask = MIC_DATA_MASK_ZPOSITION;

	lx_result err = MIC_DataGet(sData);
	if (err) return (int32_t)err;

	err = MIC_Convert_Dev2Phys(sData.uiDataUsageMask, sData.iZPOSITION, *zPos_um);
	return (int32_t)err;
}

// Set Z position
EXTERN TI2WRAP_API int32_t ti2_setZPos(const double zPos_um) {
	MIC_Data sDataIn, sDataOut;
	sDataIn.uiDataUsageMask = MIC_DATA_MASK_ZPOSITION;
	lx_int32 iDevVal;

	lx_result err = MIC_Convert_Phys2Dev(sDataIn.uiDataUsageMask, zPos_um, iDevVal);
	if (err) return (int32_t)err;

	sDataIn.iZPOSITION = iDevVal;
	sDataIn.iZPOSITIONSpeed = 1;
	sDataIn.iZPOSITIONTolerance = 0;

	err = MIC_DataSet(sDataIn, sDataOut, TRUE);
	return (int32_t)err;
}

// Get custom Z speed
EXTERN TI2WRAP_API int32_t ti2_getCustomZSpeed(const int32_t tableNumber, int32_t pUserOutParam[7]) {
	MIC_Command sCommand;
	const wchar_t wszCommandString[] = TI2_DEDICATED_GET_Z_CUSTOM_SPEED;
	memcpy(&(sCommand.wszCommandString), wszCommandString, strlen((char*)wszCommandString) + 1);
	lx_int32 pCommandData[] = { tableNumber };
	sCommand.pCommandData = pCommandData;
	lx_result err = MIC_DedicatedCommand(sCommand, pUserOutParam);
	return (int32_t)err;
}

// Set custom Z speed
EXTERN TI2WRAP_API int32_t ti2_setCustomZSpeed() {
	MIC_Command sCommand;
	const wchar_t wszCommandString[] = TI2_DEDICATED_SET_Z_CUSTOM_SPEED;
	memcpy(&(sCommand.wszCommandString), wszCommandString, strlen((char*)wszCommandString) + 1);
	lx_int32 pCommandData[7] = {0, 0, 0, 0, 0, 0, 0};
	sCommand.pCommandData = pCommandData;
	lx_result err = MIC_DedicatedCommand(sCommand, NULL);
	return (int32_t)err;
}

// Get light path
EXTERN TI2WRAP_API int32_t ti2_getLightPath(int32_t* lightPath) {
	MIC_Data sData;
	sData.uiDataUsageMask = MIC_DATA_MASK_LIGHTPATH;

	lx_result err = MIC_DataGet(sData);
	*lightPath = (int32_t)sData.iLIGHTPATH;
	return (int32_t)err;
}

// Set light path
EXTERN TI2WRAP_API int32_t ti2_setLightPath(const int32_t lightPath) {
	MIC_Data sDataIn, sDataOut;
	sDataIn.uiDataUsageMask = MIC_DATA_MASK_LIGHTPATH;
	sDataIn.iLIGHTPATH = (lx_int32)lightPath;

	lx_result err = MIC_DataSet(sDataIn, sDataOut, TRUE);
	return (int32_t)err;
}

// Get X position
EXTERN TI2WRAP_API int32_t ti2_getXPos(double* xPos_um) {
	MIC_Data sDataX;
	sDataX.uiDataUsageMask = MIC_DATA_MASK_XPOSITION;

	lx_result err = MIC_DataGet(sDataX);
	if (err) return (int32_t)err;
	err = MIC_Convert_Dev2Phys(sDataX.uiDataUsageMask, sDataX.iXPOSITION, *xPos_um);

	return (int32_t)err;
}


// Set X position
EXTERN TI2WRAP_API int32_t ti2_setXPos(const double xPos_um, const int32_t speed) {
	MIC_Data sDataInX, sDataOutX;
	sDataInX.uiDataUsageMask = MIC_DATA_MASK_XPOSITION;
	lx_int32 iDevValX;

	lx_result err = MIC_Convert_Phys2Dev(sDataInX.uiDataUsageMask, xPos_um, iDevValX);
	if (err) return (int32_t)err;
	sDataInX.iXPOSITION = iDevValX;
	sDataInX.iXPOSITIONSpeed = speed;
	sDataInX.iXPOSITIONTolerance = 0;

	err = MIC_DataSet(sDataInX, sDataOutX, TRUE);

	return (int32_t)err;
}

// Get Y position
EXTERN TI2WRAP_API int32_t ti2_getYPos(double* yPos_um) {
	MIC_Data sDataY;
	sDataY.uiDataUsageMask = MIC_DATA_MASK_YPOSITION;

	lx_result err = MIC_DataGet(sDataY);
	if (err) return (int32_t)err;
	err = MIC_Convert_Dev2Phys(sDataY.uiDataUsageMask, sDataY.iYPOSITION, *yPos_um);
	
	return (int32_t)err;
}


// Set Y position
EXTERN TI2WRAP_API int32_t ti2_setYPos(const double yPos_um, const int32_t speed) {
	MIC_Data sDataInY, sDataOutY;
	sDataInY.uiDataUsageMask = MIC_DATA_MASK_YPOSITION;
	lx_int32 iDevValY;

	lx_result err = MIC_Convert_Phys2Dev(sDataInY.uiDataUsageMask, yPos_um, iDevValY);
	if (err) return (int32_t)err;
	sDataInY.iYPOSITION = iDevValY;
	sDataInY.iYPOSITIONSpeed = speed;
	sDataInY.iYPOSITIONTolerance = 0;

	err = MIC_DataSet(sDataInY, sDataOutY, TRUE);
	return (int32_t)err;
}

