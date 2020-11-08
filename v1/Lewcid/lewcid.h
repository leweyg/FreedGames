
//
// The Lewcid Composition and Interation Engine
//
// Version 0.1 (Rewrite 1)
//
// Rewrite 1 - 5/2/2004
// Rewrite 2 - 6/12/2004
//

//#define UseAsserts 1

//#if UseAsserts
#include <Assert.h>
#define Assert assert
//#define Assert(T) {if(!(T)){int*_damn=0; *_damn=5;}}
//#else
//#define Assert(C)
//#endif

#define LCG_UseStencil 0

#include "pnt.cpp"
#include "matrixclass.cpp"
#include "earray.cpp"
#include "barray.cpp"

#define LC_SETTING_NOUPDATE		0

typedef unsigned int uint;
//typedef unsigned short ushort;
typedef unsigned char byte;
typedef uint handle;
typedef unsigned short Index;
#define null 0
typedef double Time;

#define TODO	Assert(false);

bool CALLEDDEBUG = false;
//#define CALLEDDEBUG	0

typedef void (*LC_Event)(void* data);

//Lewcid Events:
LC_Event LC_Event_DestroyCompNode = 0;
LC_Event LC_Event_ContextFlushDone = 0;
LC_Event LC_Event_GLSettingsInit = 0;

#define FIRE(event,data)	{if(event) (*event)(data);}
#define IFNOTFIRE(event,data)	if(event){(*event)(data);}else

#define FlagOn(V,F) {V |= F;}
#define FlagOff(V,F) {(V) &= ~(F);}
#define IsFlag(V,F) ((V & F)!=0)

float flabs(float a)
{
	return ((a >= 0)? a : -a);
}

double RandF()
{
	double val = rand();
	return (val/((double)RAND_MAX));
}

struct Color4f
{
	float Red, Green, Blue, Alpha;


	float& operator[] (int i) {return *((&Red)+i);};
	void operator *=(Color4f& other);
	Color4f operator *(Color4f& other);

	static Color4f Create(float red, float green, float blue)
	{
		Color4f ans;
		ans.Red = red;
		ans.Green = green;
		ans.Blue = blue;
		ans.Alpha = 1.0f;
		return ans;
	}

	static Color4f FromRGB(unsigned int rgb)
	{
		Color4f ans;

		ans.Blue = ((float)(rgb & 0xff)) / 255.0f;
		ans.Green = ((float)((rgb>>8) & 0xff)) / 255.0f;
		ans.Red = ((float)((rgb>>16) & 0xff)) / 255.0f;
		ans.Alpha = ((float)((rgb>>24) & 0xff)) / 255.0f;

		return ans;
	}

	void Set(float red, float green, float blue, float alpha)
	{
		Red = red;
		Green = green;
		Blue = blue;
		Alpha = alpha;
	};
};

struct MatrixStruct
{
	float Val[16];

	void operator=(MatrixStruct& other);
	operator float*() {return Val;};
};

class GenRes;

typedef void (*ResVisitFunc)(GenRes* res, GenRes* ptr, void* extra);

#define RESVISIT(M)		virtual void VisitRes(ResVisitFunc func, void* data) { M };
#define RESMEM(X)		(*func)( this, X, data);
#define RESPARENT(T)	{T::VisitRes(func,data);}
#define NOMEMS			{}

#define LC_ResFlag_Changed				(1<<0)
#define LC_ResFlag_IsBinding			(1<<1)
#define LC_ResFlag_ShallowChange		(1<<2)
#define LC_ResFlag_ChildChanged			(1<<3)
#define LC_ResFlag_FreeMe				(1<<4)
#define LC_ResFlag_MeshRenderContext	(1<<5)

class GenRes
{
public:
	uint _Flags;

	virtual void Destroy() {};
	virtual void VisitRes(ResVisitFunc func, void* data)=0;
	
	GenRes() {};
	virtual ~GenRes()	{};

	void Changed() {_Flags |= LC_ResFlag_Changed;};
};

class ResBinding : public GenRes
{
public:
	GenRes* mTarget;

	virtual void Update(Time dt) = 0;
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

class ResAnimateFromTo : public ResBinding
{
public:
	double mTrueProgress;
	ResAnimate *mFrom, *mTo;
	Time mLength;
	double mProgress;
	uint mFlags;
	void* mToken;
	LC_Event mEvent_WhenDone;

	virtual void Update(Time dt);
	static ResAnimateFromTo* Create(ResAnimate* target, ResAnimate* from, ResAnimate* to, Time duration, uint flags = LC_AniFlag_Bounce);

	ResAnimateFromTo() {mTrueProgress=0; mProgress=0; mFlags=0; mEvent_WhenDone=0; mToken=0;};
	RESVISIT( RESMEM(mFrom) RESMEM(mTo)  RESMEM(mTarget) );
};

class ResPoint : public ResAnimate
{
public:
	fpnt mPos;

	static ResPoint* Create(float x, float y, float z);
	static ResPoint* Create(fpnt at);

	virtual double RatioBetween(ResAnimate* from, ResAnimate* to);
	virtual void Interp(ResAnimate* from, ResAnimate* to, double ratio);

	RESVISIT( NOMEMS );
};

class ResColor : public ResAnimate
{
public:
	Color4f mValue;

	static ResColor* Create();
	static ResColor* Create(Color4f col);
	static ResColor* Create(float r, float g, float b, float a);
	virtual double RatioBetween(ResAnimate* from, ResAnimate* to);
	virtual void Interp(ResAnimate* from, ResAnimate* to, double ratio);

	RESVISIT( NOMEMS );
};

class ResFloat : public ResAnimate
{
public:
	float mValue;

	static ResFloat* Create(float val);
	virtual double RatioBetween(ResAnimate* from, ResAnimate* to);
	virtual void Interp(ResAnimate* from, ResAnimate* to, double ratio);

	RESVISIT( NOMEMS );
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

	void Add(ResTransform* res);
	void Remove(ResTransform* ans);
	int Count();
	ResTransform* Get(int i);

	virtual void GetTransformMatrix(MatrixStruct* to);
	virtual void VisitRes(ResVisitFunc func, void* data);

	static ResTransformCollection* Create();
};

class ResMatrixTransform : public ResTransform
{
public:
	MatrixStruct mMatrix;

	static ResMatrixTransform* Create();
	virtual void GetTransformMatrix(MatrixStruct* to);

	RESVISIT( NOMEMS );
};

class ResTranslateTransform : public ResTransform
{
public:
	ResPoint* mOffset;

	static ResTranslateTransform* Create(fpnt offset);
	virtual void GetTransformMatrix(MatrixStruct* to);

	RESVISIT( RESMEM(mOffset) );
};

class ResSpinTransform : public ResTransform
{
public:
	ResFloat* mAngle;
	int mAxis;

	static ResSpinTransform* Create(int axis, float ang);
	virtual void GetTransformMatrix(MatrixStruct* to);

	RESVISIT( RESMEM(mAngle) );
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

//
// Mesh Declarations
//

/*
struct VertData_Vf
{
	fpnt mVertex;
};

struct VertData_C4fVf
{
	Color4f mColor;
	fpnt mVertex;
};
*/

struct VertexType_C4fNfVf
{
	Color4f mColor;
	fpnt mNormal;
	fpnt mVertex;
};

//Values for all vertices:
#define HASNORMALS			1
#define HASCOLOR			1
typedef VertexType_C4fNfVf	VertType;
#define SIZEOFVERT	(sizeof(VertType))
VertType _vt_ignoreme;
#define COLOROFFSET		(((byte*)&_vt_ignoreme.mColor) - ((byte*)&_vt_ignoreme))
#define LOCOFFSET		(((byte*)&_vt_ignoreme.mVertex) - ((byte*)&_vt_ignoreme))
#define GLVERTFORMAT	GL_C4F_N3F_V3F
#define GLINDEXTYPE		GL_UNSIGNED_SHORT

//Mesh Flags
#define LC_MF_Tri		(1<<0)
#define LC_MF_Quad		(1<<1)
#define LC_MF_Line		(1<<2)
#define LC_MF_Indexed	(1<<3)

struct MeshFormat
{
	int VPP; //vertices per polygon
	byte Flags;

	bool IsIndexed();

	GLenum GLPolyType()
	{
		switch(VPP)
		{
		case 2:
			return GL_LINES;
		case 3:
			return GL_TRIANGLES;
		case 4:
			return GL_QUADS;
		default:
			Assert( false );
			return 0;
		}
	}

	static MeshFormat Create(int vpp, bool isindexed)
	{
		byte format = ((isindexed)? LC_MF_Indexed : 0);
		if (vpp==2)
			format |= LC_MF_Line;
		if (vpp==3)
			format |= LC_MF_Tri;
		if (vpp==4)
			format |= LC_MF_Quad;
		Assert( (vpp>=2) && (vpp<=4) );

		MeshFormat ans;
		ans.Flags = format;
		ans.VPP = vpp;
		return ans;
	}
	
