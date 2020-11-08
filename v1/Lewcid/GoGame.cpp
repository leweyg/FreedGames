
//.  Freed Go - based on Lewcid
//		By Lewey Geselowitz
//
// 6/15/2004

#include "Board.cpp"

#define USENET	1

#include "LCNet.h"

#define WHITE	0
#define BLACK	1
#define EMPTY	2
#define TAKEN	10

#define TYPES	3
#define OTHERTURN(T)	((T+1)%2)

char GoError[2][40] = {
	"Spot already taken",
	"Going there would be suicide",
};

class GoRules
{
public:
	char* MakeMove(BoardState* bs, int at, byte turn);
	void LandStats(BoardState* bs, int* whiteland, int* blackland, int* freeland);

	GoRules() {Buffer=0; BuffSize=0;};
	~GoRules() {if (Buffer) delete [] Buffer;};

private:
	BoardState* mCurState;
	BoardState mBack;

	int* Buffer;
	int BuffSize;
	int CurSize;
	int CurLiberties;
	byte CurColor;
	bool HitWhite, HitBlack;


	void FindConnected(int at);
	void NeighborTaken(int at);
	void subFindConnected(int at);

	int FillEmptyFrom(int at);

	void CheckBuffer(Board* b) 
	{
		if (BuffSize!=b->NumNodes) {
			if (Buffer)
				delete [] Buffer; 
			Buffer = new int[b->NumNodes];
			BuffSize = b->NumNodes;
		}
	};
};

int GoRules::FillEmptyFrom(int at)
{
	byte b = mCurState->NodeStates[at];
	if (b != EMPTY )
	{
		if (b == WHITE)
			HitWhite=1;
		if (b == BLACK)
			HitBlack=1;
		return 0;
	}

	mCurState->NodeStates[at] = TAKEN;
	int c = 1;
	GoNode* node = &mCurState->mBoard->Nodes[at];

	for (int i=0; i<node->NumNeighbors; i++)
	{
		c += FillEmptyFrom( node->Neighbors[i] );
	}

	return c;
}

void GoRules::LandStats(BoardState* bs, int* whiteland, int* blackland, int* freeland)
{
	*whiteland = 0;
	*blackland = 0;
	*freeland = 0;
	mCurState = bs;

	for (int i=0; i<bs->mBoard->NumNodes; i++)
	{
		if ( (*bs)[i] == EMPTY )
		{
			HitWhite = 0;
			HitBlack = 0;
			int c = FillEmptyFrom(i);
			if (!((HitWhite) ^ (HitBlack)))
				*freeland += c;
			if ((HitWhite) && (!HitBlack))
				*whiteland += c;
			if ((!HitWhite) && (HitBlack))
				*blackland += c;
		}
	}

	for (int j=0; j<bs->mBoard->NumNodes; j++)
	{
		if ( (*bs)[j] == TAKEN )
			(*bs)[j] = EMPTY;
	}
}

char* GoRules::MakeMove(BoardState* bs, int at, byte turn)
{
	if ( bs->NodeStates[at] != EMPTY )
		return &GoError[0][0];

	mBack.Init(bs);

	bs->NodeStates[at] = turn;
	mCurState = bs;

	GoNode* node = & bs->mBoard->Nodes[at];
	for (int i=0; i<node->NumNeighbors; i++)
	{
		int n = node->Neighbors[i];
		if (mCurState->NodeStates[n] == OTHERTURN(turn))
			NeighborTaken(n);
	}

	FindConnected(at);
	if (CurLiberties==0)
	{
		bs->Init(&mBack);
		return &GoError[1][0];
	}

	return 0;
}

void GoRules::NeighborTaken(int at)
{
	FindConnected(at);
	if (CurLiberties==0)
	{
		for (int i=0; i<CurSize; i++)
		{
			mCurState->NodeStates[Buffer[i]] = EMPTY;
		}
	}
}

void GoRules::subFindConnected(int at)
{
	if ( mCurState->NodeStates[at] == OTHERTURN(CurColor) )
		return;

	for (int i=0; i<CurSize; i++)
	{
		if (Buffer[i] == at)
			return;
	}
	Buffer[CurSize] = at;
	CurSize++;

	if (mCurState->NodeStates[at] == EMPTY)
		CurLiberties++;
	else
	{
		GoNode* node = & mCurState->mBoard->Nodes[at];
		for (int j=0; j<node->NumNeighbors; j++)
		{
			subFindConnected( node->Neighbors[j] );
		}
	}
}

