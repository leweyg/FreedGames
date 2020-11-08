
//
//The Shuzzle Game Core
//

uint SolidRenderContext;
uint WireRenderContext;

int FloatToInt(float f)
{
	if (f > 0.5)
		return 1;
	if (f < -0.5)
		return -1;
	return 0;
}

ipnt IPntTimesFloatMatrix4(ipnt vec, float * mat)
{
	ipnt to;
	int x, y;
	for (x=0; x!=3; x++)
	{
		//CHANGED THIS 4/15/2003 from "to[x]=0"
		to[x]= mp(mat, x, 3);
		for (y=0; y!=3; y++)
			to[x] += (FloatToInt(mp(mat,x,y)) * ((float)vec[y]));
	}
	return to;
}

struct Rect3
{
	ipnt Min, Max;

	bool Includes(ipnt pos);
	void AddBoundedPoint(ipnt pos);
	void Zero();
	void Widen(int by);
	fpnt Clip(fpnt val);
	void Set(ipnt min, ipnt max) {Min=min; Max=max;};
	void operator += (ipnt offset);
	ResMesh* GenMesh();
};

fpnt Rect3::Clip(fpnt val)
{
	if (val.x < Min.x)
		val.x = Min.x;
	if (val.x > Max.x)
		val.x = Max.x;

	if (val.y < Min.y)
		val.y = Min.y;
	if (val.y > Max.y)
		val.y = Max.y;

	if (val.z < Min.z)
		val.z = Min.z;
	if (val.z > Max.z)
		val.z = Max.z;

	return val;
}

ResMesh* Rect3::GenMesh()
{
	ResMesh* ans = ResMesh::Create(MeshFormat::Create(4, false), 6*4, 0);
	VertType* verts = (VertType*)ans->VertData.Raw();

	int to, top, bot;

	bot=0;
	verts[bot+0].mVertex.Set( Min.x, Min.y, Min.z );
	verts[bot+1].mVertex.Set( Min.x, Max.y, Min.z );
	verts[bot+2].mVertex.Set( Max.x, Max.y, Min.z );
	verts[bot+3].mVertex.Set( Max.x, Min.y, Min.z );

	top=4;
	verts[top+0].mVertex.Set( Min.x, Min.y, Max.z );
	verts[top+1].mVertex.Set( Max.x, Min.y, Max.z );
	verts[top+2].mVertex.Set( Max.x, Max.y, Max.z );
	verts[top+3].mVertex.Set( Min.x, Max.y, Max.z );

	to = 8;
	verts[to+0].mVertex = verts[bot+0].mVertex;
	verts[to+3].mVertex = verts[bot+1].mVertex;
	verts[to+2].mVertex = verts[top+3].mVertex;
	verts[to+1].mVertex = verts[top+0].mVertex;

	to = 12;
	verts[to+0].mVertex = verts[bot+3].mVertex;
	verts[to+1].mVertex = verts[bot+2].mVertex;
	verts[to+2].mVertex = verts[top+2].mVertex;
	verts[to+3].mVertex = verts[top+1].mVertex;

	to = 16;
	verts[to+0].mVertex = verts[bot+1].mVertex;
	verts[to+3].mVertex = verts[bot+2].mVertex;
	verts[to+2].mVertex = verts[top+2].mVertex;
	verts[to+1].mVertex = verts[top+3].mVertex;

	to = 20;
	verts[to+0].mVertex = verts[bot+0].mVertex;
	verts[to+1].mVertex = verts[bot+3].mVertex;
	verts[to+2].mVertex = verts[top+1].mVertex;
	verts[to+3].mVertex = verts[top+0].mVertex;

	PaintMesh(ans, Color4f::Create(0.5, 1, 0.5));
	return ans;
}

void Rect3::Widen(int by)
{
	ipnt b = IPNT(by, by, by);
	Min -= b;
	Max += b;
}

bool Rect3::Includes(ipnt pos)
{
	if ((pos.x < Min.x) || (pos.x > Max.x))
		return false;
	if ((pos.y < Min.y) || (pos.y > Max.y))
		return false;
	if ((pos.z < Min.z) || (pos.z > Max.z))
		return false;
	return true;
}

void Rect3::operator +=(ipnt offset)
{
	Min += offset;
	Max += offset;
}

void Rect3::Zero()
{
	Min.Set(0,0,0);
	Max.Set(0,0,0);
}

void Rect3::AddBoundedPoint(ipnt pos)
{
	if (pos.x<Min.x)
		Min.x = pos.x;
	if (pos.y<Min.y)
		Min.y = pos.y;
	if (pos.z<Min.z)
		Min.z = pos.z;

	if (pos.x>Max.x)
		Max.x = pos.x;
	if (pos.y>Max.y)
		Max.y = pos.y;
	if (pos.z>Max.z)
		Max.z = pos.z;
}

class Board;

class Block
{
public:
	ipnt mCenter;
	int mNumBoxes;
	int mIndex;
	Board* mBoard;
	ipnt* mBoxes;
	Rect3 mBounds;
	ResCompNode* mNode;

	void Init(int nboxes);
	void Init(Rect3* r);
	void UnInit();
	void FindBounds();
	int BoxIndex(ipnt at);
	bool Includes(ipnt at) {return (BoxIndex(at)!=-1);};

	bool Spin(bool isright);
	bool Flip(int overaxis, bool isover);
	bool Shift(ipnt offset);

	void Show();

	ResMesh* GenOutlineMesh();
	ResCompNode* GenCompNode(ResMesh* cubemesh, uint context);

	Block() {mBoxes=0; mNumBoxes=0; mCenter=IPNT(0,0,0);};
	~Block() {UnInit();};

private:
	bool CanApplyMatrix(MatrixStruct* mat);
	void ApplyMatrix(MatrixStruct* mat);
};

void Block_AniDone(void* data);

void Block::ApplyMatrix(MatrixStruct* mat)
{
	for (int i=0; i<mNumBoxes; i++)
	{
		mBoxes[i] = IPntTimesFloatMatrix4(mBoxes[i], *mat);
	}
	FindBounds();
}

int Block::BoxIndex(ipnt at)
{
	if (!mBounds.Includes(at))
		return -1;
	at -= mCenter;
	for (int i=0; i<mNumBoxes; i++)
	{
		if (mBoxes[i] == at)
			return i;
	}
	return -1;
}

