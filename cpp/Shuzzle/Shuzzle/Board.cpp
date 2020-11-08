
//General Arbitary Topological Go Board class

#ifndef SpecialRatio
#define SpecialRatio 1.0
#endif

#include <iostream>
#include <fstream>
//#include <iostream.h>

char EndGameMessage[200] = {0};

struct GoNode
{
	int Index;
	int* Neighbors;
	int NumNeighbors;
	fpnt Position;
	fpnt Normal;

	void Init(int n) {NumNeighbors=n; Neighbors= new int[n];};
	void AddNeighbor(int ni);
	void SafeAddNeighbor(int ni);
	int& operator[] (int i) {return Neighbors[i];};
};

void GoNode::SafeAddNeighbor(int ni)
{
	for (int i=0; i<NumNeighbors; i++)
	{
		if (Neighbors[i] == ni)
			return;
	}
	AddNeighbor(ni);
}

void GoNode::AddNeighbor(int ni)
{
	int* nr = new int [NumNeighbors+1];
	for (int i=0; i<NumNeighbors; i++)
	{
		nr[i] = Neighbors[i];
	}
	nr[NumNeighbors] = ni;
	NumNeighbors++;

	delete [] Neighbors;
	Neighbors = nr;
}

class Board
{
public:
	GoNode* Nodes;
	char* Name;
	int NumNodes;
	double NodeScale;
	int StartP0, StartP1;

	void InitName(char* name);
	void Init(int size);
	void UnInit() 
	{
		NumNodes=0; 
		if (Nodes) 
			delete [] Nodes; 
		if (Name) 
			delete[]Name; 
		Name=0; 
		Nodes = 0;
	};
	GoNode* operator[] (int i) {return &Nodes[i];};

	Board() {Nodes=0; NumNodes=0; NodeScale=1.0f; Name=0;};
	~Board() {UnInit();};
};

void Board::InitName(char* name)
{
	if (Name)
	{
		delete [] Name;
	}
	int len = strlen(name)+2;
	Name = new char[len];
	strcpy(Name, name);
}

void Board::Init(int size)
{
	UnInit();
	NumNodes = size;
	Nodes = new GoNode[size];
	for (int i=0; i<size; i++)
	{
		Nodes[i].Index = i;
	}
}

class BoardState
{
public:
	Board* mBoard;
	byte* NodeStates;

	void Init(Board* board);
	void Init(BoardState* other);
	void UnInit();
	byte& operator[] (int i) {return NodeStates[i];};

	BoardState() {mBoard=0; NodeStates=0;};
	~BoardState() {UnInit();};
};

struct NodeDelta
{
	int Index;
	byte From, To;
};

class BoardDelta
{
public:
	NodeDelta* Deltas;
	int NumDeltas;
	int TurnFrom, TurnTo;

	void Init(int turnfrom, BoardState* from, int turnto, BoardState* to);
	void UnInit() {if (Deltas) delete [] Deltas; Deltas=0; NumDeltas=0;};
	int UndoEffect(BoardState* bs);
	bool CancelEachOther(BoardDelta* other);

	BoardDelta() {Deltas=0; NumDeltas=0;};
	~BoardDelta() {UnInit();};
};

bool BoardDelta::CancelEachOther(BoardDelta* other)
{
	if (NumDeltas != other->NumDeltas)
		return false;

	for (int i=0; i<NumDeltas; i++)
	{
		bool found = false;
		for (int j=0; (!found)&&(j<other->NumDeltas); j++)
		{
			if (Deltas[i].Index == other->Deltas[j].Index)
			{
				found = true;
				if ((Deltas[i].From != other->Deltas[j].To) ||
					(Deltas[i].To != other->Deltas[j].From))
				{
					return false;
				}
			}
		}
		if (!found)
			return false;
	}
	return true;
}

int BoardDelta::UndoEffect(BoardState* bs)
{
	for (int i=0; i<NumDeltas; i++)
	{
		(*bs)[ Deltas[i].Index ] = Deltas[i].From;
	}

	return TurnFrom;
}