void GoRules::FindConnected(int at)
{
	CurLiberties = 0;
	CurSize = 0;
	CheckBuffer( mCurState->mBoard );
	CurColor = mCurState->NodeStates[at];

	if ( mCurState->NodeStates[at] == EMPTY )
		return;

	subFindConnected(at);

	int to=0;
	for (int i=0; i<CurSize; i++)
	{
		if ( mCurState->NodeStates[Buffer[i]] != EMPTY )
		{
			Buffer[to] = Buffer[i];
			to++;
		}
	}
	CurSize = to;
}

int AddIndexPair(ResMesh* mesh, int curinds, int from, int to)
{
	Index* inds = mesh->Indices.Raw();
	for (int i=0; i<curinds; i+=2)
	{
		if ((inds[i]==to) && (inds[i+1]==from))
			return curinds;
	}
	inds[curinds] = from;
	inds[curinds+1] = to;
	return curinds+2;
}

Color4f GameColors[TYPES] = {
	{ 1.0, 1.0, 1.0, 1.0 },
	{ 0.45, 0.45, 0.45, 1.0 },
	{ 1.0, 0.5, 0.0, 1.0 },
};
char ColorNames[TYPES][20] = {
	"White",
	"Black",
	"Empty",
};

struct HitToken
{
	GoNode* Node;
};

#define HOSTPLAYER	BLACK

#define GPT_Move		10
#define GPT_Undo		11
#define GPT_Pass		12
#define GPT_RequestUndo	13
#define GPT_NewGame		14
#define GPT_RequestNew	15
#define GPT_BusyPacket	16
#define GPT_NewBoard	17
#define GPT_RequestBoard 18
#define GPT_Remove		19

#define GP_PLEASE			20
#define GP_DENIED			21
#define GP_SYSPLEASE		22
#define GP_UNSUPPORTED		23
#define GP_SYSUNSUPPORTED	24

struct GoPacket
{
	int Type;
	int Extra;
};

class Game : public GenRes
{
public:
	Board* mBoard;
	BoardState* mCurState;
	BoardState mBackBuff;
	ResCompNode* mNodes;
	uint mSolidContext, mWireContext;
	ResScaleTransform *mSmallScaler, *mBigScaler;
	int CurTurn;
	bool GameOver;
	bool IsTwoPlayer;
	bool RemoveDeadMode;
	int Scores[2];
	GoRules mRules;

	int mNetPlayer;
	bool mIsNetGame;
	Net_Transport mTrans;

	GoNode* mPlayers[2];

	void SwitchBoard(char* name, bool userclicked);
	void Init(ResCompNode* parent, Board* board, uint tris, uint wires);
	void NewGame(bool userclick);
	void Clicked(int at, bool userclick);
	void Undo(bool userclick, bool accepted);
	void EndGame();
	void Pass(bool userclick);
	void RandomBoard(int m);
	bool CheckNetwork();
	void SendPacket(int type, int extra);
	void SendPacket(int type, int extra, char* text);

	void InitRemoteGame();

	RESVISIT( RESMEM(mNodes) RESMEM(mSmallScaler) RESMEM(mBigScaler) );

	Game();
	virtual ~Game() {};

	static Game* Create();

private:
	BArray<BoardDelta> Deltas;
	BoardState _priState;

	bool CheckTurn(bool userclick);
	void UpdateSingleColor(int i);
	void UpdateColors();

	void GenerateBoardCompTree(Board* b, ResCompNode* parent, uint tris, uint wires);
};

char game_textbuffer[400];
char game_textbuffer2[400];

bool Game::CheckTurn(bool userclick)
{
	if ((mIsNetGame) && (userclick))
	{
		if (CurTurn != mNetPlayer)
		{
			ShowMessage("Not your turn", "Game Error");
			return false;
		}
	}
	return true;
}

