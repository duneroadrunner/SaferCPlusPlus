
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*
This example file has become quite large (and has spilled into msetl_example2.cpp) and holds examples for many data
types. Your best bet is probably to use a find/search to get to the data type your interested in.
*/

#include "msetl_example_defs.h"

//include "msetl.h"
#include "mseprimitives.h"
#include "mseregistered.h"
#include "msecregistered.h"
#include "msenorad.h"
#include "mserefcounting.h"
#include "msescope.h"
#include "mseasyncshared.h"
#include "msepoly.h"
#include "msemsearray.h"
#include "msemstdarray.h"
#include "msemsevector.h"
#include "msemstdvector.h"
#include "mseivector.h"
#include "msevector_test.h"
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

	/* This function will be used to demonstrate using rsv::as_a_returnable_fparam() to enable template functions to return
	one of their function parameters, potentially of the scope reference variety which would otherwise be rejected (with a
	compile error) as an unsafe return value. */
	template<class _TPointer1, class _TPointer2>
	static auto longest(const _TPointer1& string1_xscpptr, const _TPointer2& string2_xscpptr) {
		auto l_string1_xscpptr = mse::rsv::as_a_returnable_fparam(string1_xscpptr);
		auto l_string2_xscpptr = mse::rsv::as_a_returnable_fparam(string2_xscpptr);
		if (l_string1_xscpptr->length() > l_string2_xscpptr->length()) {
			/* If string1_xscpptr were a regular TXScopeItemFixedPointer<mse::nii_string> and we tried to return it
			directly instead of l_string1_xscpptr, it would have induced a compile error. */
			return mse::return_value(l_string1_xscpptr);
		}
		else {
			/* mse::return_value() usually returns its input argument unmolested, but in this case it will return
			a type different from the input type. This is to prevent any function that receives this return value
			from, in turn, returning the value, as that might be unsafe. */
			return mse::return_value(l_string2_xscpptr);
		}
	}

	/* This function will be used to demonstrate using rsv::as_an_fparam() to enable template functions to accept scope 
	pointers to temporary objects. */
	template<class _TPointer1, class _TPointer2>
	static bool second_is_longer(const _TPointer1& string1_xscpptr, const _TPointer2& string2_xscpptr) {
		auto l_string1_xscpptr = mse::rsv::as_an_fparam(string1_xscpptr);
		auto l_string2_xscpptr = mse::rsv::as_an_fparam(string2_xscpptr);
		return (l_string1_xscpptr->length() > l_string2_xscpptr->length()) ? false : true;
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

#if !defined(MSE_MSTDVECTOR_DISABLED) && !defined(MSE_MSTD_VECTOR_CHECK_USE_AFTER_FREE)
		try {
			/* At this point, the vint_type object is cleared from vvi, but (with the current library implementation) it has
			not actually been deallocated/destructed yet because it "knows" that there is an iterator, namely vi_it, that is
			still referencing it. It will be deallocated when there are no more iterators referencing it. */
			auto value = (*vi_it); /* In debug mode this will fail an assert. In non-debug mode it'll just work (safely). */
			assert(5 == value);
			vint_type vi2;
			vi_it = vi2.begin();
			/* The vint_type object that vi_it was originally pointing to is now deallocated/destructed, because vi_it no longer
			references it. */
		}
		catch (...) {
			/* At present, no exception will be thrown. With future library implementations, maybe. */
			std::cerr << "potentially expected exception" << std::endl;
		}
#endif // !defined(MSE_MSTDVECTOR_DISABLED) && !defined(MSE_MSTD_VECTOR_CHECK_USE_AFTER_FREE)
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
		auto scp_iter1 = mse::mstd::make_xscope_begin_iterator(&vector1_scpobj);
		auto scp_iter2 = mse::mstd::make_xscope_end_iterator(&vector1_scpobj);

		std::sort(scp_iter1, scp_iter2);

		auto scp_citer3 = mse::mstd::make_xscope_begin_const_iterator(&vector1_scpobj);
		scp_citer3 = scp_iter1;
		scp_citer3 = mse::mstd::make_xscope_begin_const_iterator(&vector1_scpobj);
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
		auto scp_citer4 = mse::mstd::make_xscope_begin_iterator(container1_m_vector_scpptr);
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

			auto scp_ss_iter1 = mse::make_xscope_begin_iterator(&vector1_scpobj);
			auto scp_ss_iter2 = mse::make_xscope_end_iterator(&vector1_scpobj);

			std::sort(scp_ss_iter1, scp_ss_iter2);

			auto scp_ss_citer3 = mse::make_xscope_begin_const_iterator(&vector1_scpobj);
			scp_ss_citer3 = scp_ss_iter1;
			scp_ss_citer3 = mse::make_xscope_begin_const_iterator(&vector1_scpobj);
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
			auto scp_ss_citer4 = mse::make_xscope_begin_iterator(container1_m_vector_scpptr);
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

			std::sort(scp_array_iter1, scp_array_iter2);

			auto scp_array_citer3 = mse::mstd::make_xscope_begin_const_iterator(&array1_scpobj);
			scp_array_citer3 = scp_array_iter1;
			scp_array_citer3 = mse::mstd::make_xscope_begin_const_iterator(&array1_scpobj);
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

			auto scp_ss_iter1 = mse::make_xscope_begin_iterator(&array1_scpobj);
			auto scp_ss_iter2 = mse::make_xscope_end_iterator(&array1_scpobj);

			std::sort(scp_ss_iter1, scp_ss_iter2);

			auto scp_ss_citer3 = mse::make_xscope_begin_const_iterator(&array1_scpobj);
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
			auto scp_ss_citer4 = mse::make_xscope_begin_iterator(container1_m_array_scpptr);
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

		mse::self_test::CPrimitivesTest1::s_test1();

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
			/***************************/
			/*   TCRegisteredPointer   */
			/***************************/

			class C;

			class D {
			public:
				virtual ~D() {}
				mse::TCRegisteredPointer<C> m_c_ptr;
			};

			class C {
			public:
				mse::TCRegisteredPointer<D> m_d_ptr;
			};

			mse::TCRegisteredObj<C> regobjfl_c;
			mse::TCRegisteredPointer<D> d_ptr = mse::cregistered_new<D>();

			regobjfl_c.m_d_ptr = d_ptr;
			d_ptr->m_c_ptr = &regobjfl_c;

			mse::cregistered_delete<D>(d_ptr);

			{
				/* Polymorphic conversions. */
				class FD : public mse::TCRegisteredObj<D> {};
				mse::TCRegisteredObj<FD> cregistered_fd;
				mse::TCRegisteredPointer<FD> FD_cregistered_ptr1 = &cregistered_fd;
				mse::TCRegisteredPointer<D> D_cregistered_ptr4 = FD_cregistered_ptr1;
				D_cregistered_ptr4 = &cregistered_fd;
				mse::TCRegisteredFixedPointer<D> D_cregistered_fptr1 = &cregistered_fd;
				mse::TCRegisteredFixedConstPointer<D> D_cregistered_fcptr1 = &cregistered_fd;
			}
		}

		mse::self_test::CRegPtrTest1::s_test1();
		mse::self_test::CCRegPtrTest1::s_test1();
	}

	{
		/*********************/
		/*   TNoradPointer   */
		/*********************/

		/* mse::TNoradPointer<>, like mse::TCRegisteredPointer<>, behaves similar to native pointers. But where registered
		pointers are automatically set to nullptr when their target is destroyed, the destruction of an object while a "norad"
		pointer is targeting it results in program termination. This drastic consequence allows norad pointers' run-time
		safety mechanism to be very lightweight (compared to that of registered pointers).
		*/

		class C;

		class D {
		public:
			virtual ~D() {}
			mse::TNoradPointer<C> m_c_ptr;
		};

		class C {
		public:
			mse::TNoradPointer<D> m_d_ptr;
		};

		mse::TNoradObj<C> noradobj_c;
		mse::TNoradPointer<D> d_ptr = mse::norad_new<D>();

		noradobj_c.m_d_ptr = d_ptr;
		d_ptr->m_c_ptr = &noradobj_c;

		/* We must make sure that there are no other references to the target of d_ptr before deleting it. Registered pointers don't
		have the same requirement. */
		noradobj_c.m_d_ptr = nullptr;

		mse::norad_delete<D>(d_ptr);

		{
			/* Polymorphic conversions. */
			class FD : public mse::TNoradObj<D> {};
			mse::TNoradObj<FD> norad_fd;
			mse::TNoradPointer<FD> FD_norad_ptr1 = &norad_fd;
			mse::TNoradPointer<D> D_norad_ptr4 = FD_norad_ptr1;
			D_norad_ptr4 = &norad_fd;
			mse::TNoradFixedPointer<D> D_norad_fptr1 = &norad_fd;
			mse::TNoradFixedConstPointer<D> D_norad_fcptr1 = &norad_fd;
		}
	}
	mse::self_test::CNoradPtrTest1::s_test1();

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
		an exception on attempted nullptr dereference. And it's smaller and faster. And like TRegisteredPointer, proper "const",
		"not null" and "fixed" (non-retargetable) versions are provided as well. */

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
		/* Using registered pointers as weak pointers with reference counting pointers. */

		/* TRefCountingPointer<> does not have an associated weak pointer like std::shared_ptr<> does. If you need weak
		pointer functionality you could just resort to using std::shared_ptr<> and std::weak_ptr<>. When operating within
		a thread, you could also use registered pointers as weak pointers for TRefCountingPointer<>, which may be
		preferable in some cases. 

		Generally you're going to want to obtain a "strong" pointer from the weak pointer, so  rather than targeting the
		registered pointer directly at the object of interest, you'd target a/the strong owning pointer of the object. */

		typedef mse::TRefCountingFixedPointer<std::string> str_rc_ptr_t; // owning pointer of a string
		typedef mse::TWRegisteredObj<str_rc_ptr_t> str_rc_ptr_regobj_t; // registered version of above so that you can obtain a (weak)
																	   // registered pointer to it

		 /* str_rc_rc_ptr1 is a "shared" owner of an owning pointer of a string  */
		auto str_rc_rc_ptr1 = mse::make_nullable_refcounting<str_rc_ptr_regobj_t>(str_rc_ptr_regobj_t(mse::make_refcounting<std::string>("some text")));
		/* You need to double dereference it to access the string value. */
		std::cout << **str_rc_rc_ptr1 << std::endl;

		/* Here we're obtaining a (weak) registered pointer to the owning pointer of the string. */
		auto str_rc_reg_ptr1 = &(*str_rc_rc_ptr1);
		/* Here you also need to double dereference it to access the string value. */
		std::cout << **str_rc_reg_ptr1 << std::endl;

		{
			/* We can obtain a (strong) owning pointer of the string from the (weak) registered pointer. */
			auto str_rc_ptr2 = *str_rc_reg_ptr1;

			std::cout << *str_rc_ptr2 << std::endl;
		}

		assert(str_rc_reg_ptr1); // just asserting the str_rc_reg_ptr1 is not null here

		/* Here we're releasing ownership of the string owning pointer. Since this was its only owner, the string owning
		pointer (and consequently the string) will be destroyed. */
		str_rc_rc_ptr1 = nullptr;

		assert(!str_rc_reg_ptr1); // here we're asserting that str_rc_reg_ptr1 has been (automatically) set to null
	}

	{
		/* Here we demonstrate using TWCRegisteredPointer<> as a safe "weak_ptr" to prevent cyclic references from becoming
		memory leaks. This isn't much different from using std::weak_ptr<> in terms of functionality, but there can be
		performance and safety advantages. */

		class CRCNode;

		typedef mse::TRefCountingFixedPointer<CRCNode> rcnode_strongptr_t;			// owning pointer of a CRCNode
		typedef mse::TWRegisteredObj<rcnode_strongptr_t> rcnode_strongptr_regobj_t; // registered version of above so that you can obtain a (weak)
																					// registered pointer to it
		typedef mse::TWRegisteredPointer<rcnode_strongptr_t> rcnode_strongptr_weakptr_t; // (weak) registered pointer to owning pointer of a CRCNode

		class CRCNode {
		public:
			CRCNode(mse::TRegisteredPointer<mse::CInt> node_count_ptr
				, rcnode_strongptr_weakptr_t root_ptr_ptr) : m_node_count_ptr(node_count_ptr), m_root_ptr_ptr(root_ptr_ptr) {
				(*node_count_ptr) += 1;
			}
			CRCNode(mse::TRegisteredPointer<mse::CInt> node_count_ptr) : m_node_count_ptr(node_count_ptr) {
				(*node_count_ptr) += 1;
			}
			virtual ~CRCNode() {
				(*m_node_count_ptr) -= 1;
			}
			static rcnode_strongptr_regobj_t MakeRoot(mse::TRegisteredPointer<mse::CInt> node_count_ptr) {
				auto retval = rcnode_strongptr_regobj_t{ mse::make_refcounting<CRCNode>(node_count_ptr) };
				(*retval).m_root_ptr_ptr = &retval;
				return retval;
			}
			auto MaybeStrongChildPtr() const { return m_maybe_child_ptr; }
			rcnode_strongptr_regobj_t MakeChild() {
				m_maybe_child_ptr.emplace(rcnode_strongptr_regobj_t{ mse::make_refcounting<CRCNode>(m_node_count_ptr, m_root_ptr_ptr) });
				return m_maybe_child_ptr.value();
			}
			void DisposeOfChild() {
				m_maybe_child_ptr.reset();
			}

		private:
			mse::TRegisteredPointer<mse::CInt> m_node_count_ptr;
			mse::mstd::optional<rcnode_strongptr_regobj_t> m_maybe_child_ptr;
			rcnode_strongptr_weakptr_t m_root_ptr_ptr;
		};

		mse::TRegisteredObj<mse::CInt> node_counter = 0;
		{
			auto root_owner_ptr = CRCNode::MakeRoot(&node_counter);
			auto kid1 = root_owner_ptr->MakeChild();
			{
				auto kid2 = kid1->MakeChild();
				auto kid3 = kid2->MakeChild();
			}
			assert(4 == node_counter);
			kid1->DisposeOfChild();
			assert(2 == node_counter);
		}
		assert(0 == node_counter);
	}

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
		/* note that the '&' ("ampersand") operator is overloaded to return a mse::TXScopeFixedPointer<>  */
		int res1 = (&a_scpobj)->b;
		int res2 = B::foo2(&a_scpobj);
		int res3 = B::foo3(&a_scpobj);
		/* mse::TXScopeOwnerPointer<> will allocate a scope object on the heap (and deallocate it at the
		end of the scope). */
		mse::TXScopeOwnerPointer<A> xscp_a_ownerptr(7);
		int res4 = B::foo2(xscp_a_ownerptr);
		int res4b = B::foo2(&(*xscp_a_ownerptr));

		/* You can use the "mse::make_xscope_pointer_to_member_v2()" function to obtain a safe pointer to a member of
		an xscope object. */
		auto xscp_s_ptr1 = mse::make_xscope_pointer_to_member_v2((&a_scpobj), &A::s);
		(*xscp_s_ptr1) = "some new text";
		auto xscp_s_const_ptr1 = mse::make_xscope_const_pointer_to_member_v2((&a_scpobj), &A::s);

		/* The return type of mse::make_xscope_pointer_to_member_v2() depends on the type of the parameters passed
		to it. In this case, the type of xscp_s_ptr1 is mse::TXScopeItemFixedPointer<A>. */

		auto res5 = H::foo6(xscp_s_ptr1, xscp_s_const_ptr1);

		/* Using mse::make_xscope_strong_pointer_store(), you can obtain a scope pointer from a refcounting pointer (or any other
		"strong" pointer). */
		/* Let's make it a const refcounting pointer, just for variety. */
		mse::TRefCountingFixedConstPointer<A> refc_cptr1 = mse::make_refcounting<A>(11);
		auto xscp_refc_cstore = mse::make_xscope_strong_pointer_store(refc_cptr1);
		auto xscp_cptr1 = xscp_refc_cstore.xscope_ptr();
		int res6 = B::foo3(xscp_cptr1);
		mse::TXScopeItemFixedConstPointer<A> xscp_cptr2 = xscp_cptr1;
		A res7 = *xscp_cptr2;

		/* Technically, you're not allowed to return a non-owning scope pointer (or any object containing a scope reference)
		from a function. (The return_value() function wrapper enforces this.) Pretty much the only time you'd legitimately
		want to do this is when the returned pointer is one of the input parameters. An example might be a "min(a, b)"
		function which takes two objects by reference and returns the reference to the lesser of the two objects. The
		library provides the xscope_chosen() function which takes a bool and two objects of the same type (in this case it
		will be two scope pointers) and returns one of the objects (scope pointers), which one depending on the value of the
		bool. You could use this function to implement the equivalent of a min(a, b) function like so: */
		auto xscp_a_ptr5 = &a_scpobj;
		auto xscp_a_ptr6 = &(*xscp_a_ownerptr);
		auto xscp_min_ptr1 = mse::xscope_chosen((*xscp_a_ptr6 < *xscp_a_ptr5), xscp_a_ptr5, xscp_a_ptr6);
		assert(5 == xscp_min_ptr1->b);

		{
			/**************************************/
			/*  rsv::TReturnableFParam<>          */
			/*  && rsv::as_a_returnable_fparam()  */
			/**************************************/

			/* Another alternative if you want to return a scope pointer (or any object containing a scope
			reference) input parameter from a function is to wrap the parameter type with the
			rsv::TXScopeReturnableFParam<> transparent template wrapper when declaring the parameter. 
			
			Normally the return_value() function wrapper will reject (with a compile error) scope pointers as unsafe return
			values. But if the scope pointer type is wrapped in the rsv::TXScopeReturnableFParam<> transparent template
			wrapper, then it will be accepted as a safe return value. Because it's generally safe to return a reference to
			an object if that reference was passed as an input parameter. Well, as long as the object is not a temporary
			one. So unlike with rsv::TXScopeFParam<>, scope reference types wrapped with rsv::TXScopeReturnableFParam<> will
			not enable support for references to temporaries, as returning a (scope) reference to a temporary would be
			unsafe even if the reference was passed as a function parameter. So for scope reference parameters you have to
			choose between being able to use it as a return value, or supporting references to temporaries. (Or neither.)
			
			In the case of function templates, sometimes you want the parameter types to be auto-deduced, and use of the
			mse::rsv::TXScopeReturnableFParam<> wrapper can interfere with that. In those cases you can instead convert
			parameters to their wrapped type after the fact using the rsv::xscope_as_a_returnable_fparam() function.
			Note that using this function (or the rsv::TXScopeReturnableFParam<> wrapper) on anything other than function
			parameters is unsafe, and currently there is no compile-time enforcement of this restriction.

			rsv::TReturnableFParam<> and rsv::as_a_returnable_fparam() can be used for situations when the type of the
			input parameter is itself a template parameter and not necessarily always a scope type or treated as a scope
			type. */

			class CD {
			public:
				static auto longest(mse::rsv::TXScopeReturnableFParam<mse::TXScopeItemFixedPointer<mse::nii_string> > string1_xscpptr
					, mse::rsv::TXScopeReturnableFParam<mse::TXScopeItemFixedPointer<mse::nii_string> > string2_xscpptr) {
					if (string1_xscpptr->length() > string2_xscpptr->length()) {
						/* If string1_xscpptr were a regular TXScopeItemFixedPointer<mse::nii_string> the next line would have
						induced a compile error. */
						return mse::return_value(string1_xscpptr);
					}
					else {
						/* mse::return_value() usually returns its input argument unmolested, but in this case it will return
						a type (slightly) different from the input type. This is to prevent any function that receives this
						return value from, in turn, returning the value, as that might be unsafe. */
						return mse::return_value(string2_xscpptr);
					}
				}
			};
			mse::TXScopeObj<mse::nii_string> xscope_string1 = "abc";
			mse::TXScopeObj<mse::nii_string> xscope_string2 = "abcd";
			auto longer_string_xscpptr = CD::longest(&xscope_string1, &xscope_string2);
			auto copy_of_longer_string = *longer_string_xscpptr;

			auto longer_string2_xscpptr = H::longest(&xscope_string1, &xscope_string2);

			class CE {
			public:
				static auto xscope_string_const_section_to_member(mse::rsv::TXScopeReturnableFParam<mse::TXScopeItemFixedConstPointer<CE> > returnable_this_cpointer) {
					/* "Pointers to members" based on returnable pointers inherit the "returnability". */
					auto returnable_cpointer_to_member = mse::make_xscope_const_pointer_to_member_v2(returnable_this_cpointer, &CE::m_string1);
					/* "scope nrp string const sections" based on returnable pointers (or iterators) inherit the "returnability". */
					auto returnable_string_const_section = mse::make_xscope_nrp_string_const_section(returnable_cpointer_to_member);
					/* Subsections of returnable sections inherit the "returnability". */
					auto returnable_string_const_section2 = returnable_string_const_section.xscope_subsection(1, 3);
					return mse::return_value(returnable_string_const_section2);
				}
			private:
				mse::nii_string m_string1 = "abcde";
			};

			mse::TXScopeObj<CE> e_xscpobj;
			auto xscope_string_const_section1 = mse::TXScopeObj<CE>::xscope_string_const_section_to_member(&e_xscpobj);
			assert(xscope_string_const_section1 == "bcd");
		}

		{
			/****************************/
			/*  rsv::TFParam<>          */
			/*  && rsv::as_an_fparam()  */
			/****************************/

			/* rsv::TFParam<> is just a transparent template wrapper for function parameter declarations. In most cases
			use of this wrapper is not necessary, but in some cases it enables functionality only available to variables
			that are function parameters. Specifically, it allows functions to support arguments that are scope
			pointer/references to temporary objects. For safety reasons, by default, scope pointer/references to
			temporaries are actually "functionally disabled" types distinct from regular scope pointer/reference types.
			Because it's safe to do so in the case of function parameters, the rsv::TFParam<> wrapper enables certain
			scope pointer/reference types (like TXScopeItemFixedPointer<>, and "random access section" scope types) to
			be constructed from their "functionally disabled" counterparts.

			In the case of function templates, sometimes you want the parameter types to be auto-deduced, and use of the
			mse::rsv::TFParam<> wrapper can interfere with that. In those cases you can instead convert parameters to their
			wrapped type after-the-fact using the rsv::as_an_fparam() function. Note that using this function (or the
			rsv::TFParam<> wrapper) on anything other than function parameters is unsafe, and currently there is no
			compile-time enforcement of this restriction.

			rsv::TXScopeFParam<> and rsv::xscope_as_an_fparam() can be used for situations when the types are necessarily
			scope types.
			*/

			class CD {
			public:
				static bool second_is_longer(mse::rsv::TXScopeFParam<mse::TXScopeItemFixedConstPointer<mse::nii_string> > string1_xscpptr
					, mse::rsv::TXScopeFParam<mse::TXScopeItemFixedConstPointer<mse::nii_string> > string2_xscpptr) {

					return (string1_xscpptr->length() > string2_xscpptr->length()) ? false : true;
				}

				static bool second_is_longer_any(mse::rsv::TXScopeFParam<mse::TXScopeAnyConstPointer<mse::nii_string> > string1_xscpptr
					, mse::rsv::TXScopeFParam<mse::TXScopeAnyConstPointer<mse::nii_string> > string2_xscpptr) {
					return (string1_xscpptr->length() > string2_xscpptr->length()) ? false : true;
				}

				static bool second_is_longer_poly(mse::rsv::TXScopeFParam<mse::TXScopePolyConstPointer<mse::nii_string> > string1_xscpptr
					, mse::rsv::TXScopeFParam<mse::TXScopePolyConstPointer<mse::nii_string> > string2_xscpptr) {
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

		mse::self_test::CXScpPtrTest1::s_test1();
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
		mse::TCRegisteredObj<H> h_rlxregobj;

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
			auto h_string1_scpptr = mse::make_xscope_pointer_to_member_v2(&h_scpobj, &H::m_string1);
			(*h_string1_scpptr) = "some new text";
			auto h_string1_scp_const_ptr = mse::make_xscope_const_pointer_to_member_v2(&h_scpobj, &H::m_string1);

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

			auto h_string1_stdshared_const_ptr = mse::make_const_pointer_to_member_v2(h_shared_immutable_ptr, &H::m_string1);
			//(*h_string1_stdshared_const_ptr) = "some new text";
		}
	}

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
			void increment() { (*m_count_ptr) += 1; }
			void decrement() { (*m_count_ptr) -= 1; }
			int m_x;
			int *m_count_ptr;
		};
	#ifndef NDEBUG
		static const int number_of_loops = 10/*arbitrary*/;
	#else // !NDEBUG
		static const int number_of_loops = 10000000/*arbitrary*/;
	#endif // !NDEBUG
		std::cout << std::endl;
		std::cout << "Some simple benchmarks: \n";
		std::cout << "number of loops: " << number_of_loops << " \n" << " \n";

		{
			std::cout << "pointer declaration, copy and assignment: \n";
			{
				int count = 0;
				CE object1(count);
				CE object2(count);
				CE* item_ptr2 = &object1;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						CE* item_ptr = (0 == (i % 2)) ? &object1 : &object2;
						item_ptr2 = item_ptr;
						(*item_ptr).increment();
						(*item_ptr2).decrement();
					}
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
				mse::TNoradObj<CE> object1(count);
				mse::TNoradObj<CE> object2(count);
				mse::TNoradPointer<CE> item_ptr2 = &object1;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TNoradPointer<CE> item_ptr = (0 == (i % 2)) ? &object1 : &object2;
						item_ptr2 = item_ptr;
						(*item_ptr).increment();
						(*item_ptr2).decrement();
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TNoradPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRefCountingPointer<CE> object1_ptr = mse::make_refcounting<CE>(count);
				mse::TRefCountingPointer<CE> object2_ptr = mse::make_refcounting<CE>(count);
				mse::TRefCountingPointer<CE> item_ptr2 = object1_ptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRefCountingPointer<CE> item_ptr = (0 == (i % 2)) ? object1_ptr : object2_ptr;
					item_ptr2 = item_ptr;
					(*item_ptr).increment();
					(*item_ptr2).decrement();
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRefCountingPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto object1_ptr = std::make_shared<CE>(count);
				auto object2_ptr = std::make_shared<CE>(count);
				auto item_ptr2 = object1_ptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto item_ptr = (0 == (i % 2)) ? object1_ptr : object2_ptr;
					item_ptr2 = item_ptr;
					(*item_ptr).increment();
					(*item_ptr2).decrement();
				}

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
				mse::TRegisteredObj<CE> object1(count);
				mse::TRegisteredObj<CE> object2(count);
				mse::TRegisteredPointer<CE> item_ptr2 = &object1;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TRegisteredPointer<CE> item_ptr = (0 == (i % 2)) ? &object1 : &object2;
						item_ptr2 = item_ptr;
						(*item_ptr).increment();
						(*item_ptr2).decrement();
					}
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
				mse::TCRegisteredObj<CE> object1(count);
				mse::TCRegisteredObj<CE> object2(count);
				mse::TCRegisteredPointer<CE> item_ptr2 = &object1;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TCRegisteredPointer<CE> item_ptr = (0 == (i % 2)) ? &object1 : &object2;
						item_ptr2 = item_ptr;
						(*item_ptr).increment();
						(*item_ptr2).decrement();
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TCRegisteredPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		{
			std::cout << "target object allocation and deallocation: \n";
			{
				int count = 0;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						CE object(count);
						if (0 == (i % 2)) {
							object.increment();
						}
						else {
							object.decrement();
						}
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "native pointer targeting the stack: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TNoradObj<CE> object(count);
						if (0 == (i % 2)) {
							object.increment();
						}
						else {
							object.decrement();
						}
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TNoradPointer targeting the stack: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TRegisteredObj<CE> object(count);
						if (0 == (i % 2)) {
							object.increment();
						}
						else {
							object.decrement();
						}
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
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TCRegisteredObj<CE> object(count);
						if (0 == (i % 2)) {
							object.increment();
						}
						else {
							object.decrement();
						}
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TCRegisteredPointer targeting the stack: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}

			{
				int count = 0;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto owner_ptr = std::make_unique<CE>(count);
					if (0 == (i % 2)) {
						(*owner_ptr).increment();
					}
					else {
						(*owner_ptr).decrement();
					}
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
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto owner_ptr = std::make_unique<mse::TNoradObj<CE>>(count);
					if (0 == (i % 2)) {
						(*owner_ptr).increment();
					}
					else {
						(*owner_ptr).decrement();
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TNoradPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRefCountingNotNullPointer<CE> owner_ptr = mse::make_refcounting<CE>(count);
					if (0 == (i % 2)) {
						(*owner_ptr).increment();
					}
					else {
						(*owner_ptr).decrement();
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRefCountingPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto owner_ptr = std::make_shared<CE>(count);
					if (0 == (i % 2)) {
						(*owner_ptr).increment();
					}
					else {
						(*owner_ptr).decrement();
					}
				}

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
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto owner_ptr = std::make_unique<mse::TRegisteredObj<CE>>(count);
					if (0 == (i % 2)) {
						(*owner_ptr).increment();
					}
					else {
						(*owner_ptr).decrement();
					}
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
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto owner_ptr = std::make_unique<mse::TCRegisteredObj<CE>>(count);
					if (0 == (i % 2)) {
						(*owner_ptr).increment();
					}
					else {
						(*owner_ptr).decrement();
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TCRegisteredPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		{
			std::cout << "dereferencing: \n";
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
					if (!cf_ptr) { MSE_THROW(""); }
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "native pointer (checked) dereferencing: " << time_span.count() << " seconds.";
				if (3 == cf_ptr->m_a) {
					std::cout << " "; /* Using cf_ptr->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
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
					mse::TNoradPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TNoradObj<CF> item1(1);
				mse::TNoradObj<CF> item2(2);
				mse::TNoradObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				mse::TNoradPointer<CF>* rpfl_ptr = std::addressof(item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					rpfl_ptr = std::addressof((*rpfl_ptr)->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TNoradPointer (checked) dereferencing: " << time_span.count() << " seconds.";
				if (3 == (*rpfl_ptr)->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;

				item1.m_next_item_ptr = nullptr;
				item2.m_next_item_ptr = nullptr;
				item3.m_next_item_ptr = nullptr;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TCRegisteredPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TCRegisteredObj<CF> item1(1);
				mse::TCRegisteredObj<CF> item2(2);
				mse::TCRegisteredObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				mse::TCRegisteredPointer<CF>* rpfl_ptr = std::addressof(item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					rpfl_ptr = std::addressof((*rpfl_ptr)->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TCRegisteredPointer (checked) dereferencing: " << time_span.count() << " seconds.";
				if (3 == (*rpfl_ptr)->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TCRegisteredPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TCRegisteredObj<CF> item1(1);
				mse::TCRegisteredObj<CF> item2(2);
				mse::TCRegisteredObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				CF* cf_ptr = static_cast<CF*>(item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					cf_ptr = static_cast<CF*>(cf_ptr->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TCRegisteredPointer unchecked dereferencing: " << time_span.count() << " seconds.";
				if (3 == cf_ptr->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
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
				mse::TRefCountingPointer<CF>* refc_ptr = std::addressof(item1_ptr->m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					refc_ptr = std::addressof((*refc_ptr)->m_next_item_ptr);
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
			std::cout << std::endl;
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

