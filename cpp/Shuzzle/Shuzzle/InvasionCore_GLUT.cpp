
//.  Freed Invasion - based on Lewcid
//		By Lewey Geselowitz
//
// 6/15/2004

#include "OSBinding.cpp"

//
// Include Lewcid
//

#include "lewcid.h"
#include "utils.h"


//
// Freed Invasion Implimentation
//

#include "InvasionGame.cpp"


//
// Lewcid game code
//

bool isfirst = true;
Game* GGame;
ResMatrixTransform* GRotation;
ResCompNode *BoardViewer;
ResCompNode *ControlHolder;
ResRotateTransform* ControlTransform;



uint SolidRenderContext;
uint WireRenderContext;
int HintW=6, HintH=10;

void saddto(char* add, char* text)
{
	int i;
	for (i=0; add[i]!=0; i++)
	{
	}
	int j;
	for (j=0; text[j]!=0; j++)
	{
		add[i+j] = text[j];
	}
	add[i+j] = 0;
}

char show_textbuffer[400];
char show_textbuffer2[400];
void ShowIntro()
{
	char* text = show_textbuffer;
	text[0]=0;

	char* text2 = show_textbuffer2;
	text2[0]=0;


	saddto(text2,"Freed Invasion - Written by Lewey Geselowitz\n");
        saddto(text2," Website: http://plaza.ufl.edu/lewey/lc/freedinvasion.html");

	saddto(text,"");
	saddto(text,"Each turn, click on a coloured box to change into that colour.\n");
	saddto(text,"The current players pieces will pulsate to indicate it is their turn.\n");
	saddto(text,"Drag the right mouse button to change your view\n");
	saddto(text,"Press 'N' for a new game.\n");
	saddto(text,"Press '2' to turn 2 player mode on and off (off by default)\n");
	saddto(text,"Press 'U' to Undo the previous move.\n");
	//saddto(text,"Press 'H' to see this message again.");

	if (UseMessageBoxForIntro)
		ShowMessage(text, text2);
	else
		printf("%s\n\n%s\n", text2, text);

	text[0] = 0;
	text2[0] = 0;

	saddto(text2, "Board Types");
	saddto(text,"Press the associated key to change to that board.\n");
	saddto(text,"F - Flat standard game board\n");
	saddto(text,"S - Sphereical board\n");
	saddto(text,"C - Cylinder board\n");
	saddto(text,"B - Box or cubed board\n");
	saddto(text,"3 - 3 neighbors board\n");
	saddto(text,"5 - 5 neighbors board\n");
	saddto(text,"6 - 6 neighbors board\n");
	saddto(text,"L - Layered planes\n");
	saddto(text,"T - Torus board\n");
	saddto(text,"M - Mobius strip board\n");
	saddto(text,"\n");

	if (UseMessageBoxForIntro)
		ShowMessage(text, text2);
	else
		printf("%s\n\n%s\n", text2, text);
}

void SetupNewBoard(BoardCreateFunc func)
{
	Board* board = new Board();
	(*func)(board, HintW, HintH);
	GGame->Init( BoardViewer, board, SolidRenderContext, WireRenderContext );
}

void Test_EachFrame()
{
	Core.Render();
}

void CheckEndMessage()
{
	if (EndGameMessage[0])
	{
		ShowMessage(EndGameMessage, "Final Score:");
		EndGameMessage[0] = 0;
	}
}

void Test_Key(char let)
{
	if ((let >= 'A') && (let <= 'Z'))
		let -= ('A' - 'a');

	switch (let)
	{
	case 'n':
		GGame->NewGame();
		break;
	case '2':
		GGame->IsTwoPlayer = !GGame->IsTwoPlayer;
		printf("2 Player Mode is ");
		if (GGame->IsTwoPlayer)
			printf("ON\n");
		else
			printf("OFF\n");
		break;
	case 'u':
		GGame->Undo();
		break;
	case 'h':
		//ShowIntro();
		break;
	case 'a':
		{
			sprintf(game_textbuffer, "Freed Invasion - Written by Lewey Geselowitz\n  http://plaza.ufl.edu/lewey/lc/freedinvasion.html");
			ShowMessage(game_textbuffer, "About");
		}
		break;

	case 'f':
	case '4':
		HintW = 10;
		HintH = 10;
		SetupNewBoard( CreateBoard_Standard );
		break;
	case 's':
		HintW = 9;
		HintH = 13;
		SetupNewBoard( CreateBoard_Sphere );
		break;
	case 'm':
		HintW = 15;
		HintH = 6;
		SetupNewBoard( CreateBoard_Mobius );
		break;
	case 't':
		HintW = 12;
		HintH = 8;
		SetupNewBoard( CreateBoard_Torus );
		break;
	case '3':
		HintW = 8;
		HintH = 8;
		SetupNewBoard( CreateBoard_HoneyComb3 );
		break;
	case '5':
		HintW = 8;
		HintH = 8;
		SetupNewBoard( CreateBoard_HoneyComb5 );
		break;
	case '6':
		HintW = 8;
		HintH = 8;
		SetupNewBoard( CreateBoard_HoneyComb6 );
		break;
	case 'l':
		HintW = 8;
		HintH = 8;
		SetupNewBoard( CreateBoard_Layered );
		break;
	case 'c':
		HintW = 15;
		HintH = 8;
		SetupNewBoard( CreateBoard_Cylinder );
		break;
	case 'b':
		HintW = 5;
		HintH = 5;
		SetupNewBoard( CreateBoard_Cube );
		break;

	case ']':
		{
			byte rec = GGame->mAI.Think( GGame->mCurState, GGame->CurTurn );
			GGame->Clicked( rec );
		}
		break;
	}
}