char game_sendpacktext[400];
void Game::SendPacket(int type, int extra, char* str)
{
	char* data = &game_sendpacktext[0];
	GoPacket* pack = (GoPacket*)data;
	pack->Type = type;
	pack->Extra = extra;

	int len = strlen(str)+2;
	*((int*)(data+sizeof(GoPacket))) = len;
	strcpy( data+sizeof(GoPacket)+sizeof(int), str );

	int size = sizeof(GoPacket)+sizeof(int)+len;
	mTrans.Send(data, size);
}

void Game::SendPacket(int type, int extra)
{
	GoPacket packet;
	packet.Type = type;
	packet.Extra = extra;
	mTrans.Send( (char*)&packet, sizeof(GoPacket) );
}

bool Game::CheckNetwork()
{
	if (!mIsNetGame)
		return false;
	bool ret = false;

	while (mTrans.DataToRecieve())
	{
		ret = true;
		bool waserror;
		GoPacket pack;
		int len = mTrans.Recieve( (char*)&pack, sizeof(GoPacket), &waserror);
		if ((len <= 0) || (len != sizeof(GoPacket)))
		{
			if (len <= 0)
				ShowMessage("Connection Lost", "Game Over");
			else
				ShowMessage("Packet not the correct size\nConnection closed", "Net Error");
			mTrans.Close();
			mIsNetGame = false;
			return true;
		}
		//GoPacket* pack = (GoPacket*)&game_textbuffer[0];

		switch(pack.Type)
		{
		case GPT_BusyPacket:
			break;
		case GPT_Remove:
			{
				bool old = this->RemoveDeadMode;
				RemoveDeadMode = true;
				Clicked(pack.Extra, false);
				RemoveDeadMode = old;
			}
			break;
		case GPT_Move:
			Clicked(pack.Extra, false);
			break;
		case GPT_Undo:
			Undo(false, true);
			break;
		case GPT_Pass:
			Pass(false);
			ShowMessage("Opponent Passed", "Passed");
			break;
		case GPT_RequestUndo:
			{
				switch(pack.Extra)
				{
				case GP_PLEASE:
					{
						int res = MessageBox(0, 
							"Undo requested.\nDo you accept?", 
							"Undo Request", MB_YESNO);
						if (res == IDYES)
						{
							Undo(true, true);
						}
						else
							SendPacket(GPT_RequestUndo, GP_DENIED);
					}
					break;
				case GP_DENIED:
					ShowMessage("Undo request denied", "Denied");
					break;
				default:
					ShowMessage("Unknown request type", "Net Error");
					break;
				}
			}
			break;
		case GPT_NewGame:
			NewGame(false);
			break;
		case GPT_NewBoard:
			mTrans.RecieveString(game_textbuffer, 400);
			SwitchBoard(game_textbuffer, false);
			break;
		case GPT_RequestBoard:
			switch(pack.Extra)
			{
			case GP_DENIED:
				ShowMessage("New board request denied", "Denied");
				break;
			case GP_SYSUNSUPPORTED:
				SendPacket(GPT_RequestBoard, GP_SYSPLEASE, "mobius");
				break;
			case GP_UNSUPPORTED:
				ShowMessage("The other player doesn't support\nthat board type", "Board error");
				break;
			case GP_PLEASE:
			case GP_SYSPLEASE:
				{
					mTrans.RecieveString(game_textbuffer, 400);
					if (!GenBoardFromName(game_textbuffer, 0, true))
					{
						int t = GP_UNSUPPORTED;
						if (pack.Extra == GP_SYSPLEASE)
							t = GP_SYSUNSUPPORTED;
						SendPacket(GPT_RequestBoard, t);
					}
					else
					{
						int res = IDNO;
						if (pack.Extra==GP_SYSPLEASE)
						{
							res = IDYES;
						}
						else
						{
							sprintf(game_textbuffer2,
								"Switch to %s board requested.\nDo you accept?",
								game_textbuffer);
							res = MessageBox(0, 
								game_textbuffer2, 
								"Board Request", MB_YESNO);
						}
						if (res == IDYES)
						{
							SwitchBoard(game_textbuffer, false);
							SendPacket(GPT_NewBoard, 0, game_textbuffer);
						}
						else
							SendPacket(GPT_RequestBoard, GP_DENIED);
					}
				}
				break;
			}
			break;
		case GPT_RequestNew:
			{
				switch (pack.Extra)
				{
				case GP_SYSPLEASE:
					NewGame(false);
					break;
				case GP_PLEASE:
					{
						int res = MessageBox(0, 
							"An new game was requested,\ndo you accept?", 
							"Undo Request", MB_YESNO);
						if (res == IDYES)
						{
							NewGame(false);
							SendPacket(GPT_NewGame, 0);
						}
						else
						{
							SendPacket(GPT_RequestNew, GP_DENIED);
						}
					}
					break;
				case GP_DENIED:
					ShowMessage("New game requst denied!", "Denied");
					break;
				default:
					ShowMessage("Unknown request type", "Net Error");
					break;
				}
			}
			break;
		default:
			ShowMessage("Unknown Packet Type", "Net Error");
			break;
		}
	}

	return ret;
}

