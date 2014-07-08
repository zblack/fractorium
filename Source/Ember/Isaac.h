#pragma once

#include "EmberDefines.h"

/// <summary>
/// C++ TEMPLATE VERSION OF Robert J. Jenkins Jr.'s
/// ISAAC Random Number Generator.
/// 
/// Ported from vanilla C to to template C++ class
/// by Quinn Tyler Jackson on 16-23 July 1998.
/// 
/// 	quinn@qtj.net
/// 
/// The function for the expected period of this
/// random number generator, according to Jenkins is:
/// 
/// 	f(a,b) = 2**((a+b*(3+2^^a)-1)
/// 
/// 	(where a is ALPHA and b is bitwidth)
/// 		
/// So, for a bitwidth of 32 and an ALPHA of 8,
/// the expected period of ISAAC is:
/// 
/// 	2^^(8+32*(3+2^^8)-1) = 2^^8295
/// 
/// Jackson has been able to run implementations
/// with an ALPHA as high as 16, or
/// 
/// 	2^^2097263
/// 
/// -Modified by Matt Feemster to eliminate needless dynamic memory allocation and virtual functions and bring inline with Ember coding style.
/// </summary>

#ifndef __ISAAC64
   typedef unsigned long int ISAAC_INT;
   const ISAAC_INT GOLDEN_RATIO = ISAAC_INT(0x9e3779b9);
#else
	typedef unsigned __int64 ISAAC_INT;
	const ISAAC_INT GOLDEN_RATIO = ISAAC_INT(0x9e3779b97f4a7c13);
#endif
	
namespace EmberNs
{
/// <summary>
/// QTIsaac class which allows using ISAAC in an OOP manner.
/// </summary>
template <int ALPHA = 4, class T = ISAAC_INT> 
class EMBER_API QTIsaac
{
public:
	typedef unsigned char byte;
	enum { N = (1 << ALPHA) };

	/// <summary>
	/// Global ISAAC RNG to be used from anywhere. This is not thread safe, so take caution to only
	/// use it when no other threads are.
	/// </summary>
	static auto_ptr<QTIsaac<ALPHA, ISAAC_INT>> GlobalRand;

	/// <summary>
	/// The structure which holds all of the random information.
	/// </summary>
	struct EMBER_API randctx
	{
		T randcnt;
		T randrsl[N];
		T randmem[N];
		T randa;
		T randb;
		T randc;
	};
   
	/// <summary>
	/// Constructor which initialized the random context using the values passed in.
	/// Leaving these as their defaults is fine, and will still give different
	/// results because time is internally used if they are default.
	/// However, specifying specific values is useful if you want to duplicate
	/// a sequence of random numbers.
	/// </summary>
	/// <param name="a">First random seed. Default: 0.</param>
	/// <param name="b">Second random seed. Default: 0.</param>
	/// <param name="c">Third random seed. Default: 0.</param>
	/// <param name="s">Pointer to a buffer of 256 random integer seeds. Default: NULL.</param>
	QTIsaac(T a = 0, T b = 0, T c = 0, T* s = NULL)
	{
		Srand(a, b, c, s);
	}

	/// <summary>
	/// Return the next random integer.
	/// </summary>
	/// <returns>The next random integer</returns>
	inline T Rand()
	{
		return (m_Rc.randcnt++ == N ? (Isaac(&m_Rc), m_Rc.randcnt=0, m_Rc.randrsl[m_Rc.randcnt]) : m_Rc.randrsl[m_Rc.randcnt]);
	}

	/// <summary>
	/// Return the next random integer between 0 and the value passed in.
	/// </summary>
	/// <returns>A value one greater than the maximum value that will be returned</returns>
	inline T Rand(T upper)
	{
		return (upper == 0) ? Rand() : Rand() % upper;
	}

	/// <summary>
	/// Returns a random floating point value between the specified minimum and maximum.
	/// Template argument expected to be float or double.
	/// </summary>
	/// <param name="fMin">The minimum value allowed, inclusive.</param>
	/// <param name="fMax">The maximum value allowed, inclusive.</param>
	/// <returns>A new random floating point value within the specified range, inclusive.</returns>
	template<typename floatType>
	inline floatType Frand(floatType fMin, floatType fMax)
	{
		floatType f = (floatType)Rand() / (floatType)std::numeric_limits<T>::max();
		return fMin + (f * (fMax - fMin));
	}

	/// <summary>
	/// Thin wrapper around a call to Frand() with a range of 0-1.
	/// Template argument expected to be float or double.
	/// </summary>
	/// <returns>A new random number in the range of 0-1, inclusive.</returns>
	template<typename floatType>
	inline floatType Frand01()
	{
		return Frand<floatType>(floatType(0), floatType(1));
	}

