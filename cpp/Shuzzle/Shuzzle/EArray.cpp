
template <typename T> class EArray
{
public:
	T* _data;
	int Size, Capacity, ChunkSize;

	T* Raw() {return _data;};
	T& operator[] (int i) {return *(_data+i);};
	T* Axs(int i) {return (_data + i);};
	void ResizeTo(int size);

	T* AddMany(int count);
	T* Add();
	void Clear(bool freemem);
	int FindEqual(T to);
	void Remove(int i);

	T* StackPush() {return Add();};
	void StackPop();
	T* StackPeak() {return (_data + (Size-1));};

	EArray() {Size=0; _data=0; Capacity=0; ChunkSize=10;};
};

template <typename T> T* EArray<T>::Add()
{
	return AddMany(1);
}

template <typename T> void EArray<T>::Remove(int i)
{
	if ((i < 0) || (i >= Size))
		return;

	for (int j=i+1; j<Size; j++)
	{
		_data[j-1] = _data[j];
	}
	Size--;
}

template <typename T> int EArray<T>::FindEqual(T to)
{
	for (int i=0; i<Size; i++)
	{
		if (_data[i] == to)
			return i;
	}
	return -1;
}

template <typename T> void EArray<T>::ResizeTo(int size)
{
	if (size <= Size)
		return;
	AddMany(size - Size);
}

template <typename T> void EArray<T>::StackPop()
{
	if (Size > 0)
	{
		Size--;
	}
}

template <typename T> void EArray<T>::Clear(bool freemem)
{
	if (!freemem)
	{
		Size = 0;
		return;
	}
	if (_data)
	{
		delete [] _data;
	}
	_data = 0;
	Size = 0;
	Capacity = 0;
}

template <typename T> T* EArray<T>::AddMany(int count)
{
	if (Size+count <= Capacity)
	{
		T* ans = &(_data[Size]);
		Size+=count;
		return ans;
	}
	int nsize = Size+count+ChunkSize;
	T* nr = new T[nsize];
	if (nr==0)
	{
		printf("Couldn't create new array\n");
		int* _p=0;
		*_p = 5;
	}
	for (int i=0; i<Size; i++)
	{
		nr[i] = _data[i];
	}
	if (_data)
	{
		delete [] _data;
	}
	_data = nr;

	Capacity = nsize;
	T* ans = _data + Size;
	Size+=count;
	return ans;
}
