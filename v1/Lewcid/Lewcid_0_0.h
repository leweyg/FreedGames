
#include <Assert.h>
#include "pnt.cpp"
#include "matrixclass.cpp"
#include "earray.cpp"
#include "barray.cpp"

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef uint handle;
typedef uint IndexType;
#define null 0

struct DataSurface
{
	void* data;
	int width, height, stride;
};

class GenRes;

#define Assert assert
//#define Assert(A) {if(!A){int*__p=0; *__p=5;}}

typedef void (*ResVisitFunc)(GenRes* res, GenRes** ptr, void* extra);

#define RESVISIT(M)	virtual void VisitRes(ResVisitFunc func, void* data) { M };
#define RESMEM(X)   (*func)( this, ((GenRes**)(&X)), data);
#define INVALIDOFFSET -1

#define FlagOn(V,F) {V |= F;}
#define FlagOff(V,F) {V &= ~F;}
#define IsFlag(V,F) ((V & F)!=0)

struct Color4f
{
	float Red, Green, Blue, Alpha;

	float& operator[] (int i) {return *((&Red)+i);};
	void operator *=(Color4f& other);

	void Set(float red, float green, float blue, float alpha)
	{
		Red = red;
		Green = green;
		Blue = blue;
		Alpha = alpha;
	};
};

typedef double Time;

struct MatrixStruct
{
	float Val[16];

	void operator=(MatrixStruct& other);
	operator float*() {return Val;};
};

#define LC_ResFlag_Changed		(1<<0)
#define LC_ResFlag_IsBinding	(1<<1)
#define LC_ResFlag_DataChanged  (1<<2)
#define LC_ResFlag_ChildChanged (1<<3)
#define LC_ResFlag_FreeMe		(1<<4)

class GenRes
{
public:
	uint _Flags;

	virtual void Destroy() {};
	void Changed() {_Flags |= LC_ResFlag_Changed;};
	virtual void VisitRes(ResVisitFunc func, void* data) {};
};

class ResAnimate : public GenRes
{
public:
	virtual double RatioBetween(ResAnimate* from, ResAnimate* to)=0;
	virtual void Interp(ResAnimate* from, ResAnimate* to, double ratio)=0;
};

#define LC_AniFlag_Loop				(1<<0)
#define LC_AniFlag_Bounce			(1<<1)
#define LC_AniFlag_StopWhenDone		(1<<2)
#define LC_AniFlag_FromCurrent		(1<<3)

class ResBinding : public GenRes
{
public:
	GenRes* mTarget;

	virtual void Update(Time dt) = 0;
};

class ResAnimateFromTo : public ResBinding
{
public:
	double mTrueProgress;
	ResAnimate *mFrom, *mTo;
	Time mLength;
	double mProgress;
	uint mFlags;

	virtual void Update(Time dt);
	static ResAnimateFromTo* Create(ResAnimate* target, ResAnimate* from, ResAnimate* to, Time duration, uint flags = LC_AniFlag_Bounce);

	ResAnimateFromTo() {mTrueProgress=0; mProgress=0; mFlags=0;};
	RESVISIT( RESMEM(mFrom) RESMEM(mTo)  RESMEM(mTarget) );
};

class ResPoint : public GenRes
{
public:
	fpnt mPos;

	static ResPoint* Create(float x, float y, float z);
	virtual fpnt GetValue() {return mPos;};
};

class ResColor : public ResAnimate
{
public:
	Color4f mValue;

	static ResColor* Create(float r, float g, float b, float a);
	virtual double RatioBetween(ResAnimate* from, ResAnimate* to);
	virtual void Interp(ResAnimate* from, ResAnimate* to, double ratio);
};

class ResFloat : public ResAnimate
{
public:
	float mValue;

	static ResFloat* Create(float val);
	virtual double RatioBetween(ResAnimate* from, ResAnimate* to);
	virtual void Interp(ResAnimate* from, ResAnimate* to, double ratio);
};

class ResTransform : public GenRes
{
public:
	virtual void GetTransformMatrix(MatrixStruct* to) {};
};

class ResTransformCollection : public ResTransform
{
public:
	EArray<ResTransform*> mTransforms;

	virtual void GetTransformMatrix(MatrixStruct* to);
	virtual void VisitRes(ResVisitFunc func, void* data);
};

class ResRotateTransform : public ResTransform
{
public:
	ResFloat* mAngle;
	ResFloat* mPitch;

	static ResRotateTransform* Create(ResFloat* pitch, ResFloat* angle);
	static ResRotateTransform* Create(float pitch, float angle);
	virtual void GetTransformMatrix(MatrixStruct* to);
	RESVISIT( RESMEM(mAngle) RESMEM(mPitch) );
};

class ResScaleTransform : public ResTransform
{
public:
	ResFloat* mFactor;

	static ResScaleTransform* Create(float val);
	static ResScaleTransform* Create(ResFloat* val);
	virtual void GetTransformMatrix(MatrixStruct* to);
	RESVISIT( RESMEM( mFactor ) );
};

//triangle formats:
#define LC_VF_T_C4fVf		2
#define LC_VF_T_IC4fVf		3
#define LC_VF_T_IC4fNfVf	4
#define LC_VF_T_C4fNfVf		5
//quad formats
#define LC_VF_Q_C4fVf		6
#define LC_VF_Q_C4fNfVf		7
#define LC_VF_Q_IC4fNfVf	9
//line formats:
#define LC_VF_L_IVf			8

typedef uint VertexFormat;

struct VertData_Vf
{
	fpnt mVertex;
};

struct VertData_C4fVf
{
	Color4f mColor;
	fpnt mVertex;
};

