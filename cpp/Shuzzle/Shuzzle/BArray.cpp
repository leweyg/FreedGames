
//Basic Array - a single linked array for use as a stack
//	or queue. Almost all funcs are O(1) except for 
//	DeleteTop, RemoveCell, DelAll, Axs, the [] operator, 
//	and DelCell which are all O(n)
//
//Use the StackPush, StackPop, StackGet to make it act
//	like a stack, or QueuePush QueuePop and QueueGet
//	All Stack and Queue functions are O(1)

#ifndef HasBArray
#define HasBArray

template <class X> struct BAMem
{
	BAMem<X> * Next;
	X Val;
};

template <class X> class BArray
{
public:
	BAMem<X> * Base, * Last;

	BAMem<X> * AddCellTop();
	BAMem<X> * AddCellBottom();
	BAMem<X> * AxsCell(int i);
	BAMem<X> * FindCell(X * data);
	void DelTop();
	void DelBottom();
	void DelCell(BAMem<X>*cell);
	void DelAll();
	void RemoveCell(BAMem<X>*work);
	void InsertCell(BAMem<X>*cell,BAMem<X>*before);
	void InsertCellAt(BAMem<X>*cell,int i) {InsertCell(cell,AxsCell(i-1));};
	int GetLength();
	int Length() {return GetLength();};
	void Resize(int nsize);
	void Reverse();
	void SwapCells(BAMem<X>* a, BAMem<X>* b);

	X * AddAt(int i);
	X * AddMember() {return (&AddCellTop()->Val);};
	X * AddTop() {return (&AddCellTop()->Val);};
	X * Add()	{return (&AddCellTop()->Val);};
	X * AddBottom() {return (&AddCellBottom()->Val);};
	X * Axs(int i) {return (&AxsCell(i)->Val);};
	X & operator [](int i) {return AxsCell(i)->Val;};

	X * StackPush() {return (&AddCellBottom()->Val);};
	void StackPop() {DelBottom();};
	X * StackGet() {if (Base) return &(Base->Val); else return 0;};
	X * StackPeak() {return StackGet();};
	X * StackGetLast() {if (Last) return &(Last->Val); else return 0;};

	X * QueuePush() {return (&AddCellTop()->Val);};
	void QueuePop() {DelBottom();};
	X * QueueGet() {if (Base) return &(Base->Val); else return 0;};
	X * QueuePeak() {return QueueGet();};

	BArray() {Base=0; Last=0;};
	~BArray() {DelAll();};
};

template <class X> void BArray<X>::SwapCells(BAMem<X>* a, BAMem<X>* b)
{
	if (a == b)
		return;

	BAMem<X>* beforea = (a != Base)? Base : 0;
	while ((beforea) && (beforea->Next != a))
		beforea = beforea->Next;

	BAMem<X>* beforeb = (b != Base)? Base : 0;
	while ((beforeb) && (beforeb->Next != b))
		beforeb = beforeb->Next;

	BAMem<X>* t;
	if (!beforeb)
	{
		t = a;
		a = b;
		b = t;
		t = beforea;
		beforea = beforeb;
		beforeb = t;
	}

	if (!beforea)
	{
		Base = b;
		t = b->Next;
		if (beforeb != a)
			b->Next = a->Next;
		else
			b->Next = a;
		a->Next = t;
		if (Last==b)
			Last = a;
		return;
	}

	t = b->Next;
	if (beforeb!=a)
	{
		b->Next = a->Next;
		beforeb->Next = a;
	}
	else
		b->Next = a;
	if (beforea!=b)
	{
		a->Next = t;
		beforea->Next = b;
	}
	else
		a->Next = b;

	if (Last==a)
		Last=b;
	else
	{
		if (Last==b)
			Last=a;
	}
}

template <class X> X * BArray<X>::AddAt(int i)
{
	BAMem<X> * n = new BAMem<X>;
	InsertCellAt(n, i);
	return &n->Val;
}

