/**
*	@file vector.h
*
*	std::vector that can be compiled on legacy systems, or just a no-op wrapper
*	around the STL variation.
*
*	@version 0.1
*
*	Version History
*
*   0.1		Initial
*
*	@copyright ALTERNATIVE A - 0BSD (www.opensource.org/license/0BSD)
*
*	Copyright (c) 2025 Ryan Norton
*
*	Permission to use, copy, modify, and/or distribute this software for
*	any purpose with or without fee is hereby granted.
*
*	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
*	WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
*	OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
*	FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
*	DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
*	AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
*	OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*	@copyright ALTERNATIVE B - Public Domain (www.unlicense.org)
*
*	This is free and unencumbered software released into the public domain.
*
*	Anyone is free to copy, modify, publish, use, compile, sell, or distribute
*	this software, either in source code form or as a compiled binary, for any
*	purpose, commercial or non-commercial, and by any means.
*
*	In jurisdictions that recognize copyright laws, the author or authors of
*	this software dedicate any and all copyright interest in the software to
*	the public domain. We make this dedication for the benefit of the public
*	at large and to the detriment of our heirs and successors. We intend this
*	dedication to be an overt act of relinquishment in perpetuity of all
*	present and future rights to this software under copyright law.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
*	ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
*	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __VECTOR_H__
#define __VECTOR_H__
#if !defined(WSMemoryCopy) && !defined(WIKICMS_VERSION)
	#include <string.h>
	#define WSMemoryCopy memcpy
	#define WSMemoryMove memmove
	#define WSMemoryFill memset
#endif
#if !defined(DECLARE_MAXACCESSORS)
	#if (!defined(ULONG_MAX) || (defined(SIZE_MAX) && ULONG_MAX != SIZE_MAX))
		#define USE_MAXACCESSORS 1
	#else
		#define USE_MAXACCESSORS 0
	#endif
	#if USE_MAXACCESSORS
		#define DECLARE_MAXACCESSORS(DATATYPE) \
			DATATYPE& operator[](ptrdiff_t i) { return mData[i]; } \
			const DATATYPE& operator[](ptrdiff_t i) const \
			{ return (const DATATYPE&)mData[i]; } \
			DATATYPE& operator[](size_t i) { return mData[i]; } \
			const DATATYPE& operator[](size_t i) const \
			{ return (const DATATYPE&)mData[i]; }
	#else
		#define DECLARE_MAXACCESSORS(DATATYPE)
	#endif
#endif

#if defined(__cplusplus)
/* malloc/free. Bring in placement new definition for compilers needing it. */
#ifdef __GNUC__
#include <new>
#else
#include <new.h>
#endif
#if !defined(WSASSERT)
	#define WSASSERT assert
	#define WSASSERTCLAMP(var, type) do {								\
		if (((type)-1) < 0) { /* signed */								\
			WSASSERT((var) >= -(1LL << (sizeof(type)*8 - 1)) &&			\
					(var) <=  ((1LL << (sizeof(type)*8 - 1)) - 1));		\
		} else { /* unsigned */											\
			WSASSERT((var) <= ((type)-1));								\
		}																\
	} while(0)
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
	#define WSVECTORUSENONSTLTEMPLATES
#endif
#if defined(WSSTLOPT)
//=============================================================================
//
//						Vector (STL Wrapper)
//
//=============================================================================
#include <vector>
// std::move(vec.begin(), (vec.begin() + end), dest); not avail on old C++
#define Vector_Transfer(vec, dest, end) \
{\
	for(size_t i = 0; i != end; ++i)  \
		(dest)[i] = *(vec.begin() + i); \
	vec.erase(vec.begin(), vec.begin() + end);\
}
#if __cplusplus >= 201103 // C++11
#define Vector_Data(vec) (vec).data()
#else
#define Vector_Data(vec) (&(vec)[0])
#endif