struct VertData_C4fNfVf
{
	Color4f mColor;
	fpnt mNormal;
	fpnt mVertex;
};

struct IndTri2
{
	ushort a, b;

	void Set(ushort one, ushort two)
	{
		a = one;
		b = two;
	};

	ushort& operator[] (int i) {return *((&a)+i);};
};

struct IndTri3
{
	ushort a, b, c;

	void Set(ushort one, ushort two, ushort three)
	{
		a = one;
		b = two;
		c = three;
	};

	ushort& operator[] (int i) {return *((&a)+i);};
};

struct IndTri4
{
	ushort a, b, c, d;

	void Set(ushort one, ushort two, ushort three, ushort four)
	{
		a = one;
		b = two;
		c = three;
		d = four;
	};

	ushort& operator[] (int i) {return *((&a)+i);};
};

class ResMesh : public GenRes
{
public:
	byte* PackedVertData;
	int NumVerts, VertDataSize;
	ushort* Indices;
	int NumIndices;
	VertexFormat Format;

	static bool FormatHasNormals(VertexFormat format);
	static int FormatSize(VertexFormat format);
	static bool FormatIsIndexed(VertexFormat format);
	static ResMesh* Create(int numverts, VertexFormat format, int numindices);
	
	virtual void Destroy();
	virtual void VisitRes(ResVisitFunc func, void* data) {};
};

class ResCompNode;

typedef void (*NodeVisitFunc)(ResCompNode* parent, ResCompNode* child, void* data);

class ResCompNode : public GenRes
{
public:
	uint mRenderStage;
	ResPoint* mPosition;
	ResTransform *mTransform;
	ResColor *mColor;
	ResMesh *mMesh;
	EArray<ResCompNode*> mChildren;
	int _mStageBufferVertexOffset;
	int _mStageBufferIndexOffset;
	void* mToken;

	static ResCompNode* Create(uint renderstage);
	void Clear();
	void VisitChildren(NodeVisitFunc func, void* data);

	virtual void Destroy() {Clear();};
	RESVISIT( RESMEM( mPosition ) RESMEM( mTransform ) RESMEM( mColor ) RESMEM( mMesh ) );

};

class LewcidCore;

#define LC_ALLRENDERSTAGES (~0)

class StageBuffer
{
public:
	EArray<char> mData;
	EArray<ushort> mIndices;
	int mNumVerts;
	VertexFormat mFormat;
	uint mRenderFlags;
	uint mStageMask;
	BOOL mRebuild;
	Color4f mDefaultColor;

	int HitTest(fpnt camera, fpnt lookdir);
	void PrepForFrame();
	void CalcNormals();
	void Flush();
};

class LewcidGLEngine
{
public:
	BArray<StageBuffer> mStages;
	StageBuffer* mStageBuffer;

	EArray<MatrixStruct> mTransformStack;
	EArray<fpnt> mTranslateStack;
	EArray<Color4f> mColorStack;
	int mCurStageMask;
	LewcidCore* mCore;

	ResCompNode* HitCompNode(int screenx, int screeny);
	ResCompNode* HitCompNode(fpnt camera, fpnt lookdir);
	uint AddRenderStage(VertexFormat format, uint flags);
	fpnt TransformVertex(fpnt vert);
	void AddMesh(ResMesh* mesh, int& vertoffset, int& indoffset);
	void Render(LewcidCore* core);
};

class LewcidCore
{
	EArray<GenRes*> mAllRes;
	LARGE_INTEGER mLastTime, mCounterFrequency;

public:
	LewcidGLEngine Graphics;
	ResCompNode* mRootNode;

	StageBuffer* GetRenderStage(uint mask);
	void ClearChanged();
	void Tick();
	void Register(GenRes* res) { *mAllRes.Add() = res; res->_Flags = LC_ResFlag_Changed; };
	void Render();
	void MarkDependencies();
	void GarbageCollect();

	LewcidCore();
} Core;

//
// Res Implimentation
//

void Color4f::operator *=(Color4f& other)
{
	Red *= other.Red;
	Green *= other.Green;
	Blue *= other.Blue;
	Alpha *= other.Alpha;
}

ResColor* ResColor::Create(float r, float g, float b, float a)
{
	ResColor* ans = new ResColor();
	Core.Register(ans);
	ans->mValue.Set(r, g, b, a);
	return ans;
}
double ResColor::RatioBetween(ResAnimate* afrom, ResAnimate* ato)
{
	ResColor* from = ((ResColor*)afrom);
	ResColor* to = ((ResColor*)ato);

	for (int i=0; i<4; i++)
	{
		float diff = (to->mValue[i] - from->mValue[i]);
		if ((diff*diff) > 0.001)
		{
			return ((mValue[i] - from->mValue[i]) / (to->mValue[i] - from->mValue[i]));
		}
	}
	i=0;
	return ((mValue[i] - from->mValue[i]) / (to->mValue[i] - from->mValue[i]));

}
void ResColor::Interp(ResAnimate* afrom, ResAnimate* ato, double ratio)
{
	ResColor* from = ((ResColor*)afrom);
	ResColor* to = ((ResColor*)ato);

	for (int i=0; i<4; i++)
	{
		mValue[i] = (from->mValue[i]*ratio + to->mValue[i]*(1.0-ratio));
	}
}

double ResFloat::RatioBetween(ResAnimate* afrom, ResAnimate* ato)
{
	ResFloat* from = (ResFloat*)afrom;
	ResFloat* to = (ResFloat*)ato;

	return ((mValue - from->mValue) / (to->mValue - from->mValue));
}