void BoardDelta::Init(int turnfrom, BoardState* from, int turnto, BoardState* to)
{
	UnInit();
	TurnFrom = turnfrom;
	TurnTo = turnto;

	int i, ds=0, size=from->mBoard->NumNodes;
	for (i=0; i<size; i++)
	{
		if ( (*from)[i] != (*to)[i] )
			ds++;
	}

	NumDeltas = ds;
	Deltas = new NodeDelta[NumDeltas];
	ds=0;
	for (i=0; i<size; i++)
	{
		if ( (*from)[i] != (*to)[i] )
		{
			Deltas[ds].Index = i;
			Deltas[ds].From = (*from)[i];
			Deltas[ds].To = (*to)[i];
			ds++;
		}
	}
}

void BoardState::UnInit()
{
	if (NodeStates)
		delete [] NodeStates;
	NodeStates = null;
	mBoard = null;
}

void BoardState::Init(BoardState* other)
{
	Init(other->mBoard);
	for (int i=0; i<other->mBoard->NumNodes; i++)
	{
		NodeStates[i] = other->NodeStates[i];
	}
}

void BoardState::Init(Board* board)
{
	if (mBoard != board)
	{
		UnInit();
		mBoard = board;
		NodeStates = new byte[board->NumNodes];
	}
}





typedef void (*BoardCreateFunc)(Board* b, int hintw, int hinth);

void SetAllNormals(Board* b, fpnt normal)
{
	for (int i=0; i<b->NumNodes; i++)
	{
		b->Nodes[i].Normal = normal;
	}
}

void CreateBoard_Sphere(Board* board, int rings, int slices)
{
	board->Init( rings*slices + 2 );
	board->NodeScale = (1.95 / (slices-1));

	board->StartP0 = board->NumNodes-2;
	board->StartP1 = board->NumNodes-1;

	GoNode* node;
	int i;
	float scale, x, y, z, d;
	for (int r=0; r<rings; r++)
	{
		z = ((float)(r+1)) / ((float)(rings+1));
		z = ((z*2.0f)-1.0f);
		scale = sqrt( 1.0f - (z*z) );
		if (z >= 0)
		{
			z += (sqrt(z) - z)/4.0f;
		}
		else
		{
			z -= (sqrt(-z) + z)/4.0f;
		}

		for (int s=0; s<slices; s++)
		{
			node = (*board)[s + r*slices];

			node->Init(0);
			node->AddNeighbor( (r*slices) + ((s+1)%slices) );
			node->AddNeighbor( (r*slices) + ((s+slices-1)%slices) );
			if (r!=0)
				node->AddNeighbor( ((r-1)*slices) + s );
			if (r<rings-1)
				node->AddNeighbor( ((r+1)*slices) + s );

			d = ((float)s)/((float)(slices));
			d *= 2*Pi;
			x = cos(d)*scale;
			y = sin(d)*scale;
			node->Position.Set(x, z, y);

			node->Normal = node->Position;
		}
	}

	node = (*board)[board->NumNodes-2];
	node->Position.Set(0, -1, 0);
	node->Normal = node->Position;
	node->Init(0);
	for (i=0; i<slices; i++)
	{
		node->AddNeighbor( i );
		(*board)[i]->AddNeighbor( board->NumNodes-2 );
	}

	node = (*board)[board->NumNodes-1];
	node->Position.Set(0, 1, 0);
	node->Normal = node->Position;
	node->Init(0);
	for (i=0; i<slices; i++)
	{
		int v = ((rings-1)*slices) + i;
		node->AddNeighbor( v );
		(*board)[v]->AddNeighbor( board->NumNodes-1 );
	}
}

