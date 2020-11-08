
#ifndef HasMatrixClass
#define HasMatrixClass

#include <math.h>

class Matrix;
class Vector;
class Segment;
class Plane;

#define loop(a) for(a=0;a!=4;a++)
#define bigl(a) for(a=0;a!=16;a++)
#define mp(m,a,b) ((m)[(a*4)+b])

#ifndef Pi
#define Pi 3.1415926535897932384626433832795f
#endif

#define HalfPi 1.57079632679489661923132169163975

#define DegToRad(d) ((d/180.0f)*Pi)
#define RadToDeg(r) ((r/Pi)*180.0f)

#pragma warning (disable : 4244)
#pragma warning (disable : 4305)

/*
float DegToRad(float deg)
{
	return ((deg/180.0f)*Pi);
}
*/

#ifndef HasBasicMatrix

float * NewVec() {return (new float [4]);}
void DelVec(float * vec) {delete [] vec;}

void CrossProd(float * vec1, float * vec2, float * ans)
{
	//gets the cross product of vec1 and vec2
	ans[0]=((vec1[1] * vec2[2]) - (vec1[2] * vec2[1]));
	ans[1]=((vec1[2] * vec2[0]) - (vec1[0] * vec2[2]));
	ans[2]=((vec1[0] * vec2[1]) - (vec1[1] * vec2[0]));
	ans[3]=1;
	for (int i=0; i!=3; i++)
		ans[i] *= -1;
}

float DotProd3(float* a, float* b)
{
	return ((a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]));
}

void SetVec(float * vec, float x, float y, float z, float w)
{
	vec[0]=x;
	vec[1]=y;
	vec[2]=z;
	vec[3]=w;
}

#endif

void Vec3TimesMatrix4(float * vec, float * mat, float * to)
{
	int x, y;
	for (x=0; x!=3; x++)
	{
		//CHANGED THIS 4/15/2003 from "to[x]=0"
		to[x]= mp(mat, x, 3);
		for (y=0; y!=3; y++)
			to[x] += (mp(mat,x,y) * vec[y]);
	}
}

void VecTimesMatrix(float * vec, float * mat, float * to)
{
	//vector times matrix
	int x, y;
	for (x=0; x!=4; x++)
	{
		to[x]=0;
		for (y=0; y!=4; y++)
			to[x] += (mp(mat,x,y) * vec[y]);
	}
}

void MatrixTimesMatrix(float * one, float * two, float * to)
{
	//matrix times matrix
	for (int y=0; y!=4; y++)
	{
		VecTimesMatrix(one+(4*y),two,to+(4*y));
	}
}

void SetMatrix_Identity(float * mat)
{
	int x;
	bigl(x)
		mat[x]=0;
	loop(x)
		mp(mat,x,x)=1;
}

float AngleBetween(float* one, float * two)
{
	//this assumes that one and two are normalized!

	return RadToDeg(acos(DotProd3(one, two)));
}

void SetMatrix_RotAbout(float* mat, float* about, float ang)
{
	SetMatrix_Identity(mat);
	float c = cos(ang);
	float s = sin(ang);
	float t = 1.0f-c;

	mat[0] =   about[0]*about[0] + c;
	mat[4] = t*about[0]*about[1] + s*about[2];
	mat[8] = t*about[0]*about[2] - s*about[1];

	mat[1] = t*about[0]*about[1] - s*about[2];
	mat[5] = t*about[1]*about[1] + c;
	mat[9] = t*about[1]*about[2] + s*about[0];

	mat[2] = t*about[0]*about[1] + s*about[1];
	mat[6] = t*about[1]*about[2] - s*about[0];
	mat[10]= t*about[2]*about[2] + c;

	/*
	mat[0] = t*about[0]*about[0] + c;
	mat[1] = t*about[0]*about[1] - s*about[2];
	mat[2] = t*about[0]*about[2] + s*about[1];

	mat[4] = t*about[0]*about[1] + s*about[2];
	mat[5] = t*about[1]*about[1] + c;
	mat[6] = t*about[1]*about[2] - s*about[0];

	mat[8] = t*about[0]*about[2] - s*about[1];
	mat[9] = t*about[1]*about[2] + s*about[0];
	mat[10]= t*about[2]*about[2] + c;
	*/
}

