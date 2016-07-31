
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTINGOFRELAXEDREGISTERED_H_
#define MSEREFCOUNTINGOFRELAXEDREGISTERED_H_

#include "mserelaxedregistered.h"
#include "mserefcounting.h"
#include <memory>
#include <iostream>
#include <utility>


/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTINGOFRELAXEDREGISTEREDPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_REFCOUNTINGOFRELAXEDREGISTEREDPOINTER_DISABLED
#else /*MSE_REFCOUNTINGOFRELAXEDREGISTEREDPOINTER_DISABLED*/
#endif /*MSE_REFCOUNTINGOFRELAXEDREGISTEREDPOINTER_DISABLED*/

	template<typename _Ty> using TRefCountingOfRelaxedRegisteredPointer = TRefCountingPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRelaxedRegisteredNotNullPointer = TRefCountingNotNullPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRelaxedRegisteredFixedPointer = TRefCountingFixedPointer<TRelaxedRegisteredObj<_Ty>>;

	template<typename _Ty> using TRefCountingOfRelaxedRegisteredConstPointer = TRefCountingConstPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRelaxedRegisteredNotNullConstPointer = TRefCountingNotNullConstPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRelaxedRegisteredFixedConstPointer = TRefCountingFixedConstPointer<TRelaxedRegisteredObj<_Ty>>;

	template <class _Ty, class... Args>
	TRefCountingOfRelaxedRegisteredFixedPointer<_Ty> make_refcountingofrelaxedregistered(Args&&... args) {
		return make_refcounting<TRelaxedRegisteredObj<_Ty>>(std::forward<Args>(args)...);
	}


	class TRefCountingOfRelaxedRegisteredPointer_test {
	public:
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountingOfRelaxedRegisteredPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			TRefCountingOfRelaxedRegisteredPointer_test* m_state_ptr;
			const std::string _id;
		};

		typedef TRefCountingOfRelaxedRegisteredPointer<Trackable> target_t;


#define MTXASSERT_EQ(a, b, c) a &= (b==c)
#define MTXASSERT(a, b) a &= (bool)(b)
		bool testBehaviour()
		{
			bool ok = true;
#ifdef MSE_SELF_TESTS
			static const TRefCountingOfRelaxedRegisteredPointer<Trackable> Nil = target_t(nullptr);

			constructions.clear();
			destructions.clear();

			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t a = make_refcountingofrelaxedregistered<Trackable>(this, "aap");

			MTXASSERT_EQ(ok, 1ul, constructions.size());
			MTXASSERT_EQ(ok, 1, constructions["aap"]);
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			MTXASSERT_EQ(ok, 0, constructions["noot"]);
			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t hold;
			{
				target_t b = make_refcountingofrelaxedregistered<Trackable>(this, "noot"),
					c = make_refcountingofrelaxedregistered<Trackable>(this, "mies"),
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
#endif // MSE_SELF_TESTS

			// ok, enuf for now
			return ok;
		}

		struct Linked : Trackable
		{
			Linked(TRefCountingOfRelaxedRegisteredPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
			TRefCountingOfRelaxedRegisteredPointer<Linked> next;
		};

		bool testLinked()
		{
			bool ok = true;

#ifdef MSE_SELF_TESTS
			constructions.clear();
			destructions.clear();
			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			TRefCountingOfRelaxedRegisteredPointer<Linked> node = make_refcountingofrelaxedregistered<Linked>(this, "parent");
			MTXASSERT(ok, (node != nullptr));
			node->next = make_refcountingofrelaxedregistered<Linked>(this, "child");

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			node = node->next;
			MTXASSERT(ok, (node != nullptr));

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 1ul, destructions.size());

			node = node->next;
			MTXASSERT(ok, (node == nullptr));

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 2ul, destructions.size());
#endif // MSE_SELF_TESTS

			return ok;
		}

		void test1() {
#ifdef MSE_SELF_TESTS
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
				static int foo2(mse::TRefCountingOfRelaxedRegisteredPointer<A> A_refcountingofrelaxedregistered_ptr) { return A_refcountingofrelaxedregistered_ptr->b; }
			protected:
				~B() {}
			};

			A* A_native_ptr = nullptr;
			/* mse::TRefCountingOfRelaxedRegisteredPointer<> is basically a slightly "safer" version of std::shared_ptr. */
			mse::TRefCountingOfRelaxedRegisteredPointer<A> A_refcountingofrelaxedregistered_ptr1;

			{
				A a;

				A_native_ptr = &a;
				A_refcountingofrelaxedregistered_ptr1 = mse::make_refcountingofrelaxedregistered<A>();
				assert(A_native_ptr->b == A_refcountingofrelaxedregistered_ptr1->b);

				mse::TRefCountingOfRelaxedRegisteredPointer<A> A_refcountingofrelaxedregistered_ptr2 = A_refcountingofrelaxedregistered_ptr1;
				A_refcountingofrelaxedregistered_ptr2 = nullptr;
#ifndef MSE_REFCOUNTINGPOINTER_DISABLED
				bool expected_exception = false;
				try {
					int i = A_refcountingofrelaxedregistered_ptr2->b; /* this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "refcountingofrelaxedregistered pointer". */
				}
				assert(expected_exception);
#endif // !MSE_REFCOUNTINGPOINTER_DISABLED

				auto relaxedregisteredfixedpointer1 = (&(*A_refcountingofrelaxedregistered_ptr1));
				B::foo1((A*)relaxedregisteredfixedpointer1);

				if (A_refcountingofrelaxedregistered_ptr2) {
				}
				else if (A_refcountingofrelaxedregistered_ptr2 != A_refcountingofrelaxedregistered_ptr1) {
					A_refcountingofrelaxedregistered_ptr2 = A_refcountingofrelaxedregistered_ptr1;
					assert(A_refcountingofrelaxedregistered_ptr2 == A_refcountingofrelaxedregistered_ptr1);
				}

				mse::TRefCountingOfRelaxedRegisteredConstPointer<A> rcp = A_refcountingofrelaxedregistered_ptr1;
				mse::TRefCountingOfRelaxedRegisteredConstPointer<A> rcp2 = rcp;
				rcp = mse::make_refcountingofrelaxedregistered<A>();
				mse::TRefCountingOfRelaxedRegisteredFixedConstPointer<A> rfcp = mse::make_refcountingofrelaxedregistered<A>();
				{
					int i = rfcp->b;
				}
			}

			int i = A_refcountingofrelaxedregistered_ptr1->b;
#endif // MSE_SELF_TESTS
		}

	};
}

#endif // MSEREFCOUNTINGOFRELAXEDREGISTERED_H_
