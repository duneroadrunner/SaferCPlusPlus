
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "msetl_example_defs.h"

#include "msetl_example2.h"
//include "msetl.h"
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
#include "mseprimitives.h"
#include "mselegacyhelpers.h"
#include "msemstdstring.h"
#include "msealgorithm.h"
#include "msethreadlocal.h"
#include "msestaticimmutable.h"
#include <algorithm>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
//include <thread>
#include <sstream>
#include <future>

/* This block of includes is required for the mse::TRegisteredRefWrapper example */
#include <algorithm>
#include <list>
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
#include <functional>


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
#pragma clang diagnostic ignored "-Wunused-local-typedefs"
//pragma clang diagnostic ignored "-Wunused-but-set-variable"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

class J {
public:
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
	/* This function takes a "random access section" (which is like an "array_view" or gsl::span) as its parameter. */
	template<class _TStringRASection>
	static void foo8(_TStringRASection ra_section) {
		size_t delay_in_milliseconds = 3000/*arbitrary*/;
		if (1 <= ra_section.size()) {
			delay_in_milliseconds /= mse::as_a_size_t(ra_section.size());
		}
		for (size_t i = 0; i < ra_section.size(); i += 1) {
			auto now1 = std::chrono::system_clock::now();
			auto tt = std::chrono::system_clock::to_time_t(now1);

			/* Just trying to obtain a string with the current time and date. The standard library doesn't yet
			seem to provide a safe, portable way to do this. */
#ifdef _MSC_VER
			static const size_t buffer_size = 64;
			char buffer[buffer_size];
			buffer[0] = '\0';
			ctime_s(buffer, buffer_size, &tt);
#else /*_MSC_VER*/
			auto buffer = ctime(&tt);
#endif /*_MSC_VER*/

			std::string now_str(buffer);
			ra_section[i] = now_str;

			std::this_thread::sleep_for(std::chrono::milliseconds(delay_in_milliseconds));
		}
	}
	/* This function just obtains a writelock_ra_section from the given "splitter access requester" and calls the given
	function with the writelock_ra_section as the first argument. */
	template<class _TAsyncSplitterRASectionReadWriteAccessRequester, class _TFunction, class... Args>
	static void invoke_with_writelock_ra_section1(_TAsyncSplitterRASectionReadWriteAccessRequester ar, _TFunction function1, Args&&... args) {
		function1(ar.writelock_ra_section(), args...);
	}

	template<typename _TParam>
	static auto foo10(_TParam param) {
		auto l_obj = param;
		/* Functions that could return a scope type need to wrap their return value with the return_value() function. */
		return mse::return_value(mse::pointer_to(l_obj));
	}

	template<class _TRASection>
	static void foo13(_TRASection ra_section) {
		for (typename _TRASection::size_type i = 0; i < ra_section.size(); i += 1) {
			ra_section[i] = 0;
		}
	}
	template<class _TRAConstSection>
	static int foo14(_TRAConstSection const_ra_section) {
		int retval = 0;
		for (typename _TRAConstSection::size_type i = 0; i < const_ra_section.size(); i += 1) {
			retval += const_ra_section[i];
		}
		return retval;
	}
	template<class _TRAConstSection>
	static int foo15(_TRAConstSection const_ra_section) {
		int retval = 0;
		for (const auto& const_item : const_ra_section) {
			retval += const_item;
		}
		return retval;
	}