void SetMatrix_Scale(float* mat, float by)
{
	SetMatrix_Identity(mat);
	mp(mat,0,0) = by;
	mp(mat,1,1) = by;
	mp(mat,2,2) = by;
}

void SetMatrix_RotateX(float * mat, float ang)
{
	ang=DegToRad(ang);
	SetMatrix_Identity(mat);
	mp(mat,1,1)=cos(ang);
	mp(mat,2,1)=sin(ang);
	mp(mat,1,2)=-sin(ang);
	mp(mat,2,2)=cos(ang);
}

void SetMatrix_RotateY(float * mat, float ang)
{
	ang=DegToRad(ang);
	SetMatrix_Identity(mat);
	mp(mat,0,0)=cos(ang);
	mp(mat,0,2)=sin(ang);
	mp(mat,2,0)=-sin(ang);
	mp(mat,2,2)=cos(ang);
}

void SetMatrix_RotateZ(float * mat, float ang)
{
	ang=DegToRad(ang);
	SetMatrix_Identity(mat);
	mp(mat,0,0)=cos(ang);
	mp(mat,1,0)=sin(ang);
	mp(mat,0,1)=-sin(ang);
	mp(mat,1,1)=cos(ang);
}

bool IsEqualVec(float * one, float * two)
{
	for (int i=0; i!=4; i++)
	{
		if (one[i] != two[i])
			return 0;
	}
	return 1;
}

float GetMag(float * vec)
{
	float d=0;
	for (int i=0; i!=3; i++)
		d += (vec[i] * vec[i]);
	d=sqrt(d);
	return d;
}

void NormalizeVec(float * vec)
{
	float d=0;
        int i;
	for (i=0; i!=3; i++)
		d += (vec[i] * vec[i]);
	d=sqrt(d);
	if ((d < 0.001) && (d > -0.001))
		return;
	for (i=0; i!=3; i++)
		vec[i] /= d;
}

void InverseMat(float * from, float * to)
{
	int x, y;
	for (y=0; y!=3; y++)
	{
		mp(to,y,3)=0;
		for (x=0; x!=3; x++)
		{
			mp(to,x,y) = mp(from,y,x);
			mp(to,y,3)-=((mp(from,y,3)) * (mp(from,x,y)));
		}
		mp(to,3,y)=0;
	}
	mp(to,3,3)=1;
}

void SetMatrix_TranslateSub(float * mat, float x, float y, float z)
{
	SetMatrix_Identity(mat);
//	mp(mat,3,0)=x;
//	mp(mat,3,1)=y;
//	mp(mat,3,2)=z;
	mp(mat,0,3)=x;
	mp(mat,1,3)=y;
	mp(mat,2,3)=z;
}

void SetMatrix_Translate(float * mat, float x, float y, float z)
{
	SetMatrix_Identity(mat);
	mp(mat,3,0)=x;
	mp(mat,3,1)=y;
	mp(mat,3,2)=z;
//	mp(mat,0,3)=x;
//	mp(mat,1,3)=y;
//	mp(mat,2,3)=z;
}

float GetAng(float x, float y)
{
	//gets the angle given by the 2D point x,y
	bool negx=0, negy=0;
	if (x < 0)
	{
		negx=1;
		x *= -1;
	}
	if (y < 0)
	{
		negy=1;
		y *= -1;
	}
	if (x==0)
	{
		if (negy)
			return -HalfPi;
		return HalfPi;
	}
	if (y==0)
	{
		if (negx)
			return Pi;
		return 0;
	}
	float ang=atan(y/x);
	if ((negx) && (negy))
		return (-(Pi - ang));
	if (negx)
	{
		return HalfPi + (HalfPi - ang);
	}
	if (negy)
		return -ang;
	return ang;
}

float * MCSwap, * MCOther;
float * VCSwap, * VCOther;
int MCNumClass=0, VCNumClass=0;

void AddMatrixClass()
{
	MCNumClass++;
	if (MCNumClass==1)
	{
		MCSwap=new float [16];
		MCOther=new float [16];
	}
}

void AddVectorClass()
{
	VCNumClass++;
	if (VCNumClass==1)
	{
		VCSwap=new float [4];
		VCOther=new float [4];
	}
}