void ResFloat::Interp(ResAnimate* afrom, ResAnimate* ato, double ratio)
{
	ResFloat* from = (ResFloat*)afrom;
	ResFloat* to = (ResFloat*)ato;
	
	this->mValue = (to->mValue*ratio + from->mValue*(1.0-ratio) );
	this->_Flags |= LC_ResFlag_Changed;
}

ResAnimateFromTo* ResAnimateFromTo::Create(ResAnimate* target, ResAnimate* from, ResAnimate* to, Time duration, uint flags)
{
	ResAnimateFromTo* ans = new ResAnimateFromTo();
	Core.Register(ans);
	ans->_Flags |= LC_ResFlag_IsBinding;
	ans->mTarget = target;
	ans->mProgress = 0.0;
	ans->mFrom = from;
	ans->mTo = to;
	ans->mLength = duration;
	ans->mFlags = flags;
	return ans;
}

void ResAnimateFromTo::Update(Time dt)
{
	if (!mTarget)
		return;

	this->_Flags |= LC_ResFlag_Changed;

	if (this->mFlags & LC_AniFlag_FromCurrent)
	{
		FlagOff( mFlags, LC_AniFlag_FromCurrent );
		mTrueProgress = ((ResAnimate*)mTarget)->RatioBetween( mFrom, mTo );
	}

	dt /= this->mLength;
	this->mTrueProgress += dt;
	if (mFlags == 0)
	{
		if (mTrueProgress <= 1.0)
			mProgress = mTrueProgress;
		else
			mProgress = 1.0;
	}
	if (mFlags & LC_AniFlag_Loop)
	{
		if (mTrueProgress > 1.0)
			mTrueProgress -= 1.0;
		mProgress = mTrueProgress;
	}
	if (mFlags & LC_AniFlag_Bounce)
	{
		if (mTrueProgress >= 2.0)
			mTrueProgress -= 2.0;

		if (mTrueProgress <= 1.0)
			mProgress = mTrueProgress;
		else
			mProgress = 1.0 - (mTrueProgress - 1.0);
	}
	if (mFlags & LC_AniFlag_StopWhenDone)
	{
		if (mTrueProgress > 1.0)
		{
			mTrueProgress = 1.0;
		}
		else
			mProgress = mTrueProgress;
	}

	((ResAnimate*)mTarget)->Interp(mFrom, mTo, mProgress);
	mTarget->_Flags |= LC_ResFlag_Changed;

	if (mFlags & LC_AniFlag_StopWhenDone)
	{
		if (mTrueProgress >= 1.0)
		{
			mTarget = null;
			return;
		}
	}
}

void MatrixStruct::operator=(MatrixStruct& other)
{
	CopyMemory( &Val[0], &other.Val[0], sizeof(MatrixStruct) );
}

ResRotateTransform* ResRotateTransform::Create(ResFloat* pitch, ResFloat* angle)
{
	ResRotateTransform* rot = new ResRotateTransform();
	rot->mAngle = angle;
	rot->mPitch = pitch;
	Core.Register(rot);
	return rot;
}

ResRotateTransform* ResRotateTransform::Create(float pitch, float angle)
{
	ResRotateTransform* rot = new ResRotateTransform();
	rot->mAngle = ResFloat::Create(angle);
	rot->mPitch = ResFloat::Create(pitch);
	Core.Register(rot);
	return rot;
}

MatrixStruct lc_resrotmat1, lc_resrotmat2;
void ResRotateTransform::GetTransformMatrix(MatrixStruct* to)
{
//	SetMatrix_RotateX( *to, this->mPitch->mValue );
	SetMatrix_RotateX(lc_resrotmat2, this->mPitch->mValue );
	SetMatrix_RotateY(lc_resrotmat1, this->mAngle->mValue );
	MatrixTimesMatrix( lc_resrotmat2, lc_resrotmat1, *to );
}

ResCompNode* ResCompNode::Create(uint renderstage)
{
	ResCompNode* node = new ResCompNode();
	node->Clear();
	node->mRenderStage = renderstage;
	Core.Register(node);
	return node;
}

ResScaleTransform* ResScaleTransform::Create(float val)
{
	ResScaleTransform* ans = new ResScaleTransform();
	ans->mFactor = ResFloat::Create(val);
	Core.Register(ans);
	return ans;
}

ResScaleTransform* ResScaleTransform::Create(ResFloat* val)
{
	ResScaleTransform* ans = new ResScaleTransform();
	ans->mFactor = val;
	Core.Register(ans);
	return ans;
}

int ResMesh::FormatSize(VertexFormat format)
{
	switch (format)
	{
	case LC_VF_T_IC4fNfVf:
		return sizeof(VertData_C4fNfVf);
		break;
	case LC_VF_T_C4fNfVf:
		return sizeof(VertData_C4fNfVf);
		break;
	case LC_VF_T_C4fVf:
		return sizeof(VertData_C4fVf);
		break;
	case LC_VF_T_IC4fVf:
		return sizeof(VertData_C4fVf);
		break;
	case LC_VF_L_IVf:
		return sizeof(VertData_Vf);
		break;
	case LC_VF_Q_C4fVf:
		return sizeof(VertData_C4fVf);
		break;
	case LC_VF_Q_C4fNfVf:
		return sizeof(VertData_C4fNfVf);
		break;
	case LC_VF_Q_IC4fNfVf:
		return sizeof(VertData_C4fNfVf);
		break;
	}
	Assert(false);
	return 0;
}