void CreateBoard_Torus(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	board->NodeScale = ((SpecialRatio*1.65) / ((float)hinth));

	board->StartP0 = 0;
	board->StartP1 = (hintw/2) + ((hinth/2)*hintw);

	GoNode* node;
	float ang, pitch;
	fpnt ringpos;
	fpnt offset;

	for (int y=0; y<hinth; y++)
	{
		for (int x=0; x<hintw; x++)
		{
			ang = Pi*2*(((float)x) / ((float)hintw));
			pitch = Pi*2.0*(((float)y) / ((float)hinth));
			ringpos.z = 0;
			ringpos.x = cos(ang);
			ringpos.y = sin(ang);

			offset.z = cos(pitch);
			offset.x = ringpos.x * sin(pitch);
			offset.y = ringpos.y * sin(pitch);
			offset *= 0.5;

			node = (*board)[x + y*hintw];
			node->Init(0);

			node->AddNeighbor( ((x+hintw-1)%hintw) + (y*hintw) );
			node->AddNeighbor( ((x+1)%hintw) + (y*hintw) );

			node->AddNeighbor( x + (((y+hinth-1)%hinth)*hintw) );
			node->AddNeighbor( x + (((y+1)%hinth)*hintw) );
			/*
			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );
			*/

			node->Position = ringpos + offset;
			node->Normal = offset;
		}
	}
}

void CreateBoard_Mobius(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	board->NodeScale = (0.75 / ((float)hinth));

	board->StartP1 = (hinth-1)*hintw;
	board->StartP0 = ((hinth-1)*hintw) + (hintw/2);

	GoNode* node;
	float ang, len, pitch;
	fpnt ringpos;
	fpnt offset;
	int x, y;

	for (y=0; y<hinth; y++)
	{
		len = ((float)y) / ((float)(hinth-1));
		len = (len*2)-1;

		for (x=0; x<hintw; x++)
		{
			ang = Pi*2*(((float)x) / ((float)hintw));
			pitch = Pi*1.0*(((float)x) / ((float)hintw));
			ringpos.z = 0;
			ringpos.x = cos(ang);
			ringpos.y = sin(ang);

			offset.z = cos(pitch);
			offset.x = ringpos.x * sin(pitch);
			offset.y = ringpos.y * sin(pitch);
			offset *= 0.5;

			node = (*board)[x + y*hintw];
			node->Init(0);

			if (x > 0)
				node->AddNeighbor( (x-1) + (y*hintw) );
			else
				node->AddNeighbor( (hintw-1) + ((hinth-y-1)*hintw) );

			if (x < hintw-1)
				node->AddNeighbor( (x+1) + (y*hintw) );
			else
				node->AddNeighbor( 0 + ((hinth-y-1)*hintw) );

			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );

			node->Position = ringpos + (offset * len);

			ringpos = ringpos.Cross( FPNT(0, 0, 1) );
			node->Normal = offset.Cross( ringpos );
			node->Normal.Normalize();
		}
	}

	for (y=0; y<hinth; y++)
	{
		for (x=0; x<hintw; x++)
		{
			GoNode* node = & board->Nodes[x + y*hintw];
			fpnt cen = node->Position;
			fpnt a = board->Nodes[node->Neighbors[0]].Position;
			fpnt b = board->Nodes[node->Neighbors[1]].Position;
			fpnt c = board->Nodes[node->Neighbors[2]].Position;
			
			c = (c-cen);
			a = ((a-cen).Cross(c));
			b = ((b-cen).Cross(c));
			if (a.Dot(b) < 0)
				a *= -1;
			node->Normal = a+b;
		}
	}
};

void CreateBoard_Cylinder(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	board->NodeScale = (1.0 / ((float)hinth));

	board->StartP0 = 0;
	board->StartP1 = ((hinth-1)*hintw) + (hintw/2);

	GoNode* node;
	float ang, len;
	fpnt ringpos;
	fpnt offset;

	for (int y=0; y<hinth; y++)
	{
		len = ((float)y) / ((float)(hinth-1));
		len = (len*2)-1;

		for (int x=0; x<hintw; x++)
		{
			ang = Pi*2*(((float)x) / ((float)hintw));
			ringpos.y = 0;
			ringpos.x = cos(ang)*0.8;
			ringpos.z = sin(ang)*0.8;
			offset.Set(0, 0.8, 0);

			node = (*board)[x + y*hintw];
			node->Init(0);

			node->AddNeighbor( ((x+hintw-1)%hintw) + (y*hintw) );
			node->AddNeighbor( ((x+1)%hintw) + (y*hintw) );
			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );

			node->Position = ringpos + (offset * len);
			node->Normal = ringpos;
		}
	}
};

