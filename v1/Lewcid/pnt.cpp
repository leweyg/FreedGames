
#ifndef Has_Pnt
#define Has_Pnt

namespace NameSpacePnt
{

template <typename T> struct pnt
{
	T x, y, z;

	inline void operator =(T * other);
	pnt<T> operator +(T * other);
	pnt<T> operator -(T * other);
	pnt<T> operator *(T other);
	void operator +=(T * other);
	void operator +=(T other);
	void operator -=(T * other);
	void operator *=(T other);
	void operator *=(pnt<T>& other);
	void operator /=(T other);
	inline void Set(T ax, T ay, T az);
	bool operator ==(T * other);
	bool operator !=(T * other);
	T Dist(T * to);
	T Product() {return (x*y*z);};
	T Sum() {return (x+y+z);};
	T Dot(T * other);
	T Length();
	T MaxValue();
	void Normalize();
	pnt<T> Cross(T * other);

	inline T & operator [] (int i);
	inline operator T* () {return &x;};

	int NumBytes() {return (3*sizeof(T));};
	char * FirstByte() {return (char*)(&x);};

	pnt<T>() {};
	pnt<T>(T ax, T ay, T az) {x=ax; y=ay; z=az;};
	pnt<T>(T * v)	{x=v[0]; y=v[1]; z=v[2];};
	pnt<T>(pnt<T> & other) {x=other.x; y=other.y; z=other.z;};
};

template <typename T> T pnt<T>::MaxValue()
{
	if (x > y)
	{
		if (x > z)
			return x;
		return z;
	}
	else
	{
		if (y > z)
			return y;
		return z;
	}
}

template <typename T> void pnt<T>::operator *= (pnt<T>& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
}

template <typename T> T pnt<T>::Length()
{
	return (T)sqrt( Dot( *this ) );
}

template <typename T> void pnt<T>::Normalize()
{
	(*this) /= (T)sqrt( x*x + y*y + z*z );
}

template <typename T> pnt<T> pnt<T>::Cross(T * other)
{
	pnt<T> theans;
	T * ans = theans;
	T * vec1 = (*this);
	T * vec2 = other;
	ans[0]=((vec1[1] * vec2[2]) - (vec1[2] * vec2[1]));
	ans[1]=((vec1[2] * vec2[0]) - (vec1[0] * vec2[2]));
	ans[2]=((vec1[0] * vec2[1]) - (vec1[1] * vec2[0]));
	for (int i=0; i!=3; i++)
		ans[i] *= -1;
	return theans;
}

template <typename T> T pnt<T>::Dot(T * other)
{
	T sum = 0;
	sum += x*other[0];
	sum += y*other[1];
	sum += z*other[2];
	return sum;
}

template <typename T> T pnt<T>::Dist(T * to)
{
	T sum=0, t;
	t = (x-to[0]);
	sum += t*t;
	t = (y-to[1]);
	sum += t*t;
	t = (z-to[2]);
	sum += t*t;
	return (T)sqrt(sum);
}

template <typename T> pnt<T> pnt<T>::operator *(T other)
{
	pnt<T> ans;
	ans.x = x * other;
	ans.y = y * other;
	ans.z = z * other;
	return ans;
}

template <typename T> bool pnt<T>::operator !=(T * other)
{
	return ((x != other[0]) || (y != other[1]) || (z != other[2]));
}

template <typename T> bool pnt<T>::operator ==(T * other)
{
	return ((x == other[0]) && (y == other[1]) && (z == other[2]));
}

template <typename T> void pnt<T>::operator *=(T other)
{
	x *= other;
	y *= other;
	z *= other;
}

template <typename T> void pnt<T>::operator /= (T  other)
{
	x /= other;
	y /= other;
	z /= other;
}


template <typename T> void pnt<T>::operator += (T other)
{
	x += other;
	y += other;
	z += other;
}

template <typename T> void pnt<T>::operator -= (T * other)
{
	x -= other[0];
	y -= other[1];
	z -= other[2];
}

template <typename T> void pnt<T>::operator += (T * other)
{
	x += other[0];
	y += other[1];
	z += other[2];
}

template <typename T> pnt<T> pnt<T>::operator -(T * other)
{
	pnt<T> ans;
	ans.x = x - other[0];
	ans.y = y - other[1];
	ans.z = z - other[2];
	return ans;
}

template <typename T> pnt<T> pnt<T>::operator +(T * other)
{
	pnt<T> ans;
	ans.x = x + other[0];
	ans.y = y + other[1];
	ans.z = z + other[2];
	return ans;
}

template <typename T> void pnt<T>::operator = (T * other)
{
	x = other[0];
	y = other[1];
	z = other[2];
}

template <typename T> void pnt<T>::Set(T ax, T ay, T az)
{
	x = ax;
	y = ay;
	z = az;
}

template <typename T> T & pnt<T>::operator [] (int i) 
{
	return *((&x)+i);
}

}

typedef NameSpacePnt::pnt<float> fpnt;
typedef NameSpacePnt::pnt<int> ipnt;

fpnt FPNT(float x, float y, float z)
{
	fpnt ans;
	ans.x = x;
	ans.y = y;
	ans.z = z;
	return ans;
}

fpnt FPNT(float * vec)
{
	fpnt ans(vec);
	return ans;
}

ipnt IPNT(int x, int y, int z)
{
	ipnt ans;
	ans.x = x;
	ans.y = y;
	ans.z = z;
	return ans;
}

ipnt IPNT(int * vec)
{
	ipnt ans(vec);
	return ans;
}

fpnt ITOFPNT(int* vec)
{
	return FPNT(vec[0], vec[1], vec[2]);
}

ipnt FTOIPNT(float* vec)
{
	return IPNT(vec[0], vec[1], vec[2]);
}

#endif Has_Pnt
