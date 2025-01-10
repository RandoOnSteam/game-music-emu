// Sets up common environment for Shay Green's libraries.
// To change configuration options, modify blargg_config.h, not this file.

#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

//
//	stdint.h wrapping
//
#if !defined(__clang__) /* For Metrowerks and others, clang complains */
	#define __STDC_LIMIT_MACROS
	#define __STDC_CONSTANT_MACROS
#endif
#include <stddef.h>
#if !defined(_MSC_VER) || _MSC_VER >= 1600 /* 2010+ */
	#ifndef UINTMAX_MAX
		#include <stdint.h> /* for NULL, size_t, other_ts */
	#endif
	#if !defined(ULONG_MAX)
		#include <limits.h>
	#endif
#elif defined(_MSC_VER) && !defined(INT_MAX) /* VC6 */
	#include <limits.h>
#endif
#ifndef INT32_MAX
	#ifdef __alpha
		typedef unsigned int uint32_t;
		typedef int int32_t;
	#else
		typedef unsigned long uint32_t;
		#if !defined(__MWERKS__) || !defined(__MACH__)
			typedef long int32_t;
		#endif /*  MWERKS MACH defs int32_t but not uint32_t */
	#endif
	#if !defined(__MWERKS__) || !defined(__MACH__)
		typedef char int8_t;
	#endif /*  MWERKS MACH defs int8_t */
	typedef unsigned char uint8_t;
	typedef short int16_t;
	typedef unsigned short uint16_t;
	#define INT32_MAX (uint32_t)(~0)
#endif

#if (defined(_MSC_VER) && defined(_WIN32) && !defined(__clang__)) \
	|| (defined(__BCPLUSPLUS__) && __BCPLUSPLUS__ >= 0x520 && defined(_WIN32)) \
	|| (defined(__SC__) && defined(__DMC__)) \
	|| (defined(__WATCOMC__) && defined(_WIN32)) \
	|| (defined(__MWERKS__) && defined(NOCRUNTIME) && defined(_WIN32))
	typedef __int64 int64_t; typedef unsigned __int64 uint64_t;
	#if defined(_WIN64)
		#define WSNATIVE64
	#endif
#elif (defined(__MINGW32__) \
	||(defined(__clang__)) \
	||(defined(SIZEOF_LONG_LONG) && SIZEOF_LONG_LONG >= 8) \
	||(defined(__GNUC__)) \
	||(defined(__CYGWIN__)) \
	||(defined(__DJGPP__) && __DJGPP__ >= 2) \
	||(defined(__MWERKS__) && !defined(NOCRUNTIME) )) \
	||(defined(SIZEOF_LONG) && SIZEOF_LONG == 8)
	/* && __option(longlong))) for MWERKS? */
#	ifndef INT64_MAX
#		ifndef __MWERKS__
		typedef long long int64_t; typedef unsigned long long uint64_t;
#		define WSNATIVE64
#		endif
#	elif defined(__LP64__) && !defined(_INT64_T)
		typedef long int64_t; typedef unsigned long uint64_t;
#		define WSNATIVE64
#	elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) \
			|| defined(_M_AMD64) || defined(__ia64__) || defined(_IA64) \
			|| defined(__ppc64__) || defined(__powerpc64__) \
			|| defined(__aarch64__) || defined(__LP64__) || defined(_LP64)
#		define WSNATIVE64
#	endif
#else/*  !(defined(_M_X64) || defined(__LP64__)) */
#define WSNO64
#endif

#if 0
	#include "system.h"
	#define blarg_memcpy WSMemoryCopy
	#define blarg_memset WSMemoryFill
	#define blarg_memmove WSMemoryMove
#else
	#define blarg_memcpy memcpy
	#define blarg_memset memset
	#define blarg_memmove memmove
	#if defined(__cplusplus)
		template <class DATATYPE> void placement_new(
			void* d, const DATATYPE& t) { new (d) DATATYPE(t); }
	#endif
#endif


//
//	Nothrow
//
#include <new>
#ifndef NOTHROW
	#if !defined(WSOLDNEW) \
	&& (!defined(__MWERKS__) || !defined(_MSL_NO_THROW_SPECS))
		#if defined(__MWERKS__) && defined(macintosh) \
		&& (!defined(TARGET_API_MAC_CARBON) || !TARGET_API_MAC_CARBON)
			extern std::nothrow_t wsnothrow;
			#define NOTHROW (wsnothrow)
		#else
			#define NOTHROW (std::nothrow)
		#endif
	#else
		#define NOTHROW
	#endif
#endif

#define BLARGG_DISABLE_NOTHROW
#if defined(_MSC_VER) && _MSC_VER <= 1400 /* 2005 <= */
	#define nullptr NULL
	#define override
	#define uint_fast32_t uint32_t
	#define int_fast16_t int16_t
	#define uint_fast16_t uint16_t
	#define uint_fast8_t uint8_t
	#define alignas(x) /* not supported */
#endif
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#if defined(new)
#undef new /* conflicts with C++ stdlib */
#endif

#undef BLARGG_COMMON_H
// allow blargg_config.h to #include blargg_common.h
#include "blargg_config.h"
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

// BLARGG_RESTRICT: equivalent to restrict, where supported
#if __GNUC__ >= 3 || _MSC_VER >= 1100
	#define BLARGG_RESTRICT __restrict