void Game::InitRemoteGame()
{
	mIsNetGame = true;
	if (mTrans.mIsHost)
		mNetPlayer = HOSTPLAYER;
	else
		mNetPlayer = OTHERTURN(HOSTPLAYER);

	if (mTrans.mIsHost)
	{
		SendPacket(GPT_RequestBoard, GP_SYSPLEASE, mBoard->Name);
	}
	//NewGame(false);
}

Game::Game()
{
	 mBoard = 0; 
	 RemoveDeadMode=false; 
	 mSmallScaler=0; 
	 mBigScaler=0; 
	 mNodes = 0;
	 IsTwoPlayer=true; 
	 mIsNetGame = false;
	 mCurState = new BoardState;

	 //Net_InitNetworking();
}

void Game::RandomBoard(int m)
{
	for (int i=0; i<mBoard->NumNodes; i++)
	{
		mCurState->NodeStates[i] = (rand() % m);
	}
	UpdateColors();
}

Game* Game::Create()
{
	Game* g = new Game();
	Core.Register(g);
	Core.mGameRes = g;
	return g;
}

void Game::EndGame()
{
	int white, black, free;
	mRules.LandStats(mCurState, &white, &black, &free);

	int twhite = Scores[WHITE]+white;
	int tblack = Scores[BLACK]+black;

	sprintf(game_textbuffer, "White: %d Land + %d Dead = %d\n", white, Scores[WHITE], twhite);
	sprintf(game_textbuffer2, "%sBlack: %d Land + %d Dead = %d\n", game_textbuffer, black, Scores[BLACK], tblack);
	sprintf(game_textbuffer, "%sFree Land: %d\n", game_textbuffer2, free);

	if (twhite != tblack)
	{
		int wins = ((twhite > tblack)? WHITE : BLACK);
		sprintf(game_textbuffer2, "%s\n%s Wins!", game_textbuffer, ColorNames[wins]);
	}
	else
	{
		sprintf(game_textbuffer2, "%s\nTie!", game_textbuffer);
	}

	ShowMessage( game_textbuffer2, "Game Results:");
}

void Game::Undo(bool userclick, bool accepted)
{
	if ((mIsNetGame) && (userclick))
	{
		if (CurTurn != mNetPlayer)
		{
			SendPacket(GPT_RequestUndo, GP_PLEASE);
			ShowMessage("An Undo request was sent", "Undo");
			return;
		}
		else
		{
			if (!accepted)
			{
				ShowMessage("You cannot undo another players move", "Invalid Undo");
				return;
			}
		}
	}

	BoardDelta* del = Deltas.StackPeak();
	if (!del)
		return;
	GameOver = false;
	CurTurn = del->UndoEffect( mCurState );
	for (int i=0; i<del->NumDeltas; i++)
	{
		if (del->Deltas[i].To == EMPTY)
		{
			Scores[OTHERTURN(del->Deltas[i].From) ]--;
		}
		UpdateSingleColor( del->Deltas[i].Index );
	}
	Deltas.StackPop();

	if ((mIsNetGame)&&(userclick))
	{
		SendPacket(GPT_Undo, 0);
	}
}

