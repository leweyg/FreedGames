
//.  Shuzzle - based on Lewcid
//		By Lewey Geselowitz
//
// 6/15/2004

#define COMPILE_SHUZZLE 1
#if COMPILE_SHUZZLE

#include "OSBinding.cpp"

//
// Include Lewcid
//

#include "lewcid.h"
#include "Utils.h"


//
// Shuzzle Implimentation
//

#include "ShuzzleGame.cpp"


//
// Lewcid game code
//

bool IsShiftDown;

#define SETSHIFT() {IsShiftDown=((glutGetModifiers() & GLUT_ACTIVE_SHIFT)!=0);}

void Test_EachFrame()
{
	Core.Render();
}

void Test_MoveDirection(int dir)
{
	fpnt v = Core.Graphics->CameraLookDir->mPos;
	v.z = 0;
	v.Normalize();
	
	if (dir==1)
		v *= -1;
	if ((dir/2)==1)
	{
		v = v.Cross( FPNT(0, 0, 1) );
		if (dir==3)
			v *= -1;
	}
	v *= 0.25;

	v += GGame->CameraLookAt->mPos;
	v = GGame->CameraBounds.Clip(v);
	GGame->CameraLookAt->mPos = v;
	SetupCamera();
}

const int NumZooms = 5;
int CurZoom = 0;
float ZoomValues[NumZooms] = {45, 35, 25, 65, 55 };

void Test_Key(char let)
{
	if ((let >= 'A') && (let <= 'Z'))
		let -= ('A' - 'a');

	SETSHIFT();
	if ((let >= '0') && (let <= '9'))
	{
		GGame->GotoLevel(let-'0');
		GGame->SetIsInv((let-'0')>2);
	}
	switch (let)
	{
	case 'n':
		GGame->NewGame();
		break;
	case 'a':
		Test_MoveDirection(2);
		break;
	case 'd':
		Test_MoveDirection(3);
		break;
	case 's':
		Test_MoveDirection(1);
		break;
	case 'w':
		Test_MoveDirection(0);
		break;
	case 'q':
		CALLEDDEBUG = !CALLEDDEBUG;
		break;
	case 'g':
		if (GGame->GetIsInv())
			GGame->SetIsInv(false);
		break;
	case 'i':
		GGame->SetIsInv( !GGame->GetIsInv() );
		break;
	case 'z':
		{
			int i = 0;
			for (i=0; i<NumZooms; i++)
			{
				if (ZoomValues[i] == Core.Graphics->CameraPerspec->mValue)
				{
					Core.Graphics->CameraPerspec->mValue = ZoomValues[(i+1)%NumZooms];
					return;
				}
			}
			Core.Graphics->CameraPerspec->mValue = ZoomValues[0];
			return;
		}
		break;
	}
	SetupCamera();
}


#define DM_NONE		0
#define DM_CAMERA	1
#define DM_LIGHT	2
#define DM_BLOCK	3
#define BUTTON_LEFT		(1<<0)
#define BUTTON_RIGHT	(1<<1)
#define BUTTON_BOTH		(BUTTON_LEFT | BUTTON_RIGHT)
int DragMode = DM_NONE;
uint ButtonState = 0;
Block* DraggedBlock = 0;
ipnt DragTotal;
int ActiveDist = 40;

void Test_LevelChanged(void* data) {
    DraggedBlock = nullptr;
    DragMode = DM_NONE;
}

int BestDragAxis(ipnt* dragtotal)
{
	fpnt drag = ITOFPNT(*dragtotal);
	float len = drag.Length();
	drag.Normalize();
	drag *= len-ActiveDist;
	*dragtotal = FTOIPNT( drag );
	int besti=-1;
	float bestd = -100;
	fpnt work2, work;
	drag.z = 0;
	ipnt* to;
	
	for (int i=0; i<4; i++)
	{
		to = (ipnt*)&Dirs[i][0];
		work = ITOFPNT(DraggedBlock->mCenter);
		work2 = Core.Graphics->Project( work + ITOFPNT(*to) );
		work = Core.Graphics->Project( work );
		work2 -= work;
		work2.z = 0;

		float cd = drag.Dot( work2 );
		if (cd > bestd)
		{
			bestd = cd;
			besti = i;
		}
	}

	return besti;
}

bool ScreenToPoint(int scrx, int scry, float height, fpnt* ans)
{
	fpnt del = Core.Graphics->LookDir(scrx, scry);
	del.Normalize();
	float len = (Core.Graphics->CameraPos->mPos.z - GGame->LightHeight());
	if (del.z < 0)
	{
		len /= del.z;
		fpnt nval = Core.Graphics->CameraPos->mPos - (del*len);
		nval.z = GGame->LightHeight();

		*ans = nval;
		return true;
	}
	return false;
}