	template<class _TAPointer>
	static void foo17(_TAPointer a_ptr) {
		{
			auto now1 = std::chrono::system_clock::now();
			auto tt = std::chrono::system_clock::to_time_t(now1);

			/* Just trying to obtain a string with the current time and date. The standard library doesn't yet
			seem to provide a safe, portable way to do this. */
#ifdef _MSC_VER
			static const size_t buffer_size = 64;
			char buffer[buffer_size];
			buffer[0] = '\0';
			ctime_s(buffer, buffer_size, &tt);
#else /*_MSC_VER*/
			auto buffer = ctime(&tt);
#endif /*_MSC_VER*/

			std::string now_str(buffer);
			a_ptr->s = now_str;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	template<class _TAPointer>
	static void foo17b(_TAPointer a_ptr) {
		static int s_count = 0;
		s_count += 1;
		a_ptr->s = std::to_string(s_count);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	template<class _TConstPointer, class _TPointer>
	static void foo18(_TConstPointer src_ptr, _TPointer dst_ptr) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		dst_ptr->s = src_ptr->s;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	/* This function will be used to demonstrate using rsv::as_an_fparam() to enable template functions to accept scope
	random access sections that reference temporary objects. */
	template<class _TRASection1, class _TRASection2>
	static bool second_is_longer(const _TRASection1& xscope_ra_csection1, const _TRASection2& xscope_ra_csection2) {
		auto l_xscope_ra_csection1 = mse::rsv::as_an_fparam(xscope_ra_csection1);
		auto l_xscope_ra_csection2 = mse::rsv::as_an_fparam(xscope_ra_csection2);
		return (l_xscope_ra_csection1.size() > l_xscope_ra_csection2.size()) ? false : true;
	}
};

MSE_DECLARE_THREAD_LOCAL_GLOBAL(mse::mstd::string) tlg_string1 = "some text";
MSE_RSV_DECLARE_GLOBAL_IMMUTABLE(mse::nii_string) gimm_string1 = "some text";

void msetl_example2() {
	{
		/********************/
		/*   nii_vector<>   */
		/********************/

		/* nii_vector<> is a safe vector designed for safe sharing between asynchronous threads. */

		typedef mse::nii_vector<mse::nii_string> nii_vector1_t;

		mse::TRegisteredObj<nii_vector1_t> rg_vo1;
		for (size_t i = 0; i < 5; i += 1) {
			rg_vo1.push_back("some text");
		}
		mse::TRegisteredPointer<nii_vector1_t> vo1_regptr1 = &rg_vo1;

		/* nii_vector<> does not have a begin() member function that returns an "implicit" iterator. You can obtain an
		iterator using the make_begin_iterator() et al. functions, which take a (safe) pointer to the container. */
		auto iter1 = mse::make_begin_iterator(vo1_regptr1);
		auto citer1 = mse::make_end_const_iterator(vo1_regptr1);
		citer1 = iter1;
		rg_vo1.emplace(vo1_regptr1, citer1, "some other text");
		rg_vo1.insert(vo1_regptr1, citer1, "some other text");
		mse::nii_string str1 = "some other text";
		rg_vo1.insert(vo1_regptr1, citer1, str1);

		class A {
		public:
			A() {}
			int m_i = 0;
		};
		/* Here we're declaring that A can be safely shared between asynchronous threads. */
		typedef mse::us::TUserDeclaredAsyncShareableObj<A> shareable_A_t;

		/* When the element type of an nii_vector<> is marked as "async shareable", the nii_vector<> itself is
		(automatically) marked as async shareable as well and can be safely shared between asynchronous threads
		using "access requesters". */
		auto access_requester1 = mse::make_asyncsharedv2readwrite<mse::nii_vector<shareable_A_t>>();
		auto access_requester2 = mse::make_asyncsharedv2readwrite<nii_vector1_t>();

		/* If the element type of an nii_vector<> is not marked as "async shareable", then neither is the
		nii_vector<> itself. So attempting to create an "access requester" using it would result in a compile
		error. */
		//auto access_requester3 = mse::make_asyncsharedv2readwrite<mse::nii_vector<A>>();
		//auto access_requester4 = mse::make_asyncsharedv2readwrite<mse::nii_vector<mse::mstd::string>>();

		typedef mse::mstd::vector<mse::nii_string> vector1_t;
		vector1_t vo2 = { "a", "b", "c" };
		/* mstd::vector<>s, for example, are not safely shareable between threads. But if its element type is
		safely shareable, then the contents of the mse::mstd::vector<>, can be swapped with a corresponding
		shareable nii_vector<>. Note that vector swaps are intrinsically fast operations. */
		std::swap(vo2, *(access_requester2.writelock_ptr()));

		{
			/* If the vector is declared as a "scope" object (which basically indicates that it is declared
			on the stack), then you can use "scope" iterators. While there are limitations on when they can
			be used, scope iterators would be the preferred iterator type where performance is a priority
			as they don't require extra run-time overhead to ensure that the vector has not been prematurely
			deallocated. */

			/* Here we're declaring an vector as a scope object. */
			mse::TXScopeObj<mse::nii_vector<int> > vector1_xscpobj = mse::nii_vector<int>{ 1, 2, 3 };

			/* Here we're obtaining a scope iterator to the vector. */
			auto xscp_iter1 = mse::make_xscope_begin_iterator(&vector1_xscpobj);
			auto xscp_iter2 = mse::make_xscope_end_iterator(&vector1_xscpobj);

			std::sort(xscp_iter1, xscp_iter2);

			auto xscp_citer3 = mse::make_xscope_begin_const_iterator(&vector1_xscpobj);
			xscp_citer3 = xscp_iter1;
			xscp_citer3 = mse::make_xscope_begin_const_iterator(&vector1_xscpobj);
			xscp_citer3 += 2;
			auto res1 = *xscp_citer3;
			auto res2 = xscp_citer3[0];

			{
				/* In order to obtain a direct scope pointer to a vector element, you first need to instantiate a "structure lock"
				object, which "locks" the vector to ensure that no resize (or reserve) operation that might cause a scope pointer
				to become invalid is performed. */
				auto xxscp_vector1_change_lock_guard = mse::make_xscope_vector_size_change_lock_guard(&vector1_xscpobj);
				auto xscp_ptr1 = xxscp_vector1_change_lock_guard.xscope_ptr_to_element(2);
				auto res4 = *xscp_ptr1;
			}
			vector1_xscpobj.push_back(4);
		}
	}

	{
		/*******************/
		/*  Poly pointers  */
		/*******************/

		/* Poly pointers are "chameleon" (or "type-erasing") pointers that can be constructed from, and retain
		the safety features of multiple different pointer types in this library. If you'd like your function
		to be able to take different types of safe pointer parameters, you can "templatize" your function, or
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
		mse::TCRegisteredObj<A> a_cregobj;

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
			auto res_using_cregptr = B::foo1(&a_cregobj);
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
			auto res_using_cregptr_via_const_poly = B::foo2(&a_cregobj);
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

		mse::self_test::CPolyPtrTest1::s_test1();
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

		mse::mstd::array<int, 4> mstd_array1{ 1, 2, 3, 4 };
		mse::us::msearray<int, 5> msearray2{ 5, 6, 7, 8, 9 };
		mse::mstd::vector<int> mstd_vec1{ 10, 11, 12, 13, 14 };
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
		/****************/
		/*  optional<>  */
		/****************/

		mse::self_test::COptionalTest1::s_test1();
	}

	{
		/***********************/
		/*  return_value()     */
		/*  && TReturnValue<>  */
		/***********************/

		/* The return_value() function (generally) just returns its argument and verifies that it is of a type that is safe
		to return from a function (basically, doesn't contain any scope pointers). If not it will induce a compile error.
		Functions that do or could return scope types should wrap their return value with this function. 

		TReturnValue<> is a transparent template wrapper that verifies that the type is safe to use as a function return
		type. If not it will induce a compile error. Functions that do or could return scope types and do not use the 
		"auto" return type should wrap their return type with this function. Alternatively, you can use
		TXScopeReturnValue<> which additionally ensures that the return type is a scope type. */

		class CB {
		public:
			/* It's generally not necessary for a function return type to be a scope type. Even if the return value
			is of a scope type, you can usually just use the underlying (non-scope) type of the scope object as the
			return type. */
			static mse::mstd::string foo1() {
				mse::TXScopeObj<mse::mstd::string> xscp_string1("some text");
				return mse::return_value(xscp_string1);
			}

			/* In the less common case where the scope type doesn't have an underlying non-scope type, it may be safe
			to return the scope object. But in order to use a scope type as a function return value, it must be
			wrapped in the transparent mse::TReturnValue<> or mse::TXScopeReturnValue<> wrapper template, which will
			induce a compile error if it deems the scope type potentially unsafe to use as a return type. */
			static mse::TXScopeReturnValue<mse::xscope_optional<mse::mstd::string> > foo2() {
				mse::xscope_optional<mse::mstd::string> xscp_returnable_obj1(mse::mstd::string("some text"));
				return mse::return_value(xscp_returnable_obj1);
			}

			/* "auto" return types don't need to be wrapped, but the return value needs to be wrapped with the
			return_value() function. */
			static auto foo3() {
				mse::xscope_optional<mse::mstd::string> xscp_returnable_obj1(mse::mstd::string("some text"));
				return mse::return_value(xscp_returnable_obj1);
			}
		};

		mse::TXScopeObj<mse::mstd::string> xscp_res1(CB::foo1());
		mse::xscope_optional<mse::mstd::string> xscp_res2(CB::foo2());

		typedef mse::TXScopeObj<mse::mstd::string> xscope_string_t;
		xscope_string_t xscp_str1 = "some text";
		/* TXScopeReturnValue<> deems xscope_string_t to be an acceptable return type because it doesn't contain
		any scope pointers. */
		mse::TXScopeReturnValue<xscope_string_t> xscpr_str1("some text");
		auto xscp_rstr1 = mse::return_value(xscp_str1);

		typedef decltype(&xscp_str1) xscope_string_ptr_t;
		/* TXScopeReturnValue<> deems xscope_string_ptr_t to be an unsafe return type because it is (or contains)
		a scope pointer. So the next line would result in a compile error. */
		//mse::TXScopeReturnValue<xscope_string_ptr_t> xscpr_sfptr1 = &xscp_str1;
		//auto xscp_rstr_ptr1 = mse::return_value(&xscp_str1);
	
		mse::TRegisteredObj<mse::mstd::string> reg_str1 = "some text";
		auto reg_ptr_res1 = J::foo10(reg_str1);
		//auto xscp_ptr_res1 = J::foo10(xscp_str1); // <-- would induce a compile error inside J::foo10() 
	}

	{
		/* Defining your own scope types. */

		/* It is (intended to be) uncommon to need to define your own scope types. In general, if you want to use a
		type as a scope type, you can just wrap it with the mse::TXScopeObj<> template. */

		/* But in cases where you're going to use a scope type as a member of a class or struct, that class or
		struct must itself be a scope type. Improperly defining a scope type could result in unsafe code. */

		/* Scope types need to publicly inherit from mse::us::impl::XScopeTagBase. And by convention, be named with a prefix
		indicating that it's a scope type. */
		class xscope_my_type1 : public mse::us::impl::XScopeTagBase {
		public:
			xscope_my_type1(const mse::xscope_optional<mse::mstd::string>& xscp_maybe_string)
				: m_xscp_maybe_string1(xscp_maybe_string) {}

			/* If your scope type does not contain any non-owning scope pointers, then it should be safe to use
			as a function return type. You can "mark" it as such by adding the following member function. If the
			type does contain non-owning scope pointers, then doing so could result in unsafe code. */
			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

			mse::xscope_optional<mse::mstd::string> m_xscp_maybe_string1;
		};

		/* If your type contains or owns any non-owning scope pointers, then it must also publicly inherit
		from mse::us::impl::ContainsNonOwningScopeReferenceTagBase. If your type contains or owns any item that can be
		independently targeted by scope pointers (i.e. basically has a '&' ("address of" operator) that yeilds
		a scope pointer), then it must also publicly inherit from mse::us::impl::ReferenceableByScopePointerTagBase.
		Failure to do so could result in unsafe code. */
		class xscope_my_type2 : public mse::us::impl::XScopeTagBase, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase
			, public mse::us::impl::ReferenceableByScopePointerTagBase
		{
		public:
			typedef mse::TXScopeItemFixedConstPointer<mse::mstd::string> xscope_string_ptr_t;

			xscope_my_type2(const mse::xscope_optional<xscope_string_ptr_t>& xscp_maybe_string_ptr) : m_xscp_maybe_string_ptr(xscp_maybe_string_ptr) {}

			/* This item (potentially) contains a non-owning scope pointer. */
			mse::xscope_optional<xscope_string_ptr_t> m_xscp_maybe_string_ptr;

			/* This item owns an object that can be independently targeted by scope pointers. That is,
			&(*m_xscp_string_owner_ptr) yields a scope pointer. */
			mse::TXScopeOwnerPointer<mse::mstd::string> m_xscp_string_owner_ptr;
		};
	}

	{
		/****************************/
		/*  TRandomAccessSection<>  */
		/****************************/

		/* "Random access sections" are basically objects that provide access to a (contiguous) subsection of an
		existing array or vector. Their interface is largely the same as std::basic_string_view<>, but are generally
		constructed using "make_random_access_section()" functions. */

		mse::mstd::array<int, 4> mstd_array1{ 1, 2, 3, 4 };
		mse::mstd::vector<int> mstd_vec1{ 10, 11, 12, 13, 14 };

		auto xscp_ra_section1 = mse::make_xscope_random_access_section(mstd_array1.begin(), 2);
		J::foo13(xscp_ra_section1);

		auto ra_const_section2 = mse::make_random_access_const_section(++mstd_vec1.begin(), 3);
		auto res6 = J::foo15(ra_const_section2);
		auto res7 = J::foo14(ra_const_section2);

		auto xscp_ra_section1_xscp_iter1 = xscp_ra_section1.xscope_begin();
		auto xscp_ra_section1_xscp_iter2 = xscp_ra_section1.xscope_end();
		auto res8 = xscp_ra_section1_xscp_iter2 - xscp_ra_section1_xscp_iter1;
		bool res9 = (xscp_ra_section1_xscp_iter1 < xscp_ra_section1_xscp_iter2);

		/* Like non-owning scope pointers, scope sections may not be used as a function return value. (The return_value()
		function wrapper enforces this.) Pretty much the only time you'd legitimately want to do this is when the
		returned section is constructed from one of the input parameters. Let's consider a simple example of a
		"first_half()" function that takes a scope section and returns a scope section spanning the first half of the
		section. The library provides the random_access_subsection() function which takes a random access section and a
		tuple containing a start index and a length and returns a random access section spanning the indicated
		subsection. You could use this function to implement the equivalent of a "first_half()" function like so: */

		auto xscp_ra_section3 = mse::random_access_subsection(xscp_ra_section1, std::make_tuple(0, xscp_ra_section1.length()/2));
		assert(xscp_ra_section3.length() == 1);

		{
			/* In this block we demonstrate the rsv::TXScopeFParam<> specializations that enable passing temporary objects to
			functions expecting scope random access section arguments. */
			class CD {
			public:
				typedef decltype(mse::make_xscope_random_access_const_section(mse::pointer_to(mse::TXScopeObj<mse::nii_vector<int> >
					(mse::nii_vector<int>{ 1, 2, 3})))) xscope_ra_csection_t;
				static bool second_is_longer(mse::rsv::TXScopeFParam<xscope_ra_csection_t> xscope_ra_csection1
					, mse::rsv::TXScopeFParam<xscope_ra_csection_t> xscope_ra_csection2) {

					return (xscope_ra_csection1.size() > xscope_ra_csection2.size()) ? false : true;
				}

				static bool second_is_longer_any(mse::rsv::TXScopeFParam<mse::TXScopeAnyRandomAccessConstSection<int> > xscope_ra_csection1
					, mse::rsv::TXScopeFParam<mse::TXScopeAnyRandomAccessConstSection<int> > xscope_ra_csection2) {
					return (xscope_ra_csection1.size() > xscope_ra_csection2.size()) ? false : true;
				}
			};

			mse::TXScopeObj<mse::nii_vector<int> > vector1(mse::nii_vector<int>{ 1, 2, 3});
			auto xscope_ra_csection1 = mse::make_xscope_random_access_const_section(&vector1);
			auto res1 = CD::second_is_longer(xscope_ra_csection1, mse::make_xscope_random_access_const_section(
				mse::pointer_to(mse::TXScopeObj<mse::nii_vector<int> >(mse::nii_vector<int>{ 1, 2, 3, 4}))));
			auto res2 = J::second_is_longer(xscope_ra_csection1, mse::make_xscope_random_access_const_section(
				mse::pointer_to(mse::TXScopeObj<mse::nii_vector<int> >(mse::nii_vector<int>{ 1, 2, 3, 4}))));
			auto res3 = CD::second_is_longer_any(xscope_ra_csection1, mse::make_xscope_random_access_const_section(
				mse::pointer_to(mse::TXScopeObj<mse::nii_vector<int> >(mse::nii_vector<int>{ 1, 2, 3, 4}))));
		}
	}

	{
		/******************/
		/*  mstd::string  */
		/*  & nii_string  */
		/******************/

		std::string str1 = "some text";
		mse::nii_string nii_str1 = "some text";
		mse::us::msestring msestr1 = "some text";
		mse::mstd::string mstdstr1 = "some text";

		std::cout << str1;
		std::cout << nii_str1;
		std::cout << msestr1;
		std::cout << mstdstr1;

		{
			std::string str2 = "some text";
			mse::nii_string nii_str2 = "some text";
			mse::us::msestring msestr2 = "some text";
			mse::mstd::string mstdstr2 = "some text";

			std::string stringvalues = "125 320 512 750 333, 125 \n320, 512 \n750, 333 \n125, 320 \n512, 750 \n333, \n";
			std::istringstream iss(stringvalues);
			iss >> mstdstr2;
			iss >> msestr2;
			iss >> nii_str2;
			iss >> str2;
			std::getline(iss, mstdstr2, ',');
			std::getline(iss, mstdstr2);
			std::getline(iss, msestr2, ',');
			std::getline(iss, msestr2);
			std::getline(iss, nii_str2, ',');
			std::getline(iss, nii_str2);
			std::getline(iss, str2);
		}

		auto str2 = str1 + str1;
		str2.replace(1, 2, str1);
		auto comp_res1 = str2.compare(str1);
		auto nii_str2 = nii_str1 + nii_str1;
		nii_str2.replace(1, 2, nii_str1);
		nii_str2.compare(nii_str1);
		auto msestr2 = msestr1 + msestr1;
		msestr2.replace(1, 2, msestr1);
		msestr2.compare(msestr1);
		auto mstdstr2 = mstdstr1 + mstdstr1;
		mstdstr2.replace(1, 2, mstdstr1);
		mstdstr2.compare(mstdstr1);

		std::string str3 = "some text";
		mse::TXScopeObj<mse::nii_string> xscp_nii_str3 = "some text";
		auto nii_str3_xscpiter1 = mse::make_xscope_begin_iterator(&xscp_nii_str3);
		nii_str2.copy(nii_str3_xscpiter1, 5);

		mse::TRegisteredObj<mse::nii_string> reg_nii_str3 = "some text";
		nii_str2.copy(mse::make_begin_iterator(&reg_nii_str3), 5);

		str2 = str2.substr(1);
		nii_str2 = nii_str2.substr(1);
		msestr2 = msestr2.substr(1);
		mstdstr2 = mstdstr2.substr(1);

		std::swap(nii_str1, nii_str2);
		std::swap(str1, nii_str1);

		std::swap(msestr1, msestr2);
		std::swap(str1, msestr1);
		std::swap(msestr1, nii_str1);
		std::swap(nii_str1, msestr1);

		std::swap(mstdstr1, mstdstr2);
		std::swap(str1, mstdstr1);
		std::swap(mstdstr1, nii_str1);
		std::swap(nii_str1, mstdstr1);

		{
			using namespace std::literals;
			auto stdstr5 = "some text"s;
		}
		{
			using namespace mse::mstd::literals;
			auto mstdstr5 = "some text"_mstds;
		}
		{
			using namespace mse::literals;
			auto niistr5 = "some text"_niis;
		}
	}

	{
		/**********************/
		/*  TStringSection<>  */
		/**********************/

		/* "String sections" are the string specialized versions of "random access sections", basically providing the
		functionality of std::string_view but supporting construction from any (safe) iterator type, not just raw
		pointer iterators. */

		mse::mstd::string mstring1("some text");
		auto string_section1 = mse::make_string_section(mstring1.begin() + 1, 7);
		auto string_section2 = string_section1.substr(4, 3);
		assert(string_section2.front() == 't');
		assert(string_section2.back() == 'x');

		/* Unlike std::string_view, string sections are available in "non-const" versions. */
		string_section2[0] = 'T';
		std::cout << string_section2;
		assert(mstring1 == "some Text");
		std::cout << string_section2;

		/* The (run-time overhead free) scope (and const) versions. */
		typedef mse::TXScopeObj< mse::nii_string > xscope_nii_string_t;
		xscope_nii_string_t xscp_nstring1("some text");
		auto xscp_citer1 = mse::make_xscope_begin_const_iterator(&xscp_nstring1);
		auto xscp_string_section1 = mse::make_xscope_string_const_section(xscp_citer1 + 1, 7);
		auto xscp_string_section2 = xscp_string_section1.xscope_substr(4, 3);
		assert(xscp_string_section2.front() == 't');
		assert(xscp_string_section2.back() == 'x');

		/* Btw, the random_access_subsection() function works on string sections just like any other random access sections. */
		auto string_section3 = mse::random_access_subsection(string_section1, std::make_tuple(0, string_section1.length() / 2));
		assert(string_section3.length() == 3);
		auto xscp_string_section4 = mse::xscope_random_access_subsection(string_section1, std::make_tuple(0, string_section1.length() / 2));
		auto xscp_string_section5 = mse::xscope_random_access_subsection(xscp_string_section1, std::make_tuple(0, string_section1.length() / 2));

		{
			/* For safety reasons, by default, you can't create a scope string section that references a temporary string. (This
			is not an issue with non-scope string sections.) However, there is one scenario when that is supported. Namely, when
			the scope string section is a function parameter and is indicated as such with the rsv::TXScopeFParam<> transparent
			template wrapper. */

			class CD {
			public:
				/* For this example function, the parameter type we'll be using is a "const scope string section that references a
				scope nii_string". It's a rather verbose type to express, and here we use decltype() to express it. But this example
				function is mostly for demonstration purposes. Generally, as demonstrated in the other example functions, when
				taking string sections as function parameters, rather than specifying a particular string section type, you would
				instead either make the function a function template or use a polymorphic string section type which are more concise
				and give the caller flexibility in terms of the type of string section they can pass. */

				typedef decltype(mse::make_xscope_string_const_section(std::declval<mse::TXScopeItemFixedConstPointer<mse::nii_string> >())) xscope_string_csection_t;
				static bool second_is_longer(mse::rsv::TXScopeFParam<xscope_string_csection_t> xscope_string_csection1
					, mse::rsv::TXScopeFParam<xscope_string_csection_t> xscope_string_csection2) {

					return (xscope_string_csection1.size() > xscope_string_csection2.size()) ? false : true;
				}

				/* Using (the polymorphic) TXScopeAnyStringConstSection<> as the parameter type will allow the caller to pass
				any type of string section. */
				static bool second_is_longer_any(mse::rsv::TXScopeFParam<mse::TXScopeAnyStringConstSection<> > xscope_string_csection1
					, mse::rsv::TXScopeFParam<mse::TXScopeAnyStringConstSection<> > xscope_string_csection2) {
					return (xscope_string_csection1.size() > xscope_string_csection2.size()) ? false : true;
				}
			};

			mse::TXScopeObj<mse::nii_string > string1(mse::nii_string{"abc"});
			auto xscope_string_csection1 = mse::make_xscope_string_const_section(&string1);

			/* In these function calls, the second parameter is a string section that refers to a temporary string. */
			auto res1 = CD::second_is_longer(xscope_string_csection1, mse::make_xscope_string_const_section(
				mse::pointer_to(mse::TXScopeObj<mse::nii_string >(mse::nii_string{"abcd"}))));
			auto res2 = J::second_is_longer(xscope_string_csection1, mse::make_xscope_string_const_section(
				mse::pointer_to(mse::TXScopeObj<mse::nii_string >(mse::nii_string{"abcd"}))));
			auto res3 = CD::second_is_longer_any(xscope_string_csection1, mse::make_xscope_string_const_section(
				mse::pointer_to(mse::TXScopeObj<mse::nii_string >(mse::nii_string{"abcd"}))));
		}
	}

	{
		/*************************/
		/*  TAnyStringSection<>  */
		/*************************/

		/* "Any" string sections are basically polymorphic string sections that can hold the value of any string
		section type. They can be used as function parameter types to enable functions to accept any type of string
		section. */

		mse::mstd::string mstring1("some text");
		mse::TAnyStringSection<char> any_string_section1(mstring1.begin()+5, 3);

		auto string_literal = "some text";
		mse::TAnyStringConstSection<char> any_string_const_section2(string_literal+5, 3);

		typedef mse::TRegisteredObj<mse::nii_string> reg_nii_string_t;
		reg_nii_string_t reg_nii_string3("some other text");
		/* This is a different type of (safe) iterator to a different type of string. */
		auto iter = reg_nii_string_t::ss_begin(&reg_nii_string3);

		/* Resulting in a different type of string section. */
		auto string_section3 = mse::make_string_section(iter+11, 3);

		mse::TAnyStringSection<char> any_string_section3 = string_section3;
		assert(any_string_section1 == any_string_section3);
		assert(any_string_section1.front() == 't');
		assert(any_string_section1.back() == 'x');
		any_string_section1 = string_section3;
		any_string_section1[1] = 'E';
	}

	{
		/***********************/
		/*  mstd::string_view  */
		/***********************/

		/* std::string_view stores an (unsafe) pointer iterator into its target string. mse::mstd::string_view can
		instead store any type of string iterator, including memory safe iterators. So for example, when assigned
		from an mse::mstd::string, mse::mstd::string_view will hold one of mse::mstd::string's safe (strong) iterators
		(obtained with a call to the string's cbegin() member function). Consequently, the mse::mstd::string_view will
		be safe against "use-after-free" bugs to which std::string_view is so prone. */
		mse::mstd::string_view msv1;
		{
			mse::mstd::string mstring1("some text");
			msv1 = mse::mstd::string_view(mstring1);
		}
#if !defined(MSE_MSTDSTRING_DISABLED) && !defined(MSE_MSTD_STRING_CHECK_USE_AFTER_FREE)
		try {
			/* This is not undefined (or unsafe) behavior. */
			auto ch1 = msv1[3]; /* In debug mode this will fail an assert. In non-debug mode it'll just work (safely). */
			assert('e' == ch1);
		}
		catch (...) {
			/* At present, no exception will be thrown. Instead, the lifespan of the string data is extended to match
			that of the mstd::string_view. It's possible that in future library implementations, an exception may be thrown. */
		}
#endif //!defined(MSE_MSTDSTRING_DISABLED) && !defined(MSE_MSTD_STRING_CHECK_USE_AFTER_FREE)

		mse::mstd::string mstring2("some other text");
		/* With std::string_view, you specify a string subrange with a raw pointer iterator and a length. With
		mse::mstd::string_view you are not restricted to (unsafe) raw pointer iterators. You can use memory safe
		iterators like those provided by mse::mstd::string. */
		auto msv2 = mse::mstd::string_view(mstring2.cbegin()+5, 7);
		assert(msv2 == "other t");

		/* mse::mstd::string_view is actually discourged in favor mse::nrp_string_view, which doesn't support
		construction from unsafe raw pointer iterators. */
	}

	{
		/* Common string_view bug. */
		{
			std::string s = "Hellooooooooooooooo ";
			//std::string_view sv = s + "World\n";
			//std::cout << sv;
		}
		{
#if !defined(MSE_MSTDSTRING_DISABLED) && !defined(MSE_MSTD_STRING_CHECK_USE_AFTER_FREE)
			/* Memory safe substitutes for std::string and std::string_view eliminate the danger. */

			mse::mstd::string s = "Hellooooooooooooooo ";
			mse::nrp_string_view sv = s + "World\n";
			try {
				/* This is not undefined (or unsafe) behavior. */
				std::cout << sv; /* In debug mode this will fail an assert. In non-debug mode it'll just work (safely). */
			}
			catch(...) {
				/* At present, no exception will be thrown. Instead, the lifespan of the string data is extended to match
				that of the mse::nrp_string_view. It's possible that in future library implementations, an exception may be thrown. */
			}
#endif //!defined(MSE_MSTDSTRING_DISABLED) && !defined(MSE_MSTD_STRING_CHECK_USE_AFTER_FREE)
		}
		{
			/* Memory safety can also be achieved without extra run-time overhead. */

			/* nii_string is a safe string type (with no extra run-time overhead). */
			mse::nii_string s = "Hellooooooooooooooo ";

			/* TXScopeObj<> is a transparent "annotation" template wrapper indicating that the object has "scope lifetime"
			(i.e. is declared on the stack). The wrapper, to the extent possible, enforces the claim. */
			mse::TXScopeObj< mse::nii_string > xscope_s2 = s + "World\n";

			/* xscope_pointer is not a raw pointer. It is an "annotated" pointer indicating its target has scope lifetime.
			The '&' operator is overloaded. */
			auto xscope_pointer = &xscope_s2;

			/* Here we create a safe "string_view"-like object we call a "string section". This version has no extra
			run-time overhead. The "xscope_" prefix indicates that this version also has scope lifetime (enforced to the
			extent possible). */
			auto xscope_sv = mse::make_xscope_nrp_string_const_section(xscope_pointer);

			std::cout << xscope_sv;

			/* The preceding block, while a little more verbose, is memory safe without extra run-time overhead. */

			/* And just to be clear: */

			/* You can't construct a string section directly from a naked nii_string (temporary or otherwise). */
			//auto xscope_sv2 = mse::make_xscope_nrp_string_const_section(s + "World\n");	 // <-- compile error

			/* And you won't be able to store a "scope" pointer to a temporary. */
			//auto xscope_pointer2 = &(mse::TXScopeObj< mse::nii_string >(s + "World\n"));	 // <-- compile error

			/* Passing a temporary scope string section that references a temporary string as a function argument is
			supported. But only if the function parameter is declared to support it. */
		}
		{
			std::string s = "Hellooooooooooooooo ";
			auto sv = mse::make_nrp_string_const_section(mse::make_refcounting<std::string>(s + "World\n"));
			std::cout << sv;
		}
		{
			std::string s = "Hellooooooooooooooo ";
			auto sv = mse::mknscs(mse::mkrclp(s + "World\n"));
			std::cout << sv;
		}
		{
#ifndef MSE_REGISTEREDPOINTER_DISABLED
			std::string s = "Hellooooooooooooooo ";
			auto sv = mse::make_nrp_string_const_section(&mse::TRegisteredObj<std::string>(s + "World\n"));
			try {
				std::cout << sv;
			}
			catch (...) {
				std::cout << "expected exception\n";
			}
#endif //!MSE_REGISTEREDPOINTER_DISABLED
		}
		{
#ifndef MSE_REGISTEREDPOINTER_DISABLED
			std::string s = "Hellooooooooooooooo ";
			auto sv = mse::mknscs(&mse::mkrolp(s + "World\n"));
			try {
				std::cout << sv;
			}
			catch (...) {
				std::cout << "expected exception\n";
			}
#endif //!MSE_REGISTEREDPOINTER_DISABLED
		}
	}

	{
		/* algorithms */

		mse::TXScopeObj<mse::nii_array<int, 3> > xscope_na1 = mse::nii_array<int, 3>{ 1, 2, 3 };
		mse::TXScopeObj<mse::nii_array<int, 3> > xscope_na2 = mse::nii_array<int, 3>{ 1, 2, 3 };
		auto xscope_na1_begin_citer = mse::make_xscope_begin_const_iterator(&xscope_na1);
		auto xscope_na1_end_citer = mse::make_xscope_end_const_iterator(&xscope_na1);
		auto xscope_na2_begin_iter = mse::make_xscope_begin_iterator(&xscope_na2);
		auto xscope_na2_end_iter = mse::make_xscope_end_iterator(&xscope_na2);

		mse::mstd::array<int, 3> ma1{ 1, 2, 3 };

		mse::TXScopeObj<mse::nii_vector<int> > xscope_nv1 = mse::nii_vector<int>{ 1, 2, 3 };
		auto xscope_nv1_begin_iter = mse::make_xscope_begin_iterator(&xscope_nv1);
		auto xscope_nv1_end_iter = mse::make_xscope_end_iterator(&xscope_nv1);

		{
			/* for_each_ptr() */

			/*  mse::for_each_ptr() is like std:::for_each() but instead of passing, to the given function, a reference
			to each item it passes a (safe) pointer to each item. The actual type of the pointer varies depending on the
			type of the given iterators. */
			typedef mse::for_each_ptr_type<decltype(ma1.begin())> item_ptr_t;
			mse::for_each_ptr(ma1.begin(), ma1.end(), [](item_ptr_t x_ptr) { std::cout << *x_ptr << std::endl; });

			mse::for_each_ptr(xscope_na1_begin_citer, xscope_na1_end_citer, [](auto x_ptr) { std::cout << *x_ptr << std::endl; });

			/* A "scope range" version is also available that bypasses the use of iterators. As well as often being more
			convenient, it can theoretically be little more performance optimal. */
			typedef mse::xscope_range_for_each_ptr_type<decltype(&xscope_na1)> range_item_ptr_t;
			mse::xscope_range_for_each_ptr(&xscope_na1, [](range_item_ptr_t x_ptr) { std::cout << *x_ptr << std::endl; });

			/* Note that for performance (and safety) reasons, vectors may be "structure locked" for the duration of the loop.
			That is, any attempt to modify the size of the vector during the loop may result in an exception. */
			mse::for_each_ptr(xscope_nv1_begin_iter, xscope_nv1_end_iter, [](auto x_ptr) { std::cout << *x_ptr << std::endl; });
			mse::xscope_range_for_each_ptr(&xscope_nv1, [](auto x_ptr) { std::cout << *x_ptr << std::endl; });
		}
		{
			/* find_if_ptr() */

			typedef mse::find_if_ptr_type<decltype(xscope_na1_begin_citer)> item_ptr_t;
			auto found_citer1 = mse::find_if_ptr(xscope_na1_begin_citer, xscope_na1_end_citer, [](item_ptr_t x_ptr) { return 2 == *x_ptr; });
			auto res1 = *found_citer1;

			auto found_citer3 = mse::find_if_ptr(ma1.cbegin(), ma1.cend(), [](auto x_ptr) { return 2 == *x_ptr; });

			/* This version returns an optional scope pointer to the found item rather than an iterator. */
			typedef mse::xscope_range_get_ref_if_ptr_type<decltype(&xscope_na1)> range_item_ptr_t;
			auto xscope_optional_xscpptr4 = mse::xscope_range_get_ref_if_ptr(&xscope_na1, [](range_item_ptr_t x_ptr) { return 2 == *x_ptr; });
			auto res4 = xscope_optional_xscpptr4.value();

			/* This version returns a scope pointer to the found item or throws an exception if an appropriate item isn't
			found. */
			auto xscope_pointer5 = mse::xscope_range_get_ref_to_element_known_to_be_present_ptr(&xscope_na1, [](auto x_ptr) { return 2 == *x_ptr; });
			auto res5 = *xscope_pointer5;
		}
	}

	{
		/*****************************************/
		/*  MSE_DECLARE_THREAD_LOCAL()           */
		/*  & MSE_DECLARE_THREAD_LOCAL_GLOBAL()  */
		/*****************************************/

		auto tlg_ptr1 = &tlg_string1;
		auto xs_tlg_store1 = mse::make_xscope_strong_pointer_store(tlg_ptr1);
		auto xs_ptr1 = xs_tlg_store1.xscope_ptr();
		*xs_ptr1 += "...";
		std::cout << *xs_ptr1 << std::endl;

		MSE_DECLARE_THREAD_LOCAL_CONST(mse::mstd::string) tlc_string2 = "abc";
		auto tlc_ptr2 = &tlc_string2;
		auto xs_tlc_store2 = mse::make_xscope_strong_pointer_store(tlc_ptr2);
		auto xs_cptr2 = xs_tlc_store2.xscope_ptr();
		std::cout << *xs_cptr2 << std::endl;

		class CA {
		public:
			auto foo1() const {
				MSE_DECLARE_THREAD_LOCAL(mse::mstd::string) tl_string = "abc";
				/* mse::return_value() just returns its argument and ensures that it's of a (pointer) type that's safe to return. */
				return mse::return_value(&tl_string);
			}
		};
		auto tl_ptr3 = CA().foo1();
		auto xs_tl_store3 = mse::make_xscope_strong_pointer_store(tl_ptr3);
		auto xs_cptr3 = xs_tl_store3.xscope_ptr();
		*xs_cptr3 += "def";
		std::cout << *xs_cptr3 << std::endl;
	}

	{
		/******************************************/
		/*  MSE_DECLARE_STATIC_IMMUTABLE()        */
		/*  & MSE_RSV_DECLARE_GLOBAL_IMMUTABLE()  */
		/******************************************/

		auto gimm_ptr1 = &gimm_string1;
		auto xs_gimm_store1 = mse::make_xscope_strong_pointer_store(gimm_ptr1);
		auto xs_ptr1 = xs_gimm_store1.xscope_ptr();
		std::cout << *xs_ptr1 << std::endl;

		MSE_DECLARE_STATIC_IMMUTABLE(mse::nii_string) simm_string2 = "abc";
		auto simm_ptr2 = &simm_string2;
		auto xs_simm_store2 = mse::make_xscope_strong_pointer_store(simm_ptr2);
		auto xs_ptr2 = xs_simm_store2.xscope_ptr();
		std::cout << *xs_ptr2 << std::endl;

		class CA {
		public:
			auto foo1() const {
				MSE_DECLARE_STATIC_IMMUTABLE(mse::nii_string) simm_string = "abc";
				/* mse::return_value() just returns its argument and ensures that it's of a (pointer) type that's safe to return. */
				return mse::return_value(&simm_string);
			}
		};
		auto simm_ptr3 = CA().foo1();
		auto xs_simm_store3 = mse::make_xscope_strong_pointer_store(simm_ptr3);
		auto xs_cptr3 = xs_simm_store3.xscope_ptr();
		std::cout << *xs_cptr3 << std::endl;
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
			mse::nii_string s = "some text ";
		};
		/* User-defined classes need to be declared as (safely) shareable in order to be accepted by the access requesters. */
		typedef mse::us::TUserDeclaredAsyncShareableObj<A> ShareableA;

		class B {
		public:
			static double foo1(mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA> A_ashar) {
				auto t1 = std::chrono::high_resolution_clock::now();
				/* mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA>::writelock_ptr() will block until it can obtain a write lock. */
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
			static int foo2(mse::TAsyncSharedV2ImmutableFixedPointer<ShareableA> A_immptr) {
				return A_immptr->b;
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
			auto ash_access_requester = mse::make_asyncsharedv2readwrite<ShareableA>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;

			{
				auto ptr1 = ash_access_requester.writelock_ptr();
				auto ptr2 = ash_access_requester.writelock_ptr();
			}

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(B::foo1, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire write pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;

			/* Btw, mse::TAsyncSharedV2ReadOnlyAccessRequester<>s can be copy constructed from
			mse::TAsyncSharedV2ReadWriteAccessRequester<>s */
			mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA> ash_read_only_access_requester(ash_access_requester);
		}
		{
			std::cout << "TAsyncSharedReadOnly:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readonly<ShareableA>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(J::foo7<mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedV2ReadWriteAccessRequester:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readwrite<ShareableA>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;

			{
				auto ptr3 = ash_access_requester.readlock_ptr();
				auto ptr1 = ash_access_requester.writelock_ptr();
				auto ptr2 = ash_access_requester.writelock_ptr();
			}

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(J::foo7<mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedV2ReadOnlyAccessRequester:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readonly<ShareableA>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;

			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(J::foo7<mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA>>, ash_access_requester));
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
			auto access_requester = mse::make_asyncsharedv2readwrite<mse::nii_string>("some text");
			auto writelock_ptr1 = access_requester.try_writelock_ptr();
			if (writelock_ptr1) {
				// lock request succeeded
				int q = 5;
			}
			auto readlock_ptr2 = access_requester.try_readlock_ptr_for(std::chrono::seconds(1));
			auto writelock_ptr3 = access_requester.try_writelock_ptr_until(std::chrono::steady_clock::now() + std::chrono::seconds(1));
		}
		{
			/* For scenarios where the shared object is immutable (i.e. is never modified), you can get away without using locks
			or access requesters. */
			auto A_immptr = mse::make_asyncsharedv2immutable<ShareableA>(5);
			int res1 = A_immptr->b;
			std::shared_ptr<const ShareableA> A_shptr(A_immptr);

			std::list<std::future<int>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(B::foo2, A_immptr));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				int res2 = (*it).get();
			}

			auto A_b_safe_cptr = mse::make_const_pointer_to_member_v2(A_immptr, &A::b);
		}
		{
			/* This block demonstrates safely allowing different threads to (simultaneously) modify different
			sections of a vector. (We use vectors in this example, but it works just as well with arrays.) */

			static const size_t num_sections = 10;
			static const size_t section_size = 5;
			const size_t num_elements = num_sections * section_size;

			typedef mse::nii_vector<mse::nii_string> async_shareable_vector1_t;
			typedef mse::mstd::vector<mse::nii_string> nonshareable_vector1_t;
			/* Let's say we have a vector. */
			nonshareable_vector1_t vector1;
			vector1.resize(num_elements);
			{
				size_t count = 0;
				for (auto& item_ref : vector1) {
					count += 1;
					item_ref = "text" + std::to_string(count);
				}
			}

			/* Only access controlled objects can be shared with other threads, so we'll make an access controlled vector and
			(temporarily) swap it with our original one. */
			auto ash_access_requester = mse::make_asyncsharedv2readwrite<async_shareable_vector1_t>();
			std::swap(vector1, (*(ash_access_requester.writelock_ptr())));

			std::cout << "mse::TAsyncRASectionSplitter<>, part 1: " << std::endl;

			{
				/* Now, we're going to use the access requester to obtain two new access requesters that provide access to
				(newly created) "random access section" objects which are used to access (disjoint) sections of the vector.
				We need to specify the position where we want to split the vector. Here we specify that it be split at index
				"num_elements / 2", right down the middle. */
				mse::TAsyncRASectionSplitter<decltype(ash_access_requester)> ra_section_split1(ash_access_requester, num_elements / 2);
				auto ar1 = ra_section_split1.first_ra_section_access_requester();
				auto ar2 = ra_section_split1.second_ra_section_access_requester();

				/* The J::foo8 template function is just an example function that operates on containers of strings. In our case the
				containers will be the random access sections we just created. We'll create an instance of the function here. */
				auto& my_foo8_function_ref = J::foo8<decltype(ar1.writelock_ra_section())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;

				/* We want to execute the my_foo8 function in a separate thread. The function takes a "random access section"
				as an argument. But as we're not allowed to pass random access sections between threads, we must pass an
				access requester instead. The "J::invoke_with_writelock_ra_section1" template function is just a helper
				function that will obtain a (writelock) random access section from the access requester, then call the given
				function, in this case my_foo8, with that random access section. So here we'll use it to create a proxy
				function that we can execute directly in a separate thread and will accept an access requester as a
				parameter. */
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar1), my_foo8_function_type>;

				std::list<mse::mstd::thread> threads;
				/* So this thread will modify the first section of the vector. */
				threads.emplace_back(mse::mstd::thread(my_foo8_proxy_function_ref, ar1, my_foo8_function_ref));
				/* While this thread modifies the other section. */
				threads.emplace_back(mse::mstd::thread(my_foo8_proxy_function_ref, ar2, my_foo8_function_ref));

				{
					int count = 1;
					for (auto it = threads.begin(); threads.end() != it; it++, count++) {
						(*it).join();
					}
				}
				int q = 5;
			}
			std::cout << "mse::TAsyncRASectionSplitter<>, part 2: " << std::endl;
			{
				/* Ok, now let's do it again, but instead of splitting the vector into two sections, let's split it into more sections: */
				/* First we create a list of a the sizes of each section. We'll use a vector here, but any iteratable container will work. */
				mse::mstd::vector<size_t> section_sizes;
				for (size_t i = 0; i < num_sections; i += 1) {
					section_sizes.push_back(section_size);
				}

				/* Just as before, TAsyncRASectionSplitter<> will generate a new access requester for each section. */
				mse::TAsyncRASectionSplitter<decltype(ash_access_requester)> ra_section_split1(ash_access_requester, section_sizes);
				auto ar0 = ra_section_split1.ra_section_access_requester(0);

				auto& my_foo8_function_ref = J::foo8<decltype(ar0.writelock_ra_section())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar0), my_foo8_function_type>;

				{
					/* Here we demonstrate scope threads. Scope threads don't support being copied or moved. Unlike mstd::thread,
					scope threads can share objects declared on the stack (which is not utilized here), and in their destructor,
					scope thread objects will wait until their thread finishes its execution (i.e "join" the thread), blocking if
					necessary. Often rather than declaring scope thread objects directly, you'll create and manage multiple scope
					threads with an "xscope_thread_carrier". An xscope_thread_carrier is just a container that holds scope
					threads. */
					mse::xscope_thread_carrier xscope_threads;
					for (size_t i = 0; i < num_sections; i += 1) {
						auto ar = ra_section_split1.ra_section_access_requester(i);
						xscope_threads.new_thread(my_foo8_proxy_function_ref, ar, my_foo8_function_ref);
					}
					/* The scope will not end until all the scope threads have finished executing. */
				}
			}

			/* Now that we're done sharing the (controlled access) vector, we can swap it back to our original vector. */
			std::swap(vector1, (*(ash_access_requester.writelock_ptr())));
			auto first_element_value = vector1[0];
			auto last_element_value = vector1.back();

			int q = 5;
		}
		{
			/* Here we demonstrate safely sharing an existing stack allocated object among threads. */

			std::cout << "xscope_future_carrier<>: " << std::endl;

			/* (Mutable) objects can be shared between threads only if they are "access controlled". You can make an
			object "access controlled" by wrapping its type with the mse::TXScopeAccessControlledObj<> template wrapper. */
			mse::TXScopeObj<mse::TXScopeAccessControlledObj<ShareableA> > a_xscpacobj(7);

			/* Here we obtain a scope access requester for the access controlled object. */
			auto xscope_access_requester = mse::make_xscope_asyncsharedv2acoreadwrite(&a_xscpacobj);

			/* xscope_future_carrier<> is just a container that holds and manages scope futures. */
			mse::xscope_future_carrier<double> xscope_futures;

			std::list<mse::xscope_future_carrier<double>::handle_t> future_handles;
			for (size_t i = 0; i < 3; i += 1) {
				/* You add a future by specifying the async() function and parameters that will return the future value. */
				auto handle = xscope_futures.new_future(J::foo7<decltype(xscope_access_requester)>, xscope_access_requester);

				/* You need to store the handle of the added future in order to later retrieve its value. */
				future_handles.emplace_back(handle);
			}
			int count = 1;
			for (auto it = future_handles.begin(); future_handles.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << xscope_futures.xscope_ptr_at(*it)->get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		{
			mse::TXScopeObj<mse::TXScopeAccessControlledObj<ShareableA> > a_xscpacobj1(3);
			mse::TXScopeObj<mse::TXScopeAccessControlledObj<ShareableA> > a_xscpacobj2(5);
			mse::TXScopeObj<mse::TXScopeAccessControlledObj<ShareableA> > a_xscpacobj3(7);

			{
				std::cout << "mse::make_xscope_aco_locker_for_sharing(): " << std::endl;

				/* The mse::make_xscope_aco_locker_for_sharing() function takes a scope pointer to an "access controlled object"
				and returns a "locker" object which then holds an exclusive reference to the given access controlled
				object. From this locker object, you can obtain either one "scope passable" (non-const) pointer, or
				any number of "scope passable" const pointers. These scope passable pointers can then be safely
				passed directly as arguments to scope threads. This is a (little) more cumbersome, more restrictive
				way of sharing an object than, say, using the library's "access requesters". So generally using
				access requesters would be preferred. But you might choose to do it this way in certain cases where
				performance is critical. When using access requesters, each thread obtains the desired lock on a
				thread-safe mutex. Here we're obtaining the lock before launching the thread(s), so the mutex does
				not need to be thread-safe, thus saving a little overhead. */
				auto xscope_aco_locker1 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj1);

				typedef decltype(xscope_aco_locker1.xscope_passable_pointer()) passable_exclusive_pointer_t;
				mse::xscope_thread xscp_thread1(J::foo17b<passable_exclusive_pointer_t>, xscope_aco_locker1.xscope_passable_pointer());
			}
			{
				auto xscope_aco_locker1 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj1);
				auto xscope_aco_locker2 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj2);
				auto xscope_aco_locker3 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj3);

				typedef decltype(xscope_aco_locker1.xscope_passable_const_pointer()) passable_const_pointer_t;
				typedef decltype(xscope_aco_locker2.xscope_passable_pointer()) passable_exclusive_pointer_t;

				mse::xscope_thread xscp_thread1(J::foo18<passable_const_pointer_t, passable_exclusive_pointer_t>
					, xscope_aco_locker1.xscope_passable_const_pointer()
					, xscope_aco_locker2.xscope_passable_pointer());

				mse::xscope_thread xscp_thread2(J::foo18<passable_const_pointer_t, passable_exclusive_pointer_t>
					, xscope_aco_locker1.xscope_passable_const_pointer()
					, xscope_aco_locker3.xscope_passable_pointer());
			}
			{
				auto xscope_aco_locker1 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj1);

				/* The mse::make_xscope_exclusive_strong_pointer_store_for_sharing() function returns the same kind of "locker" object that
				mse::make_xscope_aco_locker_for_sharing() does, but instead of taking a scope pointer to an "access controlled object", it
				accepts any recognized "exclusive" pointer. That is, a pointer that, while it exists, holds exclusive access to
				its target object. */
				auto xscope_xstrong_ptr_store1 = mse::make_xscope_exclusive_strong_pointer_store_for_sharing(xscope_aco_locker1.xscope_passable_pointer());

				auto xscope_aco_locker2 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj2);
				auto xscope_aco_locker3 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj3);

				typedef decltype(xscope_aco_locker1.xscope_passable_const_pointer()) passable_const_pointer_t;
				typedef decltype(xscope_aco_locker2.xscope_passable_pointer()) passable_exclusive_pointer_t;

				mse::xscope_thread xscp_thread1(J::foo18<passable_const_pointer_t, passable_exclusive_pointer_t>
					, xscope_xstrong_ptr_store1.xscope_passable_const_pointer()
					, xscope_aco_locker2.xscope_passable_pointer());

				mse::xscope_thread xscp_thread2(J::foo18<passable_const_pointer_t, passable_exclusive_pointer_t>
					, xscope_xstrong_ptr_store1.xscope_passable_const_pointer()
					, xscope_aco_locker3.xscope_passable_pointer());
			}
			{
				/* In this block we demonstrate obtaining various types of (const and non-const) pointers you might need from
				an exclusive pointer that might be passed to a thread. */

				std::cout << "mse::TXScopeExclusiveStrongPointerStoreForAccessControlFParam<>: " << std::endl;

				a_xscpacobj1.pointer()->s = "";

				auto xscope_aco_locker1 = mse::make_xscope_aco_locker_for_sharing(&a_xscpacobj1);

				typedef decltype(xscope_aco_locker1.xscope_passable_pointer()) passable_exclusive_pointer_t;
				typedef decltype(xscope_aco_locker1.xscope_passable_const_pointer()) passable_const_pointer_t;

				class CD {
				public:
					/* mse::TXScopeExclusiveStrongPointerStoreForAccessControl<> is a data type that stores an exclusive strong
					pointer. From this data type you can obtain const, non-const and exclusive pointers. 
					mse::TXScopeExclusiveStrongPointerStoreForAccessControlFParam<> is the version for use as a function parameter.
					So this function expects to be passed a pointer of type passable_exclusive_pointer_t. */
					static void foo1(mse::TXScopeExclusiveStrongPointerStoreForAccessControlFParam<passable_exclusive_pointer_t> xscope_store, int count) {
						{
							auto xsptr = xscope_store.xscope_pointer();
							xsptr->s.append(std::to_string(count));
						}
						{
							/* Here, from the exclusive (non-const) pointer passed to this function, we're going to obtain a couple
							of const pointers that we can pass to different (scope) threads. */
							auto xscope_xstrong_ptr_store1 = mse::make_xscope_exclusive_strong_pointer_store_for_sharing(xscope_store.xscope_exclusive_pointer());

							mse::xscope_thread xscp_thread1(CD::foo2, xscope_xstrong_ptr_store1.xscope_passable_const_pointer());
							mse::xscope_thread xscp_thread2(CD::foo2, xscope_xstrong_ptr_store1.xscope_passable_const_pointer());
						}
						if (1 <= count) {
							/* And here we're going to (re)obtain an exclusive strong pointer like the one that was passed to this
							function, then we're going to use it to recursively call this function again in another (scope) thread. */
							auto xscope_xstrong_ptr_store1 = mse::make_xscope_exclusive_strong_pointer_store_for_sharing(xscope_store.xscope_exclusive_pointer());
							mse::xscope_thread xscp_thread1(CD::foo1, xscope_xstrong_ptr_store1.xscope_passable_pointer(), count - 1);
						}
					}
					static void foo2(passable_const_pointer_t xscope_A_cptr) {
						std::cout << xscope_A_cptr->s << std::endl;
					}
				};

				mse::xscope_thread xscp_thread1(CD::foo1, xscope_aco_locker1.xscope_passable_pointer(), 3);

				std::cout << std::endl;
			}
		}

		{
			/* TExclusiveWriterObj<> is a specialization of TAccessControlledObj<> for which all non-const pointers are
			exclusive. That is, when a non-const pointer exists, no other pointer may exist. */
			mse::TXScopeObj<mse::TExclusiveWriterObj<ShareableA> > a_xscpxwobj1(3);
			mse::TXScopeObj<mse::TExclusiveWriterObj<ShareableA> > a_xscpxwobj2(5);
			mse::TXScopeObj<mse::TExclusiveWriterObj<ShareableA> > a_xscpxwobj3(7);

			{
				/* A (non-const) pointer of an "exclusive writer object" qualifies as an "exclusive strong" pointer, and
				thus you can obtain an xscope shareable pointer from it in the standard way. */
				auto xscope_xwo_pointer_store1 = mse::make_xscope_exclusive_strong_pointer_store_for_sharing(a_xscpxwobj1.pointer());

				typedef decltype(xscope_xwo_pointer_store1.xscope_passable_pointer()) passable_exclusive_pointer_t;
				mse::xscope_thread xscp_thread1(J::foo17b<passable_exclusive_pointer_t>, xscope_xwo_pointer_store1.xscope_passable_pointer());
			}
			{
				/* But uniquely, you can obtain an xscope shareable const pointer from a (non-exclusive) const pointer of an
				"exclusive writer object". There is a special function for this purpose: */
				auto xscope_xwo_const_pointer_store1 = mse::make_xscope_exclusive_write_obj_const_pointer_store_for_sharing(a_xscpxwobj1.const_pointer());

				auto xscope_xwo_pointer_store2 = mse::make_xscope_exclusive_strong_pointer_store_for_sharing(a_xscpxwobj2.pointer());
				auto xscope_xwo_pointer_store3 = mse::make_xscope_exclusive_strong_pointer_store_for_sharing(a_xscpxwobj3.pointer());

				typedef decltype(xscope_xwo_const_pointer_store1.xscope_passable_const_pointer()) passable_const_pointer_t;
				typedef decltype(xscope_xwo_pointer_store2.xscope_passable_pointer()) passable_exclusive_pointer_t;

				mse::xscope_thread xscp_thread1(J::foo18<passable_const_pointer_t, passable_exclusive_pointer_t>
					, xscope_xwo_const_pointer_store1.xscope_passable_const_pointer()
					, xscope_xwo_pointer_store2.xscope_passable_pointer());

				mse::xscope_thread xscp_thread2(J::foo18<passable_const_pointer_t, passable_exclusive_pointer_t>
					, xscope_xwo_const_pointer_store1.xscope_passable_const_pointer()
					, xscope_xwo_pointer_store3.xscope_passable_pointer());
			}
		}

		{
			/* This block is similar to a previous one that demonstrates safely allowing different threads to (simultaneously)
			modify different sections of a vector. The difference is just that here the shared vector is a pre-existing one
			declared as a local variable (i.e. on the stack). */

			static const size_t num_sections = 10;
			static const size_t section_size = 5;
			const size_t num_elements = num_sections * section_size;

			typedef mse::nii_vector<mse::nii_string> async_shareable_vector1_t;
			typedef mse::mstd::vector<mse::nii_string> nonshareable_vector1_t;
			/* Let's say we have a vector. */
			nonshareable_vector1_t vector1;
			vector1.resize(num_elements);
			{
				size_t count = 0;
				for (auto& item_ref : vector1) {
					count += 1;
					item_ref = "text" + std::to_string(count);
				}
			}

			/* Only access controlled objects can be shared with other threads, so we'll make an access controlled vector and
			(temporarily) swap it with our original one. */
			mse::TXScopeObj<mse::TXScopeAccessControlledObj<async_shareable_vector1_t> > xscope_acobj;
			auto xscope_ash_access_requester = mse::make_xscope_asyncsharedv2acoreadwrite(&xscope_acobj);
			//auto ash_access_requester = mse::make_asyncsharedv2readwrite<async_shareable_vector1_t>();
			std::swap(vector1, (*(xscope_ash_access_requester.writelock_ptr())));

			std::cout << "access controlled, mse::TAsyncRASectionSplitter<>, part 1: " << std::endl;

			{
				/* Now, we're going to use the access requester to obtain two new access requesters that provide access to
				(newly created) "random access section" objects which are used to access (disjoint) sections of the vector.
				We need to specify the position where we want to split the vector. Here we specify that it be split at index
				"num_elements / 2", right down the middle. */
				mse::TXScopeAsyncRASectionSplitter<decltype(xscope_ash_access_requester)> xscope_ra_section_split1(xscope_ash_access_requester, num_elements / 2);
				auto ar1 = xscope_ra_section_split1.first_ra_section_access_requester();
				auto ar2 = xscope_ra_section_split1.second_ra_section_access_requester();

				/* The J::foo8 template function is just an example function that operates on containers of strings. In our case the
				containers will be the random access sections we just created. We'll create an instance of the function here. */
				auto& my_foo8_function_ref = J::foo8<decltype(ar1.writelock_ra_section())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;

				/* We want to execute the my_foo8 function in a separate thread. The function takes a "random access section"
				as an argument. But as we're not allowed to pass random access sections between threads, we must pass an
				access requester instead. The "J::invoke_with_writelock_ra_section1" template function is just a helper
				function that will obtain a (writelock) random access section from the access requester, then call the given
				function, in this case my_foo8, with that random access section. So here we'll use it to create a proxy
				function that we can execute directly in a separate thread and will accept an access requester as a
				parameter. */
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar1), my_foo8_function_type>;

				mse::xscope_thread_carrier threads;
				/* So this thread will modify the first section of the vector. */
				threads.new_thread(my_foo8_proxy_function_ref, ar1, my_foo8_function_ref);
				/* While this thread modifies the other section. */
				threads.new_thread(my_foo8_proxy_function_ref, ar2, my_foo8_function_ref);

				int q = 5;
			}

			std::cout << "access controlled, mse::TAsyncRASectionSplitter<>, part 2: " << std::endl;

			{
				/* Ok, now let's do it again, but instead of splitting the vector into two sections, let's split it into more sections: */
				/* First we create a list of a the sizes of each section. We'll use a vector here, but any iteratable container will work. */
				mse::mstd::vector<size_t> section_sizes;
				for (size_t i = 0; i < num_sections; i += 1) {
					section_sizes.push_back(section_size);
				}

				/* Just as before, TXScopeAsyncRASectionSplitter<> will generate a new access requester for each section. */
				mse::TXScopeAsyncRASectionSplitter<decltype(xscope_ash_access_requester)> xscope_ra_section_split1(xscope_ash_access_requester, section_sizes);
				auto ar0 = xscope_ra_section_split1.ra_section_access_requester(0);

				auto& my_foo8_function_ref = J::foo8<decltype(ar0.writelock_ra_section())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar0), my_foo8_function_type>;

				{
					mse::xscope_thread_carrier xscope_threads;
					for (size_t i = 0; i < num_sections; i += 1) {
						auto ar = xscope_ra_section_split1.ra_section_access_requester(i);
						xscope_threads.new_thread(my_foo8_proxy_function_ref, ar, my_foo8_function_ref);
					}
					/* The scope will not end until all the scope threads have finished executing. */
				}
			}

			/* Now that we're done sharing the (controlled access) vector, we can swap it back to our original vector. */
			std::swap(vector1, (*(xscope_ash_access_requester.writelock_ptr())));
			auto first_element_value = vector1[0];
			auto last_element_value = vector1.back();

			int q = 5;
		}
	}
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