ResMesh* ResMesh::Create(int numverts, VertexFormat format, int numindices)
{
	ResMesh* ans = new ResMesh();

	ans->Format = format;
	ans->NumIndices = numindices;
	ans->NumVerts = numverts;
	ans->VertDataSize = ResMesh::FormatSize(format);
	ans->PackedVertData = new byte[numverts * ans->VertDataSize];
	if (numindices!=0)
		ans->Indices = new ushort[numindices];
	else
		ans->Indices = null;

	Core.Register(ans);
	return ans;
}

void ResMesh::Destroy()
{
	if (Indices)
		delete [] Indices;
	delete [] PackedVertData;
}

void ResCompNode::Clear()
{
	mRenderStage = 0;
	mPosition = 0;
	mTransform = 0;
	mColor = 0;
	mMesh = 0;
	_mStageBufferIndexOffset = INVALIDOFFSET;
	_mStageBufferVertexOffset = INVALIDOFFSET;
	mChildren.Clear(true);
}

void ResCompNode::VisitChildren(NodeVisitFunc func, void* data)
{
	for (int i=0; i<mChildren.Size; i++)
	{
		(*func)(this, mChildren[i], data);
	}
}

ResFloat* ResFloat::Create(float val)
{
	ResFloat* ans = new ResFloat();
	ans->mValue = val;
	Core.Register(ans);
	return ans;
}

ResPoint* ResPoint::Create(float x, float y, float z)
{
	ResPoint* ans = new ResPoint();
	ans->mPos.Set(x, y, z);
	Core.Register(ans);
	return ans;
}

void ResScaleTransform::GetTransformMatrix(MatrixStruct* to)
{
	Assert( mFactor );
	SetMatrix_Scale( *to, mFactor->mValue );
}


//
// Core Implimentation
//

StageBuffer* LewcidCore::GetRenderStage(uint mask)
{
	BAMem<StageBuffer>* work = Graphics.mStages.Base;
	while (work)
	{
		if (work->Val.mStageMask & mask)
		{
			return &work->Val;
		}
		work = work->Next;
	}
	return null;
}

void lc_core_gc_markresused(GenRes* res, GenRes** ptr, void* extra)
{
	if (!*ptr)
		return;

	if (!IsFlag( (*ptr)->_Flags, LC_ResFlag_FreeMe ))
		return;

	FlagOff( (*ptr)->_Flags, LC_ResFlag_FreeMe );

	(*ptr)->VisitRes( lc_core_gc_markresused, null );
}

void lc_core_gc_marknodeused(ResCompNode* parent, ResCompNode* child, void* extra)
{
	if (!IsFlag(child->_Flags, LC_ResFlag_FreeMe))
		return;

	FlagOff( child->_Flags, LC_ResFlag_FreeMe );

	child->VisitRes( lc_core_gc_markresused, null );

	child->VisitChildren( lc_core_gc_marknodeused, null );
}

void LewcidCore::GarbageCollect()
{
	for (int i=0; i<mAllRes.Size; i++)
	{
		mAllRes[i]->_Flags |= LC_ResFlag_FreeMe;
	}

	lc_core_gc_marknodeused(null, mRootNode, null);

	for (int j=0; j<mAllRes.Size; j++)
	{
		if (IsFlag( mAllRes[j]->_Flags, LC_ResFlag_IsBinding))
		{
			ResBinding* bind = ((ResBinding*)mAllRes[j]);
			if ((bind->mTarget) && (!IsFlag(bind->mTarget->_Flags, LC_ResFlag_FreeMe)))
			{
				FlagOff( bind->_Flags, LC_ResFlag_FreeMe );
				bind->VisitRes( lc_core_gc_markresused, null );
			}
		}
	}

	int removed = 0, to=0;
	GenRes* res;
	for (int k=0; k<mAllRes.Size; k++)
	{
		res = mAllRes[k];
		if (IsFlag( res->_Flags, LC_ResFlag_FreeMe ))
		{
			res->Destroy();
			delete res;
			removed++;
		}
		else
		{
			mAllRes[to] = res;
			to++;
		}
	}
	mAllRes.Size -= removed;
}

LewcidCore::LewcidCore()
{
	QueryPerformanceCounter(&mLastTime);
}

EArray<GenRes*> lc_resstack;
EArray<ResCompNode*> lc_nodestack;

void lc_core_markdepend(GenRes* res, GenRes** ptr, void* extra)
{
	if (!*ptr)
		return;

	if ((*ptr)->_Flags & LC_ResFlag_Changed)
	{
		*((bool*)extra) = true;
		for (int i=0; i<lc_resstack.Size; i++)
		{
			lc_resstack[i]->_Flags |= LC_ResFlag_Changed;
		}
		return;
	}

	*lc_resstack.StackPush() = *ptr;

	(*ptr)->VisitRes( lc_core_markdepend, extra);

	lc_resstack.StackPop();
}

void lc_core_marknodechanged(ResCompNode* parent, ResCompNode* child, void* extra)
{
	bool changed = false;
	child->VisitRes(lc_core_markdepend, &changed);
	child->_Flags |= *((uint*)extra);
	uint tochild = 0;
	if (changed)
	{
		tochild = LC_ResFlag_DataChanged | LC_ResFlag_ChildChanged;
		child->_Flags |= tochild;
		for (int i=0; i<lc_nodestack.Size; i++)
		{
			lc_nodestack[i]->_Flags |= LC_ResFlag_ChildChanged;
		}
	}

	*lc_nodestack.StackPush() = child;

	child->VisitChildren(lc_core_marknodechanged, &tochild);

	lc_nodestack.StackPop();
}

void LewcidCore::MarkDependencies()
{
	lc_nodestack.Clear(false);
	lc_resstack.Clear(false);

	uint nada = 0;
	lc_core_marknodechanged(null, this->mRootNode, &nada);
}

