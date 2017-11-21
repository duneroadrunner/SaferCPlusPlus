
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
	template<class _TStringRAContainerPtr>
	static void foo8(_TStringRAContainerPtr ra_container_ptr) {
		size_t delay_in_milliseconds = 3000/*arbitrary*/;
		if (1 <= (*ra_container_ptr).size()) {
			delay_in_milliseconds /= (*ra_container_ptr).size();
		}
		for (size_t i = 0; i < (*ra_container_ptr).size(); i += 1) {
			auto now1 = std::chrono::system_clock::now();
			auto tt = std::chrono::system_clock::to_time_t(now1);

#ifdef _MSC_VER
			static const size_t buffer_size = 64;
			char buffer[buffer_size];
			buffer[0] = '\0';
			ctime_s(buffer, buffer_size, &tt);
#else /*_MSC_VER*/
			auto buffer = ctime(&tt);
#endif /*_MSC_VER*/

			std::string now_str(buffer);
			(*ra_container_ptr)[i] = now_str;

			std::this_thread::sleep_for(std::chrono::milliseconds(delay_in_milliseconds));
		}
	}
	/* This function just obtains a writelock_ptr from the given access requester and calls the given function with the
	writelock_ptr as the first argument. This can be convenient when you want to call a function asychronously with a
	shared object. */
	template<class _TAsyncSharedReadWriteAccessRequester, class _TFunction, class... Args>
	static void invoke_with_writelock_ptr1(_TAsyncSharedReadWriteAccessRequester ar, _TFunction function1, Args&&... args) {
		function1(ar.writelock_ptr(), args...);
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
		auto nii_ar_iter1 = mse::make_xscope_iterator(&xscp_nii_ar1);
		nii_ar_iter1 += 2;
		auto xscp_nii_ar_ifptr = mse::xscope_pointer_to_array_element<std::string, 5>(nii_ar_iter1);
		auto nii_ar_res1 = (*xscp_nii_ar_ifptr);

		mse::TXScopeObj<mse::msearray<std::string, 5> > xscp_msear1 = mse::msearray<std::string, 5>({ "0", "1", "2", "3", "4" });
		auto msear_iter1 = mse::make_xscope_iterator(&xscp_msear1);
		msear_iter1 += 2;
		auto xscp_msear_ifptr = mse::xscope_pointer_to_array_element<std::string, 5>(msear_iter1);
		auto msear_res1 = (*xscp_msear_ifptr);

		mse::TXScopeObj<mse::mstd::array<std::string, 5> > xscp_mstd_ar1 = mse::mstd::array<std::string, 5>({ "0", "1", "2", "3", "4" });
		auto mstd_ar_iter1 = mse::make_xscope_iterator(&xscp_mstd_ar1);
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
		typedef mse::TUserDeclaredAsyncShareableObj<A> ShareableA;

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
		{
			/* This block demonstrates safely allowing different threads to (simultaneously) modify different
			sections of an array. */

			static const size_t num_sections = 10;
			static const size_t section_size = 5;
			const size_t num_elements = num_sections * section_size;

			typedef mse::nii_array<mse::nii_string, num_elements> async_shareable_array1_t;
			typedef mse::mstd::array<mse::nii_string, num_elements> nonshareable_array1_t;
			/* Let's say we have an array. */
			nonshareable_array1_t array1;
			{
				size_t count = 0;
				for (auto& item_ref : array1) {
					count += 1;
					item_ref = "text" + std::to_string(count);
				}
			}

			/* Only access controlled objects can be shared with other threads, so we'll make an access controlled array and
			(temporarily) swap it with our original one. */
			auto ash_access_requester = mse::make_asyncsharedv2readwrite<async_shareable_array1_t>();
			std::swap(array1, (*(ash_access_requester.writelock_ptr())));

			{
				/* Now, we're going to use the access requester to obtain two new access requesters that provide access to
				(newly created) "random access section" objects which are used to access (disjoint) sections of the array.
				We need to specify the position where we want to split the array. Here we specify that it be split at index
				"num_elements / 2", right down the middle. */
				auto ra_rection_split1 = mse::TAsyncRASectionSplitter<decltype(ash_access_requester)>(ash_access_requester, num_elements / 2);
				auto ar1 = ra_rection_split1.first_ra_section_access_requester();
				auto ar2 = ra_rection_split1.second_ra_section_access_requester();

				/* The J::foo8 template function is just an example function that operates on containers of strings. In our case the
				containers will be the random access sections we just created. We'll create an instance of the function here. */
				auto& my_foo8_function_ref = J::foo8<decltype(ar1.writelock_ptr())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;

				/* We want to execute the my_foo8 function in a separate thread. The function takes a pointer to a random access
				section as an argument. But as we're not allowed to pass pointers between threads, we must pass an access requester
				instead. The J::invoke_with_writelock_ptr1 template function is just a helper function that will obtain a (writelock)
				pointer from the access requester, then call the given function, in this case my_foo8, with that pointer. So here
				we'll use it to create a proxy function that we can execute directly in a separate thread and will accept an access
				requester as a parameter. */
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ptr1<decltype(ar1), my_foo8_function_type>;

				std::list<std::thread> threads;
				/* So this thread will modify the first section of the array. */
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
				/* Ok, now let's do it again, but instead of splitting the array into two sections, let's split it into more sections: */
				/* First we create a list of a the sizes of each section. We'll use a vector here, but any iteratable container will work. */
				mse::mstd::vector<size_t> section_sizes;
				for (size_t i = 0; i < num_sections; i += 1) {
					section_sizes.push_back(section_size);
				}

				/* Just as before, TAsyncRASectionSplitter<> will generate a new access requester for each section. */
				auto ra_rection_split1 = mse::TAsyncRASectionSplitter<decltype(ash_access_requester)>(ash_access_requester, section_sizes);
				auto ar0 = ra_rection_split1.ra_section_access_requester(0);

				auto& my_foo8_function_ref = J::foo8<decltype(ar0.writelock_ptr())>;
				typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;
				auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ptr1<decltype(ar0), my_foo8_function_type>;

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

			/* Now that we're done sharing the (controlled access) array, we can swap it back to our original array. */
			std::swap(array1, (*(ash_access_requester.writelock_ptr())));
			auto first_element_value = array1[0];
			auto last_element_value = array1.back();

			int q = 5;
		}
		{
			mse::mstd::string mstd_string1;
			mse::TUserDeclaredAsyncShareableObj<std::string> aso1;
			//mse::TUserDeclaredAsyncShareableObj<mse::msearray<std::string, 3>> aso2;
			mse::TUserDeclaredAsyncShareableObj<mse::nii_array<std::string, 3>> aso3;

			mse::TAccessControlledReadWriteObj<mse::TUserDeclaredAsyncShareableObj<std::string>> aco1;
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

