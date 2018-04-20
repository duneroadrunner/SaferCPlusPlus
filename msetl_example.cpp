
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*
This example file has become quite large (and has spilled into msetl_example2.cpp) and holds examples for many data
types. Your best bet is probably to use a find/search to get to the data type your interested in.
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
#include "mseregistered.h"
#include "mserelaxedregistered.h"
#include "mserefcounting.h"
#include "mserefcountingofregistered.h"
#include "mserefcountingofrelaxedregistered.h"
#include "msescope.h"
#include "mseasyncshared.h"
#include "msepoly.h"
#include "msemsearray.h"
#include "msemstdarray.h"
#include "msemsevector.h"
#include "msemstdvector.h"
#include "mseivector.h"
#include "msevector_test.h"
#include "mseprimitives.h"
#include "mselegacyhelpers.h"
#include "msemstdstring.h"
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

#include "msetl_example2.h"


#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4702 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
//pragma clang diagnostic ignored "-Wunused-but-set-variable"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

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

	/* This function will be used to demonstrate using us::value_from_fparam() to enable template functions to accept scope 
	pointers to temporary objects. */
	template<class _TPointer1, class _TPointer2>
	static bool second_is_longer(const _TPointer1& string1_xscpptr, const _TPointer2& string2_xscpptr) {
		auto l_string1_xscpptr = mse::us::value_from_fparam(string1_xscpptr);
		auto l_string2_xscpptr = mse::us::value_from_fparam(string2_xscpptr);
		return (l_string1_xscpptr->length() > l_string2_xscpptr->length()) ? false : true;
	}

	/* A member function that provides a safe pointer/reference to a class/struct member is going to need to
	take a safe version of the "this" pointer as a parameter. */
	template<class this_type>
	static auto safe_pointer_to_member_string1(this_type safe_this) {
		return mse::make_pointer_to_member_v2(safe_this, &H::m_string1);
	}

	mse::nii_string m_string1 = "initial text";
};
/* User-defined classes need to be declared as (safely) shareable in order to be accepted by the access requesters. */
typedef mse::us::TUserDeclaredAsyncShareableObj<H> ShareableH;

int main(int argc, char* argv[])
{
	mse::msevector_test msevector_test;
	msevector_test.run_all();

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
#ifndef MSE_MSTDVECTOR_DISABLED
		try {
			v4.insert(v4.begin(), v2.begin(), v3.begin());
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by a comparision of incompatible "safe" iterators. */
		}
#endif // !MSE_MSTDVECTOR_DISABLED

		/* And of course the iterators can be used with the standard algorithms, just like those of std::vector. */
		std::sort(v3.begin(), v3.end());
	}

	{
		/* Here's how mse::mstd::vector<>::iterator handles occurrences of "use-after-free".  */

		typedef mse::mstd::vector<int> vint_type;
		mse::mstd::vector<vint_type> vvi;
		{
			vint_type vi;
			vi.push_back(5);
			vvi.push_back(vi);
		}
		auto vi_it = vvi[0].begin();
		vvi.clear();

#ifndef MSE_MSTDVECTOR_DISABLED
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
			/* At present, no exception will be thrown. In the future, an exception may be thrown in debug builds. */
			std::cerr << "potentially expected exception" << std::endl;
		}