	static MeshFormat Create(byte flags)
	{
		MeshFormat ans; 
		ans.Flags=flags;

		ans.VPP = -1;
		if (IsFlag(ans.Flags, LC_MF_Tri))
			ans.VPP=3;
		if (IsFlag(ans.Flags, LC_MF_Quad))
			ans.VPP=4;
		if (IsFlag(ans.Flags, LC_MF_Line))
			ans.VPP=2;
		Assert( ans.VPP!=-1 );

		return ans;
	};
};

struct IndPoly2
{
	Index a, b;
	void Set(Index one, Index two)
	{
		a = one;
		b = two;
	};
	Index& operator[] (int i) {return *((&a)+i);};
};

struct IndPoly3
{
	Index a, b, c;
	void Set(Index one, Index two, Index three)
	{
		a = one;
		b = two;
		c = three;
	};
	Index& operator[] (int i) {return *((&a)+i);};
};

struct IndPoly4
{
	Index a, b, c, d;

	void Set(Index one, Index two, Index three, Index four)
	{
		a = one;
		b = two;
		c = three;
		d = four;
	};
	Index& operator[] (int i) {return *((&a)+i);};
};

class ResCompNode;
class ResMesh;
class ResCompTreePath;
class ResMeshEdger;

typedef bool (*HitPolyFunc)(void* extra, int poly, fpnt at);
typedef bool (*HitNodeFunc)(ResCompTreePath* path);

class ResMesh : public GenRes
{
public:
	EArray<byte> VertData;
	EArray<Index> Indices;
	ResMeshEdger* mEdger;	//THIS IS NOT GARBAGE COLLECTED!!!
	int NumVerts;
	int NumIndices;
	MeshFormat Format;

	void Clear(bool freemem);
	int NumPolys();
	void ResizeTo(int numverts, int numindices);
	void Add(int numverts, int numindices);
	int PolyHitTest(fpnt camera, fpnt lookdir, HitPolyFunc func=0, void* extra=0);

	byte* RecentVertData(int vertcount);
	Index* RecentIndices(int count);

	static ResMesh* Create(MeshFormat format, int numverts, int numindices);
	//static ResMesh* CreateFromOFF(char* filename);
	//void ExportToOFF(char* filename);

	void UpdateNormals();
	void UpdateNormals(int vstart, int vend, int istart, int iend);

	virtual void Destroy() {VertData.Clear(true); Indices.Clear(true);};
	//virtual void VisitRes(ResVisitFunc func, void* data);
	RESVISIT( NOMEMS );
};

struct MeshEdge
{
	int Vert1, Vert2;
	int Poly1, Poly2;
};

class ResMeshEdger : public GenRes
{
public:
	ResMesh* mBoundMesh;		//the mesh this is bound to
	EArray<fpnt> mPolyNormals;	//array of normals for each polygon
	EArray<MeshEdge> mEdges;	//list of all edges in the mesh
	EArray<Index> mOutline;		//pairs of indices of vertices which make up the outline
	ResPoint* mRefPoint;		//reference point used to calculate outline
	float mLineWidth;
	Index* mFakeIndices;

	void CheckBufferSize();		//checks the buffer sizes to match current mesh
	virtual void UpdateNormals(int vstart, int vend, int istart, int iend);	//updates the polynormal buffer
	void UpdateNormals()		{UpdateNormals(0, mBoundMesh->NumVerts, 0,-1);};
	int GetUnIndexedIndex(int vert);
	void UpdateEdgeList();		//recalculates the edge list
	virtual void UpdateOutline();		//recalculates the outline (assumes everything is up to date)

	void FlushOutline();		//renders the outline using OpenGL

	ResMeshEdger() {mBoundMesh=0; mRefPoint=0; mLineWidth=8; mFakeIndices=0;};
	~ResMeshEdger() {if (mFakeIndices) {delete [] mFakeIndices;}}
	static ResMeshEdger* Create(ResMesh* mesh);

	RESVISIT( RESMEM(mBoundMesh) RESMEM(mRefPoint) );

private:
	void AddEdge(int v1, int v2, int poly);
};

class ResStencilEdger : public ResMeshEdger
{
public:
	EArray<fpnt> mVerts;
	EArray<Index> mInvMesh;
	bool mIsInvisable;

	virtual void UpdateOutline();
	void FlushShadow();

	ResStencilEdger() {mIsInvisable=false;};
	static ResStencilEdger* Create(ResMesh* mesh);

	RESVISIT( RESPARENT(ResMeshEdger) );
};

//
// CompNode Declaration
//

class RenderContext : public GenRes
{
public:
	uint ContextMask;
	Color4f DefaultColor;
	bool mIgnoreHit;

	LC_Event mEvent_PreFlush;
	LC_Event mEvent_ReplaceFlush;
	LC_Event mEvent_PostFlush;

	virtual void PushTransform(ResTransform* trans)=0;
	virtual void PushPosition(fpnt pos)=0;
	virtual void PushColor(ResColor* color)=0;

	virtual void PopTransform()=0;
	virtual void PopPosition()=0;
	virtual void PopColor()=0;

	virtual void Clear(bool freemem)=0;
	virtual void DrawMesh(uint mask, ResMesh* from, bool update)=0;
	virtual void Flush()=0;
	virtual void DrawCompTree(ResCompNode* tree, bool update);
	virtual void ResetForUpdate()=0;
	virtual void SkipThisMesh(uint mask, ResMesh* node)=0;

	RenderContext() {mIgnoreHit=false; DefaultColor.Set(0.5, 0.5, 0.5, 1.0); mEvent_PreFlush=0; mEvent_ReplaceFlush=0; mEvent_PostFlush=0;};
	virtual ResCompNode* HitTest(ResCompNode* root, fpnt camera, fpnt lookdir, HitNodeFunc func=0, void* extra=0, ResCompTreePath* path=0)=0;

protected:
};

typedef void (*NodeVisitFunc)(ResCompNode* parent, ResCompNode* child, void* data);

class ResCompNode : public GenRes
{
public:
	uint mRenderContextMask;
	ResPoint* mPosition;
	ResTransform *mTransform;
	ResColor *mColor;
	ResMesh *mMesh;
	EArray<ResCompNode*> mChildren;
	void* mToken;

	void AddChild(ResCompNode* child);
	void RemoveChild(ResCompNode* child);
	void RemoveAllChildren();
	static ResCompNode* Create(uint renderstage);
	void Clear();
	void VisitChildren(NodeVisitFunc func, void* data);
	void RenderTo(RenderContext* to, bool full);

	virtual void Destroy() { FIRE(LC_Event_DestroyCompNode,this); Clear();};
	RESVISIT( RESMEM( mPosition ) RESMEM( mTransform ) RESMEM( mColor ) RESMEM( mMesh ) );
};

//
// Core Implimentation
//

class LewcidCore;

class ResGraphicsCore : public GenRes
{
public:
	ResPoint* CameraPos;
	ResPoint* CameraLookDir;
	ResPoint* CameraUp;
	ResPoint* LightPos;
	ResColor* BackgroundColor;
	ResFloat* CameraPerspec;
	bool mIsShiny;

	int ScrWidth;
	int ScrHeight;
	RenderContext* mRenderContext;
	bool RebuildTree;

	void LookAt(fpnt p);
	fpnt LookDir(int screenx, int screeny);
	ResCompNode* HitCompNode(int screenx, int screeny, bool best=true);
	ResCompNode* HitCompNode(fpnt camera, fpnt lookdir);
	ResCompNode* BestHitCompNode(fpnt camera, fpnt lookdir);
	fpnt Project(fpnt world);

	uint AddRenderStage(MeshFormat format);
	RenderContext* GetRenderStage(uint mask);

	//be sure to call "InnerRender", setup OGL, and then "Flush"
	void InnerRender(LewcidCore* core);
	void Flush();

	void GLInitFrame();
	void GLInitView();

	ResGraphicsCore();

	RESVISIT( RESMEM(CameraPos) RESMEM(CameraLookDir)  RESMEM(CameraUp) 
		RESMEM(LightPos) RESMEM(mRenderContext) RESMEM(BackgroundColor) );

private:
	LewcidCore* mCore;
};

class LewcidCore
{
	EArray<GenRes*> mAllRes;
	Time mLastTime;

public:
	ResGraphicsCore* Graphics;
	ResCompNode* mRootNode;
	GenRes* mGameRes;

	void ClearChanged();
	void Tick();
	void Register(GenRes* res) { *mAllRes.Add() = res; res->_Flags = LC_ResFlag_Changed; };
	void Render();
	void MarkDependencies();
	void GarbageCollect();

	LewcidCore();
} Core;

class ResCompTreePath : public GenRes
{
public:
	EArray<ResCompNode*> Path;
	void* Extra;
	fpnt HitAt;