void DelVectorClass()
{
	VCNumClass--;
	if (VCNumClass==0)
	{
		delete [] VCSwap;
		delete [] VCOther;
	}
}

class Matrix
{
public:
	//A flat 4x4 float matrix organized by coloums
	float * Mat;

	void Identity() {SetMatrix_Identity(Mat);};
//	void Show();
	void RotateX(float ang);
	void RotateY(float ang);
	void RotateZ(float ang);
	void Scale(float ang);
	void InverseOf(float * mat);
	void Inverse();
	void Flip();
	void Normal(float*to);
	bool PointInPoly(float * pnt);
	void Translate(float x, float y, float z);
	void operator*=(float * mat);
	void operator=(float * mat);
	void operator=(Matrix & mat) {(*this)=mat.Mat;};
	operator float*() {return Mat;};
	float * operator [] (int i) {return (Mat+(4*i));};

	void Swap();

	Matrix();
	~Matrix();
};

class Vector
{
public:
	float * Vec;

	void operator*=(Matrix & m);
	void operator*=(float con);
	void operator/=(float con);
	void Normalize();
	float GetMag();
	float DistBetween(float * pnt);
	void GetRot(float * rot);
	void Cross(float * vec);
	void IsCross(float * vec, float * vec2);
	void Set(float x, float y, float z, float w);
	float Dot(float * vec);
	void operator+=(float * vec);
	void operator-=(float * vec);
	void operator=(float * vec);
	void operator=(Vector & vec) {(*this) = vec.Vec;};
	bool operator==(Vector & other);
	operator float*() {return Vec;};
	float & operator [](int i) {return Vec[i];};
//	void Show();
//	void Enter();

	void TimesOtherMat(float * mat);

	void Swap() {float*t=Vec;Vec=VCSwap,VCSwap=t;};

	Vector();
	~Vector();
};

bool Vector::operator ==(Vector & other)
{
	for (int i=0; i!=3; i++)
	{
		if (Vec[i] != other.Vec[i])
			return 0;
	}
	return 1;
}

void Vector::TimesOtherMat(float * mat)
{
	int x, y;
	for (x=0; x!=4; x++)
	{
		VCSwap[x]=0;
		for (y=0; y!=4; y++)
			VCSwap[x] += (mp(mat,y,x) * Vec[y]);
	}
	Swap();
}

void Matrix::Flip()
{
	int x, y;
	for (y=0; y!=4; y++)
	{
		for (x=0; x!=4; x++)
		{
			mp(MCSwap,x,y) = mp(Mat,y,x);
		}
	}
	Swap();
}

void Matrix::Swap()
{
	float * t=Mat;
	Mat=MCSwap;
	MCSwap=t;
}

void Matrix::operator =(float * mat)
{
	for (int i=0; i!=16; i++)
		Mat[i]=mat[i];
}

void Matrix::Inverse()
{
	Swap();
	InverseOf(MCSwap);
}

void Matrix::InverseOf(float * mat)
{
	InverseMat(mat, Mat);
}

float Vector::Dot(float * vec)
{
	float ans=0;
	for (int i=0; i!=3; i++)
		ans+=(Vec[i] * vec[i]);
	return ans;
}

void Vector::GetRot(float * rot)
{
	//puts the X, Y and Z rotations of the vector into rot
	rot[0]=GetAng(Vec[2],Vec[1]);
	rot[1]=GetAng(Vec[0],Vec[2]);
	rot[2]=GetAng(Vec[0],Vec[1]);
}

void Vector::Set(float x, float y, float z, float w)
{
	Vec[0]=x;
	Vec[1]=y;
	Vec[2]=z;
	Vec[3]=w;
}

void Vector::operator *=(float con)
{
	for (int i=0; i!=3; i++)
		Vec[i] *= con;
}

void Vector::operator /=(float con)
{
	for (int i=0; i!=3; i++)
		Vec[i] *= con;
}

void Matrix::Normal(float * to)
{
	//gets the normal of the matrix (if it represents a 3D triangle)
	Vector one, two;
	one=(Mat+4);
	two=(Mat+8);
	one-=Mat;
	two-=Mat;
	CrossProd(one.Vec,two.Vec,to);
}

