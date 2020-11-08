
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
#include "ioutils.cpp"


//
// Freed Invasion Implimentation
//

//
// Lewcid game code
//

bool isfirst = true;


uint EnvRenderContext;
uint ItemRenderContext;
int HintW=6, HintH=10;

ResCompNode* Environment;
ResCompNode* WItems;

void Test_EachFrame()
{
	Core.Render();
}

void Test_Key(char let)
{
	if ((let >= 'A') && (let <= 'Z'))
		let -= ('A' - 'a');

	switch (let)
	{
	case 'a':
		ShowMessage("Lewcid Graphics Test", "Yup");
		break;
	}
}

float camang=90, camphi=0;

void SetupCamera()
{
	fpnt cam;
	cam.y = sin(camphi);
	cam.x = cos(camang)*cos(camphi);
	cam.z = sin(camang)*cos(camphi);
	cam *= 4.5;
	Core.Graphics->CameraPos->mPos = cam;
	Core.Graphics->LookAt(FPNT(0,0,0));
}

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

char Text_Big[] = "Big";
char Text_Small[] = "Small";
char Text_Back[] = "Back";

void Test_MouseUp(int x, int y)
{
	ResCompNode* node = Core.Graphics->HitCompNode( x, y );
	if (!node)
		return;
	if (!node->mToken)
	{
		printf("No token\n");
		return;
	}
	char* text = (char*)node->mToken;
	printf("Text = '%s'\n", text);
}

ResStencilEdger* Edger=0;

void Test_AfterFlushed(void* data)
{
	static bool first = true;
	if (first)
	{
		first = false;

		RenderContext* rc = Core.Graphics->GetRenderStage( ItemRenderContext );
		ResMesh* mesh = ((MeshRenderContext*)rc)->Target;

		//IO_Mesh_ExportToIOFF(mesh, "Balls", "balls.ioff", true, true);
	}

	/*
	if (!Edger)
	{
		RenderContext* rc = Core.Graphics->GetRenderStage( ItemRenderContext );
		ResMesh* mesh = ((MeshRenderContext*)rc)->Target;
		//Edger = ResMeshEdger::Create( mesh );
		Edger = ResStencilEdger::Create( mesh );
		mesh->mEdger = Edger;
		Edger->mRefPoint = Core.Graphics->LightPos;
		Edger->UpdateEdgeList();
		Edger->UpdateNormals();

		Core.mGameRes = Edger;
	}

	Edger->UpdateOutline();
	Edger->FlushShadow();
	*/
}

void Test_Main()
{
	LC_Event_ContextFlushDone = Test_AfterFlushed;
	Core.Graphics->LightPos->mPos = FPNT(1.5,-1.5,3);

	Core.Graphics->BackgroundColor = ResColor::Create( 98.0/255.0, 120.0/255.0, 191.0/255.0, 1.0);
//	Core.Graphics->BackgroundColor = ResColor::Create( Color4f::FromRGB( 0x005073 ) );

	EnvRenderContext = Core.Graphics->AddRenderStage(MeshFormat::Create(3, true));
	ItemRenderContext = Core.Graphics->AddRenderStage(MeshFormat::Create(3, true));

	ResCompNode* root = ResCompNode::Create(0);
	Core.mRootNode = root;

	ResMesh* mesh = GenSphereMesh(5, 8);
	PaintMesh(mesh, Color4f::Create( 1.0, 1.0, 1.0 ));

	ResCompNode* work = ResCompNode::Create( ItemRenderContext );
	work->mMesh = mesh;
	work->mColor = ResColor::Create( 0.5, 1, 0.5, 0.5 );
	work->mPosition = ResPoint::Create( 0, 0, 1.5);
	root->AddChild(work);

	work = ResCompNode::Create( ItemRenderContext );
	work->mMesh = mesh;
	work->mColor = ResColor::Create( 0.5, 0.5, 1.0, 0.5 );
	work->mPosition = ResPoint::Create( 0, 0, -1.5);
	root->AddChild(work);

	/*
	ResMesh* mesh = GenSphereMesh(5, 8);
	InversePolygonRotation(mesh);
	PaintMesh(mesh, Core.Graphics->BackgroundColor->mValue );
	Environment = ResCompNode::Create(EnvRenderContext);
	Environment->mTransform = ResScaleTransform::Create( 5.0f );
	Environment->mMesh = mesh;
	Environment->mToken = Text_Back;
	root->AddChild( Environment );

	WItems = ResCompNode::Create(ItemRenderContext);
	ResFloat* pit = ResFloat::Create(0);
	ResFloat* ang = ResFloat::Create(0);
	ResRotateTransform* trans = ResRotateTransform::Create(pit, ang);
	WItems->mTransform = trans;
	//mesh = GenSphereMesh(3, 4);
	mesh = GenCubeMesh();
	WItems->mMesh = mesh;
	WItems->mToken = Text_Big;
	root->AddChild( WItems );

	ResCompNode* work = ResCompNode::Create(ItemRenderContext);
	work->mMesh = mesh;
	work->mTransform = ResScaleTransform::Create(0.25);
	work->mPosition = ResPoint::Create( 0, -0.1, 1.5);
	work->mToken = Text_Small;
	WItems->AddChild( work );

	ResAnimateFromTo::Create(ang, ResFloat::Create(-180), ResFloat::Create(180), 7.0);
	*/

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

int main(int argc,char **argv) 
{ 
  glutInit(&argc,argv); 


	SetRandomSeed();

	printf("Lewcid Graphics Test");

  Test_Main();

  glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH|GLUT_STENCIL  ); 
  glutInitWindowSize(DefaultWinW,DefaultWinH); 
  glutInitWindowPosition(100,100); 
  glutCreateWindow("Freed Invasion - by Lewey Geselowitz"); 
  glutDisplayFunc(Display); 
  glutReshapeFunc(Resize); 
  glutMouseFunc(Mouse); 
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(KeyFunc);
  
  glutSetWindowTitle("Lewcid Graphics Test");
  
  //Comment this to remove frame meter:
  glutIdleFunc(Display);

  glutMainLoop(); 

  return 0; 
} 