void LewcidCore::ClearChanged()
{
	for (int i=0; i<mAllRes.Size; i++)
	{
		FlagOff( mAllRes[i]->_Flags, LC_ResFlag_Changed | LC_ResFlag_DataChanged | LC_ResFlag_ChildChanged );
	}
}

void LewcidCore::Render()
{
	Assert( mRootNode );

	Tick();
	MarkDependencies();
	Graphics.Render(this);
	ClearChanged();
	GarbageCollect();
}

void LewcidCore::Tick()
{
	LARGE_INTEGER curtime;
	QueryPerformanceCounter(&curtime);
	QueryPerformanceFrequency(&mCounterFrequency);

	Time dt = ((Time)(curtime.QuadPart - mLastTime.QuadPart)) / ((Time)mCounterFrequency.QuadPart);
	mLastTime = curtime;

	ResBinding* bind;
	for (int i=0; i<mAllRes.Size; i++)
	{
		if (mAllRes[i]->_Flags & LC_ResFlag_IsBinding)
		{
			bind = (ResBinding*)mAllRes[i];
			bind->Update(dt);
		}
	}
}

//
// GL Core Implimentation
//

struct lc_gl_findnodedata
{
	uint rendermask;
	int vertoffset;
	ResCompNode* best;
	int bestoffset;

	void Setup(uint rmask, int voffset)
	{
		rendermask = rmask;
		vertoffset = voffset;
		best = null;
		bestoffset = -1;
	};
};

void lc_gl_findnode(ResCompNode* parent, ResCompNode* child, void* extra)
{
	lc_gl_findnodedata* data = ((lc_gl_findnodedata*)extra);

	if (data->rendermask == child->mRenderStage)
	{
		if ((child->mMesh) && (child->_mStageBufferVertexOffset != INVALIDOFFSET))
		{
			int voffset = child->_mStageBufferVertexOffset;
			if (voffset <= data->vertoffset)
			{
				if (voffset > data->bestoffset)
				{
					data->bestoffset = voffset;
					data->best = child;
				}
			}
		}
	}

	child->VisitChildren(lc_gl_findnode, extra);
}

ResCompNode* LewcidGLEngine::HitCompNode(int screenx, int screeny)
{
	//use OpenGL to project the screen point into the world
	GLint viewport[4]; 
	GLdouble modelview[16],projection[16]; 
	GLdouble wx,wy,wz; 
	GLdouble ox, oy, oz;

	wx = (float)screenx;
	glGetIntegerv(GL_VIEWPORT,viewport); 
	wy=(float)screeny; 
	wy=viewport[3]-wy; 
 
	glGetDoublev(GL_MODELVIEW_MATRIX,modelview); 
	glGetDoublev(GL_PROJECTION_MATRIX,projection); 
 
	ipnt from, to, f, t, d, hit;

	wz = 0.98f;
	gluUnProject(wx,wy,wz,modelview,projection,viewport,&ox,&oy,&oz);

	fpnt lookdir = FPNT(ox, oy, oz);

	//have the world points, now just pass to the normal
	// hit finder

	return HitCompNode( FPNT(0, 0, 0), lookdir );
}

ResCompNode* LewcidGLEngine::HitCompNode(fpnt camera, fpnt lookdir)
{
	BAMem<StageBuffer>* work = mStages.Base;
	while (work)
	{
		int hit = work->Val.HitTest(camera, lookdir);
		if (hit != -1)
		{
			lc_gl_findnodedata data;
			data.Setup(work->Val.mStageMask, hit);

			lc_gl_findnode(null, mCore->mRootNode, &data);

			Assert( data.best );

			return data.best;
		}
		work = work->Next;
	}
	return null;
}

fpnt lc_glht_camera;
fpnt lc_glht_lookdir;
fpnt* lc_glht_v0;
fpnt* lc_glht_v1;
fpnt* lc_glht_v2;

bool lc_glht_SameSide()
{
	fpnt cross = (  *lc_glht_v0 - lc_glht_camera);
	cross = cross.Cross( *lc_glht_v1 - lc_glht_camera );

	float other = cross.Dot( *lc_glht_v2 - lc_glht_camera );
	float look = cross.Dot( lc_glht_lookdir );

	return ((other * look) >= 0.0f);
}

int StageBuffer::HitTest(fpnt camera, fpnt lookdir)
{
	lc_glht_camera = camera;
	lc_glht_lookdir = lookdir;
	int j;
	bool same;

	//TODO: later add checking to make sure that correct z order is used
	// currently just returns the first hit object

	switch(mFormat)
	{
	case LC_VF_Q_C4fNfVf:
		{
			VertData_C4fNfVf* data = (VertData_C4fNfVf*)mData._data;
			for (int i=0; i<mNumVerts; i+=4)
			{
				j=0;
				same = true;
				while((j<4) && (same))
				{
					lc_glht_v0 = &data[i+j].mVertex;
					lc_glht_v1 = &data[i+((j+1)%4)].mVertex;
					lc_glht_v2 = &data[i+((j+2)%4)].mVertex;

					same = lc_glht_SameSide();
					j++;
				}
				if (same)
				{
					return i;
				}
			}
		}
		break;
	case LC_VF_L_IVf:
		//can't intersect with a line
		break;
	default:
		Assert( false );
		break;
	};
	return -1;
}