void CreateBoard_HoneyComb6(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	if (hintw > hinth)
		board->NodeScale = (1.0 / ((float)hintw));
	else
		board->NodeScale = (1.0 / ((float)hinth));

	for (int i=0; i<board->NumNodes; i++)
	{
		board->Nodes[i].Init(0);
	}

	board->StartP0 = (hinth-1)*hintw;
	board->StartP1 = (hintw-1);
	GoNode* node, *other;

	for (int y=0; y<hinth; y++)
	{
		for (int x=0; x<hintw; x++)
		{
			node = (*board)[x + y*hintw];

				if (x!=0)
					node->AddNeighbor( (x-1) + (y*hintw) );
				if (x<hintw-1)
					node->AddNeighbor( (x+1) + (y*hintw) );

			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );

			if (((y%2)==0) && (y+1 < hinth) && (x > 0))
			{
				int io = (x-1) + ((y+1)*hintw);
				node->AddNeighbor( io );
				other = (*board)[ io ];
				other->AddNeighbor( x + (y*hintw) );
			}
			if (((y%2)==1) && (y < hinth-1) && (x < hintw-1))
			{
				int io = (x+1) + ((y+1)*hintw);
				node->AddNeighbor( io );
				other = (*board)[ io ];
				other->AddNeighbor( x + (y*hintw) );
			}

			node->Position.x = 2.0*(((double)x) / ((double)(hintw-1))) - 1.0;
			node->Position.y = 2.0*(((double)y) / ((double)(hinth-1))) - 1.0;
			node->Position.z = 0.0;
			if ((y%2) == 1)
				node->Position.x += 1.0 / ((float)(hintw-1));
		}
	}
	SetAllNormals(board, FPNT(0, 0, 1));
	node=0;
}

void CreateBoard_HoneyComb5(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	if (hintw > hinth)
		board->NodeScale = (1.0 / ((float)hintw));
	else
		board->NodeScale = (1.0 / ((float)hinth));

	for (int i=0; i<board->NumNodes; i++)
	{
		board->Nodes[i].Init(0);
	}

	board->StartP0 = (hinth-1)*hintw;
	board->StartP1 = (hintw-1);

	GoNode* node, *other;
	for (int y=0; y<hinth; y++)
	{
		for (int x=0; x<hintw; x++)
		{
			node = (*board)[x + y*hintw];

				if (x!=0)
					node->AddNeighbor( (x-1) + (y*hintw) );
				if (x<hintw-1)
					node->AddNeighbor( (x+1) + (y*hintw) );

			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );

			if ((((y-1)%4)==0) && (y < hinth-1) && (x > 0))
			{
				int io = (x-1) + ((y+1)*hintw);
				node->AddNeighbor( io );
				other = (*board)[ io ];
				other->AddNeighbor( x + (y*hintw) );
			}
			if ((((y-3)%4)==0) && (y < hinth-1) && (x < hintw-1))
			{
				int io = (x+1) + ((y+1)*hintw);
				node->AddNeighbor( io );
				other = (*board)[ io ];
				other->AddNeighbor( x + (y*hintw) );
			}

			node->Position.x = 2.0*(((double)x) / ((double)(hintw-1))) - 1.0;
			node->Position.y = 2.0*(((double)y) / ((double)(hinth-1))) - 1.0;
			node->Position.z = 0.0;
			if ((y%4) >= 2)
				node->Position.x += 1.0 / ((float)(hintw-1));
		}
	}
	SetAllNormals(board, FPNT(0, 0, 1));
	node=0;
}