float camang=90, camphi=0;
fpnt RaiseOffset = FPNT(0, 0.25, 0);

void SetupCamera()
{
	fpnt cam;
	cam.y = sin(camphi);
	cam.x = cos(camang)*cos(camphi);
	cam.z = sin(camang)*cos(camphi);
	cam *= 4.5;
	Core.Graphics->CameraPos->mPos = cam + RaiseOffset;
	Core.Graphics->LookAt(RaiseOffset);
	Core.Graphics->LightPos->mPos = Core.Graphics->CameraPos->mPos;

	ControlTransform->mAngle->mValue = 90.0f -RadToDeg(camang);
	ControlTransform->mAngle->Changed();
	ControlTransform->mPitch->mValue = RadToDeg(camphi);
	ControlTransform->mPitch->Changed();
}

MatrixStruct mone, mtwo;
void Test_MouseDrag(int dx, int dy, bool isright)
{
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
	if (i->Type==0)
		GGame->Clicked( i->Color );
	if (i->Type==1)
		GGame->Clicked( GGame->mCurState->NodeStates[i->Node->Index] );
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

	Core.Graphics->BackgroundColor = ResColor::Create( 98.0/255.0, 120.0/255.0, 191.0/255.0, 1.0);
//	Core.Graphics->BackgroundColor = ResColor::Create( Color4f::FromRGB( 0x005073 ) );

	uint smask = Core.Graphics->AddRenderStage(MeshFormat::Create(3, true));
	uint wmask = Core.Graphics->AddRenderStage(MeshFormat::Create(2, true));
	SolidRenderContext = smask;
	WireRenderContext = wmask;

	ResCompNode* root = ResCompNode::Create(smask);
	Core.mRootNode = root;

	ControlHolder = ResCompNode::Create(0);
	ControlTransform = ResRotateTransform::Create(0.0f, 0.0f);
	ControlHolder->mTransform = ControlTransform;
	ControlHolder->mPosition = ResPoint::Create(RaiseOffset.x, RaiseOffset.y, RaiseOffset.z);

	ResMesh* cmesh = GenCubeMesh();
	ResScaleTransform* cscale = ResScaleTransform::Create( 1.0 / ((float)NumColors) );
	for (int i=0; i<NumColors; i++)
	{
		ResCompNode* n = ResCompNode::Create(smask);
		n->mMesh = cmesh;
		float x = (((float)i) / ((float)(NumColors-1)));
		x = (3.0*x) - 1.5;
		n->mPosition = ResPoint::Create( x, 1.5f, 0.0f);
		n->mTransform = cscale;
		n->mColor = ResColor::Create( GameColors[i] );
		HitToken* tok = new HitToken;
		tok->Type = 0;
		tok->Color = i;
		n->mToken = tok;

		ControlHolder->AddChild( n );
	}

	root->AddChild( ControlHolder );

	ResCompNode* boardview = ResCompNode::Create(smask);
	root->AddChild( boardview );
	BoardViewer = boardview;

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

	CheckEndMessage();
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

int main(int argc,char **argv) 
{ 
  glutInit(&argc,argv); 


	SetRandomSeed();

	ShowIntro();


  Test_Main();

  glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH  ); 
  glutInitWindowSize(DefaultWinW,DefaultWinH); 
  glutInitWindowPosition(100,100); 
  glutCreateWindow("Freed Invasion - by Lewey Geselowitz"); 
  glutDisplayFunc(Display); 
  glutReshapeFunc(Resize); 
  glutMouseFunc(Mouse); 
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(KeyFunc);
  
  glutSetWindowTitle("Freed Invasion - by Lewey Geselowitz");
  
  //Comment this to remove frame meter:
  glutIdleFunc(Display);

  glutMainLoop(); 

  return 0; 
} 