MatrixStruct lcgl_Mat1, lcgl_Mat2;
void lcgl_MainPass(ResCompNode* parent, ResCompNode* child, void* data)
{
	LewcidGLEngine* eng = ((LewcidGLEngine*)data);

	if ((!eng->mStageBuffer->mRebuild) && (!IsFlag(child->_Flags, LC_ResFlag_ChildChanged)))
		return;

	if (child->mColor)
	{
		Color4f col = *eng->mColorStack.StackPeak();
		col *= child->mColor->mValue;
		*eng->mColorStack.StackPush() = col;
	}
	if (child->mTransform)
	{
		fpnt pos = *eng->mTranslateStack.StackPeak();
		if (child->mPosition)
		{
			pos += child->mPosition->GetValue();
		}
		eng->mTranslateStack.StackPush()->Set(0, 0, 0);

		MatrixStruct* n = eng->mTransformStack.StackPush();
		MatrixStruct* old = eng->mTransformStack.Axs( eng->mTransformStack.Size-2 );

		SetMatrix_Translate(lcgl_Mat1, pos.x, pos.y, pos.z);
		MatrixTimesMatrix(*old, lcgl_Mat1, lcgl_Mat2);

		child->mTransform->GetTransformMatrix(&lcgl_Mat1);
		MatrixTimesMatrix(lcgl_Mat2, lcgl_Mat1, *n);
	}
	else
	{
		if (child->mPosition)
		{
			fpnt pos = *eng->mTranslateStack.StackPeak();
			pos += child->mPosition->GetValue();
			*eng->mTranslateStack.StackPush() = pos;
		}
	}


	child->VisitChildren(lcgl_MainPass, data);

	if (child->mRenderStage & eng->mCurStageMask)
	{
		if (child->mMesh)
		{
			if ((eng->mStageBuffer->mRebuild) || (child->_Flags & (LC_ResFlag_DataChanged | LC_ResFlag_Changed) ) )
			{
				// put the mesh into the the stage buffer
				eng->AddMesh( child->mMesh, child->_mStageBufferVertexOffset, child->_mStageBufferIndexOffset );
			}
		}
	}

	if (child->mColor)
	{
		eng->mColorStack.StackPop();
	}
	if (child->mTransform)
	{
		eng->mTransformStack.StackPop();
		eng->mTranslateStack.StackPop();
	}
	else
	{
		if (child->mPosition)
			eng->mTranslateStack.StackPop();
	}
}

fpnt LewcidGLEngine::TransformVertex(fpnt vert)
{
	fpnt to;
	vert += *(mTranslateStack.StackPeak());
	Vec3TimesMatrix4(vert, *this->mTransformStack.StackPeak(), to);
	return to;
}

void LewcidGLEngine::AddMesh(ResMesh* mesh, int& avertoffset, int& aindoffset)
{
	if (this->mStageBuffer->mRebuild)
	{
		avertoffset = mStageBuffer->mNumVerts;
		aindoffset = mStageBuffer->mIndices.Size;
	}
	Color4f* colmod = mColorStack.StackPeak();

	switch(mStageBuffer->mFormat)
	{
	case LC_VF_T_IC4fNfVf:
		{
			int i, numverts;

			if (mesh->Format == LC_VF_T_IC4fVf)
			{
				VertData_C4fNfVf* tovert;
				ushort* toind;
				if (mStageBuffer->mRebuild)
				{
					tovert = ((VertData_C4fNfVf*)mStageBuffer->mData.AddMany( sizeof(VertData_C4fNfVf)*(mesh->NumVerts) ));
					numverts = mStageBuffer->mNumVerts;
					mStageBuffer->mNumVerts += mesh->NumVerts;

					toind = mStageBuffer->mIndices.AddMany( mesh->NumIndices );
				}
				else
				{
					tovert = ((VertData_C4fNfVf*)mStageBuffer->mData.Axs( sizeof(VertData_C4fNfVf)*avertoffset ));
					numverts = avertoffset;

					toind = mStageBuffer->mIndices.Axs(aindoffset);
				}
				VertData_C4fVf* fromvert = ((VertData_C4fVf*)mesh->PackedVertData);
				
				for (i=0; i<mesh->NumVerts; i++)
				{
					tovert[i].mColor = fromvert[i].mColor;
					tovert[i].mColor *= *colmod;
					tovert[i].mVertex = TransformVertex( fromvert[i].mVertex );
				}

				for (i=0; i<mesh->NumIndices; i++)
				{
					toind[i] = numverts + mesh->Indices[i];
				}
			}
			else if (mesh->Format == LC_VF_T_C4fVf)
			{
				VertData_C4fNfVf* tovert;
				ushort* toind;
				if (mStageBuffer->mRebuild)
				{
					tovert = ((VertData_C4fNfVf*)mStageBuffer->mData.AddMany( sizeof(VertData_C4fNfVf)*(mesh->NumVerts) ));
					numverts = mStageBuffer->mNumVerts;
					mStageBuffer->mNumVerts += mesh->NumVerts;

					toind = mStageBuffer->mIndices.AddMany( mesh->NumVerts );
				}
				else
				{
					tovert = ((VertData_C4fNfVf*)mStageBuffer->mData.Axs( sizeof(VertData_C4fNfVf)*avertoffset ));
					numverts = avertoffset;

					toind = mStageBuffer->mIndices.Axs(aindoffset);
				}

			//	numverts = mStageBuffer->mNumVerts;
			//	VertData_C4fNfVf* tovert = ((VertData_C4fNfVf*)
			//		mStageBuffer->mData.AddMany( sizeof(VertData_C4fNfVf)*(mesh->NumVerts) ));
			//	mStageBuffer->mNumVerts += mesh->NumVerts;
			//	ushort* toind = mStageBuffer->mIndices.AddMany( mesh->NumVerts );
				VertData_C4fVf* fromvert = ((VertData_C4fVf*)mesh->PackedVertData);

				for (i=0; i<mesh->NumVerts; i++)
				{
					tovert[i].mColor = fromvert[i].mColor;
					tovert[i].mColor *= *colmod;
					tovert[i].mVertex = TransformVertex( fromvert[i].mVertex );

					toind[i] = numverts+i;
				}
			}
			else
			{
				Assert(false);
			}
		}
		break;
	case LC_VF_L_IVf:
		{
			if (mesh->Format == LC_VF_L_IVf)
			{
				VertData_Vf* vertto;
				ushort* indto;
				if (mStageBuffer->mRebuild)
				{
					vertto = (VertData_Vf*)mStageBuffer->mData.AddMany( sizeof(VertData_Vf) * mesh->NumVerts );
					mStageBuffer->mNumVerts += mesh->NumVerts;
					indto = mStageBuffer->mIndices.AddMany( mesh->NumIndices );
				}
				else
				{
					vertto = (VertData_Vf*)mStageBuffer->mData.Axs( avertoffset * sizeof(VertData_Vf) );
					indto = mStageBuffer->mIndices.Axs( aindoffset );
				}

				VertData_Vf* vertfrom = (VertData_Vf*)mesh->PackedVertData;
				ushort* indfrom = mesh->Indices;
				int i;
				for (i=0; i<mesh->NumVerts; i++)
				{
					vertto[i].mVertex = TransformVertex( vertfrom[i].mVertex );
				}
				for (i=0; i<mesh->NumIndices; i++)
				{
					indto[i] = avertoffset + indfrom[i];
				}
			}
			else
			{
				Assert(false);
			}
		}
		break;
	case LC_VF_Q_C4fNfVf:
		{
			int i;

			if (mesh->Format == LC_VF_Q_C4fVf)
			{
				VertData_C4fNfVf* tovert;
				if (mStageBuffer->mRebuild)
				{
					tovert = ((VertData_C4fNfVf*)
						mStageBuffer->mData.AddMany( sizeof(VertData_C4fNfVf)*(mesh->NumVerts) ));
					mStageBuffer->mNumVerts += mesh->NumVerts;
				}
				else
				{
					tovert = ((VertData_C4fNfVf*)mStageBuffer->mData.Axs( avertoffset * sizeof(VertData_C4fNfVf) ));
				}

				VertData_C4fVf* fromvert = ((VertData_C4fVf*)mesh->PackedVertData);

				for (i=0; i<mesh->NumVerts; i++)
				{
					tovert[i].mColor = fromvert[i].mColor;
					tovert[i].mColor *= *colmod;
					tovert[i].mVertex = TransformVertex( fromvert[i].mVertex );
				}
			}
			else
			{
				Assert(false);
			}
		}
		break;
	default:
		Assert(false);
		break;
	}

}

