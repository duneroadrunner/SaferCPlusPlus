
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*
This example file has become quite large and holds examples for many data types. Your best bet is probably to use a find/search to
get to the data type your interested in.
*/


//define MSE_SAFER_SUBSTITUTES_DISABLED /* This will replace all the classes with their native/standard counterparts. */

/* Each of the following will replace a subset of the classes with their native/standard counterparts. */
//define MSE_MSTDVECTOR_DISABLED
//define MSE_REGISTEREDPOINTER_DISABLED
//define MSE_SAFERPTR_DISABLED /* MSE_SAFERPTR_DISABLED implies MSE_REGISTEREDPOINTER_DISABLED too. */
//define MSE_PRIMITIVES_DISABLED
//define MSE_REFCOUNTINGPOINTER_DISABLED
//define MSE_SCOPEPOINTER_DISABLED

/* The following adds run-time checks to scope pointers in debug mode */
//define MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED
//define MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED // This adds them to non-debug modes too.

/* The following will result in program termination instead of exceptions being thrown. */
//define MSE_CUSTOM_THROW_DEFINITION(x) std::cerr << std::endl << x.what(); exit(-11)
/* Note that MSE_CUSTOM_THROW_DEFINITION(x) can be applied on a "per header file" basis if desired. */

/* The following directs the vectors and arrays to use the safe substitutes for native primitives (like int
and size_t) in their interface and implementation. This adds a marginal increase in safety. (Mostly due to
the interface.) */
//define MSE_MSEVECTOR_USE_MSE_PRIMITIVES 1
//define MSE_MSEARRAY_USE_MSE_PRIMITIVES 1

/* msvc2015's incomplete support for "constexpr" means that range checks that should be done at compile time would
be done at run time, at significant cost. So they are disabled by default for that compiler. Here we're "forcing"
them to be enabled. */
#define MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED

#define MSE_SELF_TESTS

//include "msetl.h"
#include "msemsearray.h"
#include "msemstdarray.h"
#include "msemsevector.h"
#include "msemstdvector.h"
#include "mseivector.h"
#include "msevector_test.h"
#include "mseregistered.h"
#include "mserelaxedregistered.h"
#include "mserefcounting.h"
#include "mserefcountingofregistered.h"
#include "mserefcountingofrelaxedregistered.h"
#include "msescope.h"
#include "mseasyncshared.h"
#include "msepoly.h"
#include "mseprimitives.h"
#include <algorithm>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
//include <thread>
//include <sstream>
#include <future>

/* This block of includes is required for the mse::TRegisteredRefWrapper example */
#include <algorithm>
#include <list>
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
#include <functional>

class H {
public:
	/* Just an example of a templated member function. In this case it's a static one, but it doesn't have to be.
	You might consider templating pointer parameter types to give the caller some flexibility as to which kind of
	(smart/safe) pointer they want to use. */
	template<typename _Tpointer>
	static int foo4(_Tpointer A_ptr) { return A_ptr->b; }

	template<typename _Tpointer, typename _Tvector>
	static int foo5(_Tpointer A_ptr, _Tvector& vector_ref) {
		int tmp = A_ptr->b;
		int retval = 0;
		vector_ref.clear();
		if (A_ptr) {
			retval = A_ptr->b;
		}
		else {
			retval = -1;
		}
		return retval;
	}

	template<class _TString1Pointer, class _TString2Pointer>
	static std::string foo6(_TString1Pointer i1ptr, _TString2Pointer i2ptr) {
		return (*i1ptr) + (*i2ptr);
	}

	template<class _TAsyncSharedReadWriteAccessRequester>
	static double foo7(_TAsyncSharedReadWriteAccessRequester A_ashar) {
		auto t1 = std::chrono::high_resolution_clock::now();
		/* A_ashar.readlock_ptr() will block until it can obtain a read lock. */
		auto ptr1 = A_ashar.readlock_ptr(); // while ptr1 exists it holds a (read) lock on the shared object
		auto t2 = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		auto timespan_in_seconds = time_span.count();
		auto thread_id = std::this_thread::get_id();
		//std::cout << "thread_id: " << thread_id << ", time to acquire read pointer: " << timespan_in_seconds << " seconds.";
		//std::cout << std::endl;
		return timespan_in_seconds;
	}
protected:
	~H() {}
};