MatrixStruct BK_Mat1, BK_Mat2;
bool Block::Flip(int overaxis, bool isover)
{
	float ang = 90.0f * ((isover)? -1.0f : 1.0f);
	int axis;
	if (overaxis==0)
	{
		SetMatrix_RotateX(BK_Mat1, ang);
		axis = 0;
	}
	else
	{
		SetMatrix_RotateY(BK_Mat1, ang);
		axis = 1;
	}
	if (!CanApplyMatrix(&BK_Mat1))
		return false;
	ResTransformCollection* tc = (ResTransformCollection*)mNode->mTransform;
	if (tc->Count() > 1)
		return false;
	ApplyMatrix(&BK_Mat1);

	ResMatrixTransform* t = (ResMatrixTransform*)tc->Get(0);
	MatrixTimesMatrix(t->mMatrix, BK_Mat1, BK_Mat2);
	t->mMatrix = BK_Mat2;
	t->Changed();

	ResSpinTransform* rt = ResSpinTransform::Create(axis, 0.0f);
	tc->Add(rt);
	ResAnimateFromTo* ani = ResAnimateFromTo::Create(rt->mAngle, ResFloat::Create(-ang), ResFloat::Create(0), 
		0.3f, LC_AniFlag_StopWhenDone);
	ani->mToken = this;
	ani->mEvent_WhenDone = Block_AniDone;

	return true;
}

bool Block::Spin(bool isright)
{
	float ang = 90.0f*((isright)? 1.0f : -1.0f );
	SetMatrix_RotateZ(BK_Mat1, ang );
	if (!CanApplyMatrix(&BK_Mat1))
		return false;
	ResTransformCollection* tc = (ResTransformCollection*)mNode->mTransform;
	if (tc->Count() > 1)
		return false;
	ApplyMatrix(&BK_Mat1);

	ResMatrixTransform* t = (ResMatrixTransform*)tc->Get(0);
	MatrixTimesMatrix(t->mMatrix, BK_Mat1, BK_Mat2);
	t->mMatrix = BK_Mat2;
	t->Changed();

	ResSpinTransform* rt = ResSpinTransform::Create(2, 0.0f);
	tc->Add(rt);
	ResAnimateFromTo* ani = ResAnimateFromTo::Create(rt->mAngle, ResFloat::Create(-ang), ResFloat::Create(0), 
		0.3f, LC_AniFlag_StopWhenDone);
	ani->mToken = this;
	ani->mEvent_WhenDone = Block_AniDone;

	return true;
}

int Dirs[6][3] = {
	{ 1, 0, 0 },
	{ -1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, -1, 0 },
	{ 0, 0, 1 },
	{ 0, 0, -1 },
};

int CrossDir1(int i)
{
	return ((i+2)%6);
}

int CrossDir2(int i)
{
	return ((i+4)%6);
}

int ICrossProd(int a, int b)
{
	int mc = ((a%2) + (b%2))%2;
	int v = ( ((3-((a>>1) + (b>>1)))<<1) + mc);
	return v;
}

ResMesh* Block::GenOutlineMesh()
{
	ResMesh* mesh = ResMesh::Create(MeshFormat::Create(2, false), 0, 0);
	ipnt realcen = mCenter;
	mCenter = IPNT(0, 0, 0);

	fpnt work, del1, del2, del3;
	fpnt offset = ITOFPNT(realcen);
	FindBounds();
	int i;

	fpnt average = FPNT(0, 0, 0);
	int avcount=0;

	VertType* verts;
	for (i=0; i<mNumBoxes; i++)
	{
		ipnt cur = mBoxes[i];
		for (int j=0; j<6; j++)
		{
			bool filled = Includes(cur + &Dirs[j][0]);
			//if (!)
			{
				for (int k=0; k<6; k++)
				{
					if ((k-(k%2)) != (j-(j%2)))
					{
						if (Includes(cur + &Dirs[k][0])==filled)
						{
							bool check = true;
							if (filled)
							{
								if (Includes(cur + &Dirs[k][0] + &Dirs[j][0]))
									check=false;
							}
							if (check)
							{
							mesh->Add(2, 0);
							verts = (VertType*)mesh->RecentVertData(2);

							del1 = ITOFPNT( &Dirs[j][0] ) * 0.45;
							del2 = ITOFPNT( &Dirs[k][0] ) * 0.45;
							del3 = ITOFPNT( &Dirs[ICrossProd(j,k)][0] ) * 0.5;
							work = ITOFPNT( cur ) + offset;

							verts[0].mVertex = work+del1+del2+del3;
							verts[1].mVertex = work+del1+del2-del3;

							average += verts[0].mVertex;
							average += verts[1].mVertex;
							avcount++;
							}
						}
					}
				}
			}
		}
	}

	mCenter = realcen;
	FindBounds();

	/*
	offset = average * (1.0f/(float)avcount);

	verts = (VertType*)mesh->VertData.Raw();
	for (i=0; i<mesh->NumVerts; i++)
	{
		verts[i].mVertex -= offset;
		verts[i].mVertex *= 0.8;
		verts[i].mVertex += offset;
	}
	*/

	PaintMesh(mesh, Color4f::Create(1, 1, 1));
	mesh->UpdateNormals();

	return mesh;
}