void Game::Pass(bool userclick)
{
	if (!CheckTurn(userclick))
		return;

	int lt = CurTurn;
	CurTurn = OTHERTURN(CurTurn);

	BoardDelta* bd = Deltas.StackPush();
	bd->Init(lt, mCurState, CurTurn, mCurState);

	if ((mIsNetGame)&&(userclick))
	{
		SendPacket(GPT_Pass, 0);
	}
}

void Game::Clicked(int at, bool userclick)
{
	if (RemoveDeadMode)
	{
		if ((mIsNetGame) && (userclick))
		{
			if (mCurState->NodeStates[at] == OTHERTURN(mNetPlayer))
			{
				ShowMessage("Can't remove other players stones", "Invalid Removal");
				return;
			}
		}

	}
	else
	{
		if (!CheckTurn(userclick))
			return;
	}

	int lt = CurTurn;
	mBackBuff.Init( mCurState );
	BoardDelta* bd;

	if (!RemoveDeadMode)
	{
		char* err = mRules.MakeMove(mCurState, at, CurTurn);
		if (err)
		{
			ShowMessage(err, "Invalid Move");
			return;
		}

		bd = Deltas.StackPush();
		bd->Init(lt, &mBackBuff, CurTurn, mCurState);
		if (Deltas.Length() > 1)
		{
			BoardDelta* prev = Deltas.Axs( 1 );
			if (bd->CancelEachOther(prev))
			{
				Deltas.StackPop();
				ShowMessage("KO Violation", "Invalid Move");
				mCurState->Init( &mBackBuff );
				return;
			}
		}

		CurTurn = OTHERTURN(CurTurn);
	}
	else
	{
		if (mCurState->NodeStates[at] == EMPTY)
		{
			ShowMessage("Can't remove empty stones.", "Invalid Removal");
			return;
		}
		mCurState->NodeStates[at] = EMPTY;

		bd = Deltas.StackPush();
		bd->Init(lt, &mBackBuff, CurTurn, mCurState);
	}


	for (int i=0; i<bd->NumDeltas; i++)
	{
		if (bd->Deltas[i].To == EMPTY)
		{
			Scores[OTHERTURN(bd->Deltas[i].From) ]++;
		}
		UpdateSingleColor( bd->Deltas[i].Index );
	}

	if ((mIsNetGame)&&(userclick))
	{
		if (RemoveDeadMode)
			SendPacket(GPT_Remove, at);
		else
			SendPacket(GPT_Move, at);
	}
}

void Game::NewGame(bool userclick)
{
	if ((mIsNetGame) && (userclick))
	{
		SendPacket(GPT_RequestNew, GP_PLEASE);
		ShowMessage("New game request sent", "Sent");
		return;
	}

	GameOver = false;
	CurTurn = BLACK;
	RemoveDeadMode = false;
	Scores[0] = 0;
	Scores[1] = 0;
	Deltas.DelAll();

	for (int i=0; i<mBoard->NumNodes; i++)
	{
		mCurState->NodeStates[i] = EMPTY;
	}

	UpdateColors();
}

MatrixStruct game_mone, game_mtwo, game_mthree;
void Game::UpdateSingleColor(int i)
{
	ResCompNode* cnode = mNodes->mChildren[i];
	if (!cnode->mColor)
		cnode->mColor = ResColor::Create();
	cnode->mColor->mValue = GameColors[ mCurState->NodeStates[i] ];
	cnode->mColor->Changed();

	if (mCurState->NodeStates[i] == EMPTY)
		cnode->mTransform = mSmallScaler;
	else
		cnode->mTransform = mBigScaler;

	GoNode* node = &mBoard->Nodes[i];
	node->Normal.Normalize();
	if (fabs(node->Normal.z) < 0.90)
	{
		if (fabs(node->Normal.y) < 0.1)
		{
			SetMatrix_RotateY(game_mthree, 90+RadToDeg(atan2(node->Normal.z, node->Normal.x)));
		}
		else if (fabs(node->Normal.z) < 0.2)
		{
			SetMatrix_RotateY(game_mone, 90);
			float b = RadToDeg(atan2(node->Normal.y, node->Normal.x));
			SetMatrix_RotateZ(game_mtwo, b);
			MatrixTimesMatrix(game_mone, game_mtwo, game_mthree);
		}
		else
		{
			float rotx = -RadToDeg(atan2(node->Normal.y, node->Normal.z));
			if (node->Normal.z > 0)
				rotx *= -1;

			SetMatrix_RotateY(game_mone, RadToDeg(atan2(node->Normal.x, node->Normal.z)));
			SetMatrix_RotateX(game_mtwo, rotx);
			MatrixTimesMatrix(game_mtwo, game_mone, game_mthree);
		}
		cnode->mTransform->GetTransformMatrix(&game_mone);
		ResMatrixTransform* res = ResMatrixTransform::Create();
		MatrixTimesMatrix(game_mthree, game_mone, res->mMatrix);
		cnode->mTransform = res;
	}
}

