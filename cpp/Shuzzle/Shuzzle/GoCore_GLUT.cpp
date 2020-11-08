
//.  Freed Go - based on Lewcid
//		By Lewey Geselowitz
//
// 6/15/2004

#include "OSBinding.cpp"

//
// Include Lewcid
//

//
// Include Lewcid
//

#include "lewcid.h"
#include "Utils.h"


//
// Freed Go Implimentation
//

#include "GoGame.cpp"


//
// Lewcid game code
//

bool havewindow = false;
Game* GGame;
ResMatrixTransform* GRotation;
ResCompNode *BoardViewer;
char* CommandLineFile=0;

uint SolidRenderContext;
uint WireRenderContext;
int HintW=6, HintH=10;

void SetupNewBoard(char* name)
{
	if (!GGame->mNodes)
	{
		Board* board = new Board();
		if (!GenBoardFromName(name, board, false))
		{
			ShowMessage("Couldn't create that board", "Board Error");
			delete board;
			return;
		}
		GGame->Init( BoardViewer, board, SolidRenderContext, WireRenderContext );
	}
	else
	{
		GGame->SwitchBoard(name, true);
	}
}


void Test_EachFrame()
{
	GGame->CheckNetwork();
	Core.Render();
}


void Test_UpdateText()
{
	if (!havewindow)
		return;

	if (GGame->CurTurn == WHITE)
		sprintf(game_textbuffer, "Freed Go - White: %d - Black: %d - By Lewey Geselowitz", GGame->Scores[0], GGame->Scores[1] );
	else
		sprintf(game_textbuffer, "Freed Go - Black: %d - White: %d - By Lewey Geselowitz", GGame->Scores[1], GGame->Scores[0] );

	glutSetWindowTitle(game_textbuffer);
}

int tickcount=0;
void Test_Tick(int val)
{
	if (GGame->CheckNetwork())
	{
		glutPostRedisplay();
		Test_UpdateText();
	}
	if (GGame->mTrans.mOpen)
	{
		tickcount++;
		if ((tickcount%5)==0)
			GGame->SendPacket(GPT_BusyPacket,0);
	}

	glutTimerFunc(250, Test_Tick, 0);
}


void Test_InitRemoteGame()
{
	if (!GGame->mIsNetGame)
	{
		if (SetupRemoting(&GGame->mTrans))
		{
			ShowMessage("Connected!", "Net Game");
			GGame->InitRemoteGame();
		}
	}
	else
	{
		ShowMessage("A remote game is already in progrss", "Hey");
	}
}

void Test_Key(char let)
{
	if ((let >= 'A') && (let <= 'Z'))
		let -= ('A' - 'a');

	switch (let)
	{
	case 'n':
		GGame->NewGame(true);
		break;
	case 'u':
		GGame->Undo(true, false);
		break;
	case 'q':
		GGame->EndGame();
		break;
	case 'p':
		GGame->Pass(true);
		break;
	case 'r':
		Test_InitRemoteGame();
		break;
#if DevEdition
	case 'r':
		GGame->RandomBoard(2);
		break;
	case 'e':
		GGame->RandomBoard(3);
		break;
	case 'w':
		GGame->RandomBoard(1);
		break;
#endif

	case 'f':
	case '4':
		SetupNewBoard( "flat" );
		break;
	case 'd':
		SetupNewBoard( "diamond" );
		break;
	case 's':
		SetupNewBoard( "sphere" );
		break;
	case 'm':
		SetupNewBoard( "mobius" );
		break;
	case 't':
		SetupNewBoard( "torus" );
		break;
	case '3':
		SetupNewBoard( "3" );
		break;
	case '5':
		SetupNewBoard( "5" );
		break;
	case '6':
		SetupNewBoard( "6" );
		break;
	case 'l':
		SetupNewBoard( "layered" );
		break;
	case 'c':
		SetupNewBoard( "cylinder" );
		break;
	case 'b':
		SetupNewBoard( "box" );
		break;
	}

	Test_UpdateText();
}

float camang=Pi/2, camphi=0;

void SetupCamera()
{
	fpnt cam;
	cam.y = sin(camphi);
	cam.x = cos(camang)*cos(camphi);
	cam.z = sin(camang)*cos(camphi);
	cam *= 4.0;
	Core.Graphics->CameraPos->mPos = cam;
	Core.Graphics->LookAt(FPNT(0, 0, 0));
	Core.Graphics->LightPos->mPos = Core.Graphics->CameraPos->mPos;
}

void Test_MouseDrag(int dx, int dy, bool isright)
{
	if (!isright)
		return;

	float fx = (Pi * ((float)dx)) / 320.0f;
	float fy = (Pi * ((float)dy)) / 240.0f;

	camang += fx;
	camphi += fy;

	float max = Pi/4.1;
	if (camphi < -max)
		camphi = -max;

	if (camphi > max)
		camphi = max;

	SetupCamera();
}