void Test_MouseDrag(int scrx, int scry, int dx, int dy)
{
	if (DragMode==DM_NONE)
		return;

	DragTotal.x += dx;
	DragTotal.y += dy;
	int m = ActiveDist;

	if (DragMode == DM_BLOCK)
	{
		if (ButtonState==BUTTON_LEFT)
		{
			if (IsShiftDown)
			{
				if (DragTotal.y > m)
				{
					DraggedBlock->Shift( IPNT(0,0,-1) );
					DragTotal.y -= m;
				}
				if (DragTotal.y < -m)
				{
					DraggedBlock->Shift( IPNT(0,0,1) );
					DragTotal.y += m;
				}
			}
			else
			{
				fpnt drag = ITOFPNT( DragTotal );
				if (drag.Length() >= m)
				{
					int besti = BestDragAxis(&DragTotal);
					if (besti!=-1)
					{
						ipnt* to = (ipnt*)&Dirs[besti][0];
						DraggedBlock->Shift( *to );
					}
				}
			}
		}
		if (ButtonState==BUTTON_RIGHT)
		{
			if (IsShiftDown)
			{
				if (DragTotal.x > m)
				{
					DraggedBlock->Spin(true);
					DragTotal.x -= m;
				}
				if (DragTotal.x < -m)
				{
					DraggedBlock->Spin(false);
					DragTotal.x += m;
				}
			}
			else
			{
				fpnt drag = ITOFPNT( DragTotal );
				if (drag.Length() >= m)
				{
					int besti = BestDragAxis(&DragTotal);
					if (besti!=-1)
					{
						int i = ((besti/2)+1)%2;
						bool neg = ((besti%2)==1);
						if (i==0)
							neg = !neg;
						DraggedBlock->Flip(i, neg);
					}
				}
			}
		}
	}

	if (DragMode == DM_LIGHT)
	{
		fpnt del = Core.Graphics->LookDir(scrx, scry);
		del.Normalize();
		float len = (Core.Graphics->CameraPos->mPos.z - GGame->LightHeight());
		len /= del.z;
		if (del.z < 0)
		{
			fpnt nval = Core.Graphics->CameraPos->mPos - (del*len);
			nval.z = GGame->LightHeight();

			//if (!GGame->CameraBounds.Includes(IPNT(nval.x, nval.y, nval.z)))
			{
				nval = GGame->CameraBounds.Clip( nval );
			}

			Core.Graphics->LightPos->mPos = nval;
			SetupCamera();
		}
	}

	if (DragMode==DM_CAMERA)
	{
		if (ButtonState==BUTTON_LEFT)
		{
			fpnt front = Core.Graphics->CameraLookDir->mPos;
			front.z = 0;
			front.Normalize();
			fpnt side = front.Cross(FPNT(0, 0, 1));

			front *= ((float)dy) / 35.0f;
			side *= ((float)dx) / 35.0f;
			front += side + GGame->CameraLookAt->mPos;
			front = GGame->CameraBounds.Clip(front);
			GGame->CameraLookAt->mPos = front;
			SetupCamera();
		}
		if (ButtonState==BUTTON_RIGHT)
		{
			float fx = (150 * ((float)dx)) / 320.0f;
			float fy = (150 * ((float)dy)) / 320.0f;

			GGame->CameraAng -= fx;
			GGame->CameraPitch += fy;

			float max = 65;
			float min = 10;
			if (GGame->CameraPitch < min)
				GGame->CameraPitch = min;

			if (GGame->CameraPitch > max)
				GGame->CameraPitch = max;

			SetupCamera();
		}
	}
}

bool DoesHitLight(int x, int y)
{
	fpnt dir = Core.Graphics->LookDir(x, y);
	fpnt cam = Core.Graphics->CameraPos->mPos - Core.Graphics->LightPos->mPos;
	return (GGame->mLightMesh->PolyHitTest(cam, dir)!=-1);
}

void Test_MouseDown(int x, int y, bool isright)
{
	DragTotal.Set(0, 0, 0);
	FlagOn(ButtonState, ((isright)? BUTTON_RIGHT : BUTTON_LEFT ) );
	if (DoesHitLight(x,y))
	{
		DragMode = DM_LIGHT;
		return;
	}
	ResCompNode* node = Core.Graphics->HitCompNode( x, y );
	if ((!node) || (!node->mToken))
	{
		DragMode = DM_CAMERA;
	}
	else
	{
		DragMode = DM_BLOCK;
		DraggedBlock = (Block*)node->mToken;
	}
}

void Test_MouseUp(int x, int y)
{
	DragMode = DM_NONE;
	ButtonState = 0;
}

void Test_GLSettings(void* data)
{
//	glShadeModel(GL_FLAT);
//	glDisable(GL_CULL_FACE);
}