void Game::UpdateColors()
{
	for (int i=0; i<mBoard->NumNodes; i++)
	{
		UpdateSingleColor(i);
		/*
		ResCompNode* n = mNodes->mChildren[i];
		if (!n->mColor)
		{
			n->mColor = ResColor::Create();
		}
		n->mColor->mValue = GameColors[ mCurState->NodeStates[i] ];
		n->mColor->Changed();
		*/
	}
}

void Game::GenerateBoardCompTree(Board* b, ResCompNode* parent, uint tris, uint wires)
{
	GoNode* node;
	ResCompNode* cnode;
	int degs = 0, i;
	parent->RemoveAllChildren();
	ResMesh* mesh = GenSphereMesh(3, 8);
	VertType* vt = (VertType*)mesh->VertData.Raw();
	for (i=0; i<mesh->NumVerts; i++)
	{
		vt[i].mVertex.z *= 0.5;
	}

	mBigScaler = ResScaleTransform::Create( b->NodeScale );
	mSmallScaler = ResScaleTransform::Create( b->NodeScale*0.5 );

	for (i=0; i<b->NumNodes; i++)
	{
		node = (*b)[i];
		cnode = ResCompNode::Create( tris );
		cnode->mMesh = mesh;
		parent->AddChild( cnode );
		degs += node->NumNeighbors;
		cnode->mPosition = ResPoint::Create( node->Position.x, node->Position.y, node->Position.z );
		HitToken* tok = new HitToken;
		tok->Node = node;
		cnode->mToken = tok;

		//UpdateSingleColor(i);

		/*
		float ang = AngleBetween(FPNT(0,0,1), node->Normal);
		//float ang = 45;
		ResMatrixTransform* res = ResMatrixTransform::Create();
		SetMatrix_RotateX(mone, ang);
		cnode->mTransform->GetTransformMatrix(&mtwo);
		MatrixTimesMatrix(mone, mtwo, res->mMatrix );
		cnode->mTransform = res;
		*/
	}

	ResMesh* frame = ResMesh::Create( 
		MeshFormat::Create(2, true), b->NumNodes, degs );
	cnode = ResCompNode::Create( wires );
	cnode->mMesh = frame;
	parent->AddChild( cnode );

	VertType* verts = (VertType*)frame->VertData.Raw();
	int c = 0;
	for (i=0; i<b->NumNodes; i++)
	{
		node = (*b)[i];
		verts[i].mVertex = node->Position;
		verts[i].mColor.Set( 1.0, 1.0, 1.0, 1.0 );
		for (int j=0; j<node->NumNeighbors; j++)
		{
			c = AddIndexPair(frame, c, i, node->Neighbors[j] );
		}
	}
	c=0;
}

void Game::SwitchBoard(char* name, bool userclicked)
{
	if ((mIsNetGame) && (userclicked))
	{
		SendPacket(GPT_RequestBoard, GP_PLEASE, name);
		ShowMessage("New board request sent", "Requested");
		return;
	}

	Board* board = new Board();
	if (!GenBoardFromName(name, board, false))
	{
		ShowMessage("Couldn't create that board", "Board Error");
		delete board;
		return;
	}
	Init( mNodes, board, mSolidContext, mWireContext );
}

void Game::Init(ResCompNode* parent, Board* board, uint tris, uint wires)
{
	if (mBoard)
	{
		delete mBoard;
	}
	mBoard = board;
	mCurState->Init( mBoard );
	mNodes = parent;
	mWireContext = wires;
	mSolidContext = tris;

	GenerateBoardCompTree(mBoard, mNodes, tris, wires );

	NewGame(false);
}