void Vector::IsCross(float *vec, float * vec2)
{
	//this = CrossProduct(vec, vec2)
	CrossProd(vec,vec2,Vec);
}

void Vector::Cross(float * vec)
{
	//this = CrossProduct(this, vec);
	CrossProd(Vec,vec,VCSwap);
	Swap();
}

void Vector::operator =(float * vec)
{
	for (int i=0; i!=4; i++)
		Vec[i] = vec[i];
}

void Vector::operator +=(float * vec)
{
	for (int i=0; i!=3; i++)
		Vec[i]+=vec[i];
}

void Vector::operator -=(float * vec)
{
	for (int i=0; i!=3; i++)
		Vec[i]-=vec[i];
}

void Vector::Normalize()
{
	//makes this a unit vector
	float d=0;
        int i;
	for (i=0; i!=3; i++)
		d += (Vec[i] * Vec[i]);
	d=sqrt(d);
	if (d < 0.001)
		return;
	for (i=0; i!=3; i++)
		Vec[i] /= d;
}

float Vector::DistBetween(float * other)
{
	//distance between this point and other
	float d=0, ot;
	for (int i=0; i!=3; i++)
	{
		ot=(Vec[i] - other[i]);
		d += (ot * ot);
	}
	d=sqrt(d);
	return d;
}

float Vector::GetMag()
{
	//length of vector
	float d=0;
	for (int i=0; i!=3; i++)
		d += (Vec[i] * Vec[i]);
	d=sqrt(d);
	return d;
}

void Vector::operator *=(Matrix & m)
{
	//runs vector through matrix
	VecTimesMatrix(Vec,m.Mat,VCSwap);
	float *t=Vec;
	Vec=VCSwap;
	VCSwap=t;
}

Vector::Vector()
{
	Vec=new float [4];
	AddVectorClass();
}

Vector::~Vector()
{
	delete [] Vec;
	DelVectorClass();
}

void Matrix::operator *=(float * mat)
{
	MatrixTimesMatrix(Mat,mat,MCSwap);
	float * t=Mat;
	Mat=MCSwap;
	MCSwap=t;
}

void Matrix::Translate(float x, float y, float z)
{
	SetMatrix_Translate(MCOther,x,y,z);
	(*this) *= MCOther;
}

void Matrix::RotateX(float ang)
{
	SetMatrix_RotateX(MCOther,ang);
	(*this) *= MCOther;
}

void Matrix::RotateY(float ang)
{
	SetMatrix_RotateY(MCOther,ang);
	(*this) *= MCOther;
}

void Matrix::RotateZ(float ang)
{
	SetMatrix_RotateZ(MCOther,ang);
	(*this) *= MCOther;
}

Matrix::Matrix()
{
	Mat=new float [16];
	AddMatrixClass();
}

Matrix::~Matrix()
{
	delete [] Mat;
//	DelMatrixClass();
}

class Segment
{
public:
	float * Orig; //point of origin
	float * Delta;//vector change (if is unit vector, time == distance)
	bool DelData;

	float TimeTo(Segment * other);
	void GetPoint(float time, float * pnt);
	float GetTime(float * pnt);
	void LockOn(float * orig, float * delta);
	float TimeTo(Plane * pl);
	bool HitsPoly(Matrix * mat, float * normal, float * howfar);
	void FromTo(float * from, float * to);

	Segment() {Orig=new float [4]; Delta=new float [4]; DelData=1;};
	Segment(float*orig,float*delta) {Orig=orig;Delta=delta;DelData=0;};
	~Segment() {if (DelData) {delete [] Orig; delete [] Delta;}};
};

void Segment::FromTo(float * from, float * to)
{
	int i;
	for (i=0; i!=4; i++)
	{
		Orig[i]=from[i];
		Delta[i]=(to[i]-from[i]);
	}
}

void Segment::LockOn(float * orig, float * delta)
{
	if (DelData)
	{
		delete [] Orig;
		delete [] Delta;
	}
	DelData=0;
	Orig=orig;
	Delta=delta;
}

float Segment::GetTime(float * pnt)
{
	//relative time to point (pnt must be on line)
	for (int i=0; i!=3; i++)
	{
		if (Delta[i] != 0)
			return ((pnt[i] - Orig[i])/Delta[i]);
	}
	return 0;
}