#define DECLARE_STDVECTOR(NAME, DATATYPE) \
class NAME : public std::vector< DATATYPE > \
{\
public:\
	NAME() {} NAME(size_type s) : std::vector< DATATYPE >(s) {}\
	NAME(size_t size, const DATATYPE& fill) \
		: std::vector< DATATYPE >(s, fill) {} \
	void BeforeWrite(size_type s = 0) { resize(s); }\
	void SetSize(size_type s = 0) { resize(s); }\
};
#define DECLARE_RAWVECTOR(NAME, DATATYPE) DECLARE_STDVECTOR(NAME, DATATYPE)
#define DECLARE_SIMPLEVECTOR(NAME, DATATYPE) DECLARE_STDVECTOR(NAME, DATATYPE)
#define DECLARE_OBJECTVECTOR(NAME, DATATYPE) DECLARE_STDVECTOR(NAME, DATATYPE)
#else
//=============================================================================
//
//						Vector (Template)
//
// For debugging with MSVC
//=============================================================================
#if defined(WSVECTORUSENONSTLTEMPLATES)
template < class DATATYPE > class Vector
{
public:
	class iterator
	{
	public:
		iterator(const iterator& it)
			: mpTree(it.mpTree), mpKey(it.mpKey) {}
		iterator(Vector* pTree, size_t pKey)
			: mpTree(pTree), mpKey(pKey) {}
		iterator	operator ++(int)
		{iterator iRet = *this; ++*this; return iRet;}
		iterator	operator --(int)
		{iterator iRet = *this; --*this; return iRet;}
		iterator&	operator ++()
		{++mpKey; return *this;}
		iterator&	operator --()
		{--mpKey; return *this;}
		iterator	operator +(size_t amount)
		{ return iterator(mpTree, mpKey + amount);}
		iterator	operator -(size_t amount)
		{ return iterator(mpTree, mpKey - amount);}
		int operator ==
			(const iterator& i) const {return this->mpKey == i.mpKey;}
		int operator !=
			(const iterator& i) const {return this->mpKey != i.mpKey;}
		iterator& operator = (const iterator& i)
		{ this->mpTree = i.mpTree; this->mpKey = i.mpKey; return *this;}
		DATATYPE& operator * () {return (*mpTree)[mpKey];}
	protected:
		Vector* mpTree;
		size_t mpKey;
		friend class Vector< DATATYPE >;
	};

	Vector() : mData(NULL), mLength(0), mSize(0) { }
	Vector(size_t size) : mLength(0)
	{
		mData = (DATATYPE*)
			::malloc((size) * sizeof(DATATYPE));
		if(mData)
		{
			mSize = size;
			for(;mLength < mSize; ++mLength)
				placement_new(((void*)&mData[mLength]), DATATYPE());
		}
		else
			mSize = 0;
	}
	Vector(size_t size, const DATATYPE& fill) : mLength(0)
	{
		mData = (DATATYPE*)
			::malloc((size) * sizeof(DATATYPE));
		if(mData)
		{
			mSize = size;
			for(;mLength < mSize; ++mLength)
				placement_new(((void*)&mData[mLength]), DATATYPE(fill));
		}
		else
			mSize = 0;
	}
	Vector(const Vector& otherArray) :
		mData(otherArray.mData), mLength(otherArray.mLength)
	{
		mSize = otherArray.mSize;
		((Vector&)otherArray).mSize = 0;
	}
	~Vector() { Destroy(); }
	void append(const DATATYPE& item, size_t amount)
	{
		WSASSERT(mSize >= mLength);
		if (!CopyBeforeWrite(mLength + amount))
			return;
		OnCopyItems(mLength, item, amount);
		mLength += amount;
	}
	void append(const DATATYPE* data, size_t amount)
	{
		if (!CopyBeforeWrite(mLength + amount))
			return;
		OnCopyItems(mLength, data, amount);
		mLength += amount;
	}
	void assign(const Vector& otherArray)
	{
		Destroy();
		mData = otherArray.mData;
		mLength = otherArray.mLength;
		mSize = otherArray.mSize;
		((Vector&)otherArray).mSize = 0;
	}
	DATATYPE& at(size_t index)
	{ WSASSERT(mSize >= mLength); return mData[index]; }
	const DATATYPE& at(size_t index) const
	{ WSASSERT(mSize >= mLength); return mData[index]; }
	void BeforeWrite(size_t len = 0) { reserve(len); }
	iterator begin() const { return iterator((Vector*)this, 0); }
	size_t capacity() const
	{ WSASSERT(mSize >= mLength); return mSize; }
	void clear()
	{ erase(0, mLength); }
	DATATYPE* data()
	{ WSASSERT(mSize >= mLength); return mData; }
	iterator end() const { return iterator((Vector*)this, mLength); }
	void erase(const iterator& where) { erase(where.mpKey); }
	void erase(size_t where, size_t amount = 1)
	{
		WSASSERT(mLength >= where + amount);
		OnDestroyItems(where, amount);
		mLength -= amount;
		if (where < mLength)
			WSMemoryCopy(&mData[where], &mData[where + amount],
							(mLength - where) * sizeof(DATATYPE));
	}
	void insert(const DATATYPE* data, size_t amount = 1,
				size_t where = 0)
	{
		WSASSERT(data != NULL);
		WSASSERT(amount != 0);
		if (!CopyBeforeWrite(mLength + amount))
			return;
		WSMemoryMove(&mData[where + amount], &mData[where],
						(mLength - where) * sizeof(DATATYPE));
		OnCopyItems(where, data, amount);
		mLength += amount;
	}
	void insert(const DATATYPE& item, size_t where = 0)
	{ insert(&item, 1, where); }
	void push_back(const DATATYPE& item)
	{ append(item, 1); }
	void push_back(DATATYPE& item)
	{ append((const DATATYPE&)item, 1); }
	void reserve(size_t newsize)
	{
		if (mSize < newsize)
			DoResize(newsize);
	}
	void resize(size_t inLength)
	{
		reserve(inLength);
		for(;mLength < mSize; ++mLength)
			placement_new(((void*)&mData[mLength]), DATATYPE());
	}
	void SetSize(size_t len = 0) { mLength = len; }
	size_t size() const
	{ return mLength; }
	void Transfer(DATATYPE* data, size_t amount)
	{
		WSASSERT(amount <= mLength);
		WSMemoryCopy(data, mData, amount * sizeof(DATATYPE));
		WSMemoryMove(mData, mData+amount,
			(mLength -= amount) * sizeof(DATATYPE));
	}
	operator DATATYPE* const() const { return mData; }
	DATATYPE& operator[](int i) { return mData[i]; }
	const DATATYPE& operator[](int i) const
	{ return (const DATATYPE&)mData[i]; }
	DATATYPE& operator[](unsigned int i) { return mData[i]; }
	const DATATYPE& operator[](unsigned int i) const
	{ return (const DATATYPE&)mData[i]; }
	DATATYPE& operator[](long i) { return mData[i]; }
	const DATATYPE& operator[](long i) const
	{ return (const DATATYPE&)mData[i]; }
	DATATYPE& operator[](unsigned long i) { return mData[i]; }
	const DATATYPE& operator[](unsigned long i) const
	{ return (const DATATYPE&)mData[i]; }
	DECLARE_MAXACCESSORS(DATATYPE)
protected:
	void OnCopyItem(size_t where, const DATATYPE& item)
	{ placement_new(((void*)&mData[where]), DATATYPE(item)); }
	void OnCopyItems(size_t where, const DATATYPE& item, size_t amount)
	{
		for (size_t i = where; i < where + amount; ++i)
			OnCopyItem(i, item);
	}
	void OnCopyItems(size_t where, const DATATYPE* items, size_t amount)
	{
		for (size_t i = where, j = 0; i < where + amount; ++i, ++j)
			OnCopyItem(i, items[j]);
	}
	void OnDestroyItems(size_t where, size_t amount)
	{
		for (size_t i = where; i < where + amount; ++i)
			mData[i].~DATATYPE();
	}
	void RemoveAll()
	{
		OnDestroyItems(0, mLength);
		mLength = 0;
	}
	void Destroy()
	{
		if (mSize)
		{
			RemoveAll();
			::free(mData);
		}
	}
	bool CopyBeforeWrite(size_t minimumSize)
	{
		WSASSERT(mSize >= mLength);
		if (minimumSize > mSize)
			return DoResize(minimumSize * 2);
		return true;
	}
	bool DoResize(size_t newSize)
	{
		bool success;
		WSASSERT(mSize >= mLength);
		DATATYPE* newData =
			(DATATYPE*)::realloc(mData, (newSize) * sizeof(DATATYPE));
		success = newData != NULL;
		if(!success)
		{
			Destroy();
			mSize = mLength = 0;
		}
		else
			mSize = newSize;
		mData = newData;
		return success;
	}
	Vector& operator =(const Vector& otherArray)
	{ assign(otherArray); return *this; }
	DATATYPE*	mData;
	size_t	mLength;
	size_t	mSize;
};
#define DECLARE_RAWVECTOR(NAME, DATATYPE) \
	class NAME : public Vector< DATATYPE > \
	{ \
	public: \
		NAME(){} NAME(size_t size) : Vector< DATATYPE >(size) {} \
		NAME(size_t size, const DATATYPE& fill) : \
			Vector< DATATYPE >(size, fill) {} \
	};
