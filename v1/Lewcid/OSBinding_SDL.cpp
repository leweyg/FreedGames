
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
//	SDL_WM_SetCaption(mess, 0);
//	MessageBox(0, mess, title, MB_OK);
}

double LC_GetCurrentTime()
{
	double val = clock();
	return (val / 1000.0);
}

#define UseMessageBoxForIntro	false

#else

#define UseMessageBoxForIntro	false

// put the Mac Carbon headers here

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void SetRandomSeed()
{
  srand( clock() );
}

void ConvertCToPascalString(char* from, unsigned char* to)
{
    int i=0;
    for(i=0; from[i]; i++)
    {
        to[i+1] = from[i];
    }
    to[0] = i;
}

void ShowMessage(char* mess, char* title)
{
    unsigned char buff1[200], buff2[200];
    ConvertCToPascalString( ((char*)mess), buff1);
    ConvertCToPascalString( ((char*)title), buff2);
    
    short res;
    StandardAlert(kAlertNoteAlert, buff2, buff1, 0, &res);
}


/*
void ShowMessage(char* mess, char* title)
{
	printf("\n%s\n", title);
	for (int i=0; title[i]; i++)
		printf("-");
	printf("\n%s\n\n", mess);
}
*/

double LC_GetCurrentTime()
{
	double val = clock();
	return (val / 100.0);
}

#endif

