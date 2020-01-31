/* Replace "dll.h" with the name of your header */
#include "dll.h"

DLLIMPORT void histogram(uint16_t* frame, uint64_t size, uint16_t histMin, uint16_t histMax, uint16_t nBins, uint64_t* hist)
{
	uint16_t i, bin;
	double binWidth = ((double)histMax - (double)histMin)/((double)nBins);
	
	for(i=0; i<nBins; i++) {
		hist[i] = 0;
	}
	
	for(i=0; i<size; i++) {
		if(frame[i] > histMax) frame[i] = histMax;
		bin = (uint16_t)((frame[i] - histMin) / binWidth);
		hist[bin]++;
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	
	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}