ResCompNode* Block::GenCompNode(ResMesh* cubemesh, uint context)
{
	ResCompNode* parent = ResCompNode::Create(context);
	mNode = parent;
	ipnt pos = mCenter;
	parent->mPosition = ResPoint::Create( pos.x, pos.y, pos.z );
	parent->mToken = this;
	ResTransformCollection* tc = ResTransformCollection::Create();
	ResMatrixTransform* trans = ResMatrixTransform::Create();
	tc->Add(trans);
	parent->mTransform = tc;

	pos = mCenter;
	mCenter = IPNT(0, 0, 0);
	FindBounds();

	ResMesh*mesh = ResMesh::Create(MeshFormat::Create(4, false), 0, 0);
	VertType* verts;
	fpnt work, del1, del2, del3;
	for (int i=0; i<mNumBoxes; i++)
	{
		ipnt cur = mBoxes[i];
		for (int j=0; j<6; j++)
		{
			/*
			for (int dk=2; dk<=6; dk++)
			{
				int k = ((j-(j%2))+dk)%6;

				if ( (Includes(cur + &Dirs[j][0]))
					&& (Includes(cur + &Dirs[k][0])) )
				{
					mesh->Add(4, 0);
					verts = (VertType*)mesh->RecentVertData(4);

					work = ITOFPNT( &Dirs[j][0] );
					del2 = ITOFPNT( &Dirs[k][0] );
					ci = ICrossProd(j,k);
					del1 = ITOFPNT( &Dirs[ci][0] );

				//	if ( ((ipnt*)&Dirs[j][0])->Sum() > 0)
					{
						verts[0].mVertex = (work*0.5 + del2*0.4) + del2*0.4;
						verts[1].mVertex = (work*0.5 + del2*0.4) - del2*0.4;
						verts[2].mVertex = (work*0.4 + del2*0.5) - del2*0.4;
						verts[3].mVertex = (work*0.4 + del2*0.5) + del2*0.4;
					}
				//	else
					{
					}
				}
			}
			*/

			if (!Includes(cur + &Dirs[j][0]))
			{
				mesh->Add(4, 0);
				verts = (VertType*)mesh->RecentVertData(4);

				del1 = ITOFPNT( &Dirs[CrossDir1(j)][0] ) * 0.5;
				del2 = ITOFPNT( &Dirs[CrossDir2(j)][0] ) * 0.5;
				work = ITOFPNT( cur );
				work += ITOFPNT( &Dirs[j][0] ) * 0.5;

				if ( ((ipnt*)&Dirs[j][0])->Sum() > 0)
				{
					verts[0].mVertex = (work - del1) - del2;
					verts[1].mVertex = (work + del1) - del2;
					verts[2].mVertex = (work + del1) + del2;
					verts[3].mVertex = (work - del1) + del2;
				}
				else
				{
					verts[0].mVertex = (work - del1) - del2;
					verts[1].mVertex = (work - del1) + del2;
					verts[2].mVertex = (work + del1) + del2;
					verts[3].mVertex = (work + del1) - del2;
				}
			}
		}
	}
	PaintMesh(mesh, Color4f::FromRGB(0xffffff) );
	parent->mMesh = mesh;

	mCenter = pos;
	FindBounds();

	/*
	ResCompNode* work;
	for (int i=0; i<mNumBoxes; i++)
	{
		pos = mBoxes[i];
		work = ResCompNode::Create(context);
		parent->AddChild(work);
		work->mMesh = cubemesh;
		work->mTransform = ResScaleTransform::Create(0.5f);
		work->mPosition = ResPoint::Create( pos.x, pos.y, pos.z );
		work->mToken = this;
	}
	*/

	return parent;
}

void Block::FindBounds()
{
	mBounds.Min = mBoxes[0];
	mBounds.Max = mBoxes[0];
	for (int i=1; i<mNumBoxes; i++)
	{
		mBounds.AddBoundedPoint( mBoxes[i] );
	}
	mBounds += this->mCenter;
}

void Block::Init(Rect3* r)
{
	int xw = (r->Max.x - r->Min.x)+1;
	int yw = (r->Max.y - r->Min.y)+1;
	int zw = (r->Max.z - r->Min.z)+1;
	Init(xw*yw*zw);
	int x, y, z;
	for (z=0; z<zw; z++)
	{
		for (y=0; y<yw; y++)
		{
			for (x=0; x<xw; x++)
			{
				mBoxes[x+(y*xw)+(z*xw*yw)] = IPNT(
					x + r->Min.x,
					y + r->Min.y,
					z + r->Min.z );
			}
		}
	}

	mCenter = (r->Min + r->Max);
	mCenter /= 2;
	for (int i=0; i<mNumBoxes; i++)
		mBoxes[i] -= mCenter;

	FindBounds();
}

void Block::Init(int nboxes)
{
	if (mNumBoxes==nboxes)
		return;
	UnInit();
	mNumBoxes = nboxes;
	mBoxes = new ipnt[mNumBoxes];
}

void Block::UnInit()
{
	if (mBoxes)
	{
		delete mBoxes;
		mBoxes=0;
		mNumBoxes=0;
	}
};

class Board
{
public:
	Block* mBlocks;
	int mNumBlocks;
	Rect3 mBounds;
	Block mGoalBlock;

	void Init(int nblocks);
	void UnInit();

	bool IsTaken(ipnt at, int exclude=-1);
	void AutoBounds(int slack);
	bool CheckForWon();
	void FindAllBounds();

	Board() {mBlocks=0; mNumBlocks=0; mBounds.Zero();};
	~Board() {UnInit();};
};

void Board::FindAllBounds()
{
	for (int i=0; i<mNumBlocks; i++)
	{
		mBlocks[i].FindBounds();
	}
}

bool Board::CheckForWon()
{
	ipnt work;
	for (int i=0; i<mGoalBlock.mNumBoxes; i++)
	{
		work = mGoalBlock.mCenter + mGoalBlock.mBoxes[i];
		if (!IsTaken(work))
		{
			return false;
		}
	}
	return true;
}

void Board::AutoBounds(int slack)
{
	mBlocks[0].FindBounds();
	mBounds = mBlocks[0].mBounds;
	//mBounds.Zero();
	for (int i=0; i<mNumBlocks; i++)
	{
		mBlocks[i].FindBounds();
		mBounds.AddBoundedPoint( mBlocks[i].mBounds.Min );
		mBounds.AddBoundedPoint( mBlocks[i].mBounds.Max );
	}
	mGoalBlock.FindBounds();
	mBounds.AddBoundedPoint( mGoalBlock.mBounds.Min );
	mBounds.AddBoundedPoint( mGoalBlock.mBounds.Max );
	
	mBounds.Widen(slack);
	mBounds.Min.z += (slack);
	//mBounds.Min.z--;
	slack=0;
}

bool Block::CanApplyMatrix(MatrixStruct* mat)
{
//IPntTimesFloatMatrix4
	for (int i=0; i<mNumBoxes; i++)
	{
		ipnt at = mCenter + IPntTimesFloatMatrix4(mBoxes[i], *mat);
		if (mBoard->IsTaken( at, mIndex ))
			return false;
	}
	return true;
}

