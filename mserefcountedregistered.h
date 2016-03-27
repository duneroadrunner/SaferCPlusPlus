
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTEDREGISTERED_H_
#define MSEREFCOUNTEDREGISTERED_H_

#include "mseregistered.h"
#include "mserefcounted.h"
#include <memory>
#include <iostream>
#include <utility>


/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTEDREGISTEREDPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_REFCOUNTEDREGISTEREDPOINTER_DISABLED
#else /*MSE_REFCOUNTEDREGISTEREDPOINTER_DISABLED*/
#endif /*MSE_REFCOUNTEDREGISTEREDPOINTER_DISABLED*/

	template<typename _Ty> using TRefCountedRegisteredPointer = TRefCountedPointer<TRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRegisteredNotNullPointer = TRefCountedNotNullPointer<TRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRegisteredFixedPointer = TRefCountedFixedPointer<TRegisteredObj<_Ty>>;

	template<typename _Ty> using TRefCountedRegisteredConstPointer = TRefCountedConstPointer<TRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRegisteredNotNullConstPointer = TRefCountedNotNullConstPointer<TRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRegisteredFixedConstPointer = TRefCountedFixedConstPointer<TRegisteredObj<_Ty>>;

	template <class _Ty, class... Args>
	TRefCountedRegisteredFixedPointer<_Ty> make_refcountedregistered(Args&&... args) {
		return TRefCountedFixedPointer<TRegisteredObj<_Ty>>::make_refcounted(args...);
	}


	class TRefCountedRegisteredPointer_test {
	public:
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountedRegisteredPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			const std::string _id;
			TRefCountedRegisteredPointer_test* m_state_ptr;
		};

		typedef TRefCountedPointer<Trackable> target_t;


#define MTXASSERT_EQ(a, b, c) a &= (b==c)
#define MTXASSERT(a, b) a &= (bool)(b)
		bool testBehaviour()
		{
			static const TRefCountedPointer<Trackable> Nil = target_t(nullptr);
			bool ok = true;

			constructions.clear();
			destructions.clear();

			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t a = make_refcounted<Trackable>(this, "aap");

			MTXASSERT_EQ(ok, 1ul, constructions.size());
			MTXASSERT_EQ(ok, 1, constructions["aap"]);
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			MTXASSERT_EQ(ok, 0, constructions["noot"]);
			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t hold;
			{
				target_t b = make_refcounted<Trackable>(this, "noot"),
					c = make_refcounted<Trackable>(this, "mies"),
					nil = Nil,
					a2 = a;

				MTXASSERT(ok, a2 == a);
				MTXASSERT(ok, nil != a);

				MTXASSERT_EQ(ok, 3ul, constructions.size());
				MTXASSERT_EQ(ok, 1, constructions["aap"]);
				MTXASSERT_EQ(ok, 1, constructions["noot"]);
				MTXASSERT_EQ(ok, 1, constructions["mies"]);
				MTXASSERT_EQ(ok, 0, constructions["broer"]);
				MTXASSERT_EQ(ok, 4ul, constructions.size());

				MTXASSERT_EQ(ok, 0ul, destructions.size());

				hold = b;
			}

			MTXASSERT_EQ(ok, 1ul, destructions.size());
			MTXASSERT_EQ(ok, 0, destructions["aap"]);
			MTXASSERT_EQ(ok, 0, destructions["noot"]);
			MTXASSERT_EQ(ok, 1, destructions["mies"]);
			MTXASSERT_EQ(ok, 3ul, destructions.size());

			hold = Nil;
			MTXASSERT_EQ(ok, 3ul, destructions.size());
			MTXASSERT_EQ(ok, 0, destructions["aap"]);
			MTXASSERT_EQ(ok, 1, destructions["noot"]);
			MTXASSERT_EQ(ok, 1, destructions["mies"]);
			MTXASSERT_EQ(ok, 4ul, constructions.size());

			// ok, enuf for now
			return ok;
		}

		struct Linked : Trackable
		{
			Linked(TRefCountedRegisteredPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
			TRefCountedPointer<Linked> next;
		};

		bool testLinked()
		{
			bool ok = true;

			constructions.clear();
			destructions.clear();
			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			TRefCountedPointer<Linked> node = make_refcounted<Linked>(this, "parent");
			MTXASSERT(ok, node.get());
			node->next = make_refcounted<Linked>(this, "child");

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			node = node->next;
			MTXASSERT(ok, node.get());

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 1ul, destructions.size());

			node = node->next;
			MTXASSERT(ok, !node.get());

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 2ul, destructions.size());

			return ok;
		}

		void test1() {
			class A {
			public:
				A() {}
				A(const A& _X) : b(_X.b) {}
				A(A&& _X) : b(std::move(_X.b)) {}
				virtual ~A() {}
				A& operator=(A&& _X) { b = std::move(_X.b); return (*this); }
				A& operator=(const A& _X) { b = _X.b; return (*this); }

				int b = 3;
			};
			class B {
			public:
				static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
				static int foo2(mse::TRefCountedRegisteredPointer<A> A_refcountedregistered_ptr) { return A_refcountedregistered_ptr->b; }
			protected:
				~B() {}
			};

			A* A_native_ptr = nullptr;
			/* mse::TRefCountedRegisteredPointer<> is basically a slightly "safer" version of std::shared_ptr. */
			mse::TRefCountedRegisteredPointer<A> A_refcountedregistered_ptr1;

			{
				A a;

				A_native_ptr = &a;
				A_refcountedregistered_ptr1 = mse::make_refcountedregistered<A>();
				assert(A_native_ptr->b == A_refcountedregistered_ptr1->b);

				mse::TRefCountedRegisteredPointer<A> A_refcountedregistered_ptr2 = A_refcountedregistered_ptr1;
				A_refcountedregistered_ptr2 = nullptr;
				bool expected_exception = false;
				try {
					int i = A_refcountedregistered_ptr2->b; /* this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "refcountedregistered pointer". */
				}
				assert(expected_exception);

				auto registeredfixedpointer1 = (&(*A_refcountedregistered_ptr1));
				B::foo1((A*)registeredfixedpointer1);

				if (A_refcountedregistered_ptr2) {
				}
				else if (A_refcountedregistered_ptr2 != A_refcountedregistered_ptr1) {
					A_refcountedregistered_ptr2 = A_refcountedregistered_ptr1;
					assert(A_refcountedregistered_ptr2 == A_refcountedregistered_ptr1);
				}

				mse::TRefCountedRegisteredConstPointer<A> rcp = A_refcountedregistered_ptr1;
				mse::TRefCountedRegisteredConstPointer<A> rcp2 = rcp;
				rcp = mse::make_refcountedregistered<A>();
				mse::TRefCountedRegisteredFixedConstPointer<A> rfcp = mse::make_refcountedregistered<A>();
				{
					int i = rfcp->b;
				}
			}

			int i = A_refcountedregistered_ptr1->b;
		}

	};
}

#endif // MSEREFCOUNTEDREGISTERED_H_
