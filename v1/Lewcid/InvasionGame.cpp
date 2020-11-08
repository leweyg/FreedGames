
#define SpecialRatio	0.9

#include "Board.cpp"

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

#define NumColors	6
Color4f GameColors[NumColors] = {
	{ 1.0, 0.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 1.0 },
	{ 0.0, 0.0, 1.0, 1.0 },
	{ 1.0, 0.0, 1.0, 1.0 },
	{ 1.0, 1.0, 0.0, 1.0 },
	{ 0.0, 1.0, 1.0, 1.0 },
};
char ColorNames[NumColors][20] = {
	"Red",
	"Green",
	"Dark Blue",
	"Purple",
	"Yellow",
	"Light Blue",
};

#define THINKS	4
#define OTHERTURN(X)	((X+1)%2)

class InvasionAI
{
public:
	int Think(BoardState* bs, int curturn);

private:
	int Favorite;
	byte BestColor;
	int BestScore;
	byte CurBaseColor;
	BoardState* InitialState;

	Board* mBoard;
	BoardState States[THINKS];
	BoardState Flags;

	int StartOf(int turn);
	int ScoreOf(BoardState* bs, int perspec, int* mescore=0, int* oscore=0);
	byte SubThink(BoardState* prevs, int level, int curturn, bool chosen);
	bool MakeMove(BoardState* b, int turn, byte color);

	BoardState* CurState;
	void subFill(int from, byte with);
	int subScore(int at);
};

int InvasionAI::StartOf(int turn)
{
	return ((turn==0)? mBoard->StartP0 : mBoard->StartP1 );
}

bool InvasionAI::MakeMove(BoardState* b, int turn, byte color)
{
	if (b->NodeStates[ mBoard->StartP0 ] == color )
		return false;
	if (b->NodeStates[ mBoard->StartP1 ] == color )
		return false;

	CurState = b;
	subFill( StartOf(turn), color );

	return true;
}

void InvasionAI::subFill(int from, byte with)
{
	byte prev = CurState->NodeStates[from];
	CurState->NodeStates[from] = with;
	GoNode* node = &mBoard->Nodes[from];
	for (int i=0; i<node->NumNeighbors; i++)
	{
		if (CurState->NodeStates[node->Neighbors[i]]==prev)
		{
			subFill( node->Neighbors[i], with );
		}
	}
}

int InvasionAI::subScore(int at)
{
	if (Flags[at])
		return 0;
	Flags[at] = 1;
	int score = 1;
	byte color = CurState->NodeStates[at];
	GoNode* node = &mBoard->Nodes[at];
	for (int i=0; i<node->NumNeighbors; i++)
	{
		if ( CurState->NodeStates[node->Neighbors[i]]==color )
			score += subScore(node->Neighbors[i]);
	}
	return score;
}

int InvasionAI::ScoreOf(BoardState* bs, int perspec, int* mescore, int* oscore)
{
	CurState = bs;
	for (int i=0; i<mBoard->NumNodes; i++)
	{
		Flags[i] = 0;
	}
	int me = subScore( StartOf(perspec) );
	int other = subScore( StartOf(OTHERTURN(perspec)) );

	if (mescore)
		*mescore = me;
	if (oscore)
		*oscore = other;

	return (me-other);
}

byte InvasionAI::SubThink(BoardState* prevs, int level, int curturn, bool chosen)
{
	BoardState* cs = &States[level];
	cs->Init( prevs );

	int lme, lother;
	int cme, cother;
	if (!chosen)
		ScoreOf(prevs, curturn, &lme, &lother);
	else
		ScoreOf(InitialState, curturn, &lme, &lother);

	byte bestcol = 200;
	int bestscore = -mBoard->NumNodes*10;

	if (level == THINKS-1)
	{
		for (byte col=0; col<NumColors; col++)
		{
			if (MakeMove(cs, curturn, col))
			{
				ScoreOf(cs, curturn, &cme, &cother);
				cme -= lme;
				int cur = ((cme) - (cother-lother));

				if (cur > bestscore)
				{
					bestscore = cur;
					bestcol = col;
				}
				cs->Init( prevs );
			}
		}

		if (chosen)
		{
			if (curturn != Favorite)
				bestscore *= -1;

			if (bestscore > BestScore)
			{
				BestScore = bestscore;
				BestColor = CurBaseColor;
			}
		}

		Assert(bestcol!=200);
		return bestcol;
	}

	for (byte col=0; col<NumColors; col++)
	{
		if (level==0)
			CurBaseColor = col;

		if (MakeMove(cs, curturn, col))
		{
			byte other = SubThink(cs, level+1, OTHERTURN(curturn), false);
			MakeMove(cs, OTHERTURN(curturn), other);

			ScoreOf(cs, curturn, &cme, &cother);
			cme -= lme;
			int cur = ((cme) - (cother-lother));

			if (cur > bestscore)
			{
				bestscore = cur;
				bestcol = col;
			}
			cs->Init( prevs );
		}
	}

	if ((chosen) || (level==1))
	{
		MakeMove(cs, curturn, bestcol);
		SubThink(cs, level+1, OTHERTURN(curturn), true);
	}

	Assert(bestcol!=200);
	return bestcol;
}

