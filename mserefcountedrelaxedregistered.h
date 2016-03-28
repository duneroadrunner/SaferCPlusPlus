
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTEDRELAXEDREGISTERED_H_
#define MSEREFCOUNTEDRELAXEDREGISTERED_H_

#include "mserelaxedregistered.h"
#include "mserefcounted.h"
#include <memory>
#include <iostream>
#include <utility>


/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTEDRELAXEDREGISTEREDPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_REFCOUNTEDRELAXEDREGISTEREDPOINTER_DISABLED
#else /*MSE_REFCOUNTEDRELAXEDREGISTEREDPOINTER_DISABLED*/
#endif /*MSE_REFCOUNTEDRELAXEDREGISTEREDPOINTER_DISABLED*/

	template<typename _Ty> using TRefCountedRelaxedRegisteredPointer = TRefCountedPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRelaxedRegisteredNotNullPointer = TRefCountedNotNullPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRelaxedRegisteredFixedPointer = TRefCountedFixedPointer<TRelaxedRegisteredObj<_Ty>>;

	template<typename _Ty> using TRefCountedRelaxedRegisteredConstPointer = TRefCountedConstPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRelaxedRegisteredNotNullConstPointer = TRefCountedNotNullConstPointer<TRelaxedRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountedRelaxedRegisteredFixedConstPointer = TRefCountedFixedConstPointer<TRelaxedRegisteredObj<_Ty>>;

	template <class _Ty, class... Args>
	TRefCountedRelaxedRegisteredFixedPointer<_Ty> make_refcountedrelaxedregistered(Args&&... args) {
		return TRefCountedFixedPointer<TRelaxedRegisteredObj<_Ty>>::make_refcounted(args...);
	}


	class TRefCountedRelaxedRegisteredPointer_test {
	public:
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountedRelaxedRegisteredPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			const std::string _id;
			TRefCountedRelaxedRegisteredPointer_test* m_state_ptr;
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
			Linked(TRefCountedRelaxedRegisteredPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
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
				static int foo2(mse::TRefCountedRelaxedRegisteredPointer<A> A_refcountedrelaxedregistered_ptr) { return A_refcountedrelaxedregistered_ptr->b; }
			protected:
				~B() {}
			};

			A* A_native_ptr = nullptr;
			/* mse::TRefCountedRelaxedRegisteredPointer<> is basically a slightly "safer" version of std::shared_ptr. */
			mse::TRefCountedRelaxedRegisteredPointer<A> A_refcountedrelaxedregistered_ptr1;

			{
				A a;

				A_native_ptr = &a;
				A_refcountedrelaxedregistered_ptr1 = mse::make_refcountedrelaxedregistered<A>();
				assert(A_native_ptr->b == A_refcountedrelaxedregistered_ptr1->b);

				mse::TRefCountedRelaxedRegisteredPointer<A> A_refcountedrelaxedregistered_ptr2 = A_refcountedrelaxedregistered_ptr1;
				A_refcountedrelaxedregistered_ptr2 = nullptr;
				bool expected_exception = false;
				try {
					int i = A_refcountedrelaxedregistered_ptr2->b; /* this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "refcountedrelaxedregistered pointer". */
				}
				assert(expected_exception);

				auto relaxedregisteredfixedpointer1 = (&(*A_refcountedrelaxedregistered_ptr1));
				B::foo1((A*)relaxedregisteredfixedpointer1);

				if (A_refcountedrelaxedregistered_ptr2) {
				}
				else if (A_refcountedrelaxedregistered_ptr2 != A_refcountedrelaxedregistered_ptr1) {
					A_refcountedrelaxedregistered_ptr2 = A_refcountedrelaxedregistered_ptr1;
					assert(A_refcountedrelaxedregistered_ptr2 == A_refcountedrelaxedregistered_ptr1);
				}

				mse::TRefCountedRelaxedRegisteredConstPointer<A> rcp = A_refcountedrelaxedregistered_ptr1;
				mse::TRefCountedRelaxedRegisteredConstPointer<A> rcp2 = rcp;
				rcp = mse::make_refcountedrelaxedregistered<A>();
				mse::TRefCountedRelaxedRegisteredFixedConstPointer<A> rfcp = mse::make_refcountedrelaxedregistered<A>();
				{
					int i = rfcp->b;
				}
			}

			int i = A_refcountedrelaxedregistered_ptr1->b;
		}

	};
}

#endif // MSEREFCOUNTEDRELAXEDREGISTERED_H_