void Test_Main()
{
	GGame = Game::Create();
	LC_Event_GLSettingsInit = Test_GLSettings;
	//Core.Graphics->mIsShiny = true;
    GGame->mEvent_LevelChanged = Test_LevelChanged;

	Core.Graphics->BackgroundColor = ResColor::Create( 98.0/255.0, 120.0/255.0, 191.0/255.0, 1.0);
	//Core.Graphics->BackgroundColor = ResColor::Create( Color4f::FromRGB( 0x005073 ) );

	uint smask = Core.Graphics->AddRenderStage(MeshFormat::Create(4, false));
	SolidRenderContext = smask;


	ResCompNode* root = ResCompNode::Create(0);
	Core.mRootNode = root;

	//Board* board = GenBoard_Level2();
	Board* board = GenBoard_Level1();
	//Board* board = GenBoard_Cuzzle();
	GGame->Init(root, 1, board);
	GGame->SetIsInv(false);

	SetupCamera();
}







//
//GLUT Code
//

int CurMouseButton = -1;
ipnt LastMouse;
ipnt MouseOffset;
bool IsFullScreen = false;
int DefaultWinW=640, DefaultWinH=480;


#define FramesPerShowFPS 100
#define TestFrameRate 0

void Display()
{

	Test_EachFrame();

	glutSwapBuffers(); 
}

void KeyFunc(unsigned char let, int x, int y)
{
	Test_Key(let);
	glutPostRedisplay();
}

int LastMX, LastMY;
void MouseMotion(int x, int y)
{
	if (CurMouseButton != -1)
	{
		Test_MouseDrag(x, y, x-LastMouse.x, y-LastMouse.y);
		glutPostRedisplay();
	}
	LastMouse.Set(x, y, 0);
}

void Mouse(int button,int state,int x,int y) 
{ 
	LastMouse.Set(x, y, 0);
	SETSHIFT();

	if (state == GLUT_UP)
	{
		Test_MouseUp(x, y);
		glutPostRedisplay();
		CurMouseButton=-1;
		return;
	}

	if(state!=GLUT_DOWN)
	{
		CurMouseButton = -1;
		return; 
	}

	if(button==GLUT_RIGHT_BUTTON)
		CurMouseButton = GLUT_RIGHT_BUTTON;
	else
		CurMouseButton = GLUT_LEFT_BUTTON;
	Test_MouseDown(x, y, CurMouseButton == GLUT_RIGHT_BUTTON);

	glutPostRedisplay(); 
}

void Resize(int x, int y)
{
	Core.Graphics->ScrWidth = x;
	Core.Graphics->ScrHeight = y;

	glutPostRedisplay();
}

class custom_cout {
    
public:
    custom_cout& operator << (const char* str) {
        printf("%s",str);
        return *this;
    };
};

int main(int argc,char **argv) 
{
    custom_cout cout;
    auto endl = "\n";
    
	cout << "Shuzzle - BETA! - by Lewey Geselowitz" << endl;
	cout << endl;
	cout << "Objective: " << endl;
	cout << "Move the blocks so that they fill the shape outlined in white" << endl;
	cout << endl;
	cout << "Controls:" << endl;
	cout << "Left drag on blocks to drag them around" << endl;
	cout << "Hold SHIFT and Left drag on a block to raise or lower it" << endl;
	cout << "Right drag on a block and move in a direction to rotate that way" << endl;
	cout << "Hold SHIFT and Right drag on blocks to spin them around" << endl;
	cout << "Left drag on the light to move it around" << endl;
	cout << "Left drag on empty space to move the camera focus around" << endl;
	cout << "Right drag on empty space to change your camera angle" << endl;
	cout << endl;
	cout << "NOTE: there is currently a bug, to go to SHIFT mode, let go of any" << endl;
	cout << "mouse button and then click again while holding shift. Will fix later" << endl;
	cout << endl;

	cout << "Keys:" << endl;
	cout << "G - when in invisable mode, press G to 'give up' and see the visable version" << endl;
	cout << "0-5 - go to that level" << endl;
	cout << endl;

	cout << "Debug keys (only in Beta): " << endl;
	cout << "I - toggle invisability" << endl;
	cout << endl;

  glutInit(&argc,argv); 


	SetRandomSeed();


  Test_Main();

  glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH|GLUT_STENCIL  ); 
  glutInitWindowSize(DefaultWinW,DefaultWinH); 
  glutInitWindowPosition(100,100); 
  glutCreateWindow("Shuzzle - by Lewey Geselowitz"); 
  glutDisplayFunc(Display); 
  glutReshapeFunc(Resize); 
  glutMouseFunc(Mouse); 
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(KeyFunc);
    glutWMCloseFunc([]() {
        exit(0);
    });
  
  glutSetWindowTitle("Shuzzle - by Lewey Geselowitz");
  
  //Comment this to remove frame meter:
  glutIdleFunc(Display);

  glutMainLoop();
    
    glutPostRedisplay();

  return 0; 
} 

#endif