	/// <summary>
	/// Thin wrapper around a call to Frand() with a range of -1-1.
	/// Template argument expected to be float or double.
	/// </summary>
	/// <returns>A new random number in the range of -1-1, inclusive.</returns>
	template<typename floatType>
	inline floatType Frand11()
	{
		return Frand<floatType>(floatType(-1), floatType(1));
	}

	/// <summary>
	/// Not sure what this does.
	/// </summary>
	/// <returns>Something that is golden</returns>
	template<typename floatType>
	inline floatType GoldenBit()
	{
		return RandBit() ? floatType(0.38196) : floatType(0.61804);
	}

	/// <summary>
	/// Returns a random 0 or 1.
	/// </summary>
	/// <returns>A random 0 or 1</returns>
	inline unsigned int RandBit()
	{
		return Rand() & 1;
	}

	/// <summary>
	/// A different way of getting a floating point rand in the range -1-1.
	/// Flam3 used this but it seems unnecessary now, keep around if it's ever needed.
	/// </summary>
	/// <returns>A new random number in the range of -1-1, inclusive.</returns>
	//double drand11()
	//{
	//	return (((int)Rand() & 0xfffffff) - 0x7ffffff) / (double) 0x7ffffff;
	//}

	/// <summary>
	/// Initializes a random context.
	/// Unsure exacly how this works, but it does.
	/// </summary>
	/// <param name="ctx">The random context to initialize</param>
	/// <param name="useSeed">Whether to use the seeds passed in to the constructor, else zero.</param>
	void RandInit(randctx* ctx, bool useSeed)
	{
		int i;
		T a, b, c, d, e, f, g, h;
		T* m = ctx->randmem;
		T* r = ctx->randrsl;
   
		a = b = c = d = e = f = g = h = GOLDEN_RATIO;
   
		if (!useSeed)
		{
			ctx->randa = 0;
			ctx->randb = 0;
			ctx->randc = 0;
		}
   
		//Scramble it.
		for (i = 0; i < 4; ++i)         
		{
			Shuffle(a, b, c, d, e, f, g, h);
		}
   
		if (useSeed) 
		{
			//Initialize using the contents of r[] as the seed.
			for (i = 0; i < N; i += 8)
			{
				a += r[i    ]; b += r[i + 1]; c += r[i + 2]; d += r[i + 3];
				e += r[i + 4]; f += r[i + 5]; g += r[i + 6]; h += r[i + 7];
		 
				Shuffle(a, b, c, d, e, f, g, h);
		 
				m[i    ] = a; m[i + 1] = b; m[i + 2] = c; m[i + 3] = d;
				m[i + 4] = e; m[i + 5] = f; m[i + 6] = g; m[i + 7] = h;
			}           
	  
			//Do a second pass to make all of the seed affect all of m.
			for (i = 0; i < N; i += 8)
			{
				a += m[i    ]; b += m[i + 1]; c += m[i + 2]; d += m[i + 3];
				e += m[i + 4]; f += m[i + 5]; g += m[i + 6]; h += m[i + 7];
		 
				Shuffle(a, b, c, d, e, f, g, h);
		 
				m[i    ] = a; m[i + 1] = b; m[i + 2] = c; m[i + 3] = d;
				m[i + 4] = e; m[i + 5] = f; m[i + 6] = g; m[i + 7] = h;
			}
		}
		else
		{
			//Fill in mm[] with messy stuff.
			Shuffle(a, b, c, d, e, f, g, h);
	  
			m[i    ] = a; m[i + 1] = b; m[i + 2] = c; m[i + 3] = d;
			m[i + 4] = e; m[i + 5] = f; m[i + 6] = g; m[i + 7] = h;
		}
   
		Isaac(ctx);      //Fill in the first set of results.
		ctx->randcnt = 0;//Prepare to use the first set of results.
	}