int InvasionAI::Think(BoardState* bs, int curturn)
{
	mBoard = bs->mBoard;
	InitialState = bs;
	Favorite = curturn;
	BestScore = -mBoard->NumNodes*10;
	BestColor = 200;

	Flags.Init( bs->mBoard );

	SubThink(bs, 0, curturn, (THINKS==1) );

	Assert(BestColor!=200);
	return BestColor;
}

struct HitToken
{
	int Type;
	union
	{
		byte Color;
		GoNode* Node;
	};
};

class Game : public GenRes
{
public:
	Board* mBoard;
	BoardState* mCurState;
	ResCompNode* mNodes;
	int CurTurn;
	bool GameOver;
	int ScoreZero, ScoreOne;
	InvasionAI mAI;
	bool IsTwoPlayer;

	ResScaleTransform* mPScales[3];

	GoNode* mPlayers[2];

	void Init(ResCompNode* parent, Board* board, uint tris, uint wires);
	void UpdateColors();
	void NewGame();
	void Clicked(byte color);
	void Undo();

	RESVISIT( RESMEM(mNodes) RESMEM(mPScales[0]) RESMEM(mPScales[1]) RESMEM(mPScales[2]) );

	Game() {IsTwoPlayer=false; mBoard = 0; mPScales[0]=0; mPScales[1]=0; mCurState = new BoardState;};
        virtual ~Game() {};

	static Game* Create();

private:
	BArray<BoardDelta> Deltas;
	BoardState _priState;

	void FillColor(int ind, byte tocolor);
	int FillTransform(int ind, ResScaleTransform* trans);
	void UpdateSingleColor(int i);
	void UpdateScales();
};

Game* Game::Create()
{
	Game* g = new Game();
	Core.Register(g);
	Core.mGameRes = g;
	return g;
}

void Game::UpdateScales()
{
	if (!mPScales[0])
	{
		ResFloat* anied = ResFloat::Create( 1.2 * mBoard->NodeScale );
		mPScales[0] = ResScaleTransform::Create( anied );
		mPScales[1] = ResScaleTransform::Create( 0.5 * mBoard->NodeScale );
		mPScales[2] = ResScaleTransform::Create( mBoard->NodeScale );

		ResAnimateFromTo::Create( anied, 
			ResFloat::Create( 1.1 * mBoard->NodeScale ),
			ResFloat::Create( 1.3 * mBoard->NodeScale ),
			0.65,
			LC_AniFlag_Bounce );
	}

	int i;
	for (i=0; i<mBoard->NumNodes; i++)
	{
		mNodes->mChildren[i]->mTransform = mPScales[2];
	}

	i = mPlayers[0]->Index;
	ScoreZero = FillTransform( this->mBoard->Nodes[i].Index, mPScales[CurTurn] );
//	mNodes->mChildren[i]->mTransform = mPScales[ CurTurn ];
	mNodes->mChildren[i]->mTransform->Changed();

	i = mPlayers[1]->Index;
	ScoreOne = FillTransform( this->mBoard->Nodes[i].Index, mPScales[ (CurTurn+1)%2 ] );
//	mNodes->mChildren[i]->mTransform = mPScales[ (CurTurn+1)%2 ];
	mNodes->mChildren[i]->mTransform->Changed();

	/*
	GameOver = true;
	for (int j=0; j<mBoard->NumNodes; j++)
	{
		if (mNodes->mChildren[j]->mTransform == mPScales[2])
			GameOver = false;
	}
	*/

	if ((ScoreZero + ScoreOne) == mBoard->NumNodes)
	{
		GameOver = true;
	}
}

char game_textbuffer[400];

void Game::Undo()
{
	BoardDelta* del = Deltas.StackPeak();
	if (!del)
		return;
	GameOver = false;
	CurTurn = del->UndoEffect( mCurState );
	for (int i=0; i<del->NumDeltas; i++)
	{
		UpdateSingleColor( del->Deltas[i].Index );
	}
	UpdateScales();
	mPScales[2]->Changed();
	Deltas.StackPop();
}