#define DECLARE_SIMPLEVECTOR(NAME, DATATYPE) DECLARE_RAWVECTOR(NAME, DATATYPE)
#define DECLARE_OBJECTVECTOR(NAME, DATATYPE) DECLARE_RAWVECTOR(NAME, DATATYPE)

#else //NON-STL MACROIZED
//=============================================================================
//
//						Vector (Macroized)
//
// Standard implementation for compatability
//=============================================================================
#define DECLARE_VECTOR(NAME, DATATYPE, ITEMCOPYFUNCS) \
class NAME\
{\
public:\
	class iterator\
	{\
	public:\
		iterator(const iterator& it)\
			: mpTree(it.mpTree), mpKey(it.mpKey) {}						   \
		iterator(NAME* pTree, size_t pKey)						 \
			: mpTree(pTree), mpKey(pKey) {}									 \
		iterator	operator ++(int)								\
		{iterator iRet = *this; ++*this; return iRet;}\
		iterator	operator --(int)								\
		{iterator iRet = *this; --*this; return iRet;}\
		iterator&	operator ++()\
		{++mpKey; return *this;}					\
		iterator&	operator --()\
		{--mpKey; return *this;}				\
		iterator	operator +(size_t amount) \
		{ return iterator(mpTree, mpKey + amount);}					\
		iterator	operator -(size_t amount) \
		{ return iterator(mpTree, mpKey - amount);}					\
		int operator == (const iterator& i) const \
		{return this->mpKey == i.mpKey;} \
		int operator != (const iterator& i) const \
		{return this->mpKey != i.mpKey;} \
		iterator& operator = (const iterator& i) \
		{ this->mpTree = i.mpTree; this->mpKey = i.mpKey; return *this;} \
		DATATYPE& operator * () {return (*mpTree)[mpKey];}					  \
	protected: \
		NAME* mpTree;													  \
		size_t mpKey;												\
		friend class NAME;\
	};\
	\
	NAME() : mData(NULL), mLength(0), mSize(0) { }\
	NAME(size_t size) : mLength(0)\
	{\
		mData = (DATATYPE*)\
			::malloc((size) * sizeof(DATATYPE));\
		if(mData) \
		{ \
			mSize = size; \
			for(;mLength < mSize; ++mLength) \
				placement_new(((void*)&mData[mLength]), DATATYPE()); \
		} \
		else \
			mSize = 0;	\
	}\
	NAME(size_t size, const DATATYPE& fill) : mLength(0) \
	{\
		mData = (DATATYPE*)\
			::malloc((size) * sizeof(DATATYPE));\
		if(mData)	\
		{\
			mSize = size;		\
			for(;mLength < mSize; ++mLength) \
				placement_new(((void*)&mData[mLength]), DATATYPE(fill)); \
		}\
		else \
			mSize = 0;	\
	}\
	NAME(const NAME& otherArray) :\
		mData(otherArray.mData), mLength(otherArray.mLength)\
	{\
		mSize = otherArray.mSize;\
		((NAME&)otherArray).mSize = 0;\
	}\
	~NAME() { Destroy(); }\
	void append(const DATATYPE& item, size_t amount)\
	{\
		if (!CopyBeforeWrite(mLength + amount))\
			return;\
		OnCopyItems(mLength, item, amount);\
		mLength += amount;\
	}\
	void append(const DATATYPE* data, size_t amount)\
	{\
		if (!CopyBeforeWrite(mLength + amount))\
			return;\
		OnCopyItems(mLength, data, amount);\
		mLength += amount;\
	}\
	void assign(const NAME& otherArray)\
	{\
		Destroy();\
		mData = otherArray.mData;\
		mLength = otherArray.mLength;\
		mSize = otherArray.mSize;\
		((NAME&)otherArray).mSize = 0;\
	}\
	DATATYPE& at(size_t index) \
	{ return mData[index]; }\
	const DATATYPE& at(size_t index) const \
	{ return mData[index]; }\
	void BeforeWrite(size_t len = 0) { reserve(len); }\
	iterator begin() const { return iterator((NAME*)this, 0); }\
	size_t capacity() const\
	{ return mSize; }\
	void clear()\
	{ erase(0, mLength); }\
	DATATYPE* data()\
	{ return mData; }\
	iterator end() const { return iterator((NAME*)this, mLength); }\
	void erase(const iterator& where) { erase(where.mpKey); } \
	void erase(size_t where, size_t amount = 1)\
	{\
		WSASSERT(mLength >= where + amount);\
		OnDestroyItems(where, amount);\
		mLength -= amount;\
		if (where < mLength)\
			WSMemoryCopy(&mData[where], &mData[where + amount],\
							(mLength - where) * sizeof(DATATYPE));\
	}\
	void insert(const DATATYPE* data, size_t amount = 1,\
				size_t where = 0)\
	{\
		WSASSERT(data != NULL);\
		WSASSERT(amount != 0);\
		if (!CopyBeforeWrite(mLength + amount))\
			return;\
		WSMemoryMove(&mData[where + amount], &mData[where],\
						(mLength - where) * sizeof(DATATYPE));\
		OnCopyItems(where, data, amount);\
		mLength += amount;\
	}\
	void insert(const DATATYPE& item, size_t where = 0)\
	{ insert(&item, 1, where); }\
	void push_back(const DATATYPE& item)\
	{ append(item, 1); }\
	void push_back(DATATYPE& item)\
	{ append((const DATATYPE&)item, 1); }\
	void push_front(DATATYPE& item)\
	{ insert(item, 0); }\
	void reserve(size_t newsize)\
	{\
		if (mSize < newsize)\
			DoResize(newsize);\
	}\
	void resize(size_t inLength)\
	{\
		reserve(inLength); \
		for(;mLength < mSize; ++mLength)\
			placement_new(((void*)&mData[mLength]), DATATYPE());\
	}\
	void SetSize(size_t len = 0) { mLength = len; } \
	size_t size() const\
	{ return mLength; }\
	void Transfer(DATATYPE* data, size_t amount)\
	{\
		WSASSERT(amount <= mLength);\
		WSMemoryCopy(data, mData, amount * sizeof(DATATYPE));\
		WSMemoryMove(mData, \
			mData+amount, (mLength -= amount) * sizeof(DATATYPE));\
	}\
	operator const DATATYPE*() const { return mData; }\
	DATATYPE& operator[](int i) { return mData[i]; }\
	const DATATYPE& operator[](int i) const \
	{ return (const DATATYPE&)mData[i]; }\
	DATATYPE& operator[](unsigned int i) { return mData[i]; }\
	const DATATYPE& operator[](unsigned int i) const \
	{ return (const DATATYPE&)mData[i]; }\
	DATATYPE& operator[](long i) { return mData[i]; }\
	const DATATYPE& operator[](long i) const \
	{ return (const DATATYPE&)mData[i]; }\
	DATATYPE& operator[](unsigned long i) { return mData[i]; }\
	const DATATYPE& operator[](unsigned long i) const \
	{ return (const DATATYPE&)mData[i]; }\
	DECLARE_MAXACCESSORS(DATATYPE)\
protected:\
	ITEMCOPYFUNCS\
	void RemoveAll()\
	{\
		OnDestroyItems(0, mLength);\
		mLength = 0;\
	}\
	void Destroy()\
	{\
		if (mSize)\
		{\
			RemoveAll();\
			::free(mData);\
		}\
	}\
	bool CopyBeforeWrite(size_t minimumSize)\
	{\
		if (minimumSize > mSize)\
			return DoResize(minimumSize * 2);\
		return true;\
	}\
	bool DoResize(size_t newSize)\
	{\
		bool success; \
		WSASSERT(mSize >= mLength); \
		DATATYPE* newData = \
			(DATATYPE*)::realloc(mData, (newSize) * sizeof(DATATYPE)); \
		success = newData != NULL; \
		if(!success) \
		{ \
			Destroy(); \
			mSize = mLength = 0; \
		} \
		else \
			mSize = newSize; \
		mData = newData; \
		return success; \
	}\
	NAME& operator =(const NAME& otherArray)\
	{ assign(otherArray); return *this; }\
	DATATYPE*	mData;\
	size_t	mLength;\
	size_t	mSize;\
};