bool Block::Shift(ipnt offset)
{
	ResTransformCollection* tc = (ResTransformCollection*)mNode->mTransform;
	if (tc->Count() > 1)
		return false;
	ipnt ncen = mCenter + offset;
	for (int i=0; i<mNumBoxes; i++)
	{
		if (mBoard->IsTaken( mBoxes[i]+ncen, mIndex))
			return false;
	}

	mCenter += offset;
	mNode->mPosition->mPos = ITOFPNT( mCenter );
	mNode->mPosition->Changed();
	FindBounds();

	ResTranslateTransform* rt = ResTranslateTransform::Create(FPNT(0,0,0));
	tc->Add(rt);
	ResAnimateFromTo* ani = ResAnimateFromTo::Create(rt->mOffset, 
		ResPoint::Create(ITOFPNT(offset)*-1.0f), 
		ResPoint::Create(FPNT(0,0,0)), 
		0.125f, LC_AniFlag_StopWhenDone);
	ani->mToken = this;
	ani->mEvent_WhenDone = Block_AniDone;

	return true;
}

bool Board::IsTaken(ipnt at, int exclude)
{
	if (!mBounds.Includes(at))
		return true;
	for (int i=0; i<mNumBlocks; i++)
	{
		if (i != exclude)
		{
			if (mBlocks[i].Includes(at))
				return true;
		}
	}
	return false;
}

void Board::Init(int nblocks)
{
	UnInit();
	mNumBlocks = nblocks;
	mBlocks = new Block[nblocks];
	for (int i=0; i<mNumBlocks; i++)
	{
		mBlocks[i].mIndex = i;
		mBlocks[i].mBoard = this;
	}
}

void Board::UnInit()
{
	if (mBlocks)
	{
		delete[] mBlocks;
		mBlocks = 0;
		mNumBlocks = 0;
	}
}

void GenBlock_Stand(Block* bk)
{
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(1, 0, 0);
	bk->mBoxes[2].Set(0, 1, 0);
	bk->mBoxes[3].Set(0, 0, 1);
}

bool GB_Cuzzle_Vals[6][20] = {
	{ 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, },
	{ 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, },
	{ 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, },
	{ 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, },
	{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, }
};

bool IsCuzzleEdge(ipnt val)
{
	for (int i=0; i<3; i++)
	{
		if ((val[i]==0)  || (val[i]==5))
			return true;
	}
	return false;
}

void GenBlock_CuzzleGoal(Block* bk)
{
	ipnt min = IPNT(-3, -2, -1);
	ipnt cur;
	int ito = 0;
	bk->Init( (6*6*6) - (4*4*4) );
	for (cur.z=0; cur.z<6; cur.z++)
	{
		for (cur.y=0; cur.y<6; cur.y++)
		{
			for (cur.x=0; cur.x<6; cur.x++)
			{
				if (IsCuzzleEdge(cur))
				{
					bk->mBoxes[ito] = cur+min;
					ito++;
				}
			}
		}
	}
	bk->FindBounds();
}

void GenBlock_Cuzzle(Block* bk, int num)
{
	bool* data = GB_Cuzzle_Vals[num];
	int count=0, i;
	for (i=0; i<20; i++)
	{
		if (data[i])
			count++;
	}
	bk->Init(count+16);

	i=0;
	for (int y=-2; y<2; y++)
	{
		for (int x=-2; x<2; x++)
		{
			bk->mBoxes[i] = IPNT(x,0,y);
			i++;
		}
	}

	count=16;
	ipnt at = IPNT(-3, 0, 2);
	ipnt delta = IPNT(1, 0, 0);
	for (i=0; i<20; i++)
	{
		if (data[i])
		{
			bk->mBoxes[count] = at;
			count++;
		}
		if (i==5)
			delta = IPNT(0, 0, -1);
		if (i==10)
			delta = IPNT(-1, 0, 0);
		if (i==15)
			delta = IPNT(0, 0, 1);
		at += delta;
	}
}

Board* GenBoard_Cuzzle()
{
	Board* board = new Board();
	board->Init(6);
	int i;

	for (i=0; i<6; i++)
	{
		GenBlock_Cuzzle( &board->mBlocks[i], i);
	}

	board->mBlocks[0].mCenter.Set(0, -6, 0);
	board->mBlocks[3].mCenter.Set(0, 6, 0);

	board->mBlocks[1].mCenter.Set(-7, -3, 0);
	board->mBlocks[2].mCenter.Set(-7, 3, 0);
	board->mBlocks[4].mCenter.Set(7, -3, 0);
	board->mBlocks[5].mCenter.Set(7, 3, 0);

	for (i=0; i<6; i++)
	{
		board->mBlocks[i].FindBounds();
	}

	GenBlock_CuzzleGoal(&board->mGoalBlock);

	board->AutoBounds( 2 );
	//board->mBounds.Max.y += 2;

	return board;
}

Board* GenBoard_Level1()
{
	Board* board = new Board();
	board->Init(2);

	Block* bk;

	bk = &board->mBlocks[0];
	bk->mCenter.Set( -4, 0, 0);
	bk->Init(4);
	bk->mBoxes[0].Set( 0, 0, 0 );
	bk->mBoxes[1].Set( 1, 0, 0 );
	bk->mBoxes[2].Set( 0, 1, 0 );
	bk->mBoxes[3].Set( 0, 0, 1 );

	bk = &board->mBlocks[1];
	bk->mCenter.Set( 3, 1, 0);
	bk->Init(4);
	bk->mBoxes[0].Set( 0, 0, 0 );
	bk->mBoxes[1].Set( -1, 0, 0 );
	bk->mBoxes[2].Set( 0, -1, 0 );
	bk->mBoxes[3].Set( 0, 0, 1 );

	Rect3 r;
	r.Set(IPNT(-1, 0, 0), IPNT(0, 1, 1));
	board->mGoalBlock.Init(&r);

	board->AutoBounds(2);
	return board;
}