void Game::Clicked(byte color)
{
	byte tocol = color;

	if (GameOver)
	{
		ShowMessage("Game Over\nPress 'N' for a new game", "Game Over");
		return;
	}

	if (( (*mCurState)[mPlayers[0]->Index] == tocol )
		|| ((*mCurState)[mPlayers[1]->Index] == tocol ) )
	{
		ShowMessage("Already taken", "Invalid");
		return;
	}

	_priState.Init( mCurState );
	int ft = CurTurn;

	FillColor( mPlayers[CurTurn]->Index, tocol );

	if (!IsTwoPlayer)
	{
		FillColor( 
			mPlayers[OTHERTURN(CurTurn)]->Index, 
			mAI.Think(mCurState, OTHERTURN(CurTurn)) );
	}
	else
		CurTurn = ((CurTurn+1)%2);

	Deltas.StackPush()->Init(ft, &_priState, CurTurn, mCurState);

	UpdateScales();

	if (GameOver)
	{
		char* name0 = ColorNames[ mCurState->NodeStates[mBoard->StartP0] ];
		char* name1 = ColorNames[ mCurState->NodeStates[mBoard->StartP1] ];


		sprintf(game_textbuffer,"Player 1 (%s) had %d nodes and\nPlayer 2 (%s) had %d nodes", name0, ScoreZero, name1, ScoreOne);
		
		if (ScoreZero == ScoreOne)
			sprintf(EndGameMessage, "%s\n\nA Tie!", game_textbuffer);
		else
		{
			char* winner = ((ScoreZero > ScoreOne)? name0 : name1);
			sprintf(EndGameMessage, "%s\n\n%s Wins!", game_textbuffer, winner);
		}
	}
}

int Game::FillTransform(int ind, ResScaleTransform* trans)
{
	byte prev = mCurState->NodeStates[ind];
	if ( mNodes->mChildren[ind]->mTransform == trans )
		return 0;

	mNodes->mChildren[ind]->mTransform = trans;

	int sum=0;
	GoNode* node = &mBoard->Nodes[ind];
	for (int i=0; i<node->NumNeighbors; i++)
	{
		int j = node->Neighbors[i];
		if ((*mCurState)[j] == prev)
			sum += FillTransform( j, trans );
	}
	return sum+1;
}

void Game::FillColor(int ind, byte tocolor)
{
	byte prev = mCurState->NodeStates[ind];
	mCurState->NodeStates[ind] = tocolor;
	UpdateSingleColor( ind );
	GoNode* node = &mBoard->Nodes[ind];
	for (int i=0; i<node->NumNeighbors; i++)
	{
		int j = node->Neighbors[i];
		if ((*mCurState)[j] == prev)
			FillColor( j, tocolor );
	}
}

void Game::NewGame()
{
	GameOver = false;
	CurTurn = 0;
	Deltas.DelAll();

	byte last = 0;
	for (int i=0; i<mBoard->NumNodes; i++)
	{
		byte cur = (rand() % (NumColors+1));
		if (cur >= NumColors)
			cur = last;
		last = cur;
		mCurState->NodeStates[i] = cur;
	}
	while (mCurState->NodeStates[mBoard->StartP0] ==
		mCurState->NodeStates[mBoard->StartP1])
	{
		mCurState->NodeStates[mBoard->StartP0] = (rand() % NumColors);
	}
	UpdateScales();
	mPScales[2]->mFactor->Changed();
	UpdateColors();
}

void Game::UpdateSingleColor(int i)
{
	mNodes->mChildren[i]->mColor->mValue = GameColors[ mCurState->NodeStates[i] ];
	mNodes->mChildren[i]->mColor->Changed();
}

void Game::UpdateColors()
{
	for (int i=0; i<mBoard->NumNodes; i++)
	{
		ResCompNode* n = mNodes->mChildren[i];
		if (!n->mColor)
		{
			n->mColor = ResColor::Create();
		}
		n->mColor->mValue = GameColors[ mCurState->NodeStates[i] ];
	}
}

void GenerateBoardCompTree(Board* b, ResCompNode* parent, uint tris, uint wires)
{
	GoNode* node;
	ResCompNode* cnode;
	int degs = 0, i;
	parent->RemoveAllChildren();
	ResMesh* mesh = GenSphereMesh(4, 8);
	ResScaleTransform* scaler = ResScaleTransform::Create( b->NodeScale );

	for (i=0; i<b->NumNodes; i++)
	{
		node = (*b)[i];
		cnode = ResCompNode::Create( tris );
		cnode->mMesh = mesh;
		cnode->mTransform = scaler;
		HitToken* tok = new HitToken;
		tok->Type = 1;
		tok->Node = node;
		cnode->mToken = tok;

		cnode->mPosition = ResPoint::Create( node->Position.x, node->Position.y, node->Position.z );
		parent->AddChild( cnode );

		degs += node->NumNeighbors;
	}

	parent->mChildren[b->StartP0]->mTransform = ResScaleTransform::Create( b->NodeScale );
	parent->mChildren[b->StartP1]->mTransform = ResScaleTransform::Create( b->NodeScale );

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

void Game::Init(ResCompNode* parent, Board* board, uint tris, uint wires)
{
	if (mBoard)
	{
		delete mBoard;
		delete mCurState;
		mCurState = new BoardState();
	}
	mBoard = board;
	mCurState->Init( mBoard );
	mNodes = parent;

	GenerateBoardCompTree(mBoard, mNodes, tris, wires );
	mPScales[0] = 0;

	mPlayers[0] = (*board)[mBoard->StartP0];
	mPlayers[1] = (*board)[mBoard->StartP1];

	NewGame();
}