void CreateBoard_HoneyComb3(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	if (hintw > hinth)
		board->NodeScale = (1.0 / ((float)hintw));
	else
		board->NodeScale = (1.0 / ((float)hinth));

	for (int i=0; i<board->NumNodes; i++)
	{
		board->Nodes[i].Init(0);
	}

	board->StartP0 = (hinth-1)*hintw;
	board->StartP1 = (hintw-1);

	GoNode* node, *other;
	for (int y=0; y<hinth; y++)
	{
		for (int x=0; x<hintw; x++)
		{
			node = (*board)[x + y*hintw];

			if ((y==0) || (y==hinth-1))
			{
				if (x!=0)
					node->AddNeighbor( (x-1) + (y*hintw) );
				if (x<hintw-1)
					node->AddNeighbor( (x+1) + (y*hintw) );
			}
			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );

			if ((((y-1)%4)==0) && (y < hinth-1) && (x > 0))
			{
				int io = (x-1) + ((y+1)*hintw);
				node->AddNeighbor( io );
				other = (*board)[ io ];
				other->AddNeighbor( x + (y*hintw) );
			}
			if ((((y-3)%4)==0) && (y < hinth-1) && (x < hintw-1))
			{
				int io = (x+1) + ((y+1)*hintw);
				node->AddNeighbor( io );
				other = (*board)[ io ];
				other->AddNeighbor( x + (y*hintw) );
			}

			node->Position.x = 2.0*(((double)x) / ((double)(hintw-1))) - 1.0;
			node->Position.y = 2.0*(((double)y) / ((double)(hinth-1))) - 1.0;
			node->Position.z = 0.0;
			if ((y%4) >= 2)
				node->Position.x += 1.0 / ((float)(hintw-1));
		}
	}
	SetAllNormals(board, FPNT(0, 0, 1));
	node=0;
}

void CreateBoard_Layered(Board* board, int hintw, int hinth)
{
	Board worker;
	CreateBoard_HoneyComb3(&worker, hintw, hinth);

	board->Init( worker.NumNodes*2 );
	board->NodeScale = worker.NodeScale;
	board->StartP0 = worker.StartP0;
	board->StartP1 = worker.StartP1+worker.NumNodes;

	int i;
	for (i=0; i<worker.NumNodes; i++)
	{
		GoNode* to = (*board)[i];
		GoNode* from = worker[i];

		to->Init(from->NumNeighbors+1);
		for (int j=0; j<from->NumNeighbors; j++)
		{
			to->Neighbors[j] = from->Neighbors[j];
		}
		to->Neighbors[to->NumNeighbors-1] = i + worker.NumNodes;

		to->Position = from->Position + FPNT(0, 0, 0.35);
	}
	for (i=0; i<worker.NumNodes; i++)
	{
		GoNode* to = (*board)[i+worker.NumNodes];
		GoNode* from = worker[i];

		to->Init(from->NumNeighbors+1);
		for (int j=0; j<from->NumNeighbors; j++)
		{
			to->Neighbors[j] = from->Neighbors[j] + worker.NumNodes;
		}
		to->Neighbors[to->NumNeighbors-1] = i;

		to->Position = from->Position + FPNT(0, 0, -0.35);
	}
	SetAllNormals(board, FPNT(0, 0, 1));

}

void CreateBoard_Cube(Board* board, int hintw, int hinth)
{
	int size = hintw;
	float scale = 0.75;
	board->Init( size*size*size );
	board->NodeScale = (scale / ((float)size));

	board->StartP0 = 0;
	board->StartP1 = ((size-1)*size*size) + ((size-1)*size) + (size-1);

	GoNode* node;
	for (int z=0; z<size; z++)
	{
		for (int y=0; y<size; y++)
		{
			for (int x=0; x<size; x++)
			{
				node = (*board)[z*size*size + y*size + x];
				node->Init(0);

				if (x!=0)
					node->AddNeighbor( (x-1) + y*size + z*size*size );
				if (x<size-1)
					node->AddNeighbor( (x+1) + y*size + z*size*size );

				if (y!=0)
					node->AddNeighbor( x + (y-1)*size + z*size*size );
				if (y<size-1)
					node->AddNeighbor( x + (y+1)*size + z*size*size );

				if (z!=0)
					node->AddNeighbor( x + y*size + (z-1)*size*size );
				if (z<size-1)
					node->AddNeighbor( x + y*size + (z+1)*size*size );

				node->Position.x = 2.0*(((double)x) / ((double)(size-1))) - 1.0;
				node->Position.y = 2.0*(((double)y) / ((double)(size-1))) - 1.0;
				node->Position.z = 2.0*(((double)z) / ((double)(size-1))) - 1.0;

				node->Position *= scale;
			}
		}
	}
	SetAllNormals(board, FPNT(0, 1, 0));
};