void Test_MouseUp(int x, int y)
{
	ResCompNode* node = Core.Graphics->HitCompNode( x, y );
	if (!node)
		return;
	HitToken* i = (HitToken*)node->mToken;
	if (i==0)
		return;
	GGame->RemoveDeadMode = ((glutGetModifiers() & GLUT_ACTIVE_SHIFT)!=0);
	GGame->Clicked( i->Node->Index, true );
	Test_UpdateText();
}

void MyEvent_DestroyCompNode(void* data)
{
	ResCompNode* node = (ResCompNode*)data;
	if (node->mToken)
		delete ((HitToken*)node->mToken);
}

void Test_Main()
{
	LC_Event_DestroyCompNode = MyEvent_DestroyCompNode;

	GGame = Game::Create();

	if (!Net_InitNetworking())
	{
		ShowMessage("Couldn't Init Networking", "Error");
	}

	uint smask = Core.Graphics->AddRenderStage(MeshFormat::Create(3, true));
	uint wmask = Core.Graphics->AddRenderStage(MeshFormat::Create(2, true));
	SolidRenderContext = smask;
	WireRenderContext = wmask;

	ResCompNode* root = ResCompNode::Create(smask);
//	root->mPosition = ResPoint::Create(0, 0, -4);
	Core.mRootNode = root;

	ResCompNode* boardview = ResCompNode::Create(smask);
//	GRotation = ResMatrixTransform::Create();
//	SetMatrix_RotateX( GRotation->mMatrix, 45 );
//	boardview->mTransform = GRotation;
	root->AddChild( boardview );
	BoardViewer = boardview;

	if (CommandLineFile)
		SetupNewBoard(CommandLineFile);
	else
		Test_Key('m');

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
		switch (CurMouseButton)
		{
		case GLUT_RIGHT_BUTTON:
			Test_MouseDrag(x-LastMouse.x, y-LastMouse.y, true);
			break;
		case GLUT_LEFT_BUTTON:
			break;
		}
		glutPostRedisplay();
	}
	LastMouse.Set(x, y, 0);
}

void Mouse(int button,int state,int x,int y) 
{ 
	LastMouse.Set(x, y, 0);

	if (state == GLUT_UP)
	{
		if (CurMouseButton == GLUT_LEFT_BUTTON)
		{
			Test_MouseUp(x, y);
			glutPostRedisplay();
		}
		CurMouseButton=-1;
		return;
	}
	if(state!=GLUT_DOWN)
	{
		CurMouseButton = -1;
		return; 
	}
	if(button==GLUT_RIGHT_BUTTON)
	{
		CurMouseButton = GLUT_RIGHT_BUTTON;
		return;
	}

	CurMouseButton = GLUT_LEFT_BUTTON;

	glutPostRedisplay(); 
}

void Resize(int x, int y)
{
	Core.Graphics->ScrWidth = x;
	Core.Graphics->ScrHeight = y;

	glutPostRedisplay();
}

char filename[]="pyramid.txt";
int main(int argc,char **argv) 
{ 
  glutInit(&argc,argv); 

//  CommandLineFile = filename;
  if (argc==2)
	  CommandLineFile = argv[1];


	SetRandomSeed();

	printf("Freed Go - by Lewey Geselowitz - lewey@ufl.edu\n");
	printf(" For More Info: http://plaza.ufl.edu/lewey/lc/freedgo.html\n");
	printf("\n");
	printf("The small brown stones represent emtpy locations.\n");
	printf("Left click on a brown stone to play at that location.\n");
	printf("Drag the right mouse button to change your view\n");
	printf("Press 'N' for a new game.\n");
//	printf("Press '2' to turn 2 player mode on and off (off by default)\n");
	printf("Press 'P' to Pass.\n");
	printf("Press 'U' to Undo previous moves\n");
	printf("Press 'R' to start a Remote game\n");
	printf("Hold down SHIFT and then left click to remove dead stones\n");
	printf("Press 'Q' to count occupied areas and show score\n");
	printf("\nBoard Types - Press the associated key to change to that board.\n");
	printf("F - Flat standard game board\n");
	printf("S - Sphereical board\n");
	printf("C - Cylinder board\n");
	printf("B - Box or cubed board\n");
	printf("D - Diamond board\n");
	printf("3 - 3 neighbors board\n");
	printf("5 - 5 neighbors board\n");
	printf("6 - 6 neighbors board\n");
	printf("L - Layered planes\n");
	printf("T - Torus board\n");
	printf("M - Mobius strip board\n");
	printf("\n");

  Test_Main();

  glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH  ); 
  glutInitWindowSize(DefaultWinW,DefaultWinH); 
  glutInitWindowPosition(100,100); 
  glutCreateWindow("Freed Go - by Lewey Geselowitz"); 
  glutDisplayFunc(Display); 
  glutReshapeFunc(Resize); 
  glutMouseFunc(Mouse); 
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(KeyFunc);
  glutTimerFunc(200, Test_Tick, 0);

  havewindow = true;

  //Comment this to remove frame meter:
#if TestFrameRate
  glutIdleFunc(Display);
#endif

  glutMainLoop(); 

  return 0; 
} 