	/// <summary>
	/// Initialize the seeds of the member random context using the specified seeds.
	/// If s is null, time plus index up to 256 is used for the random buffer.
	/// </summary>
	/// <param name="a">First random seed. Default: 0.</param>
	/// <param name="b">Second random seed. Default: 0.</param>
	/// <param name="c">Third random seed. Default: 0.</param>
	/// <param name="s">Pointer to a buffer of 256 random integer seeds. Default: NULL.</param>
	void Srand(T a = 0, T b = 0, T c = 0, T* s = NULL)
	{
		if (s == NULL)//Default to using time plus index as the seed if s was NULL.
		{
			for (int i = 0; i < N; i++)
				m_Rc.randrsl[i] = (T)time(0) + i;
		}
		else
		{
			for (int i = 0; i < N; i++)
				m_Rc.randrsl[i] = s[i];
		}

		if (a == 0 && b == 0 && c == 0)
		{
			m_Rc.randa = (T)time(0);
			m_Rc.randb = (T)time(0) * (T)time(0);
			m_Rc.randc = (T)time(0) * (T)time(0) * (T)time(0);
		}
		else
		{
			m_Rc.randa = a;
			m_Rc.randb = b;
			m_Rc.randc = c;
		}
   
		RandInit(&m_Rc, true);
	}
   
protected:
	/// <summary>
	/// Compute the next batch of random numbers for a random context.
	/// </summary>
	/// <param name="ctx">The context to populate.</param>
	void Isaac(randctx* ctx)
	{
		T x,y;
   
		T* mm = ctx->randmem;
		T* r  = ctx->randrsl;
   
		T a = (ctx->randa);
		T b = (ctx->randb + (++ctx->randc));
   
		T* m    = mm; 
		T* m2   = (m + (N / 2));
		T* mend = m2;
   
		for(; m < mend; )
		{
		#ifndef __ISAAC64
			RngStep((a << 13), a, b, mm, m, m2, r, x, y);
			RngStep((a >> 6) , a, b, mm, m, m2, r, x, y);
			RngStep((a << 2) , a, b, mm, m, m2, r, x, y);
			RngStep((a >> 16), a, b, mm, m, m2, r, x, y);
		#else   // __ISAAC64
			RngStep(~(a ^ (a << 21)), a, b, mm, m, m2, r, x, y);
			RngStep(  a ^ (a >> 5)  , a, b, mm, m, m2, r, x, y);
			RngStep(  a ^ (a << 12) , a, b, mm, m, m2, r, x, y);
			RngStep(  a ^ (a >> 33) , a, b, mm, m, m2, r, x, y);
		#endif  // __ISAAC64
		}
   
		m2 = mm;
   
		for(; m2<mend;)
		{
		#ifndef __ISAAC64
			RngStep((a << 13), a, b, mm, m, m2, r, x, y);
			RngStep((a >> 6) , a, b, mm, m, m2, r, x, y);
			RngStep((a << 2) , a, b, mm, m, m2, r, x, y);
			RngStep((a >> 16), a, b, mm, m, m2, r, x, y);
		#else   // __ISAAC64
			RngStep(~(a ^ (a << 21)), a, b, mm, m, m2, r, x, y);
			RngStep(  a ^ (a >> 5)  , a, b, mm, m, m2, r, x, y);
			RngStep(  a ^ (a << 12) , a, b, mm, m, m2, r, x, y);
			RngStep(  a ^ (a >> 33) , a, b, mm, m, m2, r, x, y);
		#endif  // __ISAAC64
		}
   
		ctx->randb = b;
		ctx->randa = a;
	}
   
	/// <summary>
	/// Retrieves a value using indirection.
	/// </summary>
	/// <param name="mm">The buffer.</param>
	/// <param name="x">The offset.</param>
	/// <returns>A new value</returns>
	inline T Ind(T* mm, T x)
	{
#ifndef __ISAAC64
		return (*(T*)((byte*)(mm) + ((x) & ((N - 1) << 2))));
#else   // __ISAAC64
		return (*(T*)((byte*)(mm) + ((x) & ((N - 1) << 3))));
#endif  // __ISAAC64
	}

	/// <summary>
	/// Unsure what this does.
	/// </summary>
	void RngStep(T mix, T& a, T& b, T*& mm, T*& m, T*& m2, T*& r, T& x, T& y)
	{
		x = *m;  
		a = (a ^ (mix)) + *(m2++); 
		*(m++) = y = Ind(mm, x) + a + b; 
		*(r++) = b = Ind(mm, y >> ALPHA) + x; 
	}

	/// <summary>
	/// Unsure what this does.
	/// </summary>
	void Shuffle(T& a, T& b, T& c, T& d, T& e, T& f, T& g, T& h)
	{ 
#ifndef __ISAAC64
		a ^= b << 11; d += a; b += c; 
		b ^= c >>  2; e += b; c += d; 
		c ^= d <<  8; f += c; d += e; 
		d ^= e >> 16; g += d; e += f; 
		e ^= f << 10; h += e; f += g; 
		f ^= g >>  4; a += f; g += h; 
		g ^= h <<  8; b += g; h += a; 
		h ^= a >>  9; c += h; a += b; 
#else // __ISAAC64
		a -= e; f ^= h >> 9;  h += a;
		b -= f; g ^= a << 9;  a += b;
		c -= g; h ^= b >> 23; b += c;
		d -= h; a ^= c << 15; c += d;
		e -= a; b ^= d >> 14; d += e;
		f -= b; c ^= e << 20; e += f;
		g -= c; d ^= f >> 17; f += g;
		h -= d; e ^= g << 14; g += h;
#endif // __ISAAC64
	}
   
private:
	randctx m_Rc;//The random context which holds all of the seed and state information as well as the random number values.
};
}