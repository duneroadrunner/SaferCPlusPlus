
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


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
		(template) member functions like "ss_begin" which take a (safe) pointer to the nii_vector<> as a parameter and
		return a (safe) iterator. */
		auto iter1 = rg_vo1.ss_begin(vo1_regptr1);
		auto citer1 = rg_vo1.ss_cend(vo1_regptr1);
		citer1 = iter1;
		rg_vo1.emplace(citer1, "some other text");
		rg_vo1.insert(citer1, "some other text");
		mse::nii_string str1 = "some other text";
		rg_vo1.insert(citer1, str1);

		class A {
		public:
			A() {}
			int m_i;
		};
		/* Here we're declaring that A can be safely shared between asynchronous threads. */
		typedef mse::TUserDeclaredAsyncShareableObj<A> shareable_A_t;

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
		class CA {
		public:
			std::string m_s1;
		};
		mse::TXScopeObj<CA> xscp_obj1;
		mse::TXScopeItemFixedPointer<CA> xscp_ifptr1 = &xscp_obj1;
		mse::TXScopeItemFixedPointer<std::string> xscp_ifptr2 = mse::make_xscope_pointer_to_member(xscp_obj1.m_s1, &xscp_obj1);
		mse::TXScopePolyPointer<CA> xscp_polyptr1 = xscp_ifptr1;
		mse::TXScopePolyPointer<std::string> xscp_polyptr2 = xscp_ifptr2;

		auto refc_ptr1 = mse::make_refcounting<std::string>("some text");
		mse::TXScopeRefCountingConstStore<std::string> xscp_refc_cstore(refc_ptr1);
		auto xscp_cptr1 = xscp_refc_cstore.xscope_ptr();
		mse::TXScopeItemFixedConstPointer<std::string> xscp_cptr2 = xscp_cptr1;
		std::string res1 = *xscp_cptr2;
		mse::TXScopeRefCountingNotNullStore<std::string> xscp_refcnn_store(refc_ptr1);
		auto xscp_ptr3 = xscp_refcnn_store.xscope_ptr();
		(*xscp_ptr3) = "some other text";
		mse::TXScopeItemFixedConstPointer<std::string> xscp_cptr4 = xscp_ptr3;
		std::string res2 = *xscp_cptr4;

		const auto& min_pointer_indicator = [](const decltype(xscp_cptr2)& a, const decltype(xscp_cptr4)& b) { return ((*b) < (*a)); };
		auto res5 = mse::xscope_chosen_pointer(min_pointer_indicator, xscp_cptr2, xscp_cptr4);

		mse::TXScopeObj<std::string> xscp_obj2;
		auto xscp_fptr1 = &xscp_obj2;
		auto xscp_fptr2 = &xscp_obj2;
		const auto& bool_min_function2_cref = [](const decltype(xscp_fptr1)& a, const decltype(xscp_fptr2)& b, bool c) { return !(((*a) < (*b)) || c); };
		auto res6 = mse::xscope_chosen_pointer(bool_min_function2_cref, xscp_fptr1, xscp_fptr2, false);

		mse::nii_vector<std::string> niiv1;
		mse::nii_vector<mse::TRegisteredObj<std::string> > niiv2;
		//mse::nii_vector<mse::TXScopeObj<std::string> > niiv3;

		mse::nii_array<std::string, 5> niiar1;
		mse::nii_array<mse::TRegisteredObj<std::string>, 5> niiar2;
		//mse::nii_array<mse::TXScopeObj<std::string>, 5> niiar3;

		mse::TXScopeObj<mse::nii_array<std::string, 5> > xscp_nii_ar1 = mse::nii_array<std::string, 5>({ "0", "1", "2", "3", "4"});
		auto nii_ar_iter1 = mse::make_xscope_ss_iterator_type(&xscp_nii_ar1);
		nii_ar_iter1 += 2;
		auto xscp_nii_ar_ifptr = mse::xscope_pointer_to_array_element<std::string, 5>(nii_ar_iter1);
		auto nii_ar_res1 = (*xscp_nii_ar_ifptr);

		mse::TXScopeObj<mse::msearray<std::string, 5> > xscp_msear1 = mse::msearray<std::string, 5>({ "0", "1", "2", "3", "4" });
		auto msear_iter1 = mse::make_xscope_ss_iterator_type(&xscp_msear1);
		msear_iter1 += 2;
		auto xscp_msear_ifptr = mse::xscope_pointer_to_array_element<std::string, 5>(msear_iter1);
		auto msear_res1 = (*xscp_msear_ifptr);

		mse::TXScopeObj<mse::mstd::array<std::string, 5> > xscp_mstd_ar1 = mse::mstd::array<std::string, 5>({ "0", "1", "2", "3", "4" });
		auto mstd_ar_iter1 = mse::mstd::make_xscope_iterator(&xscp_mstd_ar1);
		mstd_ar_iter1 = xscp_mstd_ar1.begin();
		mstd_ar_iter1 += 2;
		auto xscp_mstd_ar_ifptr = mse::mstd::xscope_pointer_to_array_element<std::string, 5>(mstd_ar_iter1);
		auto mstd_ar_res1 = (*xscp_mstd_ar_ifptr);

		mse::mstd::vector<std::string> mstdv1;
		mstdv1.swap(niiv1);
		bool b1 = (mstdv1 == mstdv1);
		std::swap(niiv1, mstdv1);
		std::swap(mstdv1, niiv1);
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

