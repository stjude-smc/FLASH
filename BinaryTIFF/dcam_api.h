//Typedefs
enum:uint32_t infoID_t {
	BUS				= 67109121,
	CAMERA_ID		= 67109122,
	VENDOR			= 67109123,
	MODEL			= 67109124,
	CAMERA_VERSION	= 67109125,
	DRIVER_VERSION	= 67109126,
	MODULE_VERSION	= 67109127,
	DCAMAPI_VERSION = 67109128
};

enum:uint32_t parameterID_t {
};

enum:int32_t captureMode_t{
	SNAP			= 1,
	SEQUENCE		= 2,
};

enum:uint32_t triggerMode_t{
	LOW				= 1,
	GLOBAL_EXPOSURE	= 2,
	PROGRAMMABLE	= 3,
	TRIGGER_READY	= 4,
	HIGH			= 5
};

enum:uint32_t polarity_t{
	NEGATIVE		= 1,
	POSITIVE		= 2,
};

//Driver
uint32_t TMCC_INITIALIZE(uint32_t *Count, int32_t *Erval);
uint32_t TMCC_DEINITIALIZE_A(int32_t force);

//Devices
uint32_t TMCC_OPENCAMERA_A(uint32_t i, int32_t force, int32_t *Erval);
uint32_t TMCC_CLOSECAMERA(uint32_t HDCAM, int32_t *Erval);
uint32_t TMCC_GETCAMERAINFO(uint32_t index, infoID_t ID, CStr string, int32_t *Erval);

//Configuration
uint32_t TMCC_GETPARAMETER(uint32_t handle, parameterID_t ID, double *value, int32_t view, int32_t *Erval);
uint32_t TMCC_SETPARAMETER(uint32_t HDCAM, parameterID_t ID, double value, int32_t view, int32_t *Erval);
uint32_t TMCC_GETPROPERTYINFO(uint32_t HDCAM, uint32_t propertyid, uint32_t *text, double *max, double *min, double *step, double *default, uint32_t *writable, int32_t *Erval);
uint32_t TMCC_GETPROPERTYTEXT(uint32_t HDCAM, uint32_t propertyid, CStr name, double value, double *nextvalue, int32_t *Erval);
uint32_t TMCC_GETNEXTPROPERTY(uint32_t HDCAM, CStr name, int32_t *prop, int32_t *Erval);

//Acquisition
uint32_t TMCC_SETAREA(uint32_t HDCAM, int32_t sho, int32_t svo, int32_t shw, int32_t svw, int32_t *Erval);
uint32_t TMCC_GETAREA_A(uint32_t index, int32_t *hov, int32_t *hon, int32_t *hox, int32_t *hoi, int32_t *vov, int32_t *von, int32_t *vox, int32_t *voi, int32_t *hwv, int32_t *hwn, int32_t *hwx, int32_t *hwi, int32_t *vwv, int32_t *vwn, int32_t *vwx, int32_t *vwi, int32_t *Erval);
uint32_t TMCC_PREPARECAPTURE(uint32_t index, captureMode_t mode, int32_t frames, uint32_t *width, uint32_t *height, uint32_t *datatype, int32_t *Erval);
uint32_t TMCC_UNPREPARECAPTURE(uint32_t index, int32_t *Erval);

//Status
uint32_t TMCC_STARTCAPTURE_B(uint32_t index, int32_t *Erval);
uint32_t TMCC_STOPCAPTURE(uint32_t index, int32_t *Erval);
uint32_t TMCC_WAITNEXTFRAME(uint32_t HDCAM, int32_t timeout, int32_t *frameindex, int32_t *framecount, int32_t *Erval);
uint32_t TMCC_GETCAPTUREINFO(uint32_t HDCAM, int32_t *frameindex, int32_t *framecount, int32_t *Erval);
uint32_t TMCC_GETFRAME(uint32_t index, int32_t frameindex, uint16_t *imgarray, int32_t *Erval);
uint32_t TMCC_GETELECTRONINFO(uint32_t index, uint32_t frame, float *C, float *O, int32_t *Erval);

//Triggering
uint32_t TMCC_SETINPUTTRIGGER(uint32_t HDCAM, triggerMode_t mode, polarity_t polarity, uint32_t times, double delay, int32_t *Erval);
uint32_t TMCC_SETOUTPUTTRIGGER(uint32_t HDCAM, triggerMode_t mode, polarity_t polarity, double period, double delay, int32_t index, int32_t *Erval);	
uint32_t TMCC_FIRETRIGGER(uint32_t HDCAM, int32_t *Erval);

//HD Recorder
uint32_t TMCC_STARTRECORDER(uint32_t handle, const CStr path, int32_t frames, int32_t *Erval);
uint32_t TMCC_STOPRECORDER(uint32_t handle, int32_t *Erval);
uint32_t TMCC_GETRECORDERSTATUS(uint32_t handle, int32_t *index, int32_t *count, int32_t *Erval);

//DCIMG File Handling
uint32_t TMCC_OPENDCIMGFILE(uint32_t *filehandle, CStr path, int32_t *totalframes, int32_t *Erval);
uint32_t TMCC_GETDCIMGFRAMEINFO(uint32_t index, int32_t frameindex, int32_t *width, int32_t *height, int32_t *Erval);
uint32_t TMCC_GETDCIMGFRAMEDATA_A(uint32_t index, int32_t frame, uint16_t *imgarray, double *timestamp, int32_t *Erval);
uint32_t TMCC_CLOSEDCIMGFILE(uint32_t filehandle, int32_t *Erval);

//Advanced
uint32_t TMCC_GETPROPERTYVALUE(uint32_t HDCAM, uint32_t propertyid, float *value, int32_t *Erval);
uint32_t TMCC_SETPROPERTYVALUE(uint32_t HDCAM, uint32_t propertyid, float value, int32_t *Erval);

//DSP

//Error Handling
uint32_t TMCC_REPORTERROR(int32_t error, CStr Erval);