bool ResMesh::FormatIsIndexed(VertexFormat format)
{
	switch (format)
	{
	case LC_VF_T_C4fNfVf:
		return false;
	case LC_VF_T_IC4fNfVf:
		return true;
	case LC_VF_T_C4fVf:
		return false;
	case LC_VF_T_IC4fVf:
		return true;
	default:
		Assert(false);
		return false;
	}
}

bool ResMesh::FormatHasNormals(VertexFormat format)
{
	switch (format)
	{
	case LC_VF_T_IC4fNfVf:
		return true;
	case LC_VF_T_C4fNfVf:
		return true;
	case LC_VF_T_C4fVf:
		return false;
	case LC_VF_T_IC4fVf:
		return false;
	case LC_VF_Q_C4fVf:
		return false;
	case LC_VF_Q_C4fNfVf:
		return true;
	case LC_VF_L_IVf:
		return false;
	default:
		Assert(false);
		return false;
	}
}

void StageBuffer::CalcNormals()
{
	//make sure has normals:
	if (!ResMesh::FormatHasNormals(mFormat))
	{
		return;
	}

	if (mFormat == LC_VF_T_IC4fNfVf)
	{
		VertData_C4fNfVf* vert = ((VertData_C4fNfVf*)mData._data);
		int i;
		IndTri3* itris = ((IndTri3*)(mIndices._data));
		int numitris = this->mIndices.Size/3;

		//set normals to 0;
		for (i=0; i<mNumVerts; i++)
		{
			vert[i].mNormal.Set(0, 0, 0);
		}

		//add up the normals for each triangle
		for (i=0; i<numitris; i++)
		{
			fpnt n = vert[itris[i].a].mVertex - vert[itris[i].b].mVertex;
			n = n.Cross( vert[itris[i].c].mVertex - vert[itris[i].b].mVertex );
			n.Normalize();
			vert[itris[i].a ].mNormal += n;
			vert[itris[i].b ].mNormal += n;
			vert[itris[i].c ].mNormal += n;
		}

		//normalize their sum
		for (i=0; i<mNumVerts; i++)
		{
			vert[i].mNormal.Normalize();
		}
	}
	else if (mFormat == LC_VF_Q_C4fNfVf)
	{
		VertData_C4fNfVf* vert = ((VertData_C4fNfVf*)mData._data);
		int i;

		//add up the normals for each quad
		for (i=0; i<mNumVerts; i+=4)
		{
			fpnt n = vert[i+1].mVertex - vert[i].mVertex;
			n = n.Cross( vert[i+1].mVertex - vert[i+2].mVertex );
			n.Normalize();
			vert[i+0].mNormal = n;
			vert[i+1].mNormal = n;
			vert[i+2].mNormal = n;
			vert[i+3].mNormal = n;
		}
	}
/*	else if (mFormat == LC_VF_T_C4fNfVf)
	{
		VertData_C4fNfVf* vert = ((VertData_C4fNfVf*)mData._data);
		for (int i=0; i < mNumVerts; i+=3)
		{
			fpnt n = vert[i+1].mVertex - vert[i].mVertex;
			n = n.Cross( vert[i+1].mVertex - vert[i+2].mVertex );
			n.Normalize();

			vert[i].mNormal = n;
			vert[i+1].mNormal = n;
			vert[i+2].mNormal = n;
		}
	} */
	else
	{
		Assert(false);
	}
}