template <class X> void BArray<X>::Reverse()
{
	if (Base == Last)
		return;
	BAMem<X> * work = Base, * prev=0;
	BAMem<X> * next = work->Next;
	
	while (work)
	{
		work->Next = prev;
		prev = work;
		work = next;
		if (next)
			next = next->Next;
	}

	work = Base;
	Base = Last;
	Last = work;
}

template <class X> void BArray<X>::Resize(int nsize)
{
	int csize = GetLength();
	if (nsize == csize)
		return;
	while (csize < nsize)
	{
		AddCellTop();
		csize++;
	}
	while (csize > nsize)
	{
		DelTop();
		csize--;
	}
}

template <class X> BAMem<X> * BArray<X>::FindCell(X * data)
{
	BAMem<X>*work=Base;
	while (work)
	{
		if (&work->Val == data)
			return work;
		work=work->Next;
	}
	return 0;
}

template <class X> int BArray<X>::GetLength()
{
	BAMem<X>*work=Base;
	int len=0;
	while (work)
	{
		len++;
		work=work->Next;
	}
	return len;
}

template <class X> void BArray<X>::DelCell(BAMem<X>*cell)
{
	BAMem<X>*work=Base;
	if(!cell)
		return;
	if (work==cell)
	{
		Base=work->Next;
		if (!work->Next)
			Last=0;
		delete cell;
		return;
	}
	while (work->Next!=cell)
		work=work->Next;
	work->Next=cell->Next;
	if (!work->Next)
		Last=work;
	delete cell;
}

template <class X> void BArray<X>::DelAll()
{
	BAMem<X> *work;
	BAMem<X> *other=Base;
	while (other)
	{
		work=other;
		other=other->Next;
		delete work;
	}
	Base=0; Last=0;
}

template <class X> BAMem<X> * BArray<X>::AxsCell(int i)
{
	BAMem<X> *work=Base;
	while ((work) && (i > 0))
	{
		work=work->Next;
		i--;
	}
	return work;
}

template <class X> BAMem<X> * BArray<X>::AddCellTop()
{
	BAMem<X>*work=new BAMem<X>;
	work->Next=0;
	if (!Base)
	{
		Base=work;
		Last=work;
		return work;
	}
	Last->Next=work;
	Last=work;
	return work;
}

template <class X> BAMem<X> * BArray<X>::AddCellBottom()
{
	BAMem<X>*work=new BAMem<X>;
	if (!Base)
	{
		work->Next=0;
		Base=work;
		Last=work;
		return work;
	}
	work->Next=Base;
	Base=work;
	return work;
}

template <class X> void BArray<X>::DelBottom()
{
	BAMem<X>*work=Base;
	if (!work)
		return;
	Base=Base->Next;
	if (!Base)
		Last=0;
	delete work;
}

template <class X> void BArray<X>::DelTop()
{
	BAMem<X>*work=Base;
	if (Last==work)
	{
		delete work;
		Base=0; Last=0;
		return;
	}
	while (work->Next!=Last)
		work=work->Next;
	Last=work;
	delete work->Next;
	work->Next=0;
}

template <class X> void BArray<X>::RemoveCell(BAMem<X>*cell)
{
	BAMem<X>*work=Base;
	if (work==cell)
	{
		Base=work->Next;
		if (!work->Next)
			Last=0;
		return;
	}
	while (work->Next!=cell)
		work=work->Next;
	work->Next=cell->Next;
	if (!work->Next)
		Last=work;
}

template <class X> void BArray<X>::InsertCell(BAMem<X>*cell,BAMem<X>*before)
{
	//inserts a cell after 'before' (or at the begining if before is null)
	if (!before)
	{
		cell->Next=Base;
		Base=cell;
		if (!Last)
			Last=cell;
		return;
	}
	cell->Next=before->Next;
	before->Next=cell;
	if (Last==before)
		Last=cell;
}

#endif