#else
	#define BLARGG_RESTRICT
#endif

// STATIC_CAST(T,expr): Used in place of static_cast<T> (expr)
#ifndef STATIC_CAST
	#define STATIC_CAST(T,expr) ((T) (expr))
#endif

#if !defined(_MSC_VER) || _MSC_VER >= 1910
	#define blaarg_static_assert(cond, msg) static_assert(cond, msg)
#else
	#define blaarg_static_assert(cond, msg) assert(cond)
#endif

// blargg_err_t (0 on success, otherwise error string)
#ifndef blargg_err_t
	typedef const char* blargg_err_t;
#endif

// blargg_vector - very lightweight vector of POD types (no constructor/destructor)
template<class T>
class blargg_vector {
	T* begin_;
	size_t size_;
public:
	blargg_vector() : begin_( 0 ), size_( 0 ) { }
	~blargg_vector() { free( begin_ ); }
	size_t size() const { return size_; }
	T* begin() const { return begin_; }
	T* end() const { return begin_ + size_; }
	blargg_err_t resize( size_t n )
	{
		void* p = realloc( begin_, n * sizeof (T) );
		if ( !p && n )
			return "Out of memory";
		begin_ = (T*) p;
		size_ = n;
		return 0;
	}
	void clear() { free( begin_ ); begin_ = nullptr; size_ = 0; }
	T& operator [] ( size_t n ) const
	{
		assert( n <= size_ ); // <= to allow past-the-end value
		return begin_ [n];
	}
};

// Use to force disable exceptions for allocations of a class
#ifndef BLARGG_DISABLE_NOTHROW
	#define BLARGG_DISABLE_NOTHROW \
		void* operator new ( size_t s ) noexcept { return malloc( s ); }\
		void* operator new ( size_t s, const std::nothrow_t& ) noexcept { return malloc( s ); }\
		void operator delete ( void* p ) noexcept { free( p ); }\
		void operator delete ( void* p, const std::nothrow_t&) noexcept { free( p ); }
#endif
#define BLARGG_NEW new NOTHROW

// BLARGG_4CHAR('a','b','c','d') = 'abcd' (four character integer constant)
#define BLARGG_4CHAR( a, b, c, d ) \
	((a&0xFF)*0x1000000L + (b&0xFF)*0x10000L + (c&0xFF)*0x100L + (d&0xFF))

#define BLARGG_2CHAR( a, b ) \
	((a&0xFF)*0x100L + (b&0xFF))

// BOOST_STATIC_ASSERT( expr ): Generates compile error if expr is 0.
#ifndef BOOST_STATIC_ASSERT
	#ifdef _MSC_VER
		#if _MSC_VER < 1910
			// MSVC6 (_MSC_VER < 1300) fails for use of __LINE__ when /Zl is specified
			#define BOOST_STATIC_ASSERT( expr, msg ) \
				//assert(expr) void blargg_failed_( int (*arg) [2 / (int) !!(expr) - 1] )
		#else
			#define BOOST_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
		#endif
	#else
		// Some other compilers fail when declaring same function multiple times in class,
		// so differentiate them by line
		#define BOOST_STATIC_ASSERT( expr, msg ) \
			void blargg_failed_( int (*arg) [2 / !!(expr) - 1] [__LINE__] )
	#endif
#endif

// BLARGG_COMPILER_HAS_BOOL: If 0, provides bool support for old compiler. If 1,
// compiler is assumed to support bool. If undefined, availability is determined.
#ifndef BLARGG_COMPILER_HAS_BOOL
	#if defined (__MWERKS__)
		#if !__option(bool)
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	#elif defined (_MSC_VER)
		#if _MSC_VER < 1100
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	#elif defined (__GNUC__)
		// supports bool
	#elif __cplusplus < 199711
		#define BLARGG_COMPILER_HAS_BOOL 0
	#endif
#endif
#if defined (BLARGG_COMPILER_HAS_BOOL) && !BLARGG_COMPILER_HAS_BOOL
	// If you get errors here, modify your blargg_config.h file
	typedef int bool;
	const bool true  = 1;
	const bool false = 0;
#endif

// blargg_long/blargg_ulong = at least 32 bits, int if it's big enough

#if INT_MAX < 0x7FFFFFFF || LONG_MAX == 0x7FFFFFFF
	typedef long blargg_long;
#else
	typedef int blargg_long;
#endif

#if UINT_MAX < 0xFFFFFFFF || ULONG_MAX == 0xFFFFFFFF
	typedef unsigned long blargg_ulong;
#else
	typedef unsigned blargg_ulong;
#endif

// int8_t etc.

// Apply minus sign to unsigned type and prevent the warning being shown
	template<typename T>
	inline T uMinus(T in)
	{
		return ~(in - 1);
	}

#if __GNUC__ >= 3
	#define BLARGG_DEPRECATED __attribute__ ((deprecated))
#else
	#define BLARGG_DEPRECATED
#endif

// Use in place of "= 0;" for a pure virtual, since these cause calls to std C++ lib.
// During development, BLARGG_PURE( x ) expands to = 0;
// virtual int func() BLARGG_PURE( { return 0; } )
#ifndef BLARGG_PURE
	#define BLARGG_PURE( def ) def
#endif

#endif
#endif