#define DECLARE_RAWVECTORFUNCS(NAME, DATATYPE)\
void OnCopyItem(size_t where, const DATATYPE& item)\
{ ((DATATYPE&)mData[where]) = (DATATYPE&)item; }\
void OnCopyItems(size_t where, const DATATYPE& item, size_t amount)\
{\
	for (size_t i = where; i < where + amount; ++i)\
		OnCopyItem(i, item);\
}\
void OnCopyItems(size_t where, const DATATYPE* items, size_t amount)\
{\
	WSMemoryCopy(&mData[where], items, amount * sizeof(DATATYPE));\
}\
void OnDestroyItems(size_t, size_t)\
{ }
#define DECLARE_SIMPLEVECTORFUNCS(NAME, DATATYPE)\
void OnCopyItem(size_t where, const DATATYPE& item)\
{ ((DATATYPE&)mData[where]) = (DATATYPE&)item; }\
void OnCopyItems(size_t where, const DATATYPE& item, size_t amount)\
{\
	for (size_t i = where; i < where + amount; ++i)\
		OnCopyItem(i, item);\
}\
void OnCopyItems(size_t where, const DATATYPE* items, size_t amount)\
{\
	for (size_t i = where, j = 0; i < where + amount; ++i, ++j)\
		OnCopyItem(i, items[j]);\
}\
void OnDestroyItems(size_t, size_t)\
{ }
#define DECLARE_OBJECTVECTORFUNCS(NAME, DATATYPE)\
void OnCopyItem(size_t where, const DATATYPE& item)\
{ placement_new(((void*)&mData[where]), DATATYPE(item)); }\
void OnCopyItems(size_t where, const DATATYPE& item, size_t amount)\
{\
	for (size_t i = where; i < where + amount; ++i)\
		OnCopyItem(i, item);\
}\
void OnCopyItems(size_t where, const DATATYPE* items, size_t amount)\
{\
	for (size_t i = where, j = 0; i < where + amount; ++i, ++j)\
		OnCopyItem(i, items[j]);\
}\
void OnDestroyItems(size_t where, size_t amount)\
{\
	for (size_t i = where; i < where + amount; ++i)\
		mData[i].~DATATYPE();\
}