#endif // !MSE_MSTDVECTOR_DISABLED
	}

	{
		/* If the vector is declared as a "scope" object (which basically indicates that it is declared
		on the stack), then you can use "scope" iterators. While there are limitations on when they can
		be used, scope iterators would be the preferred iterator type where performance is a priority
		as they don't require extra run-time overhead to ensure that the vector has not been prematurely
		deallocated. */

		/* Here we're declaring an vector as a scope object. */
		mse::TXScopeObj<mse::mstd::vector<int>> vector1_scpobj = mse::mstd::vector<int>{ 1, 2, 3 };

		/* Here we're obtaining a scope iterator to the vector. */
		auto scp_iter1 = mse::mstd::make_xscope_iterator(&vector1_scpobj);
		scp_iter1 = vector1_scpobj.begin();
		auto scp_iter2 = mse::mstd::make_xscope_iterator(&vector1_scpobj);
		scp_iter2 = vector1_scpobj.end();

		std::sort(scp_iter1, scp_iter2);

		auto scp_citer3 = mse::mstd::make_xscope_const_iterator(&vector1_scpobj);
		scp_citer3 = scp_iter1;
		scp_citer3 = vector1_scpobj.cbegin();
		scp_citer3 += 2;
		auto res1 = *scp_citer3;
		auto res2 = scp_citer3[0];

		/* Here we demonstrate the case where the vector is a member of a class/struct declared as a scope object. */
		class CContainer1 {
		public:
			CContainer1() : m_vector({ 1, 2, 3 }) {}
			mse::mstd::vector<int> m_vector;
		};
		mse::TXScopeObj<CContainer1> container1_scpobj;
		auto container1_m_vector_scpptr = mse::make_xscope_pointer_to_member_v2(&container1_scpobj, &CContainer1::m_vector);
		auto scp_citer4 = mse::mstd::make_xscope_iterator(container1_m_vector_scpptr);
		scp_citer4 = (*container1_m_vector_scpptr).begin();
		scp_citer4++;
		auto res3 = *scp_citer4;

		{
			/* In order to obtain a direct scope pointer to a vector element, you first need to instantiate a "structure lock"
			object, which "locks" the vector to ensure that no resize (or reserve) operation that might cause a scope pointer
			to become invalid is performed. */
			auto xscp_vector1_change_lock_guard = mse::mstd::make_xscope_vector_size_change_lock_guard(&vector1_scpobj);
			auto scp_ptr1 = xscp_vector1_change_lock_guard.xscope_ptr_to_element(2);
			auto res4 = *scp_ptr1;
		}
		vector1_scpobj.push_back(4);
	}

	{
		/*********************************/
		/*   us::msevector<>::ipointer   */
		/*********************************/

		/* mse::us::msevector<> is another vector that is highly compatible with std::vector<>. But mse::us::msevector<> also
		supports a new type of iterator called "ipointer". ipointers make more (intuitive) sense than standard vector
		iterators. Upon insert or delete, ipointers continue to point to the same item, not (necessarily) the same
		position. And they don't become "invalid" upon insert or delete, unless the item they point to is deleted. They
		support all the standard iterator operators, but also have member functions with "friendlier" names. */

#ifdef MSVC2010_COMPATIBLE
		int a1[4] = { 1, 2, 3, 4 };
		mse::us::msevector<int> v1(a1, a1+4);
#else /*MSVC2010_COMPATIBLE*/
		mse::us::msevector<int> v1 = { 1, 2, 3, 4 };
#endif /*MSVC2010_COMPATIBLE*/

		mse::us::msevector<int> v = v1;

		{
			mse::us::msevector<int>::ipointer ip1 = v.ibegin();
			ip1 += 2;
			assert(3 == (*ip1));
			auto ip2 = v.ibegin(); /* ibegin() returns an ipointer */
			v.erase(ip2); /* remove the first item */
			assert(3 == (*ip1)); /* ip1 continues to point to the same item, not the same position */
			ip1--;
			assert(2 == (*ip1));
			for (mse::us::msevector<int>::cipointer cip = v.cibegin(); v.ciend() != cip; cip++) {
				/* You might imagine what would happen if cip were a regular vector iterator. */
				v.insert(v.ibegin(), (*cip));
			}
		}
		v = v1;
		{
			/* This code block is equivalent to the previous code block, but uses ipointer's more "readable" interface
			that might make the code a little more clear to those less familiar with C++ syntax. */
			mse::us::msevector<int>::ipointer ip_vit1 = v.ibegin();
			ip_vit1.advance(2);
			assert(3 == ip_vit1.item());
			auto ip_vit2 = v.ibegin();
			v.erase(ip_vit2);
			assert(3 == ip_vit1.item());
			ip_vit1.set_to_previous();
			assert(2 == ip_vit1.item());
			mse::us::msevector<int>::cipointer cip(v);
			for (cip.set_to_beginning(); cip.points_to_an_item(); cip.set_to_next()) {
				v.insert_before(v.ibegin(), (*cip));
			}
		}

		/* Btw, ipointers are compatible with stl algorithms, like any other stl iterators. */
		std::sort(v.ibegin(), v.iend());

		/* And just to be clear, mse::us::msevector<> retains its original (high performance) stl::vector iterators. */
		std::sort(v.begin(), v.end());

		/* mse::us::msevector<> also provides "safe" (bounds checked) versions of the original stl::vector iterators. */
		std::sort(v.ss_begin(), v.ss_end());

		/* mse::ivector<> is another vector for when safety and "correctness" are more of a priority than performance
		or compatibility. It is completely safe like mse::mstd::vector<> but only supports the "ipointer" iterators.
		It does not support the (problematic) standard vector iterator behavior. */
		mse::ivector<int> iv = { 1, 2, 3, 4 };
		std::sort(iv.begin(), iv.end());
		mse::ivector<int>::ipointer ivip = iv.begin();

		{
			/* A "scope" version of the safe iterators can be used when the vector is declared as a scope
			object. There are limitations on when they can be used, but unlike the other msevector iterators,
			those restrictions ensure that they won't be used to access the vector after it's been deallocated. */

			mse::TXScopeObj<mse::us::msevector<int>> vector1_scpobj = mse::us::msevector<int>{ 1, 2, 3 };

			auto scp_ss_iter1 = mse::make_xscope_iterator(&vector1_scpobj);
			scp_ss_iter1.set_to_beginning();
			auto scp_ss_iter2 = mse::make_xscope_iterator(&vector1_scpobj);
			scp_ss_iter2.set_to_end_marker();

			std::sort(scp_ss_iter1, scp_ss_iter2);

			auto scp_ss_citer3 = mse::make_xscope_const_iterator(&vector1_scpobj);
			scp_ss_citer3 = scp_ss_iter1;
			scp_ss_citer3 = vector1_scpobj.ss_cbegin();
			scp_ss_citer3 += 2;
			auto res1 = *scp_ss_citer3;
			auto res2 = scp_ss_citer3[0];

			/* Here we demonstrate the case where the vector is a member of a class/struct declared as a
			scope object. */
			class CContainer1 {
			public:
				CContainer1() : m_vector({ 1, 2, 3 }) {}
				mse::us::msevector<int> m_vector;
			};
			mse::TXScopeObj<CContainer1> container1_scpobj;
			auto container1_m_vector_scpptr = mse::make_xscope_pointer_to_member_v2(&container1_scpobj, &CContainer1::m_vector);
			auto scp_ss_citer4 = mse::make_xscope_iterator(container1_m_vector_scpptr);
			scp_ss_citer4++;
			auto res3 = *scp_ss_citer4;

			{
				/* In order to obtain a direct scope pointer to a vector element, you first need to instantiate a "structure lock"
				object, which "locks" the vector to ensure that no resize (or reserve) operation that might cause a scope pointer
				to become invalid is performed. */
				auto xscp_vector1_change_lock_guard = mse::make_xscope_vector_size_change_lock_guard(&vector1_scpobj);
				auto scp_ptr1 = xscp_vector1_change_lock_guard.xscope_ptr_to_element(2);
				auto res4 = *scp_ptr1;
			}
			vector1_scpobj.push_back(4);
		}
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

#ifndef MSE_MSTDARRAY_DISABLED
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
#endif // !MSE_MSTDARRAY_DISABLED

		/* And of course the iterators can be used with the standard algorithms, just like those of std::array. */
		std::sort(a2.begin(), a2.end());

		{
			/* If the array is declared as a "scope" object (which basically indicates that it is declared
			on the stack), then you can use "scope" iterators. While there are limitations on when they can
			be used, scope iterators would be the preferred iterator type where performance is a priority
			as they don't require extra run-time overhead to ensure that the array has not been prematurely
			deallocated. */

			/* Here we're declaring an array as a scope object. */
			mse::TXScopeObj<mse::mstd::array<int, 3>> array1_scpobj = mse::mstd::array<int, 3>{ 1, 2, 3 };

			/* Here we're obtaining a scope iterator to the array. */
			auto scp_array_iter1 = mse::mstd::make_xscope_begin_iterator(&array1_scpobj);
			auto scp_array_iter2 = mse::mstd::make_xscope_end_iterator(&array1_scpobj);

			/*
			auto scp_array_iter1 = mse::mstd::make_xscope_iterator(&array1_scpobj);
			scp_array_iter1 = array1_scpobj.begin();
			auto scp_array_iter2 = mse::mstd::make_xscope_iterator(&array1_scpobj);
			scp_array_iter2 = array1_scpobj.end();
			*/

			std::sort(scp_array_iter1, scp_array_iter2);

			auto scp_array_citer3 = mse::mstd::make_xscope_begin_const_iterator(&array1_scpobj);
			scp_array_citer3 = scp_array_iter1;
			scp_array_citer3 = mse::mstd::make_xscope_begin_const_iterator(&array1_scpobj);
			//scp_array_citer3 = array1_scpobj.cbegin();
			scp_array_citer3 += 2;
			auto res1 = *scp_array_citer3;
			auto res2 = scp_array_citer3[0];

			/* Here we demonstrate the case where the array is a member of a class/struct declared as a
			scope object. */
			class CContainer1 {
			public:
				CContainer1() : m_array{ 1, 2, 3 } {}
				mse::mstd::array<int, 3> m_array/* = { 1, 2, 3 }*/;
			};
			mse::TXScopeObj<CContainer1> container1_scpobj;
			auto container1_m_array_scpptr = mse::make_xscope_pointer_to_member_v2(&container1_scpobj, &CContainer1::m_array);
			auto scp_iter4 = mse::mstd::make_xscope_begin_iterator(container1_m_array_scpptr);
			//scp_iter4 = (*container1_m_array_scpptr).begin();
			scp_iter4++;
			auto res3 = *scp_iter4;

			/* You can also obtain a corresponding scope pointer from a scope iterator. */
			auto scp_ptr1 = mse::mstd::xscope_pointer_to_array_element<int, 3>(scp_iter4);
			auto res4 = *scp_ptr1;
			/* Or with a scope pointer to the array and an index. */
			auto scp_cptr2 = mse::mstd::xscope_const_pointer_to_array_element<int, 3>(container1_m_array_scpptr, 2/*element index*/);
			auto res5 = *scp_cptr2;
		}

		mse::mstd::array_test testobj1;
		testobj1.test1();
	}

	{
		/**********************/
		/*   us::msearray<>   */
		/**********************/

		/* mse::us::msearray<> is another array implementation that's not quite as safe as mse::mstd::array<> in the sense
		that its iterators are not "lifespan aware" (i.e. could be used to access an array after it's been deallocated).
		And it provides both "safe" (bounds-checked) and unsafe iterators. Basically, mse::us::msearray<> is a compromise
		between performance and safety. */

		mse::us::msearray<int, 3> a1 = { 1, 2, 3 };
		mse::us::msearray<int, 3> a2 = { 11, 12, 13 };

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

		std::sort(a2.ss_begin(), a2.ss_end());

		{
			/* A "scope" version of the safe iterators can be used when the array is declared as a scope
			object. There are limitations on when they can be used, but unlike the other msearray iterators,
			those restrictions ensure that they won't be used to access the array after it's been deallocated. */

			mse::TXScopeObj<mse::us::msearray<int, 3>> array1_scpobj = mse::us::msearray<int, 3>{ 1, 2, 3 };

			auto scp_ss_iter1 = mse::make_xscope_iterator(&array1_scpobj);
			scp_ss_iter1.set_to_beginning();
			auto scp_ss_iter2 = mse::make_xscope_iterator(&array1_scpobj);
			scp_ss_iter2.set_to_end_marker();

			std::sort(scp_ss_iter1, scp_ss_iter2);

			auto scp_ss_citer3 = mse::make_xscope_const_iterator(&array1_scpobj);
			scp_ss_citer3 = scp_ss_iter1;
			scp_ss_citer3 = array1_scpobj.ss_cbegin();
			scp_ss_citer3 += 2;
			auto res1 = *scp_ss_citer3;
			auto res2 = scp_ss_citer3[0];

			/* Here we demonstrate the case where the array is a member of a class/struct declared as a
			scope object. */
			class CContainer1 {
			public:
				CContainer1() : m_array({ 1, 2, 3 }) {}
				mse::us::msearray<int, 3> m_array;
			};
			mse::TXScopeObj<CContainer1> container1_scpobj;
			auto container1_m_array_scpptr = mse::make_xscope_pointer_to_member_v2(&container1_scpobj, &CContainer1::m_array);
			auto scp_ss_citer4 = mse::make_xscope_iterator(container1_m_array_scpptr);
			scp_ss_citer4++;
			auto res3 = *scp_ss_citer4;

			/* You can also obtain a corresponding scope pointer from a scope iterator. */
			auto scp_ptr1 = mse::xscope_pointer_to_array_element<int, 3>(scp_ss_citer4);
			auto res4 = *scp_ptr1;
			/* Or with a scope pointer to the array and an index. */
			auto scp_ptr2 = mse::xscope_pointer_to_array_element<int, 3>(container1_m_array_scpptr, 2/*element index*/);
			auto res5 = *scp_ptr2;
		}

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

		mse::CPrimitivesTest1::s_test1();

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
			B::foo1(static_cast<A*>(A_registered_ptr1));

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

			/* To obtain a safe pointer to a member of a registered object you could just make the
			member itself a registered object. */
			mse::TRegisteredPointer<std::string> reg_s_registered_ptr1 = &(E_registered_ptr1->reg_s);

			/* Or you can use the "mse::make_pointer_to_member_v2()" function. */
			auto s2_safe_ptr1 = mse::make_pointer_to_member_v2(E_registered_ptr1, &E::s2);
			(*s2_safe_ptr1) = "some new text";
			auto s2_safe_const_ptr1 = mse::make_const_pointer_to_member_v2(E_registered_ptr1, &E::s2);

			/* The return type of mse::make_pointer_to_member_v2() depends on the type of the parameters passed
			to it. In this case, the type of s2_safe_ptr1 is mse::TSyncWeakFixedPointer<std::string, 
			mse::TRegisteredPointer<E>>. s2_safe_ptr1 here is essentially a pointer to "E.s2"
			(string member of class E) with a registered pointer to E to in its pocket. It uses the registered
			pointer to ensure that it is safe to access the object. */

			/* In practice, rather than declaring a specific mse::TSyncWeakFixedPointer parameter, we expect
			functions intended for general use to be "templatized" so that they can accept any type of pointer. */
			std::string res1 = H::foo6(s2_safe_ptr1, s2_safe_const_ptr1);
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

		mse::CRegPtrTest1::s_test1();
		mse::CRelaxedRegPtrTest1::s_test1();

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
				mse::TRegisteredObj<CE> place_holder1(count);
				mse::TRegisteredPointer<CE> item_ptr2 = &place_holder1;
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
			for (auto i : v) { std::cout << static_cast<mse::CInt&>(i) << ' '; } std::cout << '\n';

			std::cout << "Doubling the values in the initial list...\n";
			for (auto& i : l) {
				i *= 2;
			}

			std::cout << "Contents of the list, as seen through a shuffled vector: ";
			for (auto i : v) { std::cout << static_cast<mse::CInt&>(i) << ' '; } std::cout << '\n';
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
			++(static_cast<mse::CInt&>(bar));
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

			/* You can use the "mse::make_pointer_to_member_v2()" function to obtain a safe pointer to a member of
			an object owned by a refcounting pointer. */
			auto s_safe_ptr1 = mse::make_pointer_to_member_v2(rcfpvector.front(), &A::s);
			(*s_safe_ptr1) = "some new text";
			auto s_safe_const_ptr1 = mse::make_const_pointer_to_member_v2(rcfpvector.front(), &A::s);

			/* The return type of mse::make_pointer_to_member_v2() depends on the type of the parameters passed
			to it. In this case, the type of s_safe_ptr1 is mse::TStrongFixedPointer<std::string,
			mse::TRefCountingFixedPointer<A>>. s_safe_ptr1 here is essentially a pointer to 
			rcfpvector.front()->s with a copy of rcfpvector.front() welded to it to make sure that the
			object is not deallocated while s_safe_ptr1 is still around. */

			B::foo2(s_safe_ptr1, rcfpvector);

			/* In practice, rather than declaring a specific mse::TStrongFixedPointer parameter, we expect
			functions intended for general use to be "templatized" so that they can accept any type of pointer. */
			std::string res1 = H::foo6(s_safe_ptr1, s_safe_const_ptr1);

			/* Obtaining a scope pointer to a member of an object owned by a reference counting pointer */
			auto strong_store1 = mse::make_xscope_strong_pointer_store(s_safe_ptr1);
			auto s_scpptr1 = strong_store1.xscope_ptr();
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
		/*****************************/
		/*  TXScopeItemFixedPointer  */
		/*****************************/

		/* The "xscope" templates basically allow the programmer to indicate that the target object has "scope
		lifetime". That is, the object is either allocated on the stack, or its "owner" pointer is allocated on
		the stack. Scope pointers may only point to scope objects. While there are limitations on when they can
		be used, scope pointers would be the preferred pointer type where performance is a priority as they don't
		require any run time overhead to ensure that they will not be used to access a target object has already
		been deallocated. */

		class A {
		public:
			A(int x) : b(x) {}
			virtual ~A() {}
			bool operator<(const A& _X) const { return (b < _X.b); }

			int b = 3;
			std::string s = "some text ";
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TXScopeItemFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
			static int foo3(mse::TXScopeItemFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
		protected:
			~B() {}
		};

		/* Here we're declaring a scope object. */
		mse::TXScopeObj<A> a_scpobj(5);
		int res1 = (&a_scpobj)->b;
		int res2 = B::foo2(&a_scpobj);
		int res3 = B::foo3(&a_scpobj);
		/* mse::TXScopeOwnerPointer<> will allocate a scope object on the heap (and deallocate it at the
		end of the scope). */
		mse::TXScopeOwnerPointer<A> xscp_a_ownerptr(7);
		int res4 = B::foo2(xscp_a_ownerptr);
		int res4b = B::foo2(&(*xscp_a_ownerptr));

		/* You can use the "mse::make_pointer_to_member_v2()" function to obtain a safe pointer to a member of
		an xscope object. */
		auto xscp_s_ptr1 = mse::make_pointer_to_member_v2((&a_scpobj), &A::s);
		(*xscp_s_ptr1) = "some new text";
		auto xscp_s_const_ptr1 = mse::make_const_pointer_to_member_v2((&a_scpobj), &A::s);

		/* The return type of mse::make_pointer_to_member_v2() depends on the type of the parameters passed
		to it. In this case, the type of xscp_s_ptr1 is mse::TXScopeItemFixedPointer<A>. */

		auto res5 = H::foo6(xscp_s_ptr1, xscp_s_const_ptr1);

		/* Using mse::make_xscope_strong_pointer_store(), you can obtain a scope pointer from a refcounting pointer. */
		/* Let's make it a const refcounting pointer, just for variety. */
		mse::TRefCountingFixedConstPointer<A> refc_cptr1 = mse::make_refcounting<A>(11);
		auto xscp_refc_cstore = mse::make_xscope_strong_pointer_store(refc_cptr1);
		auto xscp_cptr1 = xscp_refc_cstore.xscope_ptr();
		int res6 = B::foo3(xscp_cptr1);
		mse::TXScopeItemFixedConstPointer<A> xscp_cptr2 = xscp_cptr1;
		A res7 = *xscp_cptr2;

		/* Technically, you're not allowed to return a non-owning scope pointer from a function. (The return_value() function
		wrapper enforces this.) Pretty much the only time you'd legitimately want to do this is when the returned pointer
		is one of the input parameters. An example might be a "min(a, b)" function which takes two objects by reference and
		returns the reference to the lesser of the two objects. The library provides the xscope_chosen_pointer() function
		which takes a bool and two scope pointers, and returns one of the scope pointers depending on the value of the
		bool. You could use this function to implement the equivalent of a min(a, b) function like so: */
		auto xscp_a_ptr5 = &a_scpobj;
		auto xscp_a_ptr6 = &(*xscp_a_ownerptr);
		auto xscp_min_ptr1 = mse::xscope_chosen_pointer((*xscp_a_ptr6 < *xscp_a_ptr5), xscp_a_ptr5, xscp_a_ptr6);
		assert(5 == xscp_min_ptr1->b);

		{
			/* If you find that there are cases where mse::xscope_chosen_pointer() is insufficiently convenient, a (less
			preferred) alternative may be to use mse::us::TXScopeReturnableItemFixedPointerFParam<>, which is just a version
			of mse::TXScopeItemFixedPointer<> that may only be used to declare function parameters and whose value can be
			used as a function return value. (But note that the type may not be used as a return type. The type may only be
			used as a function parameter type.) */
			class CD {
			public:
				static auto longest(mse::us::TXScopeReturnableItemFixedPointerFParam<mse::nii_string> string1_xscpptr
					, mse::us::TXScopeReturnableItemFixedPointerFParam<mse::nii_string> string2_xscpptr) {
					if (string1_xscpptr->length() > string2_xscpptr->length()) {
						/* If string1_xscpptr were a regular TXScopeItemFixedPointer<mse::nii_string> the next line would have
						induced a compile error. */
						return mse::return_value(string1_xscpptr);
					}
					else {
						/* mse::return_value() usually returns its input argument unmolested, but in this case it will return
						a type different from the input type. This is to prevent any function that receives this return value
						from, in turn, returning the value, as that might be unsafe. */
						return mse::return_value(string2_xscpptr);
					}
				}
			};

			mse::TXScopeObj<mse::nii_string> xscope_string1 = "abc";
			mse::TXScopeObj<mse::nii_string> xscope_string2 = "abcd";
			auto longer_string_xscpptr = CD::longest(&xscope_string1, &xscope_string2);
			auto copy_of_longer_string = *longer_string_xscpptr;
		}

		{
			/********************************/
			/*  us::TFParam<>               */
			/*  && us::value_from_fparam()  */
			/********************************/

			/* us::TFParam<> is just a transparent template wrapper for function parameter declarations. In most cases
			use of this wrapper is not necessary, but in some cases it enables functionality only available to variables
			that are function parameters. Specifically, it allows functions to support scope pointer/references to
			temporary objects. For safety reasons, by default, scope pointer/references to temporaries are actually
			"functionally disabled" types distinct from regular scope pointer/reference types. Because it's safe to do so
			in the case of function parameters, the us::TFParam<> wrapper enables certain scope pointer/reference types
			(like TXScopeItemFixedPointer<>, and "random access section" scope types) to be constructed from their
			"functionally disabled" counterparts.

			In the case of function templates, sometimes you want the parameter types to be auto-deduced, and use of the
			mse::us::TFParam<> wrapper can interfere with that. In those cases you can instead convert parameters to their
			wrapped type after the fact using the us::value_from_fparam() function. Note that using this function (or the
			us::TFParam<> wrapper) on anything other than function parameters is unsafe, and currently there is no
			compile-time enforcement of this restriction.

			us::TXScopeFParam<> and us::xscope_value_from_fparam() can be used for situations when the types are necessarily
			scope types.
			*/

			class CD {
			public:
				static bool second_is_longer(mse::us::TXScopeFParam<mse::TXScopeItemFixedConstPointer<mse::nii_string> > string1_xscpptr
					, mse::us::TXScopeFParam<mse::TXScopeItemFixedConstPointer<mse::nii_string> > string2_xscpptr) {

					return (string1_xscpptr->length() > string2_xscpptr->length()) ? false : true;
				}

				static bool second_is_longer_any(mse::us::TXScopeFParam<mse::TXScopeAnyConstPointer<mse::nii_string> > string1_xscpptr
					, mse::us::TXScopeFParam<mse::TXScopeAnyConstPointer<mse::nii_string> > string2_xscpptr) {
					return (string1_xscpptr->length() > string2_xscpptr->length()) ? false : true;
				}

				static bool second_is_longer_poly(mse::us::TXScopeFParam<mse::TXScopePolyConstPointer<mse::nii_string> > string1_xscpptr
					, mse::us::TXScopeFParam<mse::TXScopePolyConstPointer<mse::nii_string> > string2_xscpptr) {
					return (string1_xscpptr->length() > string2_xscpptr->length()) ? false : true;
				}
			};

			mse::TXScopeObj<mse::nii_string> xscope_string1 = "abc";
			/* Here we're using the pointer_to() function to obtain a ("caged") pointer to the temporary scope object. The '&'
			(ampersand) operator would also work, but would not correspond to valid native C++, as C++ does not support taking
			the address of an r-value. */
			auto res1 = CD::second_is_longer(&xscope_string1, mse::pointer_to(mse::TXScopeObj<mse::nii_string>(xscope_string1 + "de")));
			auto res2 = H::second_is_longer(&xscope_string1, mse::pointer_to(mse::TXScopeObj<mse::nii_string>(xscope_string1 + "de")));
			auto res3 = CD::second_is_longer_any(&xscope_string1, mse::pointer_to(mse::TXScopeObj<mse::nii_string>(xscope_string1 + "de")));
			auto res4 = CD::second_is_longer_poly(&xscope_string1, mse::pointer_to(mse::TXScopeObj<mse::nii_string>(xscope_string1 + "de")));
		}

		{
			/***********************/
			/*  xscope_ifptr_to()  */
			/***********************/

			/* Scope pointers cannot (currently) be retargeted after construction. If you need a pointer that will point to
			multiple different scope objects over its lifespan, you can use a registered pointer. This means that the target
			objects will also need to be registered objects. If the object is a registered scope object, then the '&'
			operator will will return a registered pointer. But at some point we're going to need a scope pointer to the
			base scope object. A convenient way to get one is to use the xscope_ifptr_to() function. */

			typedef mse::TXScopeObj<mse::nii_string> xscp_nstring_t;
			typedef mse::TXScopeItemFixedPointer<mse::nii_string> xscp_nstring_ptr_t;
			class CB {
			public:
				static void foo1(xscp_nstring_ptr_t xscope_ptr1) {
					std::cout << *xscope_ptr1;
				}
			};
			typedef mse::TRegisteredObj< xscp_nstring_t > regxscp_nstring_t;
			typedef mse::TRegisteredPointer<xscp_nstring_t> regxscp_nstring_ptr_t;
			regxscp_nstring_t regxscp_nstring1("some text");
			regxscp_nstring_ptr_t registered_ptr1 = &regxscp_nstring1;

			auto xscope_ptr1 = mse::xscope_ifptr_to(*registered_ptr1);
			CB::foo1(xscope_ptr1);

			regxscp_nstring_t regxscp_nstring2("some other text");
			registered_ptr1 = &regxscp_nstring2;
			CB::foo1(mse::xscope_ifptr_to(*registered_ptr1));

			{
				regxscp_nstring_t regxscp_nstring3("other text");
				registered_ptr1 = &regxscp_nstring3;
				CB::foo1(mse::xscope_ifptr_to(*registered_ptr1));
			}
			/* Attempting to dereference registered_ptr1 here would result in an exception. */
			//*registered_ptr1;
		}

		mse::CXScpPtrTest1::s_test1();
	}

	{
		/*********************************/
		/*  make_pointer_to_member_v2()  */
		/*********************************/

		/* If you have a safe pointer to an object, you can get a safe pointer to a member of that object 
		using the make_pointer_to_member_v2() function. */

		/* To demonstrate, first we'll declare some objects such that we can obtain safe pointers to those
		objects. For better or worse, this library provides a bunch of different safe pointers types. */
		mse::TXScopeObj<H> h_scpobj;
		auto h_refcptr = mse::make_refcounting<H>();
		mse::TRegisteredObj<H> h_regobj;
		mse::TRelaxedRegisteredObj<H> h_rlxregobj;

		/* Safe iterators are a type of safe pointer too. */
		mse::mstd::vector<H> h_mstdvec;
		h_mstdvec.resize(1);
		auto h_mstdvec_iter = h_mstdvec.begin();
		mse::us::msevector<H> h_msevec;
		h_msevec.resize(1);
		auto h_msevec_ipointer = h_msevec.ibegin();
		auto h_msevec_ssiter = h_msevec.ss_begin();

		/* And don't forget the safe async sharing pointers. */
		auto h_access_requester = mse::make_asyncsharedv2readwrite<ShareableH>();
		auto h_writelock_ptr = h_access_requester.writelock_ptr();
		auto h_shared_immutable_ptr = mse::make_asyncsharedv2immutable<ShareableH>();

		{
			/* So here's how you get a safe pointer to a member of the object using mse::make_pointer_to_member_v2(). */
			auto h_string1_scpptr = mse::make_pointer_to_member_v2(&h_scpobj, &H::m_string1);
			(*h_string1_scpptr) = "some new text";
			auto h_string1_scp_const_ptr = mse::make_const_pointer_to_member_v2(&h_scpobj, &H::m_string1);

			auto h_string1_refcptr = mse::make_pointer_to_member_v2(h_refcptr, &H::m_string1);
			(*h_string1_refcptr) = "some new text";

			auto h_string1_regptr = mse::make_pointer_to_member_v2(&h_regobj, &H::m_string1);
			(*h_string1_regptr) = "some new text";

			auto h_string1_rlxregptr = mse::make_pointer_to_member_v2(&h_rlxregobj, &H::m_string1);
			(*h_string1_rlxregptr) = "some new text";

			auto h_string1_mstdvec_iter = mse::make_pointer_to_member_v2(h_mstdvec_iter, &H::m_string1);
			(*h_string1_mstdvec_iter) = "some new text";

			auto h_string1_msevec_ipointer = mse::make_pointer_to_member_v2(h_msevec_ipointer, &H::m_string1);
			(*h_string1_msevec_ipointer) = "some new text";

			auto h_string1_msevec_ssiter = mse::make_pointer_to_member_v2(h_msevec_ssiter, &H::m_string1);
			(*h_string1_msevec_ssiter) = "some new text";

			auto h_string1_writelock_ptr = mse::make_pointer_to_member_v2(h_writelock_ptr, &H::m_string1);
			(*h_string1_writelock_ptr) = "some new text";

			auto h_string1_stdshared_const_ptr = mse::make_pointer_to_member_v2(h_shared_immutable_ptr, &H::m_string1);
			//(*h_string1_stdshared_const_ptr) = "some new text";
		}

		{
			/* Though the type of the safe pointer to the object member varies depending on how the object was
			declared, you can make a (templated) accessor function that will return a safe pointer of the
			appropriate type. */
			auto h_string1_scpptr = H::safe_pointer_to_member_string1(&h_scpobj);
			(*h_string1_scpptr) = "some new text";

			auto h_string1_refcptr = H::safe_pointer_to_member_string1(h_refcptr);
			(*h_string1_refcptr) = "some new text";

			auto h_string1_regptr = H::safe_pointer_to_member_string1(&h_regobj);
			(*h_string1_regptr) = "some new text";

			auto h_string1_rlxregptr = H::safe_pointer_to_member_string1(&h_rlxregobj);
			(*h_string1_rlxregptr) = "some new text";

			auto h_string1_mstdvec_iter = H::safe_pointer_to_member_string1(h_mstdvec_iter);
			(*h_string1_mstdvec_iter) = "some new text";

			auto h_string1_msevec_ipointer = H::safe_pointer_to_member_string1(h_msevec_ipointer);
			(*h_string1_msevec_ipointer) = "some new text";

			auto h_string1_msevec_ssiter = H::safe_pointer_to_member_string1(h_msevec_ssiter);
			(*h_string1_msevec_ssiter) = "some new text";

			auto h_string1_writelock_ptr = H::safe_pointer_to_member_string1(h_writelock_ptr);
			(*h_string1_writelock_ptr) = "some new text";

			auto h_string1_stdshared_const_ptr = H::safe_pointer_to_member_string1(h_shared_immutable_ptr);
			//(*h_string1_stdshared_const_ptr) = "some new text";
		}
	}

	{
		/*******************/
		/*  Poly pointers  */
		/*******************/

		/* Poly pointers are "chameleon" pointers that can be constructed from, and retain the safety
		features of multiple different pointer types in this library. If you'd like your function to be
		able to take different types of safe pointer parameters, you can "templatize" your function, or
		alternatively, you can declare your pointer parameters as poly pointers. */

		class A {
		public:
			A() {}
			A(std::string x) : b(x) {}
			virtual ~A() {}

			std::string b = "some text ";
		};
		class D : public A {
		public:
			D(std::string x) : A(x) {}
		};
		class B {
		public:
			static std::string foo1(mse::TXScopePolyPointer<A> ptr) {
				std::string retval = ptr->b;
				return retval;
			}
			static std::string foo2(mse::TXScopePolyConstPointer<A> ptr) {
				std::string retval = ptr->b;
				return retval;
			}
			static std::string foo3(mse::TXScopePolyPointer<std::string> ptr) {
				std::string retval = (*ptr) + (*ptr);
				return retval;
			}
			static std::string foo4(mse::TXScopePolyConstPointer<std::string> ptr) {
				std::string retval = (*ptr) + (*ptr);
				return retval;
			}
		protected:
			~B() {}
		};

		/* To demonstrate, first we'll declare some objects such that we can obtain safe pointers to those
		objects. For better or worse, this library provides a bunch of different safe pointers types. */
		mse::TXScopeObj<A> a_scpobj;
		auto a_refcptr = mse::make_refcounting<A>();
		mse::TRegisteredObj<A> a_regobj;
		mse::TRelaxedRegisteredObj<A> a_rlxregobj;

		/* Safe iterators are a type of safe pointer too. */
		mse::mstd::vector<A> a_mstdvec;
		a_mstdvec.resize(1);
		auto a_mstdvec_iter = a_mstdvec.begin();
		mse::us::msevector<A> a_msevec;
		a_msevec.resize(1);
		auto a_msevec_ipointer = a_msevec.ibegin();
		auto a_msevec_ssiter = a_msevec.ss_begin();

		/* And note that safe pointers to member elements need to be wrapped in an mse::TXScopeAnyPointer<> for
		mse::TXScopePolyPointer<> to accept them. */
		auto b_member_a_refc_anyptr = mse::TXScopeAnyPointer<std::string>(mse::make_pointer_to_member_v2(a_refcptr, &A::b));
		auto b_member_a_reg_anyptr = mse::TXScopeAnyPointer<std::string>(mse::make_pointer_to_member_v2(&a_regobj, &A::b));
		auto b_member_a_mstdvec_iter_anyptr = mse::TXScopeAnyPointer<std::string>(mse::make_pointer_to_member_v2(a_mstdvec_iter, &A::b));

		{
			/* All of these safe pointer types happily convert to an mse::TXScopePolyPointer<>. */
			auto res_using_scpptr = B::foo1(&a_scpobj);
			auto res_using_refcptr = B::foo1(a_refcptr);
			auto res_using_regptr = B::foo1(&a_regobj);
			auto res_using_rlxregptr = B::foo1(&a_rlxregobj);
			auto res_using_mstdvec_iter = B::foo1(a_mstdvec_iter);
			auto res_using_msevec_ipointer = B::foo1(a_msevec_ipointer);
			auto res_using_msevec_ssiter = B::foo1(a_msevec_ssiter);
			auto res_using_member_refc_anyptr = B::foo3(b_member_a_refc_anyptr);
			auto res_using_member_reg_anyptr = B::foo3(b_member_a_reg_anyptr);
			auto res_using_member_mstdvec_iter_anyptr = B::foo3(b_member_a_mstdvec_iter_anyptr);

			/* Or an mse::TXScopePolyConstPointer<>. */
			auto res_using_scpptr_via_const_poly = B::foo2(&a_scpobj);
			auto res_using_refcptr_via_const_poly = B::foo2(a_refcptr);
			auto res_using_regptr_via_const_poly = B::foo2(&a_regobj);
			auto res_using_rlxregptr_via_const_poly = B::foo2(&a_rlxregobj);
			auto res_using_mstdvec_iter_via_const_poly = B::foo2(a_mstdvec_iter);
			auto res_using_msevec_ipointer_via_const_poly = B::foo2(a_msevec_ipointer);
			auto res_using_msevec_ssiter_via_const_poly = B::foo2(a_msevec_ssiter);
			auto res_using_member_refc_anyptr_via_const_poly = B::foo4(b_member_a_refc_anyptr);
			auto res_using_member_reg_anyptr_via_const_poly = B::foo4(b_member_a_reg_anyptr);
			auto res_using_member_mstdvec_iter_anyptr_via_const_poly = B::foo4(b_member_a_mstdvec_iter_anyptr);
		}

		mse::TNullableAnyPointer<A> nanyptr1;
		mse::TNullableAnyPointer<A> nanyptr2(nullptr);
		mse::TNullableAnyPointer<A> nanyptr3(a_refcptr);
		mse::TAnyPointer<A> anyptr3(a_refcptr);
		nanyptr1 = nullptr;
		nanyptr1 = 0;
		nanyptr1 = NULL;
		nanyptr1 = nanyptr2;
		nanyptr1 = mse::TNullableAnyPointer<A>(&a_regobj);
		nanyptr1 = mse::TNullableAnyPointer<A>(a_refcptr);
		auto res_nap1 = *nanyptr1;
		
		mse::CPolyPtrTest1::s_test1();
		int q = 3;
	}

	{
		/*********************************/
		/*  TAnyRandomAccessIterator<>   */
		/*  & TAnyRandomAccessSection<>  */
		/*********************************/

		/* Like TAnyPointer<>, TAnyRandomAccessIterator<> and TAnyRandomAccessSection<> are polymorphic iterators and
		"sections" that can be used to enable functions to take as arguments any type of iterator or section of any
		random access container (like an array or vector). */

		mse::mstd::array<int, 4> mstd_array1 { 1, 2, 3, 4 };
		mse::us::msearray<int, 5> msearray2 { 5, 6, 7, 8, 9 };
		mse::mstd::vector<int> mstd_vec1 { 10, 11, 12, 13, 14 };
		class B {
		public:
			static void foo1(mse::TXScopeAnyRandomAccessIterator<int> ra_iter1) {
				ra_iter1[1] = 15;
			}
			static int foo2(mse::TXScopeAnyRandomAccessConstIterator<int> const_ra_iter1) {
				const_ra_iter1 += 2;
				--const_ra_iter1;
				const_ra_iter1--;
				return const_ra_iter1[2];
			}
			static void foo3(mse::TXScopeAnyRandomAccessSection<int> ra_section) {
				for (mse::TXScopeAnyRandomAccessSection<int>::size_type i = 0; i < ra_section.size(); i += 1) {
					ra_section[i] = 0;
				}
			}
			static int foo4(mse::TXScopeAnyRandomAccessConstSection<int> const_ra_section) {
				int retval = 0;
				for (mse::TXScopeAnyRandomAccessSection<int>::size_type i = 0; i < const_ra_section.size(); i += 1) {
					retval += const_ra_section[i];
				}
				return retval;
			}
			static int foo5(mse::TXScopeAnyRandomAccessConstSection<int> const_ra_section) {
				int retval = 0;
				for (const auto& const_item : const_ra_section) {
					retval += const_item;
				}
				return retval;
			}
		};

		auto mstd_array_iter1 = mstd_array1.begin();
		mstd_array_iter1++;
		auto res1 = B::foo2(mstd_array_iter1);
		B::foo1(mstd_array_iter1);

		auto msearray_const_iter2 = msearray2.ss_cbegin();
		msearray_const_iter2 += 2;
		auto res2 = B::foo2(msearray_const_iter2);

		auto res3 = B::foo2(mstd_vec1.cbegin());
		B::foo1(++mstd_vec1.begin());
		auto res4 = B::foo2(mstd_vec1.begin());

		mse::TXScopeAnyRandomAccessIterator<int> ra_iter1 = mstd_vec1.begin();
		mse::TXScopeAnyRandomAccessIterator<int> ra_iter2 = mstd_vec1.end();
		auto res5 = ra_iter2 - ra_iter1;
		ra_iter1 = ra_iter2;

		{
			std::array<int, 4> std_array1{ 1, 2, 3, 4 };
			mse::TXScopeAnyRandomAccessIterator<int> ra_iter1(std_array1.begin());
			mse::TXScopeAnyRandomAccessIterator<int> ra_iter2 = std_array1.end();
			auto res5 = ra_iter2 - ra_iter1;
			ra_iter1 = ra_iter2;
			int q = 3;
		}

		mse::TXScopeObj<mse::mstd::array<int, 4>> mstd_array3_scbobj = mse::mstd::array<int, 4>({ 1, 2, 3, 4 });
		auto mstd_array_scpiter3 = mse::mstd::make_xscope_begin_iterator(&mstd_array3_scbobj);
		//mstd_array_scpiter3 = mstd_array3_scbobj.begin();
		++mstd_array_scpiter3;
		B::foo1(mstd_array_scpiter3);

		mse::TXScopeAnyRandomAccessSection<int> xscp_ra_section1(mstd_array_iter1, 2);
		B::foo3(xscp_ra_section1);

		mse::TXScopeAnyRandomAccessSection<int> xscp_ra_section2(++mstd_vec1.begin(), 3);
		auto res6 = B::foo5(xscp_ra_section2);
		B::foo3(xscp_ra_section2);
		auto res7 = B::foo4(xscp_ra_section2);

		auto xscp_ra_section1_xscp_iter1 = xscp_ra_section1.xscope_begin();
		auto xscp_ra_section1_xscp_iter2 = xscp_ra_section1.xscope_end();
		auto res8 = xscp_ra_section1_xscp_iter2 - xscp_ra_section1_xscp_iter1;
		bool res9 = (xscp_ra_section1_xscp_iter1 < xscp_ra_section1_xscp_iter2);

		auto ra_section1 = mse::make_random_access_section(mstd_array_iter1, 2);
		B::foo3(ra_section1);
		auto ra_const_section2 = mse::make_random_access_const_section(mstd_vec1.cbegin(), 2);
		B::foo4(ra_const_section2);

		int q = 5;
	}

	{
		/********************/
		/*  legacy helpers  */
		/********************/

		{
			MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(int) iptrwbv1 = MSE_LH_ALLOC_DYN_ARRAY1(MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(int), 2 * sizeof(int));
			iptrwbv1[0] = 1;
			iptrwbv1[1] = 2;
			MSE_LH_REALLOC(int, iptrwbv1, 5 * sizeof(int));
			auto res10 = iptrwbv1[0];
			auto res11 = iptrwbv1[1];
			auto res12 = iptrwbv1[2];
		}

		{
			struct s1_type {
				MSE_LH_FIXED_ARRAY_DECLARATION(int, 3, nar11) = { 1, 2, 3 };
			} s1, s2;

			MSE_LH_FIXED_ARRAY_DECLARATION(int, 5, nar1) = { 1, 2, 3, 4, 5 };
			auto res14 = nar1[0];
			auto res15 = nar1[1];
			auto res16 = nar1[2];

			s2 = s1;

			s2.nar11[1] = 4;
			s1 = s2;
			auto res16b = s1.nar11[1];

			MSE_LH_ARRAY_ITERATOR_TYPE(int) naraiter1 = s1.nar11;
			auto res16c = naraiter1[1];
		}

		{
			MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(int) iptrwbv1 = MSE_LH_ALLOC_DYN_ARRAY1(MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(int), 2 * sizeof(int));
			iptrwbv1[0] = 1;
			iptrwbv1[1] = 2;

			MSE_LH_ARRAY_ITERATOR_TYPE(int) naraiter1;
			MSE_LH_ARRAY_ITERATOR_TYPE(int) naraiter2 = nullptr;
			MSE_LH_ARRAY_ITERATOR_TYPE(int) naraiter3 = iptrwbv1;
			naraiter1 = nullptr;
			naraiter1 = 0;
			naraiter1 = NULL;
			naraiter1 = naraiter2;
			naraiter1 = iptrwbv1;
			auto res17 = naraiter1[1];
		}

		{
			typedef int dyn_arr2_element_type;
			MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(dyn_arr2_element_type) dyn_arr2;
			MSE_LH_ALLOC(dyn_arr2_element_type, dyn_arr2, 64/*bytes*/);
			//dyn_arr2 = MSE_LH_ALLOC_DYN_ARRAY1(MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(dyn_arr2_element_type), 64/*bytes*/);

			MSE_LH_MEMSET(dyn_arr2, 99, 64/*bytes*/);
			auto dyn_arr2b = dyn_arr2;

			MSE_LH_FREE(dyn_arr2);
		}

		{
			typedef int arr_element_type;
			MSE_LH_FIXED_ARRAY_DECLARATION(arr_element_type, 3/*elements*/, array1) = { 1, 2, 3 };
			MSE_LH_FIXED_ARRAY_DECLARATION(arr_element_type, 3/*elements*/, array2) = { 4, 5, 6 };

			MSE_LH_MEMSET(array1, 99, 3/*elements*/ * sizeof(arr_element_type));
			MSE_LH_MEMCPY(array2, array1, 3/*elements*/ * sizeof(arr_element_type));
			auto res18 = array2[1];
		}
	}

	msetl_example2();

	return 0;
}


#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