void CreateBoard_Standard(Board* board, int hintw, int hinth)
{
	board->Init( hintw * hinth );
	if (hintw > hinth)
		board->NodeScale = (1.0 / ((float)hintw));
	else
		board->NodeScale = (1.0 / ((float)hinth));

	board->StartP0 = (hinth-1)*hintw;
	board->StartP1 = (hintw-1);

	GoNode* node;
	for (int y=0; y<hinth; y++)
	{
		for (int x=0; x<hintw; x++)
		{
			node = (*board)[x + y*hintw];
			node->Init(0);

			if (x!=0)
				node->AddNeighbor( (x-1) + (y*hintw) );
			if (x<hintw-1)
				node->AddNeighbor( (x+1) + (y*hintw) );
			if (y!=0)
				node->AddNeighbor( x + ((y-1)*hintw) );
			if (y<hinth-1)
				node->AddNeighbor( x + ((y+1)*hintw) );

			node->Position.x = 2.0*(((double)x) / ((double)(hintw-1))) - 1.0;
			node->Position.y = 2.0*(((double)y) / ((double)(hinth-1))) - 1.0;
			node->Position.z = 0.0;
		}
	}
	SetAllNormals(board, FPNT(0, 0, 1));

};

void Dmd_IndexToLayerSide(int ind, int& sidelen, int& x, int& y)
{
	bool below=true;
	int layer=1;
	int area=layer*layer;
	//depth=0;
	while (ind >= area)
	{
		ind -= area;
		if (below)
			layer+=2;
		else
			layer-=2;

		Assert(layer>0);
		if (layer==7)
			below=false;
		area=layer*layer;
	}
	sidelen = layer;
	x = (ind % layer);
	y = (ind / layer);
}

Board* Dmd_Board;
GoNode* Dmd_FindNode(fpnt atafter)
{
	GoNode* node=0;
	for (int i=0; i<Dmd_Board->NumNodes; i++)
	{
		node = &Dmd_Board->Nodes[i];
		if ((node->Position.x==atafter.x) && (node->Position.y==atafter.y))
		{
			if (node->Position.z <= atafter.z)
			{
				return node;
			}
		}
	}
	return 0;
}

struct dmb_data
{
	GoNode* node;
	int dx;
};

void CreateBoard_Diamond(Board* board, int hintw, int hinth)
{
	int depth = 4, i;
	Dmd_Board = board;
	board->Init( 1+9+25+47+25+9+1 -12-12-8);
	board->StartP0 = 0;
	board->StartP1 = 1;

	int x, y, len;
	fpnt work;
	for (i=0; i<board->NumNodes; i++)
	{
		Dmd_IndexToLayerSide(i, len, x, y);
		work.x = ((float)x)-((float)(len>>1));
		work.y = ((float)y)-((float)(len>>1));
		work.z = -100;
		board->Nodes[i].Init(0);
		board->Nodes[i].Position = work;
	}

	int qsize=0, dx;
	fpnt delta;
	GoNode *node, *other;

	EArray<dmb_data> stack;
	dmb_data* data = stack.Add();
	data->node = &board->Nodes[0];
	data->dx = 1;
	data->node->Position.z = 1;
	while (qsize < stack.Size)
	{
		data = stack.Axs(qsize);
		qsize++;
		node = data->node;
		dx = data->dx;

		delta.Set( data->dx, ((data->dx+1)%2), -0.15 );

		other = Dmd_FindNode(node->Position+delta);
		if (other)
		{
			node->AddNeighbor( other->Index );
			other->AddNeighbor( node->Index );
			if (other->Position.z < -50)
			{
				data = stack.Add();
				data->node = other;
				data->dx = ((dx+1)%2);
				other->Position.z = node->Position.z + delta.z;
			}
		}

		delta.x *= -1;
		delta.y *= -1;
		other = Dmd_FindNode(node->Position+delta);
		if (other)
		{
			node->AddNeighbor( other->Index );
			other->AddNeighbor( node->Index );
			if (other->Position.z < -50)
			{
				data = stack.Add();
				data->node = other;
				data->dx = ((dx+1)%2);
				other->Position.z = node->Position.z + delta.z;
			}
		}
	}

	board->NodeScale = 0.15;
	fpnt scale = FPNT(0.3, 0.3, 1.3);
	for (i=0; i<board->NumNodes; i++)
	{
		work = board->Nodes[i].Position;
		work *= scale;
		float t = work.y;
		work.y = work.z;
		work.z = t;
		board->Nodes[i].Position = work;
	}

	SetAllNormals(board, FPNT(0, 1, 0));
}