#define DECLARE_RAWVECTOR(NAME, DATATYPE) \
DECLARE_VECTOR(NAME, DATATYPE, DECLARE_RAWVECTORFUNCS(NAME, DATATYPE))
#define DECLARE_SIMPLEVECTOR(NAME, DATATYPE) \
DECLARE_VECTOR(NAME, DATATYPE, DECLARE_SIMPLEVECTORFUNCS(NAME, DATATYPE))
#define DECLARE_OBJECTVECTOR(NAME, DATATYPE) \
DECLARE_VECTOR(NAME, DATATYPE, DECLARE_OBJECTVECTORFUNCS(NAME, DATATYPE))
#endif /* NON-STL MACROIZED END */

#define Vector_Transfer(vec, dest, end) (vec).Transfer((dest), end)
#define Vector_Data(vec) (vec).data()

#endif /* NON-STL END */
#endif /* __cplusplus */
/*****************************************************************************/
/**********************************Vector (c)*********************************/
/*****************************************************************************/
#ifndef DECLARE_SIMPLEVECTORFUNCS_C
#define DECLARE_SIMPLEVECTORFUNCS_C(NAME, FUNCNAME, DATATYPE)\
void FUNCNAME##_OnCopyItem(NAME* pThis, size_t loc, const DATATYPE* item)\
{ pThis->mData[loc] = *item; }\
void FUNCNAME##_OnCopyItemMultiple(NAME* pThis, size_t loc, \
	const DATATYPE* item, size_t amount)\
{\
	size_t i;\
	for (i = loc; i < loc + amount; ++i)\
		FUNCNAME##_OnCopyItem(pThis, i, item);\
}\
void FUNCNAME##_OnCopyItems(NAME* pThis, size_t loc, \
	const DATATYPE** items, size_t amount)\
{\
	size_t i, j;\
	for (i = loc, j = 0; i < loc + amount; ++i, ++j)\
		FUNCNAME##_OnCopyItem(pThis, i, &(*items)[j]);\
}\
void FUNCNAME##_OnDestroyItems(NAME* pThis, size_t loc, size_t amount)\
{ (void)(pThis); (void)(loc); (void)(amount); }

