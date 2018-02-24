
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED
#define MSE_SELF_TESTS

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
			delay_in_milliseconds /= ra_section.size();
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
		vo2.swap(*(access_requester2.writelock_ptr()));

		int q = 5;
	}

	{
		/****************/
		/*  optional<>  */
		/****************/

		mse::COptionalTest1::s_test1();
	}

	{
		/*************************/
		/*  TXScopeReturnable<>  */
		/*************************/

		class CB {
		public:
			/* While there is a rule against using scope types as function return types, you can usually just use the
			underlying (non-scope) type of the scope object as the return type. */
			static mse::mstd::string foo1() {
				mse::TXScopeObj<mse::mstd::string> xscp_string1("some text");
				return xscp_string1;
			}

			/* In the less common case where the scope type doesn't have an underlying non-scope type, it may be safe
			to return the scope object. But in order to use a scope type as a function return value, it must be
			wrapped in the transparent mse::TXScopeReturnable<> wrapper template, which will induce a compile error
			if it deems the scope type potentially unsafe to use as a return type. */
			static mse::TXScopeReturnable<mse::xscope_optional<mse::mstd::string> > foo2() {
				mse::xscope_optional<mse::mstd::string> xscp_returnable_obj1(mse::mstd::string("some text"));
				return xscp_returnable_obj1;
			}
		};

		mse::TXScopeObj<mse::mstd::string> xscp_res1(CB::foo1());
		mse::xscope_optional<mse::mstd::string> xscp_res2(CB::foo2());

		typedef mse::TXScopeObj<mse::mstd::string> xscope_string_t;
		xscope_string_t xscp_str1 = "some text";
		/* TXScopeReturnable<> deems xscope_string_t to be an acceptable return type because it doesn't contain
		any scope pointers. */
		mse::TXScopeReturnable<xscope_string_t> xscpr_str1("some text");

		typedef decltype(&xscp_str1) xscope_string_ptr_t;
		/* TXScopeReturnable<> deems xscope_string_ptr_t to be an unsafe return type because it is (or contains)
		a scope pointer. So the next line would result in a compile error. */
		//mse::TXScopeReturnable<xscope_string_ptr_t> xscpr_sfptr1 = &xscp_str1;
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
		assert(mse::mstd::string(string_section2) == "tex");

		/* Unlike std::string_view, string sections are available in "non-const" versions. */
		string_section2[0] = 'T';
		std::cout << string_section2;
		assert(mstring1 == "some Text");
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
		assert(mse::mstd::string(any_string_section1) == "tex");
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
			msv1 = mstring1;
		}
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

		mse::mstd::string mstring2("some other text");
		/* With std::string_view, you specify a string subrange with a raw pointer iterator and a length. With
		mse::mstd::string_view you are not restricted to (unsafe) raw pointer iterators. You can use memory safe
		iterators like those provided by mse::mstd::string. */
		auto msv2 = mse::mstd::string_view(mstring2.cbegin()+5, 7);
		assert(msv2 == "other t");
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
				futures.emplace_back(std::async(B::foo1, ash_access_requester));
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
				futures.emplace_back(std::async(J::foo7<mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA>>, ash_access_requester));
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
				futures.emplace_back(std::async(J::foo7<mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA>>, ash_access_requester));
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
				futures.emplace_back(std::async(J::foo7<mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA>>, ash_access_requester));
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
				futures.emplace_back(std::async(B::foo2, A_immptr));
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
				auto ra_rection_split1 = mse::TAsyncRASectionSplitter<decltype(ash_access_requester)>(ash_access_requester, num_elements / 2);
				auto ar1 = ra_rection_split1.first_ra_section_access_requester();
				auto ar2 = ra_rection_split1.second_ra_section_access_requester();

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

				std::list<std::thread> threads;
				/* So this thread will modify the first section of the vector. */
				threads.emplace_back(std::thread(my_foo8_proxy_function_ref, ar1, my_foo8_function_ref));
				/* While this thread modifies the other section. */
				threads.emplace_back(std::thread(my_foo8_proxy_function_ref, ar2, my_foo8_function_ref));

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
				auto ra_rection_split1 = mse::TAsyncRASectionSplitter<decltype(ash_access_requester)>(ash_access_requester, section_sizes);
				auto ar0 = ra_rection_split1.ra_section_access_requester(0);

				auto& my_foo8_function_ref = J::foo8<decltype(ar0.writelock_ra_section())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar0), my_foo8_function_type>;

				std::list<std::thread> threads;
				for (size_t i = 0; i < num_sections; i += 1) {
					auto ar = ra_rection_split1.ra_section_access_requester(i);
					threads.emplace_back(std::thread(my_foo8_proxy_function_ref, ar, my_foo8_function_ref));
				}

				{
					int count = 1;
					for (auto it = threads.begin(); threads.end() != it; it++, count++) {
						(*it).join();
					}
				}
			}

			/* Now that we're done sharing the (controlled access) vector, we can swap it back to our original vector. */
			std::swap(vector1, (*(ash_access_requester.writelock_ptr())));
			auto first_element_value = vector1[0];
			auto last_element_value = vector1.back();

			int q = 5;
		}
		{
			mse::mstd::string mstd_string1;
			mse::us::TUserDeclaredAsyncShareableObj<std::string> aso1;
			//mse::us::TUserDeclaredAsyncShareableObj<mse::us::msearray<std::string, 3>> aso2;
			mse::us::TUserDeclaredAsyncShareableObj<mse::nii_array<std::string, 3>> aso3;

			mse::TAccessControlledReadWriteObj<mse::us::TUserDeclaredAsyncShareableObj<std::string>> aco1;
			auto asxwp_ar = mse::make_asyncsharedv2xwpreadwrite(aco1.exclusive_writelock_ptr());
			mse::TAccessControlledReadWriteObj<mse::nii_array<mse::nii_string, 3>> aco2;
			//mse::TAccessControlledReadWriteObj<mse::nii_array<std::string, 3>> aco3;

			auto as_ar = mse::make_asyncsharedv2readwrite<mse::nii_string>("some text");
			//auto as_ar2 = mse::make_asyncsharedv2readwrite<std::string>("some text");
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