Board* GenBoard_Soma_Basic()
{
	Board* board = new Board();
	board->Init(7);

	Block* bk;

	bk = &board->mBlocks[0];
	bk->mCenter.Set(0, 0, 0);
	bk->Init(3);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, 1, 0);
	bk->mBoxes[2].Set(0, 0, 1);

	bk = &board->mBlocks[1];
	bk->mCenter.Set(2, 0, 0);
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, -1, 1);
	bk->mBoxes[2].Set(0, 1, 0);
	bk->mBoxes[3].Set(0, -1, 0);

	bk = &board->mBlocks[2];
	bk->mCenter.Set(4, 0, 0);
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, -1, 0);
	bk->mBoxes[2].Set(0, 0, 1);
	bk->mBoxes[3].Set(0, 1, 0);

	bk = &board->mBlocks[3];
	bk->mCenter.Set(6, 0, 1);
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, 1, -1);
	bk->mBoxes[2].Set(0, 0, 1);
	bk->mBoxes[3].Set(0, 1, 0);

	bk = &board->mBlocks[4];
	bk->mCenter.Set(0, 7, 0);
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, 1, 0);
	bk->mBoxes[2].Set(1, 0, 0);
	bk->mBoxes[3].Set(1, 0, 1);

	bk = &board->mBlocks[5];
	bk->mCenter.Set(4, 7, 0);
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, 1, 0);
	bk->mBoxes[2].Set(-1, 0, 0);
	bk->mBoxes[3].Set(-1, 0, 1);

	bk = &board->mBlocks[6];
	bk->mCenter.Set(6, 7, 0);
	bk->Init(4);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[2].Set(1, 0, 0);
	bk->mBoxes[1].Set(0, 1, 0);
	bk->mBoxes[3].Set(0, 0, 1);

	bk = &board->mGoalBlock;
	Rect3 r;
	r.Set(IPNT(3, 3, 0), IPNT(5, 5, 2));
	bk->Init(&r);

	board->AutoBounds(1);
	board->mBounds.Min.z = 0;
	return board;
}

Board* GenBoard_Floating()
{
	Board* board = GenBoard_Soma_Basic();
	Block* bk;

	bk = &board->mBlocks[6];
	bk->Init(3);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(1, -1, 1);
	bk->mBoxes[2].Set(1, 1, 1);

	bk = &board->mBlocks[0];
	bk->mCenter.Set(-1, 0, 0);
	bk->Init(3);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, -1, 0);
	bk->mBoxes[2].Set(1, 1, 1);

	bk = &board->mBlocks[3];
	bk->mCenter.Set(6, 0, 1);
	bk->Init(5);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[3].Set(0, 1, 0);
	bk->mBoxes[1].Set(0, 1, -1);
	bk->mBoxes[2].Set(0, 0, 1);
	bk->mBoxes[4].Set(0, -1, 1);


	board->AutoBounds(1);
	board->mBounds.Min.z = 0;

	return board;
}

Board* GenBoard_Soma_Flat()
{
	Board* board = GenBoard_Soma_Basic();

	Block* bk;

	bk = &board->mGoalBlock;
	int x, y;
	for (y=0; y<5; y++)
	{
		for (x=0; x<5; x++)
		{
			bk->mBoxes[x+(y*5)].Set(x-2, y-2, 0);
		}
	}
	int i = 4+(4*5);
	bk->mBoxes[i].Set(-2, -2, 1); i++;
	bk->mBoxes[i].Set(2, -2, 1); i++;
	bk->mBoxes[i].Set(-2, 2, 1); i++;
	bk->mCenter.z = 0;
	bk->mCenter.x-=1;
	bk->mCenter.y++;

	for (i=4; i<7; i++)
	{
		board->mBlocks[i].mCenter.y += 2;
	}

	board->AutoBounds(1);
	board->mBounds.Min.z = 0;

	return board;
}

Board* GenBoard_Soma_Tunnel()
{
	Board* board = GenBoard_Soma_Basic();

	Block* bk;

	bk = &board->mGoalBlock;
	ipnt work;
	for (int i=0; i<bk->mNumBoxes; i++)
	{
		work = bk->mBoxes[i];
		if (work.x==0)
		{
			if (work.z==-1)
			{
				work.x=-2;
				bk->mBoxes[i] = work;
			}
			if (work.z==0)
			{
				work.x=2;
				work.z=-1;
				bk->mBoxes[i] = work;
			}
		}
	}
	bk->mCenter.x -= 1;

	board->AutoBounds(1);
	board->mBounds.Min.z = 0;

	return board;
}

bool GB_FullStar_IsCorner(ipnt at)
{
	for (int i=0; i<3; i++)
	{
		if (at[i]==1)
			return false;
	}
	return true;
}

Board* GenBoard_FullStar()
{
	Board* board = new Board();
	board->Init(5);

	Block* bk;

	bk = &board->mBlocks[0];
	bk->Init(4);
	bk->mCenter.Set( -3, -3, 0);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(1, 0, 0);
	bk->mBoxes[2].Set(0, 1, 0);
	bk->mBoxes[3].Set(0, 0, 1);

	bk = &board->mBlocks[1];
	bk->Init(4);
	bk->mCenter.Set( 2, -3, 0);
	bk->mBoxes[0].Set(-1, 0, 0);
	bk->mBoxes[1].Set(0, 0, 0);
	bk->mBoxes[2].Set(1, 0, 0);
	bk->mBoxes[3].Set(1, 0, 1);

	bk = &board->mBlocks[2];
	bk->Init(4);
	bk->mCenter.Set( 3, 3, 0);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, -1, 0);
	bk->mBoxes[2].Set(0, 0, 1);
	bk->mBoxes[3].Set(0, 1, 1);

	bk = &board->mBlocks[3];
	bk->Init(4);
	bk->mCenter.Set( 0, 3, 0);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(1, 0, 0);
	bk->mBoxes[2].Set(0, 0, 1);
	bk->mBoxes[3].Set(-1, 0, 0);

	bk = &board->mBlocks[4];
	bk->Init(4);
	bk->mCenter.Set( -3, 3, 0);
	bk->mBoxes[0].Set(0, 0, 0);
	bk->mBoxes[1].Set(0, 1, 0);
	bk->mBoxes[2].Set(0, 0, 1);
	bk->mBoxes[3].Set(0, -1, 0);

	bk = &board->mGoalBlock;
	bk->Init(9+5+5+1);
	bk->mCenter.Set(0, 0, 1);
	ipnt cur;
	ipnt offset = IPNT(-1, -1, -1);
	int ito=0;
	for (cur.z=0; cur.z<3; cur.z++)
	{
		for (cur.y=0; cur.y<3; cur.y++)
		{
			for (cur.x=0; cur.x<3; cur.x++)
			{
				if (!GB_FullStar_IsCorner(cur))
				{
					bk->mBoxes[ito] = cur + offset;
					ito++;
				}
			}
		}
	}
	bk->mBoxes[bk->mNumBoxes-1].Set(-1, -1, -1);

	board->AutoBounds(1);
	board->mBounds.Min.z = 0;

	return board;
}