#define DECLARE_OBJECTVECTORFUNCS_C(NAME, FUNCNAME, DATATYPE, \
	CREATECOPY, DESTROY)\
void FUNCNAME##_OnCopyItem(NAME* pThis, size_t loc, const DATATYPE* item)\
{ CREATECOPY(&pThis->mData[loc], item); }\
void FUNCNAME##_OnCopyItemMultiple(NAME* pThis, size_t loc, \
	const DATATYPE* item, size_t amount)\
{\
	size_t i;\
	for (i = loc; i < loc + amount; ++i)\
		FUNCNAME##_OnCopyItem(pThis, i, item);\
}\
void FUNCNAME##_OnCopyItems(NAME* pThis, size_t loc, \
	const DATATYPE** items, size_t amount)\
{\
	size_t i, j;\
	for (i = loc, j = 0; i < loc + amount; ++i, ++j)\
		FUNCNAME##_OnCopyItem(pThis, i, &(*items)[j]);\
}\
void FUNCNAME##_OnDestroyItems(NAME* pThis, size_t loc, size_t amount)\
{\
	size_t i;\
	for (i = loc; i < loc + amount; ++i)\
	{	DESTROY(&pThis->mData[i]); }\
}


#define DECLARE_VECTOR_C(NAME, FUNCNAME, DATATYPE, ITEMCOPYFUNCS) \
typedef struct NAME\
{\
	DATATYPE* mData;\
	size_t mLength;\
	size_t mSize;\
} NAME;\
ITEMCOPYFUNCS \
void FUNCNAME##_RemoveAll(NAME* pThis)\
{\
	FUNCNAME##_OnDestroyItems(pThis, 0, pThis->mLength);\
	pThis->mLength = 0;\
}\
void FUNCNAME##_Destroy(NAME* pThis)\
{\
	if (pThis->mSize)\
	{\
		FUNCNAME##_RemoveAll(pThis);\
		free(pThis->mData);\
	}\
}\
wsbool FUNCNAME##_DoResize(NAME* pThis, size_t newSize)\
{\
	wsbool success; \
	DATATYPE* newData = \
		(DATATYPE*)realloc(pThis->mData, \
			(size_t)(newSize) * sizeof(DATATYPE));\
	success = newData != NULL; \
	if(!success)\
	{\
		FUNCNAME##_Destroy(pThis);\
		pThis->mSize = pThis->mLength = 0;\
	}\
	else\
		pThis->mSize = newSize;\
	pThis->mData = newData;\
	return success;\
}\
wsbool FUNCNAME##_CopyBeforeWrite(NAME* pThis, size_t minimumSize)\
{\
	if (minimumSize > pThis->mSize)\
		return FUNCNAME##_DoResize(pThis, minimumSize * 2);\
	return TRUE;\
}\
void FUNCNAME##_reserve(NAME* pThis, size_t newsize)\
{\
	if (pThis->mSize < newsize)\
		FUNCNAME##_DoResize(pThis, newsize);\
}\
void FUNCNAME##_resize(NAME* pThis, size_t inLength)\
{ FUNCNAME##_reserve(pThis, inLength); pThis->mLength = inLength; }\
void FUNCNAME##_append(NAME* pThis, const DATATYPE* item)\
{\
	if (!FUNCNAME##_CopyBeforeWrite(pThis, pThis->mLength + 1))\
		return;\
	FUNCNAME##_OnCopyItem(pThis, pThis->mLength, item);\
	pThis->mLength += 1;\
}\
void FUNCNAME##_AppendEmpty(NAME* pThis)\
{\
	if (!FUNCNAME##_CopyBeforeWrite(pThis, pThis->mLength + 1))\
		return;\
	pThis->mLength += 1;\
}\
void FUNCNAME##_AppendWithItem(NAME* pThis, \
	const DATATYPE* item, size_t amount)\
{\
	if (!FUNCNAME##_CopyBeforeWrite(pThis, pThis->mLength + amount))\
		return;\
	FUNCNAME##_OnCopyItemMultiple(pThis, pThis->mLength, item, amount);\
	pThis->mLength += amount;\
}\
void FUNCNAME##_AppendWithItems(NAME* pThis, \
	const DATATYPE** data, size_t amount)\
{\
	if (!FUNCNAME##_CopyBeforeWrite(pThis, pThis->mLength + amount))\
		return;\
	FUNCNAME##_OnCopyItems(pThis, pThis->mLength, data, amount);\
	pThis->mLength += amount;\
}\
void FUNCNAME##_assign(NAME* pThis, const NAME* otherArray)\
{\
	FUNCNAME##_Destroy(pThis);\
	pThis->mData = otherArray->mData;\
	pThis->mLength = otherArray->mLength;\
	pThis->mSize = otherArray->mSize;\
	((NAME*)otherArray)->mSize = 0;\
}\
void FUNCNAME##_BeforeWrite(NAME* pThis, size_t len) \
{ FUNCNAME##_reserve(pThis, len); }\
size_t FUNCNAME##_capacity(const NAME* pThis)\
{ return pThis->mSize; }\
void FUNCNAME##_erase(NAME* pThis, size_t loc, size_t amount)\
{\
	WSASSERT(pThis->mLength >= loc + amount);\
	FUNCNAME##_OnDestroyItems(pThis, loc, amount);\
	pThis->mLength -= amount;\
	if (loc < pThis->mLength)\
		WSMemoryCopy(&pThis->mData[loc], &pThis->mData[loc + amount],\
						(size_t)(pThis->mLength - loc) * sizeof(DATATYPE));\
}\
void FUNCNAME##_clear(NAME* pThis)\
{ FUNCNAME##_erase(pThis, 0, pThis->mLength); }\
wsbool FUNCNAME##_empty(NAME* pThis)\
{ return pThis->mLength == 0; }\
DATATYPE* FUNCNAME##_data(NAME* pThis)\
{ return pThis->mData; }\
void FUNCNAME##_insert(NAME* pThis, const DATATYPE** data, size_t amount,\
			size_t loc)\
{\
	WSASSERT(data != NULL && *data != NULL);\
	WSASSERT(amount != 0);\
	if (!FUNCNAME##_CopyBeforeWrite(pThis, pThis->mLength + amount))\
		return;\
	WSMemoryMove(&pThis->mData[loc + amount], &pThis->mData[loc],\
					(size_t)(pThis->mLength - loc) * sizeof(DATATYPE));\
	FUNCNAME##_OnCopyItems(pThis, loc, data, amount);\
	pThis->mLength += amount;\
}\
void FUNCNAME##_InsertItem(NAME* pThis, const DATATYPE* item, size_t loc)\
{ FUNCNAME##_insert(pThis, &item, 1, loc); }\
void FUNCNAME##_push_back(NAME* pThis, const DATATYPE* item)\
{ FUNCNAME##_append(pThis, item); }\
void FUNCNAME##_push_back_nonconst(NAME* pThis, DATATYPE* item)\
{ FUNCNAME##_append(pThis, (const DATATYPE*)item); }\
void FUNCNAME##_push_front(NAME* pThis, const DATATYPE* item)\
{ FUNCNAME##_insert(pThis, &item, 1, 0); }\
void FUNCNAME##_SetSize(NAME* pThis, size_t len) { pThis->mLength = len; } \
size_t FUNCNAME##_size(const NAME* pThis)\
{ return pThis->mLength; }\
void FUNCNAME##_Transfer(NAME* pThis, DATATYPE** data, size_t amount)\
{\
	WSASSERT(amount <= pThis->mLength);\
	WSMemoryCopy(*data, pThis->mData, (size_t)(amount) * sizeof(DATATYPE));\
	WSMemoryMove(pThis->mData, pThis->mData+amount, \
		(size_t)(pThis->mLength -= amount) * sizeof(DATATYPE));\
}\
void FUNCNAME##_Construct(NAME* pThis)\
{\
	pThis->mData = NULL;\
	pThis->mLength = pThis->mSize = 0;\
}\
void FUNCNAME##_ConstructWithSize(NAME* pThis, size_t size) \
{\
	pThis->mLength = 0; \
	pThis->mData = (DATATYPE*)\
		malloc((size_t)(size) * sizeof(DATATYPE));\
	if(pThis->mData)					\
		pThis->mSize = size;		\
	else \
		pThis->mSize = 0;	\
}\
void FUNCNAME##_ConstructCopy(NAME* pThis, const NAME* otherArray) \
{\
	pThis->mData = otherArray->mData; \
	pThis->mLength = otherArray->mLength;\
	pThis->mSize = otherArray->mSize;\
	((NAME*)otherArray)->mSize = 0;\
}\
void FUNCNAME##_Destruct(NAME* pThis) { FUNCNAME##_Destroy(pThis); }\
DATATYPE* FUNCNAME##_At(NAME* pThis, size_t index)\
{ return &pThis->mData[index]; }

#define DECLARE_SIMPLEVECTOR_C(NAME, FUNCNAME, DATATYPE) \
DECLARE_VECTOR_C(NAME, FUNCNAME, DATATYPE, \
	DECLARE_SIMPLEVECTORFUNCS_C(NAME, FUNCNAME, DATATYPE))
#define DECLARE_OBJECTVECTOR_C(NAME, FUNCNAME, DATATYPE, CREATECOPY, DESTROY) \
DECLARE_VECTOR_C(NAME, FUNCNAME, DATATYPE, \
	DECLARE_OBJECTVECTORFUNCS_C(NAME, FUNCNAME, DATATYPE, CREATECOPY, DESTROY))
#endif /* #ifndef DECLARE_SIMPLEVECTORFUNCS_C */

#endif /* __VECTOR_H__ */