	void Push(ResCompNode* node) {*Path.StackPush() = node;};
	void Pop() {Path.StackPop();};
	ResCompNode* Current() {return *Path.StackPeak();};
	void Clear() {Path.Clear(false);};

	RESVISIT( NOMEMS );

	ResCompTreePath() {Core.Register(this);};

	static ResCompTreePath* Create() {return new ResCompTreePath();};
};

//
// Res Implimentation
//

bool MeshFormat::IsIndexed()
{
	return IsFlag( Flags, LC_MF_Indexed );
}

Color4f Color4f::operator *(Color4f& other)
{
	Color4f ans;
	ans.Red = (Red * other.Red);
	ans.Blue = (Blue * other.Blue);
	ans.Green = (Green * other.Green);
	ans.Alpha = (Alpha * other.Alpha);
	return ans;
}

void Color4f::operator *=(Color4f& other)
{
	Red *= other.Red;
	Green *= other.Green;
	Blue *= other.Blue;
	Alpha *= other.Alpha;
}

ResColor* ResColor::Create(Color4f c)
{
	return ResColor::Create(c.Red, c.Green, c.Blue, c.Alpha);
}

ResColor* ResColor::Create()
{
	return ResColor::Create( 1.0, 1.0, 1.0, 1.0 );
}

ResTransformCollection* ResTransformCollection::Create()
{
	ResTransformCollection* ans = new ResTransformCollection();
	Core.Register(ans);
	return ans;
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

        int i;
	for (i=0; i<4; i++)
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

double ResPoint::RatioBetween(ResAnimate* afrom, ResAnimate* ato)
{
	ResPoint* from = (ResPoint*)afrom;
	ResPoint* to = (ResPoint*)ato;

	if (flabs(from->mPos.x - to->mPos.x) != 0)
	{
		return (mPos.x - from->mPos.x) / (to->mPos.x - from->mPos.x);
	}
	if (flabs(from->mPos.y - to->mPos.y) != 0)
	{
		return (mPos.y - from->mPos.y) / (to->mPos.y - from->mPos.y);
	}
	return (mPos.z - from->mPos.z) / (to->mPos.z - from->mPos.z);
}

void ResPoint::Interp(ResAnimate* afrom, ResAnimate* ato, double ratio)
{
	ResPoint* from = (ResPoint*)afrom;
	ResPoint* to = (ResPoint*)ato;

	mPos.x = to->mPos.x*ratio + from->mPos.x*(1.0-ratio);
	mPos.y = to->mPos.y*ratio + from->mPos.y*(1.0-ratio);
	mPos.z = to->mPos.z*ratio + from->mPos.z*(1.0-ratio);
	this->Changed();
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

ResMatrixTransform* ResMatrixTransform::Create()
{
	ResMatrixTransform* ans = new ResMatrixTransform();
	Core.Register(ans);
	SetMatrix_Identity( ans->mMatrix );
	return ans;
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
		while (mTrueProgress > 1.0)
			mTrueProgress -= 1.0;
		mProgress = mTrueProgress;
	}
	if (mFlags & LC_AniFlag_Bounce)
	{
		while (mTrueProgress >= 2.0)
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
			mProgress = mTrueProgress;
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
			FIRE(mEvent_WhenDone, this);
			mTarget = null;
			return;
		}
	}
}

void MatrixStruct::operator=(MatrixStruct& other)
{
//	CopyMemory( &Val[0], &other.Val[0], sizeof(MatrixStruct) );
    for (int i=0; i<16; i++)
        Val[i] = other.Val[i];
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

void ResMatrixTransform::GetTransformMatrix(MatrixStruct* to)
{
	*to = mMatrix;
}

ResTranslateTransform* ResTranslateTransform::Create(fpnt offset)
{
	ResTranslateTransform* ans = new ResTranslateTransform();
	Core.Register(ans);
	ans->mOffset = ResPoint::Create( offset );
	return ans;
}

ResSpinTransform* ResSpinTransform::Create(int axis, float ang)
{
	ResSpinTransform* ans = new ResSpinTransform();
	Core.Register(ans);
	ans->mAxis = axis;
	ans->mAngle = ResFloat::Create(ang);
	return ans;
}

void ResSpinTransform::GetTransformMatrix(MatrixStruct* to)
{
	Assert( mAngle!= 0 );
	Assert( (mAxis>=0) && (mAxis<=3) );
	switch(mAxis)
	{
	case 0:
		SetMatrix_RotateX( *to, mAngle->mValue );
		break;
	case 1:
		SetMatrix_RotateY( *to, mAngle->mValue );
		break;
	case 2:
		SetMatrix_RotateZ( *to, mAngle->mValue );
		break;
	default:
		Assert( false );
	}
}

void ResTranslateTransform::GetTransformMatrix(MatrixStruct* to)
{
	Assert( mOffset!=0 );
	fpnt off = mOffset->mPos;
	SetMatrix_TranslateSub(*to, off.x, off.y, off.z);
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
	node->mToken = 0;
	node->mRenderContextMask = renderstage;
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

void ResCompNode::Clear()
{
	mRenderContextMask = 0;
	mPosition = 0;
	mTransform = 0;
	mColor = 0;
	mMesh = 0;
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

ResPoint* ResPoint::Create(fpnt at)
{
	ResPoint* ans = new ResPoint();
	ans->mPos = at;
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

void ResCompNode::AddChild(ResCompNode* child)
{
	(*mChildren.Add()) = child;
	Changed();
}

void ResCompNode::RemoveAllChildren()
{
	mChildren.Clear(true);
}

void ResCompNode::RemoveChild(ResCompNode* child)
{
	int i = mChildren.FindEqual(child);
	if (i >= 0)
	{
		mChildren.Remove( i );
		Changed();
	}
}

//
// Core Implimentation
//

void lc_core_gc_markresused(GenRes* res, GenRes* ptr, void* extra)
{
	if (!ptr)
		return;

	if (!IsFlag( (ptr)->_Flags, LC_ResFlag_FreeMe ))
		return;

	FlagOff( (ptr)->_Flags, LC_ResFlag_FreeMe );

	(ptr)->VisitRes( lc_core_gc_markresused, null );
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
	lc_core_gc_markresused(null, Graphics, null);
	lc_core_gc_markresused(null, mGameRes, null);

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
	if (removed!=0) printf("Collected %d resources(s)\n", removed);
	mAllRes.Size -= removed;
}

LewcidCore::LewcidCore()
{
	mLastTime = -1;

	mGameRes = 0;
	Graphics = new ResGraphicsCore();
}

EArray<GenRes*> lc_resstack;
EArray<ResCompNode*> lc_nodestack;

void lc_core_markdepend(GenRes* res, GenRes* ptr, void* extra)
{
	if (!ptr)
		return;

	if (ptr->_Flags & LC_ResFlag_Changed)
	{
		*((bool*)extra) = true;
		for (int i=0; i<lc_resstack.Size; i++)
		{
			lc_resstack[i]->_Flags |= LC_ResFlag_Changed;
		}
		return;
	}

	*lc_resstack.StackPush() = ptr;

	(ptr)->VisitRes( lc_core_markdepend, extra);

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
		if ((child->mMesh) && (child->mMesh->_Flags & LC_ResFlag_Changed))
			child->_Flags |= LC_ResFlag_Changed;

		tochild = LC_ResFlag_ShallowChange | LC_ResFlag_ChildChanged;
		child->_Flags |= tochild;
		for (int i=0; i<lc_nodestack.Size; i++)
		{
			lc_nodestack[i]->_Flags |= LC_ResFlag_ChildChanged;
		}
	}
	if (child->_Flags & LC_ResFlag_Changed)
		Core.Graphics->RebuildTree = true;

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

	bool changed = false;
	lc_core_markdepend(null, this->Graphics, &changed);
}

void LewcidCore::ClearChanged()
{
	for (int i=0; i<mAllRes.Size; i++)
	{
		mAllRes[i]->_Flags &= ~(LC_ResFlag_Changed | LC_ResFlag_ShallowChange | LC_ResFlag_ChildChanged );
	}
	Graphics->RebuildTree = false;
}

void LewcidCore::Render()
{
	Assert( mRootNode );

	Tick();
	MarkDependencies();
	Graphics->InnerRender(this);
	Graphics->Flush();
	ClearChanged();
	GarbageCollect();
}

void LewcidCore::Tick()
{
	Time ct = LC_GetCurrentTime();
	if (mLastTime < 0)
		mLastTime = ct;
	Time dt = (ct - mLastTime);
	mLastTime = ct;

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
// Graphics Tools Implimentation
//

ResStencilEdger* ResStencilEdger::Create(ResMesh* mesh)
{
	ResStencilEdger* res = new ResStencilEdger();
	Core.Register(res);
	res->mBoundMesh = mesh;
	return res;
}

ResMeshEdger* ResMeshEdger::Create(ResMesh* mesh)
{
	ResMeshEdger* res = new ResMeshEdger();
	Core.Register(res);
	res->mBoundMesh = mesh;
	return res;
}

void ResMeshEdger::CheckBufferSize()
{
	Assert( mBoundMesh!=0 );

	int nump = mBoundMesh->NumPolys();
	if (mPolyNormals.Size == nump)
		return;

	mPolyNormals.ResizeTo( nump );
}

void ResMeshEdger::UpdateNormals(int vstart, int vend, int istart, int iend)
{
	CheckBufferSize();

	//TODO: right now it just calculates all the normals, possibly fix this later

	if (iend == -1)
	{
		vstart = 0;
		vend = mBoundMesh->NumVerts;
		istart = 0;
		iend = mBoundMesh->Indices.Size-1;
	}

	int vpp = mBoundMesh->Format.VPP;
	Assert( vpp >= 3 );
	VertType* data = (VertType*)mBoundMesh->VertData.Raw();
	Index* inds = mBoundMesh->Indices.Raw();

	fpnt a, b, c;
	int i;

	if (mBoundMesh->Format.IsIndexed())
	{
		for (i=istart; i<iend; i+=vpp)
		{
			a = data[inds[i+1]].mVertex - data[inds[i]].mVertex;
			b = data[inds[i+1]].mVertex - data[inds[i+2]].mVertex;
			mPolyNormals[i/vpp] = a.Cross(b);
		}
	}
	else
	{
		for (i=vstart; i<vend; i+=vpp)
		{
			a = data[i+1].mVertex - data[i].mVertex;
			b = data[i+1].mVertex - data[i+2].mVertex;
			mPolyNormals[i/vpp] = a.Cross(b);
		}
	}

	/*
	for (int i=0; i<mPolyNormals.Size; i++)
		mPolyNormals[i].Set(0, 0, 0);

	int vpp = mBoundMesh->Format.VPP;
	int poly=-1;
	int numi = mBoundMesh->Indices.Size;
	VertType* data = (VertType*)mBoundMesh->VertData.Raw();
	Index* inds = mBoundMesh->Indices.Raw();

	for (int j=0; j<numi; j++)
	{
		if ((j%vpp)==0)
			poly++;

		mPolyNormals[poly] += data[ inds[j] ].mNormal;
	}
	*/
}

void ResMeshEdger::AddEdge(int v1, int v2, int poly)
{
	MeshEdge* edges = mEdges.Raw(), *edge;
	int size = mEdges.Size;

	for (int i=0; i<size; i++)
	{
		edge = &edges[i];
		if (((edge->Vert1==v1) && (edge->Vert2==v2)) ||
			((edge->Vert1==v2) && (edge->Vert2==v1)))
		{
			//TODO: take this assert out, only to test my code
			Assert( edge->Poly2==-1 );
			//if (edge->Poly2!=-1)
			//	printf("Edge problem: %d replaces %d\n", poly, edge->Poly2);

			edge->Poly2 = poly;
			return;
		}
	}

	edge = mEdges.Add();
	edge->Vert1 = v1;
	edge->Vert2 = v2;
	edge->Poly1 = poly;
	edge->Poly2 = -1;
}

int ResMeshEdger::GetUnIndexedIndex(int vert)
{
	VertType* verts = (VertType*)mBoundMesh->VertData.Raw();
	fpnt f = verts[vert].mVertex;
	for (int i=0; i<vert; i++)
	{
		if (verts[i].mVertex == f)
			return i;
	}
	return vert;
}

void ResMeshEdger::UpdateEdgeList()
{
	Assert( mBoundMesh!=0 );

	Index* inds = mBoundMesh->Indices.Raw();
	int ni = mBoundMesh->Indices.Size;
	int poly=-1;
	int vpp = mBoundMesh->Format.VPP;
	int next[3] = {1, 1, -2};
	mEdges.Clear(false);

	if (mBoundMesh->Format.IsIndexed())
	{
		for (int i=0; i<ni; i++)
		{
			if ((i%vpp)==0)
				poly++;

			if ((i%vpp)!=(vpp-1))
				AddEdge( inds[i], inds[i+1], poly);
			else
				AddEdge( inds[i-(vpp-1)], inds[i], poly);
		}
	}
	else
	{
		ni = mBoundMesh->NumVerts;
		mFakeIndices = new Index[ mBoundMesh->NumVerts ];
		inds = mFakeIndices;
		int i;
		for (i=0; i<ni; i++)
		{
			inds[i] = GetUnIndexedIndex(i);
		}
		for (i=0; i<ni; i++)
		{
			if ((i%vpp)==0)
				poly++;

			if ((i%vpp)!=(vpp-1))
				AddEdge( inds[i], inds[i+1], poly);
			else
				AddEdge( inds[i-(vpp-1)], inds[i], poly);
		}
	}
}

void ResStencilEdger::UpdateOutline()
{
	Assert( mBoundMesh!=0 );
	Assert( mRefPoint!=0 );

	mOutline.Clear(false);
	mVerts.ResizeTo( mBoundMesh->NumVerts*2 );
	Index* nline;
	Index* inds = mFakeIndices;
	if (mBoundMesh->Format.IsIndexed())
		inds = mBoundMesh->Indices.Raw();

	MeshEdge* edge;
	fpnt work, work2;
	VertType* verts = (VertType*)mBoundMesh->VertData.Raw();
	fpnt* shadverts = mVerts.Raw();
	int vpp = mBoundMesh->Format.VPP;

	for (int v=0; v<mBoundMesh->NumVerts; v++)
	{
		shadverts[v] = verts[v].mVertex;
		work = verts[v].mVertex;
		work -= mRefPoint->mPos;
		work *= 40.0f;
		work += mRefPoint->mPos;
		shadverts[v + mBoundMesh->NumVerts] = work;
	}

	if (mIsInvisable)
	{
		if (mBoundMesh->Format.IsIndexed())
		{
			TODO;
		}
		else
		{
			mInvMesh.Clear(false);
			for (int j=0; j<mPolyNormals.Size; j++)
			{
				work = mRefPoint->mPos - verts[ j*vpp ].mVertex;
				float a = work.Dot( mPolyNormals[j] );
				if (a > 0)
				{
					nline = mInvMesh.AddMany(vpp);
					for (int k=0; k<vpp; k++)
						nline[k] = (j*vpp)+k;
				}
			}
		}
	}

	for (int i=0; i<mEdges.Size; i++)
	{
		edge = mEdges.Axs(i);

		work = mRefPoint->mPos - verts[ edge->Vert1 ].mVertex;
		work2 = mRefPoint->mPos - verts[ edge->Vert2 ].mVertex;
		work += work2;
		float d1 = work.Dot( mPolyNormals[ edge->Poly1 ] );
		float d2 = work.Dot( mPolyNormals[ edge->Poly2 ] );
		if (d1*d2 <= 0)
		{
			//make sure edge is oriented correctly:
			//TODO: make this faster
			Index* poly;
			if (d1 >= d2)
				poly = &inds[edge->Poly1*vpp];
			else
				poly = &inds[edge->Poly2*vpp];

			if (vpp==3)
			{
				if (poly[0]!=edge->Vert1)
				{
					if (poly[1]==edge->Vert1)
					{
						if (poly[0]==edge->Vert2)
						{
							Index t = edge->Vert1;
							edge->Vert1 = edge->Vert2;
							edge->Vert2 = t;
						}
					}
					else
					{
						if( poly[2] != edge->Vert1 )
							printf("damn\n");

						if (poly[1] == edge->Vert2)
						{
							Index t = edge->Vert1;
							edge->Vert1 = edge->Vert2;
							edge->Vert2 = t;
						}
						else
						{
							printf("f-poly\n");
						}
					}
				}
			}
			else if (vpp==4)
			{
				if (poly[0]==edge->Vert1)
				{
					if (poly[3]==edge->Vert2)
					{
						Index t = edge->Vert1;
						edge->Vert1 = edge->Vert2;
						edge->Vert2 = t;
					}
				}
				else if (poly[1] == edge->Vert1)
				{
					if (poly[0] == edge->Vert2)
					{
						Index t = edge->Vert1;
						edge->Vert1 = edge->Vert2;
						edge->Vert2 = t;
					}
				}
				else if (poly[2] == edge->Vert1)
				{
					if (poly[1] == edge->Vert2)
					{
						Index t = edge->Vert1;
						edge->Vert1 = edge->Vert2;
						edge->Vert2 = t;
					}
				}
				else if (poly[3] == edge->Vert1)
				{
					if (poly[2] == edge->Vert2)
					{
						Index t = edge->Vert1;
						edge->Vert1 = edge->Vert2;
						edge->Vert2 = t;
					}
				}
				else
				{
					printf("what?\n");
				}
				/*
				if (poly[0]!=edge->Vert1)
				{
					if (poly[1]==edge->Vert1)
					{
						if (poly[0]==edge->Vert2)
						{
							Index t = edge->Vert1;
							edge->Vert1 = edge->Vert2;
							edge->Vert2 = t;
						}
					}
					else
					{
						if (poly[2]==edge->Vert1)
						{
							if (poly[3]!=edge->Vert2)
							{
								Index t = edge->Vert1;
								edge->Vert1 = edge->Vert2;
								edge->Vert2 = t;
							}
						}
						else
						{

							Assert( poly[3]==edge->Vert1 );

							Index t = edge->Vert1;
							edge->Vert1 = edge->Vert2;
							edge->Vert2 = t;
						}
					}
				}
				*/
			}
			else {TODO;}

			//add edge:
			nline = mOutline.AddMany(4);

			nline[0] = edge->Vert1;
			nline[1] = edge->Vert1 + mBoundMesh->NumVerts;
			nline[2] = edge->Vert2 + mBoundMesh->NumVerts;
			nline[3] = edge->Vert2;
		}
	}
}

void ResMeshEdger::UpdateOutline()
{
	Assert( mBoundMesh!=0 );
	Assert( mRefPoint!=0 );

	//Assumes the EdgeList is populated
	//Assumes the normals are up to date

	mOutline.Clear(false);
	Index* nline;

	MeshEdge* edge;
	fpnt work;
	VertType* verts = (VertType*)mBoundMesh->VertData.Raw();

	for (int i=0; i<mEdges.Size; i++)
	{
		edge = mEdges.Axs(i);

		work = mRefPoint->mPos - verts[ edge->Vert1 ].mVertex;
		float d1 = work.Dot( mPolyNormals[ edge->Poly1 ] );
		float d2 = work.Dot( mPolyNormals[ edge->Poly2 ] );
		if (d1*d2 <= 0)
		{
			nline = mOutline.AddMany(2);
			nline[0] = edge->Vert1;
			nline[1] = edge->Vert2;
		}
	}
}

#define LC_mul_CallAll(D,U) virtual D {BAMem<RenderContext*>*work=SubContext.Base;while(work){work->Val->U; work=work->Next;}};

class MultiRenderContext : public RenderContext
{
public:
	BArray<RenderContext*> SubContext;

	LC_mul_CallAll(void Clear(bool freemem), Clear(freemem));
	LC_mul_CallAll(void ResetForUpdate(), ResetForUpdate());
	LC_mul_CallAll(void Flush(), Flush());
	LC_mul_CallAll(void PopColor(), PopColor());
	LC_mul_CallAll(void PopTransform(), PopTransform());
	LC_mul_CallAll(void PopPosition(), PopPosition());
	LC_mul_CallAll(void PushColor(ResColor* color), PushColor(color));
	LC_mul_CallAll(void PushTransform(ResTransform* trans), PushTransform(trans));
	LC_mul_CallAll(void PushPosition(fpnt pos), PushPosition(pos));
	LC_mul_CallAll(void SkipThisMesh(uint mask, ResMesh* mesh), SkipThisMesh(mask,mesh));

	virtual void DrawMesh(uint mask, ResMesh* from, bool update);
	virtual ResCompNode* HitTest(ResCompNode* root, fpnt camera, fpnt lookdir, HitNodeFunc func=0, void* extra=0, ResCompTreePath* path=0);
	virtual void VisitRes(ResVisitFunc func, void* data);

	void AddRenderContext(RenderContext* target);
	void RemoveRenderContext(RenderContext* target);
	RenderContext* GetRenderContext(uint mask);

	MultiRenderContext() {ContextMask=0; Core.Register(this);};
        virtual ~MultiRenderContext() {};
};

ResGraphicsCore::ResGraphicsCore()
{
	mIsShiny = false;
	CameraUp = ResPoint::Create(0, 1, 0);
	CameraPos = ResPoint::Create(0, 0, 0); 
	CameraLookDir = ResPoint::Create(0, 0, -1.0);
	CameraPerspec = ResFloat::Create(45);
	LightPos = ResPoint::Create(20, 0, 0);
	BackgroundColor = ResColor::Create(104.0/255.0, 177.0/255.0, 84.0/255.0, 1.0);
	mRenderContext = new MultiRenderContext();
	Core.Register(this);
}

RenderContext* MultiRenderContext::GetRenderContext(uint mask)
{
	BAMem<RenderContext*>* work = SubContext.Base;
	while (work)
	{
		if (mask & work->Val->ContextMask)
			return work->Val;
		work = work->Next;
	}
	return null;
}

void ResTransformCollection::Add(ResTransform* res)
{
	*mTransforms.Add() = res;
	Changed();
}

void ResTransformCollection::Remove(ResTransform* res)
{
	int fi = mTransforms.FindEqual(res);
	Assert( fi != -1 );
	if (fi != -1)
	{
		mTransforms.Remove(fi);
		Changed();
	}
}

MatrixStruct lc_restranscol[2];
void ResTransformCollection::GetTransformMatrix(MatrixStruct* to)
{
	//todo: if size==0, just return the identity matrix
	Assert( mTransforms.Size > 0 );
	if (mTransforms.Size==1)
	{
		(*mTransforms.Axs(0))->GetTransformMatrix(to);
		return;
	}
	if (mTransforms.Size==2)
	{
		(*mTransforms.Axs(0))->GetTransformMatrix(&lc_restranscol[0]);
		(*mTransforms.Axs(1))->GetTransformMatrix(&lc_restranscol[1]);
		MatrixTimesMatrix(lc_restranscol[0], lc_restranscol[1], *to);
		return;
	}
	TODO;
}

int ResTransformCollection::Count()
{
	return mTransforms.Size;
}

ResTransform* ResTransformCollection::Get(int i)
{
	return *mTransforms.Axs(i);
}

void ResTransformCollection::VisitRes(ResVisitFunc func, void* data)
{
	for(int i=0; i<this->mTransforms.Size; i++)
	{
		(*func)(this, this->mTransforms[i], data);
	}
}

void MultiRenderContext::VisitRes(ResVisitFunc func, void* data)
{
	BAMem<RenderContext*>* work = SubContext.Base;
	while (work)
	{
		(*func)(this, work->Val, data);
		work = work->Next;
	}
}

void MultiRenderContext::AddRenderContext(RenderContext* target)
{
	*SubContext.AddMember() = target;
	ContextMask |= target->ContextMask;
}

void MultiRenderContext::RemoveRenderContext(RenderContext* target)
{
	BAMem<RenderContext*>*work = SubContext.Base;
	while (work)
	{
		if (work->Val == target)
		{
			SubContext.RemoveCell(work);
			return;
		}
		work = work->Next;
	}
}

ResCompNode* MultiRenderContext::HitTest(ResCompNode* root, fpnt camera, fpnt lookdir, HitNodeFunc func, void* extra, ResCompTreePath* path)
{
	if (mIgnoreHit)
		return 0;
	BAMem<RenderContext*>*work = SubContext.Base;
	ResCompNode* result = null;
	while (work)
	{
		ResCompNode* ans = work->Val->HitTest(root, camera, lookdir, func, extra, path);
		if (ans)
		{
			result = ans;
			if (!func)
				return result;
		}
		work = work->Next;
	}
	return result;
}

void ResGraphicsCore::LookAt(fpnt p)
{
	CameraLookDir->mPos = (p - CameraPos->mPos);
}

void MultiRenderContext::DrawMesh(uint mask, ResMesh* from, bool update)
{
	BAMem<RenderContext*>*work = SubContext.Base;
	while (work)
	{
		if (mask & work->Val->ContextMask)
		{
			work->Val->DrawMesh(mask, from, update);
		}
		work = work->Next;
	}
}

/*
ResMesh* ResMesh::CreateFromOFF(char* filename)
{
	ifstream fin(filename);
	ResMesh* mesh = new ResMesh();

	int numverts, numpoly, other;
	char buffer[10];

	fin >> buffer >> numverts >> numpoly >> other;

	mesh->VertData.ResizeTo( numverts * sizeof(VertType) );
	VertType* verts = (VertType*)mesh->VertData.Raw();
	mesh->NumVerts = numverts;

	for (int v=0; v<numverts; v++)
	{
		fin >> verts[v].mVertex.x;
		fin >> verts[v].mVertex.y;
		fin >> verts[v].mVertex.z;

		verts[v].mColor.Set( 0.5, 0.5, 0.5, 1.0);
	}

	int vpp;
	fin >> vpp;
	mesh->Format = MeshFormat::Create(vpp, true);
	mesh->NumIndices = vpp * numpoly;
	mesh->Indices.ResizeTo( mesh->NumIndices );

	for (int k=0; k<vpp; k++)
	{
		fin >> mesh->Indices[k];
	}

	for (int r=1; r<numpoly; r++)
	{
		fin >> vpp;
		for (int f=0; f<vpp; f++)
		{
			fin >> mesh->Indices[f + vpp*r];
		}
	}
	
	Core.Register(mesh);
	return mesh;
}

void ResMesh::ExportToOFF(char* filename)
{
	Assert( Format.IsIndexed() );

	ofstream fout(filename);
	if (!fout)
		return;

	fout << "OFF\n";

	fout << NumVerts << " " << NumPolys() << " " << NumPolys()*Format.VPP << "\n";

	int vpp = Format.VPP;

	for (int i=0; i<NumVerts; i++)
	{
		fpnt* p = (fpnt*)(VertData.Raw() + (i*SIZEOFVERT) + LOCOFFSET);
		fout << p->x << " " << p->y << " " << p->z << "\n";
	}

	for (int p=0; p<NumIndices; p+=vpp)
	{
		fout << vpp << " ";
		for (int j=0; j<vpp; j++)
		{
			fout << Indices[p+j] << " ";
		}
		fout << "\n";
	}

	fout.close();
}
*/

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

int ResMesh::PolyHitTest(fpnt camera, fpnt lookdir, HitPolyFunc func, void* extra)
{
	if (Format.VPP < 3)
		return -1;

	int vpp = Format.VPP;
	int vsize = SIZEOFVERT;
	int locoff = LOCOFFSET;
	byte* data = VertData.Raw();
	int result = -1;

	lc_glht_camera = camera;
	lc_glht_lookdir = lookdir;

	if (Format.IsIndexed())
	{
		Index* inds = Indices.Raw();
		for (int p=0; p<NumIndices; p+=vpp)
		{
			bool good = true;
			for (int j=0; (j<vpp)&&(good); j++)
			{
				lc_glht_v0 = (fpnt*)(data + locoff + (inds[(p + ((j+0)%vpp))]*vsize) );
				lc_glht_v1 = (fpnt*)(data + locoff + (inds[(p + ((j+1)%vpp))]*vsize) );
				lc_glht_v2 = (fpnt*)(data + locoff + (inds[(p + ((j+2)%vpp))]*vsize) );

				good &= lc_glht_SameSide();
			}

			if (good)
			{
				result = p/vpp;

				if (func)
				{
					//TODO: this is an estimation, it estimes the point of contact
					//as one of the vertices

					(*func)(extra, result, *lc_glht_v0);
				}
				else
					return result;
			}
		}
	}
	else
	{
		for (int p=0; p<NumVerts; p+=vpp)
		{
			bool good = true;
			for (int j=0; (j<vpp)&&(good); j++)
			{
				lc_glht_v0 = (fpnt*)(data + locoff + ((p + ((j+0)%vpp) )*vsize) );
				lc_glht_v1 = (fpnt*)(data + locoff + ((p + ((j+1)%vpp) )*vsize) );
				lc_glht_v2 = (fpnt*)(data + locoff + ((p + ((j+2)%vpp) )*vsize) );

				good &= lc_glht_SameSide();
			}

			if (good)
			{
				result = p/vpp;

				if (func)
				{
					//TODO: this is an estimation, it estimes the point of contact
					//as one of the vertices

					(*func)(extra, result, *lc_glht_v0);
				}
				else
					return result;
			}
		}
	}

	return result;
}

void ResMesh::Clear(bool freemem)
{
	NumIndices = 0;
	NumVerts = 0;
	Indices.Clear(freemem);
	VertData.Clear(freemem);
}

ResMesh* ResMesh::Create(MeshFormat format, int numverts, int numindices)
{
	ResMesh* mesh = new ResMesh();
	mesh->mEdger = 0;
	mesh->Format = format;
	mesh->ResizeTo(numverts, numindices);

	Core.Register(mesh);
	return mesh;
}

int ResMesh::NumPolys()
{
	int vpp = Format.VPP;

	if (Format.IsIndexed())
		return NumIndices / vpp;

	return NumVerts / vpp;
}

class SoftTransformRenderContext : public RenderContext
{
public:
	EArray<fpnt> TranslateStack;
	EArray<MatrixStruct> TransformStack;
	EArray<Color4f> ColorStack;

	fpnt TransformVertex(fpnt from);
	Color4f TransformColor(Color4f from);

	//if you inherit from this class, be sure to call this function as well
	// when overloading
	virtual void Clear(bool freemem);


	virtual void PushTransform(ResTransform* trans);
	virtual void PushPosition(fpnt pos);
	virtual void PushColor(ResColor* color);

	virtual void PopTransform();
	virtual void PopPosition() {TranslateStack.StackPop();};
	virtual void PopColor() {ColorStack.StackPop();};

private:
	MatrixStruct mmatrix1, mmatrix2;
};

void SoftTransformRenderContext::PopTransform()
{
	TransformStack.StackPop();
	TranslateStack.StackPop();
}

void SoftTransformRenderContext::PushTransform(ResTransform* trans)
{
	fpnt offset = *TranslateStack.StackPeak();
	TranslateStack.StackPush()->Set(0, 0, 0);

	MatrixStruct* n = TransformStack.StackPush();
	MatrixStruct* old = TransformStack.Axs( TransformStack.Size-2 );

	SetMatrix_Translate(mmatrix1, offset.x, offset.y, offset.z);
	MatrixTimesMatrix(*old, mmatrix1, mmatrix2);

	trans->GetTransformMatrix(&mmatrix1);
	MatrixTimesMatrix(mmatrix2, mmatrix1, *n);
}

void SoftTransformRenderContext::PushPosition(fpnt pos)
{
	fpnt* last = TranslateStack.StackPeak();
	pos += *last;
	*TranslateStack.StackPush() = pos;
}

void SoftTransformRenderContext::PushColor(ResColor* color)
{
	Color4f last = *ColorStack.StackPeak();
	last *= color->mValue;
	*ColorStack.StackPush() = last;
}

void SoftTransformRenderContext::Clear(bool freemem)
{
	TranslateStack.Clear(freemem);
	TransformStack.Clear(freemem);
	ColorStack.Clear(freemem);

	TranslateStack.StackPush()->Set(0, 0, 0);
	SetMatrix_Identity( *TransformStack.StackPush() );
	ColorStack.StackPush()->Set(1.0, 1.0, 1.0, 1.0);
}

fpnt SoftTransformRenderContext::TransformVertex(fpnt from)
{
	fpnt to;
	from += *(TranslateStack.StackPeak());
	Vec3TimesMatrix4(from, *this->TransformStack.StackPeak(), to);
	return to;
}

Color4f SoftTransformRenderContext::TransformColor(Color4f from)
{
	return (from * (*ColorStack.StackPeak()));
}

class MeshRenderContext : public SoftTransformRenderContext
{
public:
	ResMesh* Target;

	static MeshRenderContext* Create(ResMesh* target);

	virtual void DrawMesh(uint mask, ResMesh* from, bool update);
	virtual void SkipThisMesh(uint mask, ResMesh* node);
	virtual void Clear(bool freemem);
	virtual void Flush();
	virtual void ResetForUpdate();

	virtual ResCompNode* HitTest(ResCompNode* root, fpnt camera, fpnt lookdir, HitNodeFunc func=0, void* extra=0, ResCompTreePath* path=0);

	//offset should point to an int with 0 in it at first
	ResCompNode* FindCompNode(ResCompNode* root, int polyind, int* offset, ResCompTreePath* path);

	RESVISIT( RESMEM(Target) );

	//don't use these publically!
	ResCompNode* _CurHitRoot;
	HitNodeFunc _CurHitFunc;
	ResCompTreePath* _CurHitPath;

private:
	void DrawWireMesh(ResMesh* mesh, bool update);
	void AddVertex(int vertindto, VertType* vertdata);

	int RenderVertTo;
	int RenderIndTo;
};

void MeshRenderContext::ResetForUpdate()
{
	RenderIndTo = 0;
	RenderVertTo = 0;
}

void MeshRenderContext::SkipThisMesh(uint mask, ResMesh* mesh)
{
	if (!(mask & ContextMask))
		return;

	RenderVertTo += mesh->NumVerts;
	RenderIndTo += mesh->NumIndices;
}

ResCompNode* MeshRenderContext::FindCompNode(ResCompNode* root, int polyind, int* offset, ResCompTreePath* path)
{
	if (path)
		path->Push(root);
	if ((root->mMesh) && (root->mRenderContextMask & ContextMask))
	{
		*offset += root->mMesh->NumPolys();
		if (polyind < *offset)
			return root;
	}
	for (int i=0; i<root->mChildren.Size; i++)
	{
		ResCompNode* ans = FindCompNode(root->mChildren[i], polyind, offset, path);
		if (ans)
			return ans;
	}
	if (path)
		path->Pop();
	return null;
}

bool lc_mrc_HitPolyFunc(void* extra, int poly, fpnt at)
{
	MeshRenderContext* mrc = (MeshRenderContext*)extra;

	int offset = 0;
	if (mrc->_CurHitPath)
	{
		mrc->_CurHitPath->Clear();
		mrc->_CurHitPath->HitAt = at;
	}
	mrc->FindCompNode(mrc->_CurHitRoot, poly, &offset,mrc->_CurHitPath);

	return (*mrc->_CurHitFunc)(mrc->_CurHitPath);
}

ResCompNode* MeshRenderContext::HitTest(ResCompNode* root, fpnt camera, fpnt lookdir, HitNodeFunc func, void* extra, ResCompTreePath* path)
{
	if (mIgnoreHit)
		return 0;
	if (!func)
	{
		int poly = Target->PolyHitTest(camera, lookdir);
		if (poly == -1)
			return null;
		int offset=0;
		if (path)
			path->Clear();
		return FindCompNode(root, poly, &offset, path);
	}
	else
	{
		_CurHitRoot = root;
		_CurHitFunc = func;
		_CurHitPath = path;
		Target->PolyHitTest(camera, lookdir, lc_mrc_HitPolyFunc, this);
		return null;
	}
}

void ResMesh::UpdateNormals()
{
	UpdateNormals(0, NumVerts, 0, NumIndices);
}

void ResMesh::UpdateNormals(int vstart, int vend, int istart, int iend)
{
	if (mEdger)
		mEdger->UpdateNormals(vstart, vend, istart, iend);

	if (!HASNORMALS)
		return;

	int vpp = Format.VPP;
	if ( vpp < 3 )
		return;

	VertType* verts = (VertType*)VertData.Raw();
	Index* inds = Indices.Raw();
	fpnt left;

	if (Format.IsIndexed())
	{
		int i;
		for (i=vstart; i<vend; i++)
		{
			verts[i].mNormal.Set(0, 0, 0);
		}
		for (i=istart; i<iend; i+=vpp)
		{
			left = verts[inds[i+0]].mVertex - verts[inds[i+1]].mVertex;
			left = left.Cross( verts[inds[i+2]].mVertex - verts[inds[i+1]].mVertex );
			//left.Normalize();

			for (int j=0; j<vpp; j++)
			{
				verts[inds[i+j]].mNormal += left;
			}
		}
		for (i=vstart; i<vend; i++)
		{
			verts[i].mNormal.Normalize();
		}
	}
	else
	{
		for (int i=vstart; i<vend; i+=vpp)
		{
			left = verts[i+0].mVertex - verts[i+1].mVertex;
			left = left.Cross( verts[i+2].mVertex - verts[i+1].mVertex );
			for (int j=0; j<vpp; j++)
			{
				verts[i+j].mNormal = left;
			}
		}
	}
}

void MeshRenderContext::Clear(bool freemem)
{
	SoftTransformRenderContext::Clear(freemem);
	Target->Clear(freemem);
}

MeshRenderContext* MeshRenderContext::Create(ResMesh* target)
{
	MeshRenderContext* ans = new MeshRenderContext();
	ans->Target = target;
	Core.Register(ans);
	ans->_Flags |= LC_ResFlag_MeshRenderContext;
	return ans;
}

byte* ResMesh::RecentVertData(int vertcount)
{
	return &VertData.Raw()[VertData.Size - (sizeof(VertType) * vertcount)];
}

Index* ResMesh::RecentIndices(int count)
{
	return &Indices.Raw()[NumIndices-count];
}

void ResMesh::Add(int numverts, int numindices)
{
	ResizeTo(NumVerts+numverts, NumIndices+numindices);
}

void ResMesh::ResizeTo(int numverts, int numindices)
{
	VertData.ResizeTo(numverts * SIZEOFVERT );
	Indices.ResizeTo(numindices);
	NumVerts = numverts;
	NumIndices = numindices;
}



void MeshRenderContext::AddVertex(int vertindto, VertType* vertdata)
{
	//OPTIMIZE THIS FUNCTION:
	//	it is most general now, but very slow, fix it up later

	VertType* vto = (VertType*)Target->VertData.Axs( vertindto * SIZEOFVERT );
	vto->mVertex = TransformVertex( vertdata->mVertex );
	vto->mColor = TransformColor( vertdata->mColor );


	/*
	byte* vertto = Target->VertData.Axs( vertindto * SIZEOFVERT );

//	*(fpnt*)(vertto + Target->Format.LOCOFFSET) =
//		TransformVertex( *(fpnt*)(vertdata + format.LOCOFFSET) );
	fpnt* to = (fpnt*)(vertto + Target->Format.LOCOFFSET);
	fpnt* from = (fpnt*)(vertdata + format.LOCOFFSET);
	*to = TransformVertex(*from);

	if (Target->Format.HasColor())
	{
		if (format.HasColor())
		{
			*(Color4f*)(vertto + Target->Format.ColorOffset()) =
				TransformColor( *(Color4f*)(vertdata + format.ColorOffset()) );
		}
		else
		{
			*(Color4f*)(vertto + Target->Format.ColorOffset()) =
				TransformColor( DefaultColor );
		}
	}
	*/
}

void MeshRenderContext::DrawMesh(uint mask, ResMesh* from, bool update)
{
	Assert( mask & ContextMask );

	if (Target->Format.VPP != from->Format.VPP)
	{
		if (Target->Format.Flags & LC_MF_Line)
		{
			TODO;
			return;
		}

		TODO;
		return;
	}

	if (Target->Format.IsIndexed() == from->Format.IsIndexed())
	{
		int startvert = Target->NumVerts;
		int startind = Target->NumIndices;

		if (!update)
			Target->Add(from->NumVerts, from->NumIndices);
		else
		{
			startvert = RenderVertTo;
			startind = RenderIndTo;
		}

		VertType* vertdata = (VertType*)from->VertData.Raw();
		Index* indto = Target->Indices.Axs( startind );
		Index* indfrom = from->Indices.Raw();

		int i;
		for (i=0; i<from->NumVerts; i++)
		{
			AddVertex(i+startvert, &vertdata[i] );
		}
		for (i=0; i<from->NumIndices; i++)
		{
			indto[i] = indfrom[i] + startvert;
		}

		Target->UpdateNormals(startvert, startvert + from->NumVerts, startind, startind + from->NumIndices );

		return;
	}

	TODO;
}

//
// Graphics Core Implimentation
//



void RenderContext::DrawCompTree(ResCompNode* tree, bool update)
{ 
#define LC_rc_updating ((!update) || (IsFlag(tree->_Flags, LC_ResFlag_ShallowChange | LC_ResFlag_ChildChanged | LC_ResFlag_Changed ) ))

	if (LC_rc_updating)
	{
		if (tree->mPosition)
			PushPosition( tree->mPosition->mPos );
		if (tree->mTransform)
			PushTransform( tree->mTransform );
		if (tree->mColor)
			PushColor( tree->mColor );
	}

	if ((tree->mMesh) && (tree->mRenderContextMask & ContextMask))
	{
		if (!update)
			DrawMesh( tree->mRenderContextMask, tree->mMesh, update );
		else
		{
			if (tree->_Flags & LC_ResFlag_ShallowChange)
				DrawMesh( tree->mRenderContextMask, tree->mMesh, update);

			//TODO: this won't work if the mesh and target formats
			// aren't vertex/ind/poly consistent
			SkipThisMesh( tree->mRenderContextMask, tree->mMesh );
		}
	}
	for (int i=0; i<tree->mChildren.Size; i++)
	{
		DrawCompTree( tree->mChildren[i], update );
	}

	if (LC_rc_updating)
	{
		if (tree->mColor)
			PopColor();
		if (tree->mTransform)
			PopTransform();
		if (tree->mPosition)
			PopPosition();
	}
}

RenderContext* ResGraphicsCore::GetRenderStage(uint mask)
{
	MultiRenderContext* mul = (MultiRenderContext*)this->mRenderContext;
	return mul->GetRenderContext(mask);
}

uint ResGraphicsCore::AddRenderStage(MeshFormat format)
{
	MultiRenderContext* mul = (MultiRenderContext*)this->mRenderContext;

	ResMesh* mesh = ResMesh::Create(format, 0, 0);
	MeshRenderContext* rc = MeshRenderContext::Create( mesh );
	rc->ContextMask = (1 << mul->SubContext.Length());
	mul->AddRenderContext( rc );

	return rc->ContextMask;
}

void ResGraphicsCore::InnerRender(LewcidCore* core)
{
	mCore = core;
	if ((RebuildTree) || (LC_SETTING_NOUPDATE))
	{
		mRenderContext->Clear(false);
		mRenderContext->DrawCompTree( mCore->mRootNode, false );
	}
	else
	{
		mRenderContext->ResetForUpdate();
		mRenderContext->DrawCompTree( mCore->mRootNode, true );
	}
}

float lc_glv_LightAmbient[]= { 0.15f, 0.15f, 0.15f, 1.0f }; 				// Ambient Light Values ( NEW )
float lc_glv_LightDiffuse[]= { 0.7f, 0.7f, 0.7f, 1.0f };				 // Diffuse Light Values ( NEW )
float lc_glv_LightSpecular[]= { 1.0, 1.0, 1.0, 1.0f };
float lc_glv_LightPosition[]= { 0.0f, 0.0f, 20.0f, 1.0f };				 // Light Position ( NEW )

void ResGraphicsCore::GLInitFrame()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(BackgroundColor->mValue.Red, BackgroundColor->mValue.Green,
		BackgroundColor->mValue.Blue, BackgroundColor->mValue.Blue);				// Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	lc_glv_LightPosition[0] = LightPos->mPos.x;
	lc_glv_LightPosition[1] = LightPos->mPos.y;
	lc_glv_LightPosition[2] = LightPos->mPos.z;

	if (mIsShiny)
	{
		GLfloat mat_specular[] = { 0.15, 0.15, 0.15, 1.0 };
		GLfloat mat_shininess[] = { 50.0 };
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	}

	glLightfv(GL_LIGHT1, GL_AMBIENT, lc_glv_LightAmbient);		// Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lc_glv_LightDiffuse);		// Setup The Diffuse Light
	glLightfv(GL_LIGHT1, GL_SPECULAR, lc_glv_LightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, lc_glv_LightPosition);	// Position The Light
	glEnable(GL_LIGHT1);							// Enable Light One
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL );

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	FIRE(LC_Event_GLSettingsInit, 0);

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

}

void ResGraphicsCore::GLInitView()
{
    float ratio;
 
   if ( ScrHeight == 0 )
	ScrHeight = 1;

    ratio = ( float )ScrWidth / ( float )ScrHeight;

    glViewport( 0, 0, ScrWidth, ScrHeight );


	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

    gluPerspective( CameraPerspec->mValue, ratio, 0.1f, 100.0f );

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	fpnt lookat = CameraPos->mPos + CameraLookDir->mPos;
	gluLookAt(CameraPos->mPos.x, CameraPos->mPos.y, CameraPos->mPos.z,
			  lookat.x, lookat.y, lookat.z,
			  CameraUp->mPos.x, CameraUp->mPos.y, CameraUp->mPos.z);
}

void ResGraphicsCore::Flush()
{
	GLInitFrame();

	GLInitView();

	mRenderContext->Flush();

	FIRE(LC_Event_ContextFlushDone, 0);
}

ResCompNode* ResGraphicsCore::HitCompNode(int screenx, int screeny, bool best)
{
	fpnt lookdir = LookDir(screenx, screeny);
	if (!best)
		return HitCompNode( CameraPos->mPos, lookdir );
	else
		return BestHitCompNode( CameraPos->mPos, lookdir);
}

struct lc_ht_bestsearch
{
	ResCompNode* Best;
	float Dist;

	fpnt Camera;
};

bool lc_ht_findBestNode(ResCompTreePath* path)
{
	lc_ht_bestsearch* work = (lc_ht_bestsearch*)path->Extra;

	float d = path->HitAt.Dist( work->Camera );
	if (d < work->Dist)
	{
		work->Best = path->Path[path->Path.Size-1];
		work->Dist = d;
	}

	return true;
}

ResCompNode* ResGraphicsCore::BestHitCompNode(fpnt camera, fpnt lookdir)
{
	ResCompTreePath* path = ResCompTreePath::Create();
	lc_ht_bestsearch find;
	find.Best = 0;
	find.Dist = 10000;
	find.Camera = camera;
	path->Extra = &find;
	Core.Graphics->mRenderContext->HitTest( Core.mRootNode, camera, lookdir, lc_ht_findBestNode, 0, path);

	return find.Best;

}

ResCompNode* ResGraphicsCore::HitCompNode(fpnt camera, fpnt lookdir)
{
	return this->mRenderContext->HitTest( mCore->mRootNode, camera, lookdir );
}

//
// OpenGL dependant stuff:
//

fpnt ResGraphicsCore::Project(fpnt world)
{
	GLint viewport[4]; 
	GLdouble modelview[16],projection[16]; 
	GLdouble ox, oy, oz;

	glGetIntegerv(GL_VIEWPORT,viewport); 
	glGetDoublev(GL_MODELVIEW_MATRIX,modelview); 
	glGetDoublev(GL_PROJECTION_MATRIX,projection); 

	gluProject(world.x, world.y, world.z, 
		modelview, projection, viewport,
		&ox, &oy, &oz );

	return FPNT(ox, viewport[3]-oy, oz);
}

fpnt ResGraphicsCore::LookDir(int screenx, int screeny)
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
 
	wz = 0.98f;
	gluUnProject(wx,wy,wz,modelview,projection,viewport,&ox,&oy,&oz);

	fpnt lookdir = FPNT(ox, oy, oz);

	return lookdir - CameraPos->mPos;
}

void ResStencilEdger::FlushShadow()
{
	glInterleavedArrays( GL_V3F, 0, this->mVerts.Raw() );
	int mid;
	glGetIntegerv(GL_STENCIL_BITS, &mid);
	Assert( mid != 0 );
	mid = (1<<mid)/2;

	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glClearStencil(mid);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc (GL_ALWAYS, 0, 0);

	/*
	glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElements( GL_QUADS, mOutline.Size, GLINDEXTYPE, mOutline.Raw() );

	glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
	glFrontFace(GL_CW);
	glDrawElements( GL_QUADS, mOutline.Size, GLINDEXTYPE, mOutline.Raw() );

	glInterleavedArrays( GL_V3F, 0, this->mVerts.Axs( mBoundMesh->NumVerts) );
	glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	glFrontFace(GL_CW);
	glDrawElements( GL_TRIANGLES, mBoundMesh->NumIndices, GLINDEXTYPE, mBoundMesh->Indices.Raw() );

	glInterleavedArrays( GL_V3F, 0, this->mVerts.Axs( 0 ) );
	glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElements( GL_TRIANGLES, mBoundMesh->NumIndices, GLINDEXTYPE, mBoundMesh->Indices.Raw() );
	*/

	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	glFrontFace(GL_CW);
	glDrawElements( GL_QUADS, mOutline.Size, GLINDEXTYPE, mOutline.Raw() );

	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glFrontFace(GL_CCW);
	glDrawElements( GL_QUADS, mOutline.Size, GLINDEXTYPE, mOutline.Raw() );

	if (mIsInvisable)
	{
		GLenum polyt = mBoundMesh->Format.GLPolyType();

		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		glFrontFace(GL_CW);
		glDrawElements( polyt, mInvMesh.Size, GLINDEXTYPE, mInvMesh.Raw() );

		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glFrontFace(GL_CCW);
		glDrawElements( polyt, mInvMesh.Size, GLINDEXTYPE, mInvMesh.Raw() );
	}

	glFrontFace(GL_CCW);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glStencilFunc(GL_LESS, mid, ~0);
	glColor3f(0, 1, 0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	//draw a full screen gray shade
	glFrontFace(GL_CCW);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glLoadIdentity();
	gluLookAt(0, 0, -1, 
		0, 0, 0, 
		0, 1, 0);

	glBegin(GL_QUADS);
		glColor4f(0.0, 0.0, 0.0, 0.5);
		glVertex3f(-1, -1, 0);
		glVertex3f(-1, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, -1, 0);
	glEnd();

	glPopMatrix();
	glEnable(GL_LIGHTING);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void ResMeshEdger::FlushOutline()
{
	GLenum glformat = GLVERTFORMAT;
	glInterleavedArrays( glformat, 0, mBoundMesh->VertData.Raw() );

	GLenum mode = GL_LINES;
	glLineWidth( mLineWidth );
	glDisable(GL_LIGHTING);
	glDrawElements( mode, mOutline.Size, GLINDEXTYPE, mOutline.Raw() );
	glEnable(GL_LIGHTING);
	glLineWidth( 1.0 );
}

void MeshRenderContext::Flush()
{
	FIRE(mEvent_PreFlush, this);

	IFNOTFIRE( mEvent_ReplaceFlush, this)
	{
		GLenum glformat = GLVERTFORMAT;
		glInterleavedArrays( glformat, 0, Target->VertData.Raw() );

		GLenum mode = 666;
		if (Target->Format.VPP == 3)
			mode = GL_TRIANGLES;
		if (Target->Format.VPP == 4)
			mode = GL_QUADS;
		if (Target->Format.VPP == 2)
			mode = GL_LINES;
		Assert( mode != 666 );

		if (!HASCOLOR)
		{
			glColor4f( DefaultColor.Red, DefaultColor.Green, DefaultColor.Blue, DefaultColor.Alpha );
		}

		if ( Target->Format.VPP < 3)
			glDisable(GL_LIGHTING);
		else
			glEnable(GL_LIGHTING);

		if (Target->Format.IsIndexed())
		{
			glDrawElements( mode, Target->NumIndices, GLINDEXTYPE, Target->Indices.Raw() );
		}
		else
		{
			glDrawArrays( mode, 0, Target->NumVerts );
		}
	}

	FIRE( this->mEvent_PostFlush, this);
}