Board* GenBoard_Level3()
{
	Board* board = new Board();
	board->Init(4);

	Block* bk;

	bk = &board->mBlocks[0];
	bk->mCenter.Set( -2, 2, 0);
	bk->Init(5);
	bk->mBoxes[0].Set( 0, 1, 0 );
	bk->mBoxes[1].Set( 1, 1, 0 );
	bk->mBoxes[2].Set( -1, 0, 0 );
	bk->mBoxes[3].Set( -1, -1, 0 );
	bk->mBoxes[4].Set( 0, 0, 0 );

	bk = &board->mBlocks[1];
	bk->mCenter.Set( -2, -2, 0);
	bk->Init(5);
	bk->mBoxes[0].Set( 0, -1, 0 );
	bk->mBoxes[1].Set( 1, -1, 0 );
	bk->mBoxes[2].Set( -1, 0, 0 );
	bk->mBoxes[3].Set( -1, 1, 0 );
	bk->mBoxes[4].Set( 0, 0, 0 );

	bk = &board->mBlocks[2];
	bk->mCenter.Set( 2, -2, 0);
	bk->Init(5);
	bk->mBoxes[0].Set( 0, -1, 0 );
	bk->mBoxes[1].Set( -1, -1, 0 );
	bk->mBoxes[2].Set( 1, 0, 0 );
	bk->mBoxes[3].Set( 1, 1, 0 );
	bk->mBoxes[4].Set( 0, 0, 0 );

	bk = &board->mBlocks[3];
	bk->mCenter.Set( 3, 3, 0);
	bk->Init(1);
	bk->mBoxes[0].Set( 0, 0, 0 );

	int ito=0;
	bk = &board->mGoalBlock;
	bk->mCenter.Set( 0, 0, 2 );
	bk->Init(3*4+1);
	bk->mBoxes[ito].Set(0, 0, 0); ito++;

	bk->mBoxes[ito].Set(1, 0, 0); ito++;
	bk->mBoxes[ito].Set(2, 0, 0); ito++;
	bk->mBoxes[ito].Set(-1, 0, 0); ito++;
	bk->mBoxes[ito].Set(-2, 0, 0); ito++;

	bk->mBoxes[ito].Set(0, 1, 0); ito++;
	bk->mBoxes[ito].Set(0, 2, 0); ito++;
	bk->mBoxes[ito].Set(0, -1, 0); ito++;
	bk->mBoxes[ito].Set(0, -2, 0); ito++;

	bk->mBoxes[ito].Set(0, 0, 1 ); ito++;
	bk->mBoxes[ito].Set(0, 0, 2); ito++;
	bk->mBoxes[ito].Set(0, 0, -1); ito++;
	bk->mBoxes[ito].Set(0, 0, -2); ito++;


	board->AutoBounds(1);
	board->mBounds.Min.z = 0;
	board->mBounds.Max.y += 2;

	return board;
}

Board* GenBoard_Level2()
{
	Board* board = new Board();
	board->Init(3);

	Block* bk;

	bk = &board->mBlocks[0];
	bk->mCenter.Set( 0, 3, 0);
	bk->Init(5);
	bk->mBoxes[0].Set( 0, 0, 0 );
	bk->mBoxes[1].Set( -1, 0, 0 );
	bk->mBoxes[2].Set( -1, 1, 0 );
	bk->mBoxes[3].Set( -1, 0, 1 );
	bk->mBoxes[4].Set( 1, 0, 0 );

	bk = &board->mBlocks[1];
	bk->mCenter.Set( -4, 0, 0);
	bk->Init(4);
	bk->mBoxes[0].Set( 0, 0, 0 );
	bk->mBoxes[1].Set( 1, 0, 0 );
	bk->mBoxes[2].Set( 0, 1, 0 );
	bk->mBoxes[3].Set( 0, 0, 1 );

	bk = &board->mBlocks[2];
	bk->mCenter.Set( 3, 0, 0);
	bk->Init(3);
	bk->mBoxes[0].Set( 0, 0, 0 );
	bk->mBoxes[1].Set( 1, 0, 0 );
	bk->mBoxes[2].Set( 0, 1, 0 );

	Rect3 r;
	r.Set(IPNT(-1,0,0), IPNT(1,1,1));
	board->mGoalBlock.Init(&r);

	board->AutoBounds(2);
	return board;
}

typedef Board* (*BoardGener)();

#define NumLevels	9
BoardGener Levels[NumLevels] = {
	GenBoard_Level1,
	GenBoard_Level2,
	GenBoard_Level3,
	GenBoard_Soma_Flat,
	GenBoard_FullStar,
	GenBoard_Soma_Tunnel,
	GenBoard_Floating,
	GenBoard_Soma_Basic,
	GenBoard_Cuzzle
};

#define OTHERAXIS(xory)		(((xory)+1)%2)

class Game : public GenRes
{
public:
	ResCompNode* mNodes;
	Board* mBoard;
	ResStencilEdger* mEdger;
	ResMesh* mBgMesh;
	ResMesh* mLightMesh;
	ResMesh* mGoalMesh;
	MeshRenderContext* mBgRenderer;
	Rect3 CameraBounds;
	int mLevel;

	LC_Event mEvent_LevelChanged;

	ResPoint* LightPos;
	ResPoint* CameraLookAt;
	int CameraAng, CameraPitch;
	float mCameraDist;

	void Init(ResCompNode* parent, int level, Board* board);
	void UnInit();
	void NewGame();
	void Clicked(Block* block);
	void Undo();
	void SetIsInv(bool set);
	bool GetIsInv();
	void FinishedMove();
	void GotoLevel(int num);

	float LightHeight();
	int BestForwardAxis(bool* positive, bool other=false);

	RESVISIT( RESMEM(mLightMesh) RESMEM(mBgRenderer) RESMEM(mNodes) RESMEM(mGoalMesh)
		RESMEM(CameraLookAt) RESMEM(mEdger) RESMEM(LightPos) RESMEM(mBgMesh) );

	Game() {mGoalMesh=0; mLightMesh=0; mBgMesh=0; mBgRenderer=0; mBoard = 0; 
		mNodes=0; mEdger=0; CameraLookAt=0; CameraAng=90; CameraPitch=45;
		mEvent_LevelChanged=0;};
	~Game() {UnInit();};

	static Game* Create();
};
Game* GGame;