int main(int argc, char* argv[])
{
	mse::msevector_test msevector_test;
	msevector_test.run_all();

#ifndef MSE_MSTDVECTOR_DISABLED
	{
		/**********************/
		/*   mstd::vector<>   */
		/**********************/

		/* mse::mstd::vector<> is an almost "completely safe" (bounds checked, iterator checked and memory managed)
		implementation of std::vector. Here we'll demonstate the safety of the insert() member function. */

		double a1[3] = { 1.0, 2.0, 3.0 };
		double *d_pointer1 = &(a1[0]);
		double a2[3] = { 4.0, 5.0, 360 };
		double *d_pointer2 = &(a2[0]);
		mse::mstd::vector<double> v1;
		//v1.insert(v1.begin(), d_pointer1, d_pointer2); /* not good */
		/* std::vector supports "naked" pointers as parameters to the insert() member
		function so mse::mstd::vector does also. Unfortunately there is no way to ensure
		that the naked pointer parameters have valid values. */

#ifdef MSVC2010_COMPATIBLE
		mse::mstd::vector<double> v2(a1, a1+3);
		mse::mstd::vector<double> v3(a2, a2+3);
		mse::mstd::vector<double> v4;
#else /*MSVC2010_COMPATIBLE*/
		mse::mstd::vector<double> v2 = { 1.0, 2.0, 3.0 };
		mse::mstd::vector<double> v3 = { 4.0, 5.0, 360 };
		mse::mstd::vector<double> v4;
#endif /*MSVC2010_COMPATIBLE*/
		try {
			v4.insert(v4.begin(), v2.begin(), v3.begin());
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by a comparision of incompatible "safe" iterators. */
		}
	}

	{
		/* Here we're demonstrating mse::mstd::vector<>'s "java-like" safety due to it's "managed" lifespan.  */

		typedef mse::mstd::vector<int> vint_type;
		mse::mstd::vector<vint_type> vvi;
		{
			vint_type vi;
			vi.push_back(5);
			vvi.push_back(vi);
		}
		auto vi_it = vvi[0].begin();
		vvi.clear();
		try {
			/* At this point, the vint_type object is cleared from vvi, but it has not been deallocated/destructed yet because it
			"knows" that there is an iterator, namely vi_it, that is still referencing it. At the moment, std::shared_ptrs are being
			used to achieve this. */
			auto value = (*vi_it); /* So this is actually ok. vi_it still points to a valid item. */
			assert(5 == value);
			vint_type vi2;
			vi_it = vi2.begin();
			/* The vint_type object that vi_it was originally pointing to is now deallocated/destructed, because vi_it no longer
			references it. */
		}
		catch (...) {
			/* At present, no exception will be thrown. We're still debating whether it'd be better to throw an exception though. */
		}
	}
#endif // !MSE_MSTDVECTOR_DISABLED

	{
		/*****************************/
		/*   msevector<>::ipointer   */
		/*****************************/

		/* mse::msevector<> is another vector that is highly compatible with std::vector<>. But mse::msevector<> also
		supports a new type of iterator called "ipointer". ipointers make more (intuitive) sense than standard vector
		iterators. Upon insert or delete, ipointers continue to point to the same item, not (necessarily) the same
		position. And they don't become "invalid" upon insert or delete, unless the item they point to is deleted. They
		support all the standard iterator operators, but also have member functions with "friendlier" names. */

#ifdef MSVC2010_COMPATIBLE
		int a1[4] = { 1, 2, 3, 4 };
		mse::msevector<int> v1(a1, a1+4);
#else /*MSVC2010_COMPATIBLE*/
		mse::msevector<int> v1 = { 1, 2, 3, 4 };
#endif /*MSVC2010_COMPATIBLE*/

		mse::msevector<int> v = v1;

		{
			mse::msevector<int>::ipointer ip1 = v.ibegin();
			ip1 += 2;
			assert(3 == (*ip1));
			auto ip2 = v.ibegin(); /* ibegin() returns an ipointer */
			v.erase(ip2); /* remove the first item */
			assert(3 == (*ip1)); /* ip1 continues to point to the same item, not the same position */
			ip1--;
			assert(2 == (*ip1));
			for (mse::msevector<int>::cipointer cip = v.cibegin(); v.ciend() != cip; cip++) {
				/* You might imagine what would happen if cip were a regular vector iterator. */
				v.insert(v.ibegin(), (*cip));
			}
		}
		v = v1;
		{
			/* This code block is equivalent to the previous code block, but uses ipointer's more "readable" interface
			that might make the code a little more clear to those less familiar with C++ syntax. */
			mse::msevector<int>::ipointer ip_vit1 = v.ibegin();
			ip_vit1.advance(2);
			assert(3 == ip_vit1.item());
			auto ip_vit2 = v.ibegin();
			v.erase(ip_vit2);
			assert(3 == ip_vit1.item());
			ip_vit1.set_to_previous();
			assert(2 == ip_vit1.item());
			mse::msevector<int>::cipointer cip(v);
			for (cip.set_to_beginning(); cip.points_to_an_item(); cip.set_to_next()) {
				v.insert_before(v.ibegin(), (*cip));
			}
		}

		/* Btw, ipointers are compatible with stl algorithms, like any other stl iterators. */
		std::sort(v.ibegin(), v.iend());

		/* And just to be clear, mse::msevector<> retains it's original (high performance) stl::vector iterators. */
		std::sort(v.begin(), v.end());

		/* mse::msevector<> also provides "safe" (bounds checked) versions of the original stl::vector iterators. */
		std::sort(v.ss_begin(), v.ss_end());

		/* mse::ivector<> is another vector for when safety and "correctness" are more of a priority than performance
		or compatibility. It is completely safe like mse::mstd::vector<> but only supports the "ipointer" iterators.
		It does not support the (problematic) standard vector iterator behavior. */
		mse::ivector<int> iv = { 1, 2, 3, 4 };
		std::sort(iv.begin(), iv.end());
		mse::ivector<int>::ipointer ivip = iv.begin();
	}

	{
		/*********************/
		/*   mstd::array<>   */
		/*********************/

		/* mse::mstd::array<> is an almost "completely safe" (bounds checked, iterator checked and "lifespan aware")
		implementation of std::array. */

		/* Here we demonstrate some iterator safety. */

		mse::mstd::array<int, 3> a1 = { 1, 2, 3 };
		mse::mstd::array<int, 3> a2 = { 11, 12, 13 };
		try {
			for (auto it1 = a1.begin(); it1 != a2.end(); it1++) {
				/* It's not going to make it here. The invalid iterator comparison will throw an exception. */
				std::cerr << "unexpected execution" << std::endl;
			}
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
		}

		/* Here we're demonstrating mse::mstd::array<> and its iterator's "lifespan awareness".  */
		mse::mstd::array<int, 2>::iterator it1;
		{
			mse::mstd::array<int, 2> a3 = { 5 }; /*Notice that initializer lists may contain fewer elements than the array.*/
			it1 = a3.begin();
			assert(5 == (*it1));
		}
		try {
			/* it1 "knows" that its target has been destroyed. It will throw an exception on any attempt to dereference it. */
			int i = (*it1);
			std::cerr << "unexpected execution" << std::endl;
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
		}

		mse::mstd::array_test testobj1;
		testobj1.test1();
	}

	{
		/******************/
		/*   msearray<>   */
		/******************/

		/* mse::msearray<> is another array implementation that's not quite as safe as mse::mstd::array<> in the sense
		that its iterators are not "lifespan aware". And it provides both unsafe and safe iterators. Basically,
		mse::msearray<> is a compromise between performance and safety. */

		mse::msearray<int, 3> a1 = { 1, 2, 3 };
		mse::msearray<int, 3> a2 = { 11, 12, 13 };

		//bool bres1 = (a1.begin() == a2.end());
		/* The previous commented out line would result in "undefined behavior. */

		try {
			/* The behavior of the next line is not "undefined". It's going to throw an exception. */
			bool bres2 = (a1.ss_begin() == a2.ss_end());
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
		}

		auto ss_cit1 = a1.ss_cbegin();
		/* These safe iterators support traditional and "friendly" iterator operation syntax. */
		ss_cit1++;
		ss_cit1.set_to_next(); /*same as previous line*/
		ss_cit1.set_to_beginning();
		bool bres3 = ss_cit1.has_previous();
		ss_cit1.set_to_end_marker();
		bool bres4 = ss_cit1.points_to_an_item();

		mse::msearray_test testobj1;
		testobj1.test1();
	}

	{
		/*******************************/
		/*   CInt, CSize_t and CBool   */
		/*******************************/

		/* The unsigned types like size_t can cause insidious bugs due to the fact that they can cause signed integers to be
		implicitly converted to unsigned. msetl provides substitutes for size_t and int that change the implicit conversion to
		instead be from unsigned to signed. */

		mse::s_type_test1();

#ifndef MSE_PRIMITIVES_DISABLED
		{
			size_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			int minimum_number_of_security_credits_required_for_access = 7;
			bool access_granted = false;
			if (number_of_security_credits - minimum_number_of_security_credits_required_for_access >= 0) {
				/* You may not even get a compiler warning about the implicit conversion from (signed) int to (unsigned) size_t. */
				access_granted = true; /*oops*/
			}
			else {
				access_granted = false;
				assert(false);
			}
		}

		{
			mse::CSize_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			int minimum_number_of_security_credits_required_for_access = 7;
			bool access_granted = false;
			if (number_of_security_credits - minimum_number_of_security_credits_required_for_access >= 0) {
				access_granted = true;
				assert(false);
			}
			else {
				access_granted = false; /* that's better */
			}
		}

		{
			size_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			mse::CInt minimum_number_of_security_credits_required_for_access = 7;
			mse::CBool access_granted = false;
			if (number_of_security_credits - minimum_number_of_security_credits_required_for_access >= 0) {
				access_granted = true;
				assert(false);
			}
			else {
				access_granted = false; /* this works too */
			}
		}
#endif // !MSE_PRIMITIVES_DISABLED

		mse::CSize_t mse_szt1 = 0;
		/* size_t szt2 = mse_szt1; */ /* This wouldn't compile. */
#ifdef MSVC2010_COMPATIBLE
		size_t szt1 = mse::as_a_size_t(mse_szt1); /* We exclude automatic conversion from mse::CSize_t to size_t because we
													 consider size_t an intrinsically error prone type. */
#else /*MSVC2010_COMPATIBLE*/
		size_t szt1 = static_cast<size_t>(mse_szt1); /* We exclude automatic conversion from mse::CSize_t to size_t because we
													 consider size_t an intrinsically error prone type. */
#endif /*MSVC2010_COMPATIBLE*/

		try {
			mse::CSize_t mse_szt2 = 0;
			mse_szt2 = -3;
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by an "out of range" assignment to an mse::CSize_t. */
		}

		try {
			mse::CSize_t mse_szt3 = 3;
			mse_szt3 -= 1; /* this is fine */
			mse_szt3 -= 4; /* this is gonna throw an exception */
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by an attempt to set an mse::CSize_t to an "out of range" value. */
		}
	}

	{
		/**************************/
		/*   TRegisteredPointer   */
		/**************************/

		/* For safety reasons we want to avoid using native pointers. "Managed" pointers like std:shared_ptr are an alternative, but
		sometimes you don't want a pointer that takes ownership (of the object's lifespan). So we provide mse::TRegisteredPointer.
		Because it doesn't take ownership, it can be used with objects allocated on the stack and is compatible with raii
		techniques. Also, in most cases, it can be used as a compatible, direct substitute for native pointers, making it
		straightforward to update legacy code. Proper "const", "not null" and "fixed" (non-retargetable) versions are provided as
		well.*/

		class A {
		public:
			virtual ~A() {}
			int b = 3;
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TRegisteredPointer<A> A_registered_ptr) { return A_registered_ptr->b; }
			/* mse::TRegisteredFixedConstPointer<A> is recommended where you might have used "const A&".*/
			static int foo3(mse::TRegisteredFixedConstPointer<A> A_registered_fc_ptr) { return A_registered_fc_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;
		/* mse::TRegisteredPointer<> is basically a "safe" version of the native pointer. */
		mse::TRegisteredPointer<A> A_registered_ptr1;

		{
			A a;
			mse::TRegisteredObj<A> registered_a;
			/* mse::TRegisteredObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == registered_a.b);
			A_native_ptr = &a;
			A_registered_ptr1 = &registered_a;
			assert(A_native_ptr->b == A_registered_ptr1->b);

			mse::TRegisteredPointer<A> A_registered_ptr2 = &registered_a;
			A_registered_ptr2 = nullptr;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
			try {
				int i = A_registered_ptr2->b; /* this is gonna throw an exception */
			}
			catch (...) {
				std::cerr << "expected exception" << std::endl;
				/* The exception is triggered by an attempt to dereference a null "registered pointer". */
			}
#endif // !MSE_REGISTEREDPOINTER_DISABLED

			/* mse::TRegisteredPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_registered_ptr1);

			B::foo3(&registered_a);
			/* mse::TRegisteredPointers don't convert directly into mse::TRegisteredFixedConstPointers because
			mse::TRegisteredFixedConstPointers are never supposed to be null, where mse::TRegisteredPointers can be. But you
			can easily obtain an mse::TRegisteredFixedPointer which does convert to an mse::TRegisteredFixedConstPointer. */
			B::foo3(&*A_registered_ptr1);

			/* Functions can be templated to allow the caller to use the (smart/safe) pointer of their choice. */
			H::foo4<mse::TRegisteredFixedConstPointer<A>>(&*A_registered_ptr1);
			/* You don't actually need to explicitly specify the template type. */
			H::foo4(&*A_registered_ptr1);

			if (A_registered_ptr2) {
				assert(false);
			}
			else if (A_registered_ptr2 != A_registered_ptr1) {
				A_registered_ptr2 = A_registered_ptr1;
				assert(A_registered_ptr2 == A_registered_ptr1);
			}
		}

#ifndef MSE_REGISTEREDPOINTER_DISABLED
		try {
			/* A_registered_ptr1 "knows" that the (registered) object it was pointing to has now been deallocated. */
			int i = A_registered_ptr1->b; /* So this is gonna throw an exception */
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
		}
#endif // !MSE_REGISTEREDPOINTER_DISABLED

		{
			/* For heap allocations mse::registered_new is kind of analagous to std::make_shared, but again,
			mse::TRegisteredPointers don't take ownership so you are responsible for deallocation. */
			auto A_registered_ptr3 = mse::registered_new<A>();
			assert(3 == A_registered_ptr3->b);
			mse::registered_delete<A>(A_registered_ptr3);
#ifndef MSE_REGISTEREDPOINTER_DISABLED
			try {
				/* A_registered_ptr3 "knows" that the (registered) object it was pointing to has now been deallocated. */
				int i = A_registered_ptr3->b; /* So this is gonna throw an exception */
			}
			catch (...) {
				std::cerr << "expected exception" << std::endl;
			}
#endif // !MSE_REGISTEREDPOINTER_DISABLED
		}

		{
			/* Remember that registered pointers can only point to registered objects. So, for example, if you want
			a registered pointer to an object's base class object, that base class object has to be a registered
			object. */
			class DA : public mse::TRegisteredObj<A> {};
			mse::TRegisteredObj<DA> registered_da;
			mse::TRegisteredPointer<DA> DA_registered_ptr1 = &registered_da;
			mse::TRegisteredPointer<A> A_registered_ptr4 = DA_registered_ptr1;
			A_registered_ptr4 = &registered_da;

			class D : public A {};
			mse::TRegisteredObj<D> registered_d;
			mse::TRegisteredPointer<D> D_registered_ptr1 = &registered_d;
			/* The next commented out line of code is not going to work because D's base class object is not a
			registered object. */
			//mse::TRegisteredPointer<A> A_registered_ptr5 = D_registered_ptr1;

			/* Note that unlike registered pointers, relaxed registered pointers can point to base class objects
			that are not relaxed registered objects. */
		}
		{
			/* Obtaining safe pointers to members of registered objects: */
			class E {
			public:
				virtual ~E() {}
				mse::TRegisteredObj<std::string> reg_s = "some text ";
				std::string s2 = "some other text ";
			};

			mse::TRegisteredObj<E> registered_e;
			mse::TRegisteredPointer<E> E_registered_ptr1 = &registered_e;

			/* The easiest way is to obtain a safe pointer to a member of a registered object is to make the
			member itself a registered object. */
			mse::TRegisteredPointer<std::string> string_registered_ptr1 = &(E_registered_ptr1->reg_s);

			/* Another option is to make a TSyncWeakFixedPointer. syncweak_string_ptr1 here is essentially
			a pointer to "E.s2" (string member of class E) with a registered pointer to E to in its pocket.
			It uses the registered pointer to ensure that it is safe to access the object. */
			auto syncweak_string_ptr1 = mse::make_syncweak(E_registered_ptr1->s2, E_registered_ptr1);

			/* In practice, rather than declaring a specific mse::TSyncWeakFixedPointer parameter, we expect
			functions to be "templatized" so that they can accept any type of pointer. */
			std::string res1 = H::foo6(syncweak_string_ptr1, syncweak_string_ptr1);

			/* Just testing the convertibility of mse::TSyncWeakFixedPointers. */
			auto E_registered_fixed_ptr1 = &registered_e;
			auto swfptr1 = mse::make_syncweak<std::string>(E_registered_fixed_ptr1->s2, E_registered_fixed_ptr1);
			mse::TSyncWeakFixedPointer<std::string, mse::TRegisteredPointer<E>> swfptr2 = swfptr1;
			mse::TSyncWeakFixedConstPointer<std::string, mse::TRegisteredFixedPointer<E>> swfcptr1 = swfptr1;
			mse::TSyncWeakFixedConstPointer<std::string, mse::TRegisteredPointer<E>> swfcptr2 = swfcptr1;
			if (swfcptr1 == swfptr1) {
				int q = 7;
			}
			if (swfptr1 == swfcptr1) {
				int q = 7;
			}
			if (swfptr1) {
				int q = 7;
			}
		}

		{
			/***********************************/
			/*   TRelaxedRegisteredPointer   */
			/***********************************/

			/* mse::TRelaxedRegisteredPointer<> behaves very similar to mse::TRegisteredPointer<> but is even more "compatible"
			with native pointers (i.e. less explicit casting is required when interacting with native pointers and native pointer
			interfaces). So if you're updating existing or legacy code to be safer, replacing native pointers with
			mse::TRelaxedRegisteredPointer<> may be more convenient than mse::TRegisteredPointer<>.
			One case where you may need to use mse::TRelaxedRegisteredPointer<> even when not dealing with legacy code is when
			you need a reference to a class before it is fully defined. For example, when you have two classes that mutually
			reference each other. mse::TRegisteredPointer<> does not support this.
			*/

			class C;

			class D {
			public:
				virtual ~D() {}
				mse::TRelaxedRegisteredPointer<C> m_c_ptr;
			};

			class C {
			public:
				mse::TRelaxedRegisteredPointer<D> m_d_ptr;
			};

			mse::TRelaxedRegisteredObj<C> regobjfl_c;
			mse::TRelaxedRegisteredPointer<D> d_ptr = mse::relaxed_registered_new<D>();

			regobjfl_c.m_d_ptr = d_ptr;
			d_ptr->m_c_ptr = &regobjfl_c;

			mse::relaxed_registered_delete<D>(d_ptr);

			{
				/* Polymorphic conversions. */
				class FD : public mse::TRelaxedRegisteredObj<D> {};
				mse::TRelaxedRegisteredObj<FD> relaxedregistered_fd;
				mse::TRelaxedRegisteredPointer<FD> FD_relaxedregistered_ptr1 = &relaxedregistered_fd;
				mse::TRelaxedRegisteredPointer<D> D_relaxedregistered_ptr4 = FD_relaxedregistered_ptr1;
				D_relaxedregistered_ptr4 = &relaxedregistered_fd;
				mse::TRelaxedRegisteredFixedPointer<D> D_relaxedregistered_fptr1 = &relaxedregistered_fd;
				mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxedregistered_fcptr1 = &relaxedregistered_fd;

				/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
				class GD : public D {};
				mse::TRelaxedRegisteredObj<GD> relaxedregistered_gd;
				mse::TRelaxedRegisteredPointer<GD> GD_relaxedregistered_ptr1 = &relaxedregistered_gd;
				mse::TRelaxedRegisteredPointer<D> D_relaxedregistered_ptr5 = GD_relaxedregistered_ptr1;
				D_relaxedregistered_ptr5 = GD_relaxedregistered_ptr1;
				mse::TRelaxedRegisteredFixedPointer<GD> GD_relaxedregistered_fptr1 = &relaxedregistered_gd;
				D_relaxedregistered_ptr5 = &relaxedregistered_gd;
				mse::TRelaxedRegisteredFixedPointer<D> D_relaxedregistered_fptr2 = &relaxedregistered_gd;
				mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxedregistered_fcptr2 = &relaxedregistered_gd;
			}
		}

		mse::s_regptr_test1();
		mse::s_relaxedregptr_test1();

		{
			/*************************/
			/*   Simple Benchmarks   */
			/*************************/

			/* Just some simple speed tests. */
			class CE {
			public:
				CE() {}
				CE(int& count_ref) : m_count_ptr(&count_ref) { (*m_count_ptr) += 1; }
				virtual ~CE() { (*m_count_ptr) -= 1; }
				int m_x;
				int *m_count_ptr;
			};
#ifndef NDEBUG
			static const int number_of_loops = 10/*arbitrary*/;
#else // !NDEBUG
			static const int number_of_loops = 1000000/*arbitrary*/;
#endif // !NDEBUG
			std::cout << std::endl;
			std::cout << "Some simple benchmarks: \n";
			std::cout << "number of loops: " << number_of_loops << " \n" << " \n";
			{
				int count = 0;
				auto item_ptr2 = new CE(count);
				delete item_ptr2; item_ptr2 = nullptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto item_ptr = new CE(count);
					item_ptr2 = item_ptr;
					delete item_ptr;
					item_ptr = nullptr;
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "native pointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRegisteredPointer<CE> item_ptr2 = mse::registered_new<CE>(count);
				mse::registered_delete<CE>(item_ptr2);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRegisteredPointer<CE> item_ptr = mse::registered_new<CE>(count);
					item_ptr2 = item_ptr;
					mse::registered_delete<CE>(item_ptr);
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRegisteredPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRelaxedRegisteredPointer<CE> item_ptr2 = mse::relaxed_registered_new<CE>(count);
				mse::relaxed_registered_delete<CE>(item_ptr2);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRelaxedRegisteredPointer<CE> item_ptr = mse::relaxed_registered_new<CE>(count);
					item_ptr2 = item_ptr;
					mse::relaxed_registered_delete<CE>(item_ptr);
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRelaxedRegisteredPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto item_ptr2 = std::make_shared<CE>(count);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto item_ptr = std::make_shared<CE>(count);
					item_ptr2 = item_ptr;
					item_ptr = nullptr;
				}
				item_ptr2 = nullptr;

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "std::shared_ptr: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRegisteredPointer<CE> item_ptr2 = &(mse::TRegisteredObj<CE>(count));
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TRegisteredObj<CE> object(count);
						mse::TRegisteredPointer<CE> item_ptr = &object;
						item_ptr2 = item_ptr;
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRegisteredPointer targeting the stack: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRefCountingPointer<CE> item_ptr2 = mse::make_refcounting<CE>(count);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRefCountingPointer<CE> item_ptr = mse::make_refcounting<CE>(count);
					item_ptr2 = item_ptr;
					item_ptr = nullptr;
				}
				item_ptr2 = nullptr;

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRefCountingPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}

			std::cout << std::endl;
			static const int number_of_loops2 = (10/*arbitrary*/)*number_of_loops;
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					CF* m_next_item_ptr;
					int m_a = 3;
				};
				CF item1(1);
				CF item2(2);
				CF item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				CF* cf_ptr = item1.m_next_item_ptr;
				for (int i = 0; i < number_of_loops2; i += 1) {
					cf_ptr = cf_ptr->m_next_item_ptr;
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "native pointer dereferencing: " << time_span.count() << " seconds.";
				if (3 == cf_ptr->m_a) {
					std::cout << " "; /* Using cf_ptr->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TRelaxedRegisteredPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TRelaxedRegisteredObj<CF> item1(1);
				mse::TRelaxedRegisteredObj<CF> item2(2);
				mse::TRelaxedRegisteredObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				mse::TRelaxedRegisteredPointer<CF>* rpfl_ptr = std::addressof(item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					rpfl_ptr = std::addressof((*rpfl_ptr)->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRelaxedRegisteredPointer (checked) dereferencing: " << time_span.count() << " seconds.";
				if (3 == (*rpfl_ptr)->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TRelaxedRegisteredPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TRelaxedRegisteredObj<CF> item1(1);
				mse::TRelaxedRegisteredObj<CF> item2(2);
				mse::TRelaxedRegisteredObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				CF* cf_ptr = (item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					cf_ptr = (cf_ptr->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRelaxedRegisteredPointer unchecked dereferencing: " << time_span.count() << " seconds.";
				if (3 == cf_ptr->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					std::weak_ptr<CF> m_next_item_ptr;
					int m_a = 3;
				};
				auto item1_ptr = std::make_shared<CF>(1);
				auto item2_ptr = std::make_shared<CF>(2);
				auto item3_ptr = std::make_shared<CF>(3);
				item1_ptr->m_next_item_ptr = item2_ptr;
				item2_ptr->m_next_item_ptr = item3_ptr;
				item3_ptr->m_next_item_ptr = item1_ptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				std::weak_ptr<CF>* wp_ptr = &(item1_ptr->m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					wp_ptr = &((*wp_ptr).lock()->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "std::weak_ptr dereferencing: " << time_span.count() << " seconds.";
				if (3 == (*wp_ptr).lock()->m_a) {
					std::cout << " "; /* Using wp_ref.lock()->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TRefCountingPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				auto item1_ptr = mse::make_refcounting<CF>(1);
				auto item2_ptr = mse::make_refcounting<CF>(2);
				auto item3_ptr = mse::make_refcounting<CF>(3);
				item1_ptr->m_next_item_ptr = item2_ptr;
				item2_ptr->m_next_item_ptr = item3_ptr;
				item3_ptr->m_next_item_ptr = item1_ptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				mse::TRefCountingPointer<CF>* refc_ptr = &(item1_ptr->m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					refc_ptr = &((*refc_ptr)->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRefCountingPointer (checked) dereferencing: " << time_span.count() << " seconds.";
				item1_ptr->m_next_item_ptr = nullptr; /* to break the reference cycle */
				if (3 == (*refc_ptr)->m_a) {
					std::cout << " "; /* Using refc_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
		}
	}

#if defined(MSEREGISTEREDREFWRAPPER) && !defined(MSE_PRIMITIVES_DISABLED)
	{
		/*****************************/
		/*   TRegisteredRefWrapper   */
		/*****************************/

		/* Stl provides a copyable, assignable wrapper for C++ references called std::reference_wrapper. std::reference_wrappers,
		like native C++ references, are not completely safe in the sense that the object they refer to can be deallocated while
		a reference to it is still available. So we provide mse::TRegisteredRefWrapper, a safe implementation of
		std::reference_wrapper that "knows" when the object being referenced has been deallocated and will throw an exception
		on any attempt to access the object after it has been destroyed.
		In most cases it is probably preferable to just use mse::TRegisteredFixedPointer instead of mse::TRegisteredRefWrapper. 
		*/
		{
			/* This example originally comes from http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper. */
			std::list<mse::TRegisteredObj<mse::CInt>> l(10);
			std::iota(l.begin(), l.end(), -4);

			std::vector<mse::TRegisteredRefWrapper<mse::CInt>> v(l.begin(), l.end());
			// can't use shuffle on a list (requires random access), but can use it on a vector
			std::shuffle(v.begin(), v.end(), std::mt19937{ std::random_device{}() });

			std::cout << '\n';
			std::cout << "TRegisteredRefWrapper test output: \n";
			std::cout << "Contents of the list: ";
			for (auto n : l) { std::cout << n << ' '; } std::cout << '\n';

			std::cout << "Contents of the list, as seen through a shuffled vector: ";
			for (auto i : v) { std::cout << (mse::CInt&)i << ' '; } std::cout << '\n';

			std::cout << "Doubling the values in the initial list...\n";
			for (auto& i : l) {
				i *= 2;
			}

			std::cout << "Contents of the list, as seen through a shuffled vector: ";
			for (auto i : v) { std::cout << (mse::CInt&)i << ' '; } std::cout << '\n';
			std::cout << '\n';
		}
		{
			/* This example originally comes from http://www.cplusplus.com/reference/functional/reference_wrapper/. */
			mse::TRegisteredObj<mse::CInt> a(10), b(20), c(30);
			// an array of "references":
			mse::TRegisteredRefWrapper<mse::CInt> refs[] = { a,b,c };
			std::cout << "refs:";
			for (mse::CInt& x : refs) std::cout << ' ' << x;
			std::cout << '\n';

			mse::TRegisteredObj<mse::CInt> foo(10);
			auto bar = mse::registered_ref(foo);
			//++bar;
			//++(mse::TRegisteredObj<mse::CInt>&)bar;
			++(mse::CInt&)bar;
			std::cout << foo << '\n';
		}
	}
#endif // defined(MSEREGISTEREDREFWRAPPER) && !defined(MSE_PRIMITIVES_DISABLED)

	{
		/*****************************/
		/*    TRefCountingPointer    */
		/*****************************/

		/* TRefCountingPointer behaves similar to std::shared_ptr. Some differences being that it foregoes any thread safety
		mechanisms, it does not accept raw pointer assignment or construction (use make_refcounting<>() instead), and it will throw
		an exception on attempted nullptr dereference. And it's faster. And like TRegisteredPointer, proper "const", "not null"
		and "fixed" (non-retargetable) versions are provided as well. */

		class A {
		public:
			A() {}
			virtual ~A() {
				int q = 3; /* just so you can place a breakpoint if you want */
			}

			int b = 3;
			std::string s = "some text ";
		};
		typedef std::vector<mse::TRefCountingFixedPointer<A>> CRCFPVector;
		class B {
		public:
			static int foo1(mse::TRefCountingPointer<A> A_refcounting_ptr, CRCFPVector& rcfpvector_ref) {
				rcfpvector_ref.clear();
				int retval = A_refcounting_ptr->b;
				A_refcounting_ptr = nullptr; /* Target object is destroyed here. */
				return retval;
			}
			static std::string foo2(mse::TStrongFixedPointer<std::string, mse::TRefCountingFixedPointer<A>> strong_string_ptr, CRCFPVector& rcfpvector_ref) {
				rcfpvector_ref.clear();
				std::string retval = (*strong_string_ptr);
				return retval;
			}
		protected:
			~B() {}
		};

		{
			CRCFPVector rcfpvector;
			{
				mse::TRefCountingFixedPointer<A> A_refcountingfixed_ptr1 = mse::make_refcounting<A>();
				rcfpvector.push_back(A_refcountingfixed_ptr1);

				/* Just to demonstrate conversion between refcounting pointer types. */
				mse::TRefCountingConstPointer<A> A_refcountingconst_ptr1 = A_refcountingfixed_ptr1;
			}
			B::foo1(rcfpvector.front(), rcfpvector);
		}
		{
			/* Obtaining a safe pointer to a member of an object owned by a reference counting pointer: */
			CRCFPVector rcfpvector;
			{
				mse::TRefCountingFixedPointer<A> A_refcountingfixed_ptr1 = mse::make_refcounting<A>();
				rcfpvector.push_back(A_refcountingfixed_ptr1);
			}

			/* strong_string_ptr1 here is essentially a pointer to "A.s" (the string member of class A) with
			a refcounting pointer to A welded to it to make sure that the object is not deallocated while
			strong_string_ptr1 is still around. */
			auto strong_string_ptr1 = mse::make_strong(rcfpvector.front()->s, rcfpvector.front());
			B::foo2(strong_string_ptr1, rcfpvector);

			/* In practice, rather than declaring a specific mse::TStrongFixedPointer parameter, we expect
			functions to be "templatized" so that they can accept any type of pointer. */
			std::string res1 = H::foo6(strong_string_ptr1, strong_string_ptr1);
		}
		{
			/* Just testing the convertibility of mse::TStrongFixedPointers. */
			auto A_refcfp = mse::make_refcounting<A>();
			auto sfptr1 = mse::make_strong<std::string>(A_refcfp->s, A_refcfp);
			mse::TStrongFixedPointer<std::string, mse::TRefCountingPointer<A>> sfptr2 = sfptr1;
			mse::TStrongFixedConstPointer<std::string, mse::TRefCountingFixedPointer<A>> sfcptr1 = sfptr1;
			mse::TStrongFixedConstPointer<std::string, mse::TRefCountingPointer<A>> sfcptr2 = sfcptr1;
			if (sfcptr1 == sfptr1) {
				int q = 7;
			}
			if (sfptr1 == sfcptr1) {
				int q = 7;
			}
			if (sfptr1) {
				int q = 7;
			}
		}

		mse::TRefCountingPointer_test TRefCountingPointer_test1;
		bool TRefCountingPointer_test1_res = TRefCountingPointer_test1.testBehaviour();
		TRefCountingPointer_test1_res &= TRefCountingPointer_test1.testLinked();
		TRefCountingPointer_test1.test1();
	}

	{
		/*************************************/
		/*  TRefCountingOfRegisteredPointer  */
		/*************************************/

		/* TRefCountingOfRegisteredPointer is simply an alias for TRefCountingPointer<TRegisteredObj<_Ty>>. TRegisteredObj<_Ty> is
		meant to behave much like, and be compatible with a _Ty. The reason why we might want to use it is because the &
		("address of") operator of TRegisteredObj<_Ty> returns a TRegisteredFixedPointer<_Ty> rather than a raw pointer, and
		TRegisteredPointers can serve as safe "weak pointers".
		*/

		class A {
		public:
			A() {}
			virtual ~A() {
				int q = 3; /* just so you can place a breakpoint if you want */
			}

			int b = 3;
		};
		typedef std::vector<mse::TRefCountingOfRegisteredFixedPointer<A>> CRCRFPVector;

		{
			CRCRFPVector rcrfpvector;
			{
				mse::TRefCountingOfRegisteredFixedPointer<A> A_refcountingofregisteredfixed_ptr1 = mse::make_refcountingofregistered<A>();
				rcrfpvector.push_back(A_refcountingofregisteredfixed_ptr1);

				/* Just to demonstrate conversion between refcountingofregistered pointer types. */
				mse::TRefCountingOfRegisteredConstPointer<A> A_refcountingofregisteredconst_ptr1 = A_refcountingofregisteredfixed_ptr1;
			}
			int res1 = H::foo5(rcrfpvector.front(), rcrfpvector);
			assert(3 == res1);

#if !defined(MSE_REGISTEREDPOINTER_DISABLED)

			rcrfpvector.push_back(mse::make_refcountingofregistered<A>());
			/* The first parameter in this case will be a TRegisteredFixedPointer<A>. */
			int res2 = H::foo5(&(*rcrfpvector.front()), rcrfpvector);
			assert(-1 == res2);

#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
		}

		mse::TRefCountingOfRegisteredPointer_test TRefCountingOfRegisteredPointer_test1;
		bool TRefCountingOfRegisteredPointer_test1_res = TRefCountingOfRegisteredPointer_test1.testBehaviour();
		TRefCountingOfRegisteredPointer_test1_res &= TRefCountingOfRegisteredPointer_test1.testLinked();
		TRefCountingOfRegisteredPointer_test1.test1();
	}

#ifndef MSE_PRIMITIVES_DISABLED
	{
		/********************************************/
		/*  TRefCountingOfRelaxedRegisteredPointer  */
		/********************************************/

		/* TRefCountingOfRelaxedRegisteredPointer is simply an alias for TRefCountingPointer<TRelaxedRegisteredObj<_Ty>>. TRelaxedRegisteredObj<_Ty> is
		meant to behave much like, and be compatible with a _Ty. The reason why we might want to use it is because the &
		("address of") operator of TRelaxedRegisteredObj<_Ty> returns a TRelaxedRegisteredFixedPointer<_Ty> rather than a raw pointer, and
		TRelaxedRegisteredPointers can serve as safe "weak pointers".
		*/

		/* Here we demonstrate using TRelaxedRegisteredFixedPointer<> as a safe "weak_ptr" to prevent "cyclic references" from
		becoming memory leaks. */
		class CRCNode {
		public:
			CRCNode(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr
				, mse::TRelaxedRegisteredPointer<CRCNode> root_ptr) : m_node_count_ptr(node_count_ptr), m_root_ptr(root_ptr) {
				(*node_count_ptr) += 1;
			}
			CRCNode(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr) : m_node_count_ptr(node_count_ptr) {
				(*node_count_ptr) += 1;
			}
			virtual ~CRCNode() {
				(*m_node_count_ptr) -= 1;
			}
			static mse::TRefCountingOfRelaxedRegisteredFixedPointer<CRCNode> MakeRoot(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr) {
				auto retval = mse::make_refcountingofrelaxedregistered<CRCNode>(node_count_ptr);
				(*retval).m_root_ptr = &(*retval);
				return retval;
			}
			mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> ChildPtr() const { return m_child_ptr; }
			mse::TRefCountingOfRelaxedRegisteredFixedPointer<CRCNode> MakeChild() {
				auto retval = mse::make_refcountingofrelaxedregistered<CRCNode>(m_node_count_ptr, m_root_ptr);
				m_child_ptr = retval;
				return retval;
			}
			void DisposeOfChild() {
				m_child_ptr = nullptr;
			}

		private:
			mse::TRegisteredFixedPointer<mse::CInt> m_node_count_ptr;
			mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> m_child_ptr;
			mse::TRelaxedRegisteredPointer<CRCNode> m_root_ptr;
		};

		mse::TRegisteredObj<mse::CInt> node_counter = 0;
		{
			mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> root_ptr = CRCNode::MakeRoot(&node_counter);
			auto kid1 = root_ptr->MakeChild();
			{
				auto kid2 = kid1->MakeChild();
				auto kid3 = kid2->MakeChild();
			}
			assert(4 == node_counter);
			kid1->DisposeOfChild();
			assert(2 == node_counter);
		}
		assert(0 == node_counter);

		mse::TRefCountingOfRelaxedRegisteredPointer_test TRefCountingOfRelaxedRegisteredPointer_test1;
		bool TRefCountingOfRelaxedRegisteredPointer_test1_res = TRefCountingOfRelaxedRegisteredPointer_test1.testBehaviour();
		TRefCountingOfRelaxedRegisteredPointer_test1_res &= TRefCountingOfRelaxedRegisteredPointer_test1.testLinked();
		TRefCountingOfRelaxedRegisteredPointer_test1.test1();
	}
#endif // !MSE_PRIMITIVES_DISABLED

	{
		/*************************/
		/*  TXScopeFixedPointer  */
		/*************************/

		/* The "xscope" templates basically just allow the programmer to indicate that the target object has "scope
		lifetime". That is, the object is either allocated on the stack, or it's "owner" pointer is allocated on
		the stack. Unfortunately there's really no way to enforce this, which makes this data type less intrinsically
		safe than say, "reference counting" pointers. Because of this, "xscope" pointers can optionally use relaxed
		registered pointers as their base class, thereby inheriting their safety features. */

		class A {
		public:
			A(int x) : b(x) {}
			virtual ~A() {}

			int b = 3;
			std::string s = "some text ";
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TXScopeFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
			static int foo3(mse::TXScopeFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
		protected:
			~B() {}
		};

		mse::TXScopeObj<A> a_scpobj(5);
		int res1 = (&a_scpobj)->b;
		int res2 = B::foo2(&a_scpobj);
		int res3 = B::foo3(&a_scpobj);
		mse::TXScopeOwnerPointer<A> a_scpoptr(7);
		int res4 = B::foo2(&(*a_scpoptr));

		auto xscopeweak_string_ptr1 = mse::make_xscopeweak((a_scpobj.s), (&a_scpobj));
		auto res5 = H::foo6(xscopeweak_string_ptr1, xscopeweak_string_ptr1);

		/* Just testing the convertibility of mse::TXScopeWeakFixedPointers. */
		auto A_xscope_fixed_ptr1 = &a_scpobj;
		auto xscpwfptr1 = mse::make_xscopeweak<std::string>(A_xscope_fixed_ptr1->s, A_xscope_fixed_ptr1);
		mse::TXScopeWeakFixedPointer<std::string, mse::TXScopeFixedConstPointer<A>> xscpwfptr2 = xscpwfptr1;
		mse::TXScopeWeakFixedConstPointer<std::string, mse::TXScopeFixedPointer<A>> xscpwfcptr1 = xscpwfptr1;
		mse::TXScopeWeakFixedConstPointer<std::string, mse::TXScopeFixedConstPointer<A>> xscpwfcptr2 = xscpwfcptr1;
		if (xscpwfcptr1 == xscpwfptr1) {
			int q = 7;
		}
		if (xscpwfptr1 == xscpwfcptr1) {
			int q = 7;
		}
		if (xscpwfptr1) {
			int q = 7;
		}

		mse::s_scpptr_test1();
	}

	{
		/******************/
		/*  TAsyncShared  */
		/******************/

		/* The TAsyncShared data types are used to safely share objects between asynchronous threads. */

		class A {
		public:
			A(int x) : b(x) {}
			virtual ~A() {}

			int b = 3;
			std::string s = "some text ";
		};
		class B {
		public:
			static double foo1(mse::TAsyncSharedReadWriteAccessRequester<A> A_ashar) {
				auto t1 = std::chrono::high_resolution_clock::now();
				/* mse::TAsyncSharedReadWriteAccessRequester<A>::writelock_ptr() will block until it can obtain a write lock. */
				auto ptr1 = A_ashar.writelock_ptr(); // while ptr1 exists it holds a (write) lock on the shared object
				auto t2 = std::chrono::high_resolution_clock::now();
				std::this_thread::sleep_for(std::chrono::seconds(1));
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				auto timespan_in_seconds = time_span.count();
				auto thread_id = std::this_thread::get_id();
				//std::cout << "thread_id: " << thread_id << ", time to acquire write pointer: " << timespan_in_seconds << " seconds.";
				//std::cout << std::endl;

				ptr1->s = std::to_string(timespan_in_seconds);
				return timespan_in_seconds;
			}
			static int foo2(std::shared_ptr<const A> A_shptr) {
				return A_shptr->b;
			}
		protected:
			~B() {}
		};

		std::cout << std::endl;
		std::cout << "AsyncShared test output:";
		std::cout << std::endl;

		{
			/* This block contains a simple example demonstrating the use of mse::TAsyncSharedReadWriteAccessRequester
			to safely share an object between threads. */

			std::cout << "TAsyncSharedReadWrite:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedreadwrite<A>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;

			{
				auto ptr1 = ash_access_requester.writelock_ptr();
				auto ptr2 = ash_access_requester.writelock_ptr();
			}

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(B::foo1, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire write pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;

			/* Btw, mse::TAsyncSharedReadOnlyAccessRequester<>s can be copy constructed from 
			mse::TAsyncSharedReadWriteAccessRequester<>s */
			mse::TAsyncSharedReadOnlyAccessRequester<A> ash_read_only_access_requester(ash_access_requester);
		}
		{
			std::cout << "TAsyncSharedReadOnly:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedreadonly<A>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(H::foo7<mse::TAsyncSharedReadOnlyAccessRequester<A>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWrite:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadwrite<A>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(H::foo7<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<A>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnly:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadonly<A>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(H::foo7<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<A>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			/* Just demonstrating the existence of the "try" versions. */
			auto access_requester = mse::make_asyncsharedreadwrite<std::string>("some text");
			auto writelock_ptr1 = access_requester.try_writelock_ptr();
			if (writelock_ptr1) {
				// lock request succeeded
				int q = 5;
			}
			auto readlock_ptr2 = access_requester.try_readlock_ptr_for(std::chrono::seconds(10));
			auto writelock_ptr3 = access_requester.try_writelock_ptr_until(std::chrono::steady_clock::now() + std::chrono::seconds(10));
		}
		{
			/* For simple "read-only" scenarios where you need, or want, the shared object to be managed by std::shared_ptrs,
			TStdSharedImmutableFixedPointer is a "safety enhanced" wrapper for std::shared_ptr. And again, beware of
			sharing objects with mutable members. */
			auto read_only_sh_ptr = mse::make_stdsharedimmutable<A>(5);
			int res1 = read_only_sh_ptr->b;

			std::list<std::future<int>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(B::foo2, read_only_sh_ptr));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				int res2 = (*it).get();
			}
		}
	}

	{
		/*******************/
		/*  Poly pointers  */
		/*******************/

		class A {
		public:
			A(int x) : b(x) {}
			virtual ~A() {}

			int b = 3;
		};
		class D : public A {
		public:
			D(int x) : A(x) {}
		};
		class B {
		public:
			static int foo1(mse::TRefCountingOrXScopeFixedPointer<A> ptr) {
				int retval = ptr->b;
				return retval;
			}
			static int foo2(mse::TRefCountingOrXScopeFixedConstPointer<A> ptr) {
				int retval = ptr->b;
				return retval;
			}
			static int foo3(mse::TRefCountingOrXScopeOrRawFixedPointer<A> ptr) {
				int retval = ptr->b;
				return retval;
			}
			static int foo4(mse::TRefCountingOrXScopeOrRawFixedConstPointer<A> ptr) {
				int retval = ptr->b;
				return retval;
			}
			static int foo5(mse::TSharedOrRawFixedPointer<A> ptr) {
				int retval = ptr->b;
				return retval;
			}
			static int foo6(mse::TSharedOrRawFixedConstPointer<A> ptr) {
				int retval = ptr->b;
				return retval;
			}
		protected:
			~B() {}
		};

		auto A_refcfp = mse::make_refcounting<A>(5);
		mse::TXScopeObj<A> a_xscpobj(7);
		A a_obj(11);
		int res1 = B::foo1(A_refcfp);
		int res2 = B::foo1(&a_xscpobj);
		int res3 = B::foo2(A_refcfp);
		int res4 = B::foo2(&a_xscpobj);

		auto D_refcfp = mse::make_refcounting<D>(5);
		mse::TXScopeObj<D> d_xscpobj(7);
		D d_obj(11);
		int res11 = B::foo1(D_refcfp);
		int res12 = B::foo1(&d_xscpobj);
		int res13 = B::foo2(D_refcfp);
		int res14 = B::foo2(&d_xscpobj);

		int res21 = B::foo3(A_refcfp);
		int res22 = B::foo3(&a_xscpobj);
		int res23 = B::foo3(&a_obj);
		int res24 = B::foo4(A_refcfp);
		int res25 = B::foo4(&a_xscpobj);
		int res26 = B::foo4(&a_obj);

		int res31 = B::foo3(D_refcfp);
		int res32 = B::foo3(&d_xscpobj);
		int res33 = B::foo3(&d_obj);
		int res34 = B::foo4(D_refcfp);
		int res35 = B::foo4(&d_xscpobj);
		int res36 = B::foo4(&d_obj);

		auto A_shp = std::make_shared<A>(5);
		int res41 = B::foo5(A_shp);
		int res42 = B::foo5(&a_obj);
		int res43 = B::foo6(A_shp);
		int res44 = B::foo6(&a_obj);

		int q = 3;
	}

	return 0;
}