bool IsFile(char* filename)
{
    std::ifstream fin(filename, std::ios::in );// | std::ios::nocreate );
	bool found = (fin.is_open()!=0);
	fin.close();
	return found;
}

bool CreateBoard_FromFile(Board* board, char* filename)
{
    std::ifstream fin(filename, std::ios::in ); //| std::ios::nocreate );
	if (!fin.is_open())
	{
		return false;
	}
	board->StartP0=0;
	board->StartP1=0;
	int numn, c, i;
	GoNode* node;
	fin >> numn >> board->NodeScale;
	board->Init(numn);
	for (i=0; i<numn; i++)
	{
		board->Nodes[i].Init(0);
	}
	for (i=0; i<numn; i++)
	{
		node = &board->Nodes[i];
		fin >> c;
		for (int j=0; j<c; j++)
		{
			int n;
			fin >> n;
			node->SafeAddNeighbor(n);
			board->Nodes[n].SafeAddNeighbor( node->Index );
		}
		fin >> node->Position.x >> node->Position.y >> node->Position.z;
	}
	fin.close();
	board->InitName(filename);
	SetAllNormals(board, FPNT(0, 0, 1));

	return true;
}


bool GenBoardFromName(char* name, Board* board, bool onlytest)
{
	int hintw, hinth;

#define GenBoard_NB(F)	{if(onlytest){return true;}else{F(board,hintw,hinth);board->InitName(name);}}

	if(strcmp(name, "diamond")==0) {
		hintw = 4;
		hinth = 4;
		GenBoard_NB( CreateBoard_Diamond );
		return true; }
	if(strcmp(name, "flat")==0) {
		hintw = 9;
		hinth = 9;
		GenBoard_NB( CreateBoard_Standard );
		return true; }
	if(strcmp(name, "sphere")==0) {
		hintw = 7;
		hinth = 15;
		GenBoard_NB( CreateBoard_Sphere );
		return true; }
	if(strcmp(name, "mobius")==0) {
		hintw = 13;
		hinth = 6;
		GenBoard_NB( CreateBoard_Mobius );
		return true; }
	if(strcmp(name, "torus")==0) {
		hintw = 13;
		hinth = 13;
		GenBoard_NB( CreateBoard_Torus );
		return true; }
	if(strcmp(name, "3")==0) {
		hintw = 9;
		hinth = 10;
		GenBoard_NB( CreateBoard_HoneyComb3 );
		return true; }
	if(strcmp(name, "5")==0) {
		hintw = 9;
		hinth = 10;
		GenBoard_NB( CreateBoard_HoneyComb5 );
		return true; }
	if(strcmp(name, "6")==0) {
		hintw = 9;
		hinth = 10;
		GenBoard_NB( CreateBoard_HoneyComb6 );
		return true; }
	if(strcmp(name, "layered")==0) {
		hintw = 9;
		hinth = 10;
		GenBoard_NB( CreateBoard_Layered );
		return true; }
	if(strcmp(name, "cylinder")==0) {
		hintw = 13;
		hinth = 9;
		GenBoard_NB( CreateBoard_Cylinder );
		return true; }
	if(strcmp(name, "box")==0) {
		hintw = 5;
		hinth = 5;
		GenBoard_NB( CreateBoard_Cube );
		return true; }
	if (onlytest)
	{
		return IsFile(name);
	}
	else
	{
		return CreateBoard_FromFile(board, name);
	}

	return false;
}