void SetupCamera()
{
	fpnt cam;
	float ang = DegToRad(GGame->CameraAng);
	float pitch = DegToRad(GGame->CameraPitch);
	cam.z = sin(pitch);
	cam.x = cos(ang)*cos(pitch);
	cam.y = sin(ang)*cos(pitch);
	cam *= GGame->mCameraDist;

	if (!Core.Graphics->CameraPerspec)
		Core.Graphics->CameraPerspec = ResFloat::Create(45);

	Core.Graphics->CameraPerspec->mValue = 45;
	Core.Graphics->CameraUp->mPos.Set(0, 0, 1);
	Core.Graphics->CameraPos->mPos = cam + GGame->CameraLookAt->mPos;
	Core.Graphics->LookAt(GGame->CameraLookAt->mPos);
	Core.Graphics->LightPos = GGame->LightPos;

//	Core.Graphics->LightPos->mPos = FPNT(-5, -5, 15);
//	Core.Graphics->LightPos->mPos = ITOFPNT( GGame->mBoard->mBounds.Max + IPNT(1, 1, 1) );
//	Core.Graphics->LightPos->mPos = Core.Graphics->CameraPos->mPos;
}

void Game::GotoLevel(int num)
{
	BoardGener func = 0;
	num--;
	Assert((num < NumLevels)&&(num>=0));
	if (num >= NumLevels)
		return;
	func = Levels[num];
	num++;
	Board* b = (*func)();
	Init(mNodes, num, b);
	FIRE(mEvent_LevelChanged, this);
}

void Block_AniDone(void* data)
{
	ResAnimateFromTo* ani = (ResAnimateFromTo*)data;
	Block* b = (Block*)ani->mToken;
	ResTransformCollection* tc = (ResTransformCollection*)b->mNode->mTransform;
	while (tc->Count() > 1)
	{
		tc->Remove( tc->Get(1) );
	}

	Game* g = (Game*)Core.mGameRes;
	g->FinishedMove();
}

char game_text[400];
char game_text2[400];
void Game::FinishedMove()
{
	if (mBoard->CheckForWon())
	{

		if (!GetIsInv())
		{
			GotoLevel(mLevel);
			SetIsInv(true);
			sprintf(game_text, "You solved it!\n\nNow try level %d without visibility!", mLevel);
			ShowMessage(game_text, "Got it!");
			return;
		}
		if (mLevel < NumLevels)
		{
			int next = mLevel+1;
			bool isvis = (next <= 2);
			GotoLevel(next);
			SetIsInv(!isvis);
			sprintf(game_text, "You Solved It!\n\nNow try level %d", next);
			if (isvis)
				sprintf(game_text2, "%s\nwith visibility!", game_text);
			else
				sprintf(game_text2, "%s\nwithout visibility!", game_text);
			ShowMessage(game_text2, "Well done!");
			return;
		}
		else
		{
			FIRE(mEvent_LevelChanged, 0);
			ShowMessage("You have achieved the status\nof Shadow Master!", "Excellent");
		}
	}
}

float Game::LightHeight()
{
	return ((float)mBoard->mBounds.Max.z) + 1.75;
}

bool BXOR(bool a, bool b)
{
	int ia = ((a)? 1 : 0);
	int ib = ((b)? 1 : 0);
	return ((ia+ib)==1);
}

int Game::BestForwardAxis(bool* positive, bool other)
{
	fpnt look = Core.Graphics->CameraLookDir->mPos;
	look.z=0;
	fpnt ax = FPNT(1, 0, 0);
	fpnt yx = FPNT(0, 1, 0);

	float x = look.Dot(ax);
	float y = look.Dot(yx);

	int i =-1;
	if (fabs(x) > fabs(y))
		i=0;
	else
		i=1;

	if (other)
	{
		i = ((i+1)%2);
		if (i==1)
			*positive = (x > 0);
		else
			*positive = (y < 0);
	}
	else
	{
		if (i==0)
			*positive = (x > 0);
		else
			*positive = (y > 0);
	}

	return i;
}

void Game::NewGame()
{
}

void Game::Clicked(Block* block)
{
}

void Game::UnInit()
{
	if (mBoard)
	{
		delete mBoard;
		mBoard = 0;
	}
	mNodes = 0;
}

#define NUMDEFCOLORS	7
Color4f DefColors[NUMDEFCOLORS] = {
	{ 1, 0, 0, 1, },
	{ 0, 1, 0, 1, },
	{ 1, 1, 0, 1, },
	{ 0, 0, 1, 1, },
	{ 1, 0, 1, 1, },
	{ 0, 1, 1, 1, },
	{ 1, 0.5, 0.5, 1 },
};

void Game_DoNothing(void* data)
{
}

bool Game::GetIsInv()
{
	RenderContext* rc = Core.Graphics->GetRenderStage(SolidRenderContext);
	return (rc->mEvent_ReplaceFlush != 0);
}

void Game::SetIsInv(bool set)
{
	RenderContext* rc = Core.Graphics->GetRenderStage(SolidRenderContext);
	if (set)
	{
		rc->mEvent_ReplaceFlush = Game_DoNothing;
		if (mEdger)
			mEdger->mIsInvisable = true;
	}
	else
	{
		rc->mEvent_ReplaceFlush = 0;
		if (mEdger)
			mEdger->mIsInvisable = false;
	}
}

ResMesh* GenEnvMesh(Rect3 bounds)
{
	int ex = (bounds.Max - bounds.Min).MaxValue();
	ex = (ex/2) + (ex/4);
	//int ex = 5;
	int w = (bounds.Max.x - bounds.Min.x) + ex*2;
	int h = (bounds.Max.y - bounds.Min.y) + ex*2;
	w += (w%2);
	h += (h%2);
	ResMesh* mesh = ResMesh::Create(
		MeshFormat::Create(4, true), w*h, (w-1)*(h-1)*4);

	VertType* verts = (VertType*)mesh->VertData.Raw();
	IndPoly4* inds = (IndPoly4*)mesh->Indices.Raw();

	float minx = bounds.Min.x - ex;
	float miny = bounds.Min.y - ex;
	float z = ((float)bounds.Min.z)-0.51;
	float bonusx, bonusy;

	int ito=0;
	for (int y=0; y<h; y++)
	{
		bonusy = 0;
		if (y < ex-1)
			bonusy = (ex-y)-1;
		if (y > h-ex)
			bonusy = y-(h-ex);

		for (int x=0; x<w; x++)
		{
			int i = x+(y*w);
			verts[x+(y*w)].mVertex.Set(minx+x, miny+y, z);

			bonusx = 0;
			if (x < ex-1)
				bonusx = (ex-x)-1;
			if (x > w-ex)
				bonusx = x-(w-ex);

			bonusx = ((bonusx > bonusy)? bonusx : bonusy);
			verts[i].mVertex.z += bonusx;

			bonusx = ((bonusx==0)? 0.2 : 1.0);
			verts[i].mVertex.z -= (RandF()*bonusx);

			if (((x+1)<w) && ((y+1)<h))
			{
				inds[ito].a = (x) + ((y+1)*w);
				inds[ito].b = (x) + ((y)*w);
				inds[ito].c = (x+1) + ((y)*w);
				inds[ito].d = (x+1) + ((y+1)*w);
				ito++;
			}
		}
	}

	PaintMesh(mesh, Color4f::Create(0.5, 0.75, 0.5));
	mesh->UpdateNormals();

	return mesh;
}

