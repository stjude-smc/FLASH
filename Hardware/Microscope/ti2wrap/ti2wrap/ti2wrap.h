// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TI2WRAP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TI2WRAP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TI2WRAP_EXPORTS
#define TI2WRAP_API __declspec(dllexport)
#else
#define TI2WRAP_API __declspec(dllimport)
#endif

// Open device connection
EXTERN TI2WRAP_API int32_t ti2_open();

// Close device connection if open
EXTERN TI2WRAP_API int32_t ti2_close();

// Get filter cube position
EXTERN TI2WRAP_API int32_t ti2_getFilter(int32_t* filterPos);

// Set filter cube position
EXTERN TI2WRAP_API int32_t ti2_setFilter(const int32_t filterPos);

// Get Z position
EXTERN TI2WRAP_API int32_t ti2_getZPos(double* zPos_um);

// Set Z position
EXTERN TI2WRAP_API int32_t ti2_setZPos(const double zPos_um, const uint32_t speed, const uint32_t tolerance);

// Escape Z  // [ 0:Normal(Refocus) , 1:Escape ]
EXTERN TI2WRAP_API int32_t ti2_setZEsc(const int32_t setting);
EXTERN TI2WRAP_API int32_t ti2_getZEsc(int32_t* setting);

// Get custom Z speed
EXTERN TI2WRAP_API int32_t ti2_getCustomZSpeed(const int32_t tableNumber, int32_t pUserOutParam[7]);

// Set custom Z speed
EXTERN TI2WRAP_API int32_t ti2_setCustomZSpeed();

// Get light path
EXTERN TI2WRAP_API int32_t ti2_getLightPath(int32_t* lightPath);

// Set light path
EXTERN TI2WRAP_API int32_t ti2_setLightPath(const int32_t lightPath);

// Get X position
EXTERN TI2WRAP_API int32_t ti2_getXPos(double* xPos_um);

// Set X position
EXTERN TI2WRAP_API int32_t ti2_setXPos(const double xPos_um, const int32_t speed);

// Get Y position
EXTERN TI2WRAP_API int32_t ti2_getYPos(double* yPos_um);

// Set Y position
EXTERN TI2WRAP_API int32_t ti2_setYPos(const double yPos_um, const int32_t speed);

// Enable/disable joystick (0=disable, 1=enable)
EXTERN TI2WRAP_API int32_t ti2_setEnableJoystick(const int32_t data);