void StageBuffer::Flush()
{
	if (mData.Size == 0)
		return;

	if (mFormat == LC_VF_T_IC4fNfVf)
	{
		CalcNormals();

		/*
		VertData_C4fNfVf* data = ((VertData_C4fNfVf*)mData._data);

		glBegin( GL_TRIANGLES );
		for (int i=0; i<mIndices.Size; i++)
		{
			int ind = mIndices[i];
			glColor3f( data[ind].mColor.Red, data[ind].mColor.Green, data[ind].mColor.Blue );
			glNormal3fv( data[ind].mNormal );
			glVertex3fv( data[ind].mVertex );
		}
		glEnd();
		*/

		/*
		fpnt* colors = ((fpnt*)mData._data);
		fpnt* verts = ((fpnt*)mData._data) + 1;

		glBegin( GL_TRIANGLES );
		for (int i=0; i<mIndices.Size; i++)
		{
			int ind = mIndices[i];
			glColor3fv( colors[ind*2] );
			glVertex3fv( verts[ind*2] );
		}
		glEnd();
		*/

		//glEnableClientState( GL_VERTEX_ARRAY  );
		//glEnableClientState( GL_COLOR_ARRAY );
		//glVertexPointer(3, GL_FLOAT, sizeof(fpnt), verts );
		//glColorPointer (3, GL_FLOAT, sizeof(fpnt), colors );


		glInterleavedArrays( GL_C4F_N3F_V3F, 0, mData._data );
		glDrawElements( GL_TRIANGLES, mIndices.Size, GL_UNSIGNED_SHORT, mIndices._data );
	}
	else if (mFormat == LC_VF_Q_C4fNfVf)
	{
		CalcNormals();
		glInterleavedArrays( GL_C4F_N3F_V3F, 0, mData._data );
		glDrawArrays( GL_QUADS, 0, mNumVerts );
	}
	else if (mFormat == LC_VF_Q_IC4fNfVf)
	{
		CalcNormals();
		glInterleavedArrays( GL_C4F_N3F_V3F, 0, mData._data );
		glDrawElements( GL_QUADS, mIndices.Size, GL_UNSIGNED_SHORT, mIndices._data );
	}
	else if (mFormat == LC_VF_L_IVf)
	{
		glColor4f( mDefaultColor.Red, mDefaultColor.Green, mDefaultColor.Blue, mDefaultColor.Alpha );
		glInterleavedArrays( GL_V3F, 0, mData._data );
		glDrawElements( GL_LINES, mIndices.Size, GL_UNSIGNED_SHORT, mIndices._data );
	}
	else
	{
		Assert(false);
	}

//	mData.Clear(false);
//	mIndices.Clear(false);
//	mNumVerts = 0;
}

void lcgl_IsStageRequiresRebuild(ResCompNode* parent, ResCompNode* child, void* data)
{
	LewcidGLEngine* eng = ((LewcidGLEngine*)data);

	if ((child->mRenderStage & eng->mCurStageMask) && (child->mMesh))
	{
		if (child->_Flags & LC_ResFlag_Changed)
		{
			eng->mStageBuffer->mRebuild = true;
			return;
		}
		if (child->_mStageBufferVertexOffset == INVALIDOFFSET)
		{
			eng->mStageBuffer->mRebuild = true;
			return;
		}
	}

	child->VisitChildren( lcgl_IsStageRequiresRebuild, data );
}

uint LewcidGLEngine::AddRenderStage(VertexFormat format, uint flags)
{
	StageBuffer* stage = this->mStages.AddMember();
	stage->mStageMask = (1<<(this->mStages.Length()-1));
	stage->mFormat = format;
	stage->mRenderFlags = flags;
	stage->mNumVerts = 0;
	stage->mData.ChunkSize = 20*ResMesh::FormatSize(format);
	stage->mIndices.ChunkSize = 20;
	return stage->mStageMask;
}

void LewcidGLEngine::Render(LewcidCore* core)
{
	mCore = core;

	// render each stage
	BAMem<StageBuffer>* work = mStages.Base;
	while (work)
	{
		mStageBuffer = &work->Val;
		this->mCurStageMask = mStageBuffer->mStageMask;

		// initialize the translate and transform stacks
		mTranslateStack.Clear(false);
		mTranslateStack.StackPush()->Set(0, 0, 0);

		mTransformStack.Clear(false);
		SetMatrix_Identity( *mTransformStack.StackPush() );

		mColorStack.Clear(false);
		mColorStack.StackPush()->Set(1, 1, 1, 1);

		//check to see if we need to rebuild
		this->mStageBuffer->mRebuild = false;
		lcgl_IsStageRequiresRebuild(null, core->mRootNode, this);

		//if rebuilding, clear the buffers
		if (this->mStageBuffer->mRebuild)
		{
			mStageBuffer->mData.Clear(false);
			mStageBuffer->mNumVerts = 0;
			mStageBuffer->mIndices.Clear(false);
		}

		//build buffer
		lcgl_MainPass(null, core->mRootNode, this);

		//render it
		mStageBuffer->Flush();

		work = work->Next;
	}
}