
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef WIN32

#include <windows.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <fstream.h>

void SetRandomSeed()
{
	LARGE_INTEGER la;
	QueryPerformanceCounter( &la );
	srand( (unsigned int)la.QuadPart );
}

void ShowMessage(char* mess, char* title)
{
	MessageBox(MainWnd, mess, title, MB_OK);
}

LARGE_INTEGER lc_time_cur;
LARGE_INTEGER lc_time_freq;
double LC_GetCurrentTime()
{
//	double val = clock();
//	return (val / 1000.0);
	QueryPerformanceCounter( &lc_time_cur );
	QueryPerformanceFrequency( &lc_time_freq );
	return ((double)lc_time_cur.QuadPart) / ((double)lc_time_freq.QuadPart);
}

#define UseMessageBoxForIntro	false

#else

#define UseMessageBoxForIntro	false

#endif