void Segment::GetPoint(float time, float * pnt)
{
	//gets a point using a time
	for (int i=0; i!=3; i++)
		pnt[i]=(Orig[i] + (time * Delta[i]));
}

float Segment::TimeTo(Segment * other)
{
	//time until segments meet (segments must meet)
	float top, bot;
	int one=1;
	if (other->Delta[1]==0)
		one=2;
	top=(Orig[0] - other->Orig[0] - ((other->Delta[0]*(Orig[one]-other->Orig[one]))/other->Delta[one]));
	bot=((other->Delta[0]*Delta[one])/other->Delta[one])-Delta[0];
	if (bot==0)
		return -777;
	return (top / bot);
}

class Plane
{
public:
	Vector Def; //values which define the plane

	void FromPoly(Matrix * mat);
	void FromNormal(float * norm, float * pnt);
	float DistFromPoint(float * pnt);
	void ClosestPoint(float * pnt, float * ans);
};

float Segment::TimeTo(Plane * pl)
{
	float top=0, bot=0;
	int i, o=0;

	for (i=0; i!=3; i++)
	{
		top -= (pl->Def[i] * Orig[i]);
		bot += (pl->Def[i] * Delta[i]);
		o++;
	}
	top -= pl->Def[3];

	if (bot==0)
		return 0;

	return (top/bot);
}

void Plane::FromPoly(Matrix * mat)
{
	Vector vec;
	mat->Normal(vec.Vec);
	FromNormal(vec,mat[0]);
}

void Plane::ClosestPoint(float * pnt, float * ans)
{
	//gets point on plane closest to pnt
	Vector norm;
	norm=Def;
	norm[3]=0;
	norm.Normalize();
	int i;
	float top=0, bot=0;

	for (i=0; i!=3; i++)
		top -= (Def[i] * pnt[i]);
	top -= Def[3];

	for (i=0; i!=3; i++)
		bot -= (Def[i] * norm[i]);
	if (bot==0)
		return;

	top/=bot;

	for (i=0; i!=3; i++)
		ans[i]=(pnt[i] - (norm[i]*top));
	ans[3]=0;
}

float Plane::DistFromPoint(float * pnt)
{
	//gets shotest dist from plane to pnt
	Vector norm;
	norm=Def;
	norm[3]=0;
	norm.Normalize();
	int i;
	float top=0, bot=0;

	for (i=0; i!=3; i++)
		top -= (Def[i] * pnt[i]);
	top -= Def[3];

	for (i=0; i!=3; i++)
		bot -= (Def[i] * norm[i]);
	if (bot==0)
		return 0;

	return (top/bot);
}

void Plane::FromNormal(float * norm, float * pnt)
{
	//sets the values of a plane using a normal vector
	//norm is the normal vector, pnt is a point on the plane
	float d=0;
	for (int i=0; i!=3; i++)
	{
		Def.Vec[i] = norm[i];
		d -= (pnt[i] * norm[i]);
	}
	Def.Vec[3]=d;
}

bool Segment::HitsPoly(Matrix * mat, float * normal, float * howfar)
{
	//sees if segment runs through poly
	//howfar is a float which is how far you can go if blocked (in time)
	Plane pl;
	pl.FromNormal(normal,mat[0]);
	float time=TimeTo(&pl);
	if ((time > 1) || (time <= 0))
	{
		if (howfar)
			*howfar=time;
		return 0;
	}
	GetPoint(time,pl.Def);
	return mat->PointInPoly(pl.Def);
}

bool Matrix::PointInPoly(float * pnt)
{
	//point must be in same plane
	Segment seg;
	Segment seg2;
	Vector one, two;
	float t;
	for (int i=0; i!=3; i++)
	{
		two =(Mat+(4*i));
		two-=pnt;
		two*=-1;
		seg.LockOn((Mat+(4*i)),two.Vec);

		one =(Mat+(4*((i+2)%3)));
		one-=(Mat+(4*((i+1)%3)));
		one*=-1;
		seg2.LockOn(Mat+(4*((i+2)%3)),one.Vec);

		t=seg.TimeTo(&seg2);
		if ((t < 1) && (t > 0))
			return 0;
	}
	return 1;
}

#endif
