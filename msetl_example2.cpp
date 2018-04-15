
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//define MSE_SAFER_SUBSTITUTES_DISABLED /* This will replace all the classes with their native/standard counterparts. */

#define MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED
#define MSE_SELF_TESTS

//define MSE_MSEVECTOR_USE_MSE_PRIMITIVES 1
//define MSE_MSEARRAY_USE_MSE_PRIMITIVES 1

#include "msetl_example2.h"
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
	template<class _TConstPointer, class _TPointer>
	static void foo18(_TConstPointer src_ptr, _TPointer dst_ptr) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		dst_ptr->s = src_ptr->s;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	/* This function will be used to demonstrate using us::value_from_fparam() to enable template functions to accept scope
	random access sections that reference temporary objects. */
	template<class _TRASection1, class _TRASection2>
	static bool second_is_longer(const _TRASection1& xscope_ra_csection1, const _TRASection2& xscope_ra_csection2) {
		auto l_xscope_ra_csection1 = mse::us::value_from_fparam(xscope_ra_csection1);
		auto l_xscope_ra_csection2 = mse::us::value_from_fparam(xscope_ra_csection2);
		return (l_xscope_ra_csection1.size() > l_xscope_ra_csection2.size()) ? false : true;
	}
};

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

		/* nii_vector<> does not have member functions like "begin(void)" that return "implicit" iterators. It does have
		(static template) member functions like "ss_begin" which take a (safe) pointer to the nii_vector<> as a parameter
		and return a (safe) iterator. */
		auto iter1 = rg_vo1.ss_begin(vo1_regptr1);
		auto citer1 = rg_vo1.ss_cend(vo1_regptr1);
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

		//std::for_each(vo2.begin(), vo2.end(), [](mse::nii_string& ns) { ns.append("z"); });
	}

	{
		/****************/
		/*  optional<>  */
		/****************/

		mse::COptionalTest1::s_test1();
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

		/* Scope types need to publicly inherit from mse::XScopeTagBase. And by convention, be named with a prefix
		indicating that it's a scope type. */
		class xscope_my_type1 : public mse::XScopeTagBase {
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
		from mse::ContainsNonOwningScopeReferenceTagBase. If your type contains or owns any item that can be
		independently targeted by scope pointers (i.e. basically has a '&' ("address of" operator) that yeilds
		a scope pointer), then it must also publicly inherit from mse::ReferenceableByScopePointerTagBase.
		Failure to do so could result in unsafe code. */
		class xscope_my_type2 : public mse::XScopeTagBase, public mse::ContainsNonOwningScopeReferenceTagBase
			, public mse::ReferenceableByScopePointerTagBase
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
			/* In this block we demonstrate the us::TXScopeFParam<> specializations that enable passing temporary objects to
			functions expecting scope random access section arguments. */
			class CD {
			public:
				typedef decltype(mse::make_xscope_random_access_const_section(mse::pointer_to(mse::TXScopeObj<mse::nii_vector<int> >
					(mse::nii_vector<int>{ 1, 2, 3})))) xscope_ra_csection_t;
				static bool second_is_longer(mse::us::TXScopeFParam<xscope_ra_csection_t> xscope_ra_csection1
					, mse::us::TXScopeFParam<xscope_ra_csection_t> xscope_ra_csection2) {

					return (xscope_ra_csection1.size() > xscope_ra_csection2.size()) ? false : true;
				}

				static bool second_is_longer_any(mse::us::TXScopeFParam<mse::TXScopeAnyRandomAccessConstSection<int> > xscope_ra_csection1
					, mse::us::TXScopeFParam<mse::TXScopeAnyRandomAccessConstSection<int> > xscope_ra_csection2) {
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
		str2.compare(str1);
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
		auto nii_str3_xscpiter1 = mse::make_xscope_iterator(&xscp_nii_str3);
		nii_str2.copy(nii_str3_xscpiter1, 5);

		mse::TRegisteredObj<mse::nii_string> reg_nii_str3 = "some text";
		nii_str2.copy(reg_nii_str3.ss_begin(&reg_nii_str3), 5);

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
		auto xscp_citer1 = mse::make_xscope_const_iterator(&xscp_nstring1);
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
			/* In this block we demonstrate the us::TXScopeFParam<> specializations that enable passing temporary objects to
			functions expecting scope string section arguments. */
			class CD {
			public:
				typedef decltype(mse::make_xscope_string_const_section(mse::pointer_to(mse::TXScopeObj<mse::nii_string>
					(mse::nii_string{"abc"})))) xscope_string_csection_t;
				static bool second_is_longer(mse::us::TXScopeFParam<xscope_string_csection_t> xscope_string_csection1
					, mse::us::TXScopeFParam<xscope_string_csection_t> xscope_string_csection2) {

					return (xscope_string_csection1.size() > xscope_string_csection2.size()) ? false : true;
				}

				static bool second_is_longer_any(mse::us::TXScopeFParam<mse::TXScopeAnyStringConstSection<> > xscope_string_csection1
					, mse::us::TXScopeFParam<mse::TXScopeAnyStringConstSection<> > xscope_string_csection2) {
					return (xscope_string_csection1.size() > xscope_string_csection2.size()) ? false : true;
				}

				/*
				static bool second_is_longer_any_ras(mse::us::TXScopeFParam<mse::TXScopeAnyRandomAccessConstSection<char> > xscope_string_csection1
					, mse::us::TXScopeFParam<mse::TXScopeAnyRandomAccessConstSection<char> > xscope_string_csection2) {
					return (xscope_string_csection1.size() > xscope_string_csection2.size()) ? false : true;
				}
				*/
			};

			mse::TXScopeObj<mse::nii_string > string1(mse::nii_string{"abc"});
			auto xscope_string_csection1 = mse::make_xscope_string_const_section(&string1);
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
#ifndef MSE_MSTDSTRING_DISABLED
		try {
			/* This is not undefined (or unsafe) behavior. Either an exception will be thrown or it will just work. */
			auto ch1 = msv1[3];
			assert('e' == ch1);
		}
		catch (...) {
			/* At present, no exception will be thrown. Instead, the lifespan of the string data is extended to match
			that of the mstd::string_view. In the future, an exception may be thrown in debug builds. */
			std::cerr << "potentially expected exception" << std::endl;
		}
#endif //!MSE_MSTDSTRING_DISABLED

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
#ifndef MSE_MSTDSTRING_DISABLED
			/* Memory safe substitutes for std::string and std::string_view eliminate the danger. */

			mse::mstd::string s = "Hellooooooooooooooo ";
			mse::nrp_string_view sv = s + "World\n";
			/* This is safe because the lifespan of the temporary string data is extended (via reference counting) to
			match that of sv. */
			std::cout << sv;
#endif /*!MSE_MSTDSTRING_DISABLED*/
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

			/* And trying to (unsafely) obtain a "scope" pointer from a temporary is not going to work. */
			//auto xscope_pointer2 = &(mse::TXScopeObj< mse::nii_string >(s + "World\n"));	 // <-- compile error
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

			auto A_b_safe_cptr = mse::make_const_pointer_to_member(A_immptr->b, A_immptr);
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
			std::cout << ": xscope_future_carrier<>";
			std::cout << std::endl;

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
			/* TExclusiveWriterObj<> is a specialization of TAccessControlledObj<> for which all non-const pointers are
			exclusive. That is, when a non-const pointer exists, no other pointer may exist. */
			mse::TXScopeObj<mse::TExclusiveWriterObj<ShareableA> > a_xscpacobj1(3);
			mse::TXScopeObj<mse::TExclusiveWriterObj<ShareableA> > a_xscpacobj2(5);
			mse::TXScopeObj<mse::TExclusiveWriterObj<ShareableA> > a_xscpacobj3(7);

			{
				/* TXScopeExclusiveWriterObjPointerStore<> is a data type that stores a (non-const, exclusive) pointer
				of a TExclusiveWriterObj<>. From this data type you can obtain a "scope shareable pointer" which can be
				safely passed to a scope thread. This is a (little) more cumbersome, more restrictive way of sharing an
				object than, say, using the library's "access requesters". So generally using access requesters would be
				preferred. But you might choose to do it this way in certain cases where performance is critical. When
				using access requesters, each thread obtains the desired lock on a thread-safe mutex. Here we're
				obtaining the lock before launching the thread(s), so the mutex does not need to be thread-safe, thus
				saving a little overhead. */
				auto xscope_xwo_pointer_store1 = mse::make_xscope_exclusive_write_obj_pointer_store<ShareableA>(a_xscpacobj1.pointer());

				typedef decltype(xscope_xwo_pointer_store1.xscope_shareable_pointer()) exclusive_pointer_t;
				mse::xscope_thread xscp_thread1(J::foo17<exclusive_pointer_t>, xscope_xwo_pointer_store1.xscope_shareable_pointer());
			}
			{
				auto xscope_xwo_const_pointer_store1 = mse::make_xscope_exclusive_write_obj_const_pointer_store<ShareableA>(a_xscpacobj1.const_pointer());
				auto xscope_xwo_pointer_store2 = mse::make_xscope_exclusive_write_obj_pointer_store<ShareableA>(a_xscpacobj2.pointer());
				auto xscope_xwo_pointer_store3 = mse::make_xscope_exclusive_write_obj_pointer_store<ShareableA>(a_xscpacobj3.pointer());

				typedef decltype(xscope_xwo_const_pointer_store1.xscope_shareable_pointer()) const_pointer_t;
				typedef decltype(xscope_xwo_pointer_store2.xscope_shareable_pointer()) exclusive_pointer_t;

				mse::xscope_thread xscp_thread1(J::foo18<const_pointer_t, exclusive_pointer_t>
					, xscope_xwo_const_pointer_store1.xscope_shareable_pointer()
					, xscope_xwo_pointer_store2.xscope_shareable_pointer());

				mse::xscope_thread xscp_thread2(J::foo18<const_pointer_t, exclusive_pointer_t>
					, xscope_xwo_const_pointer_store1.xscope_shareable_pointer()
					, xscope_xwo_pointer_store3.xscope_shareable_pointer());
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