void Game_ContextFlush(void* data)
{
	Game* g = (Game*)Core.mGameRes;
	if (!g->mEdger)
	{
		MeshRenderContext* rc = (MeshRenderContext*)
			Core.Graphics->GetRenderStage(SolidRenderContext);

		g->mEdger = ResStencilEdger::Create( rc->Target );
		g->mEdger->mIsInvisable = true;
		rc->Target->mEdger = g->mEdger;
		g->mEdger->mRefPoint = g->LightPos;
		g->mEdger->UpdateEdgeList();
	}
	g->SetIsInv( g->GetIsInv() );
	if (!g->mLightMesh)
	{
		g->mLightMesh = GenSphereMesh(3, 6);
		InversePolygonRotation(g->mLightMesh);
		MatrixStruct mat;
		SetMatrix_Scale(mat, 0.25);
		ApplyMatrixOnMesh(g->mLightMesh, &mat);
		PaintMesh(g->mLightMesh, Color4f::Create(1, 1, 0));
		g->mLightMesh->UpdateNormals();
		g->mBgRenderer = MeshRenderContext::Create(g->mLightMesh);
	}
	if (!g->mBgMesh)
	{
		g->mBgMesh = GenEnvMesh(g->mBoard->mBounds);
	}

	g->mBgRenderer->Target = g->mBgMesh;
	g->mBgRenderer->Flush();

	g->mBgRenderer->Target = g->mGoalMesh;
	g->mBgRenderer->Flush();

	glPushMatrix();
		fpnt lp = Core.Graphics->LightPos->mPos;
		glTranslatef(lp.x, lp.y, lp.z);
		glDisable(GL_LIGHTING);
		g->mBgRenderer->Target = g->mLightMesh;
		g->mBgRenderer->Flush();
		glEnable(GL_LIGHTING);
	glPopMatrix();

	g->mEdger->UpdateNormals();
	g->mEdger->UpdateOutline();
	if (CALLEDDEBUG)
		CALLEDDEBUG = false;
	g->mEdger->FlushShadow();
}

void Game::Init(ResCompNode* parent, int level, Board* board)
{
	UnInit();
	mNodes = parent;
	mBoard = board;
	mNodes->RemoveAllChildren();

	mLevel = level;
	ResMesh* mesh = GenQuadCubeMesh();
	ResCompNode* work;

	mEdger = 0;
	LC_Event_ContextFlushDone = Game_ContextFlush;

	Core.Graphics->GetRenderStage(SolidRenderContext)->mEvent_ReplaceFlush = Game_DoNothing;

	for (int i=0; i<mBoard->mNumBlocks; i++)
	{
		mBoard->mBlocks[i].FindBounds();
		work = mBoard->mBlocks[i].GenCompNode(mesh, SolidRenderContext);
		work->mColor = ResColor::Create( DefColors[i%NUMDEFCOLORS] );
		mNodes->AddChild(work);
	}

	mGoalMesh = mBoard->mGoalBlock.GenOutlineMesh();

//	work = ResCompNode::Create( background );
//	Rect3 r = mBoard->mBounds;
//	r.Min.z -= 2;
//	r.Max.z = r.Min.z+1;
//	work->mMesh = r.GenMesh();
//	work->mMesh = GenEnvMesh( mBoard->mBounds );
//	mNodes->AddChild( work );
	mBgMesh = 0;

	mCameraDist = (mBoard->mBounds.Max - mBoard->mBounds.Min).MaxValue();
	mCameraDist *= 0.7;
	CameraBounds = mBoard->mBounds;
	CameraBounds.Widen(2);

	CameraLookAt = ResPoint::Create( ITOFPNT(mBoard->mBounds.Min) + ITOFPNT(mBoard->mBounds.Max) );
	CameraLookAt->mPos /= 2.0f;
	CameraLookAt->mPos.z = LightHeight();

	CameraAng=90;
	CameraPitch=45;

	LightPos = ResPoint::Create( CameraLookAt->mPos );
	LightPos->mPos.z = LightHeight();
	LightPos->mPos += FPNT(0.1, 0.1, 0.1);

	float yoff = mBoard->mBounds.Max.y - mBoard->mBounds.Min.y;
	CameraLookAt->mPos.y += yoff/3.0f;

	SetupCamera();

	/*
	work = ResCompNode::Create(quads);
	mNodes->AddChild(work);
	{
		ResMesh* bottom = ResMesh::Create(MeshFormat::Create(4, true), 4, 4);
		PaintMesh(bottom, Color4f::FromRGB(0xc0c0c0));
		VertType* verts = (VertType*)bottom->VertData.Raw();
		IndPoly4* inds = (IndPoly4*)bottom->Indices.Raw();
		verts[0].mVertex = ITOFPNT( board->mBounds.Min );
		verts[1].mVertex = FPNT( board->mBounds.Max.x, board->mBounds.Max.y, board->mBounds.Min.z );
		verts[2].mVertex = verts[0].mVertex;
		verts[2].mVertex.x = verts[1].mVertex.x;
		verts[3].mVertex = verts[1].mVertex;
		verts[3].mVertex.y = verts[0].mVertex.y;
		inds->Set(0, 1, 2, 3);

		work->mMesh = bottom;
	}
	*/
}

Game* Game::Create()
{
	Game* g = new Game();
	Core.Register(g);
	Core.mGameRes = g;
	return g;
}