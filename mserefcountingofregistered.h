
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* This file and its elements are deprecated. */

#pragma once
#ifndef MSEREFCOUNTINGOFREGISTERED_H_
#define MSEREFCOUNTINGOFREGISTERED_H_

#include "mseregistered.h"
#include "mserefcounting.h"

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4996 )
#endif /*_MSC_VER*/


#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTINGOFREGISTEREDPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_REFCOUNTINGOFREGISTEREDPOINTER_DISABLED
#else /*MSE_REFCOUNTINGOFREGISTEREDPOINTER_DISABLED*/
#endif /*MSE_REFCOUNTINGOFREGISTEREDPOINTER_DISABLED*/

	template<typename _Ty> using TRefCountingOfRegisteredPointer MSE_DEPRECATED = TRefCountingPointer<TNDRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRegisteredNotNullPointer MSE_DEPRECATED = TRefCountingNotNullPointer<TNDRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRegisteredFixedPointer MSE_DEPRECATED = TRefCountingFixedPointer<TNDRegisteredObj<_Ty>>;

	template<typename _Ty> using TRefCountingOfRegisteredConstPointer MSE_DEPRECATED = TRefCountingConstPointer<TNDRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRegisteredNotNullConstPointer MSE_DEPRECATED = TRefCountingNotNullConstPointer<TNDRegisteredObj<_Ty>>;
	template<typename _Ty> using TRefCountingOfRegisteredFixedConstPointer MSE_DEPRECATED = TRefCountingFixedConstPointer<TNDRegisteredObj<_Ty>>;

	template <class _Ty, class... Args>
	MSE_DEPRECATED TRefCountingOfRegisteredFixedPointer<_Ty> make_refcountingofregistered(Args&&... args) {
		return make_refcounting<TNDRegisteredObj<_Ty>>(std::forward<Args>(args)...);
	}


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class TRefCountingOfRegisteredPointer_test {
	public:
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountingOfRegisteredPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			TRefCountingOfRegisteredPointer_test* m_state_ptr;
			const std::string _id;
		};

		typedef TRefCountingOfRegisteredPointer<Trackable> target_t;


#define MTXASSERT_EQ(a, b, c) a &= (b==c)
#define MTXASSERT(a, b) a &= static_cast<bool>(b)
		bool testBehaviour()
		{
			bool ok = true;
#ifdef MSE_SELF_TESTS
			static const TRefCountingOfRegisteredPointer<Trackable> Nil = target_t(nullptr);

			constructions.clear();
			destructions.clear();

			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t a = make_refcountingofregistered<Trackable>(this, "aap");

			MTXASSERT_EQ(ok, 1ul, constructions.size());
			MTXASSERT_EQ(ok, 1, constructions["aap"]);
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			MTXASSERT_EQ(ok, 0, constructions["noot"]);
			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t hold;
			{
				target_t b = make_refcountingofregistered<Trackable>(this, "noot"),
					c = make_refcountingofregistered<Trackable>(this, "mies"),
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
			Linked(TRefCountingOfRegisteredPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
			TRefCountingOfRegisteredPointer<Linked> next;
		};

		bool testLinked()
		{
			bool ok = true;

#ifdef MSE_SELF_TESTS
			constructions.clear();
			destructions.clear();
			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			TRefCountingOfRegisteredPointer<Linked> node = make_refcountingofregistered<Linked>(this, "parent");
			MTXASSERT(ok, (node != nullptr));
			node->next = make_refcountingofregistered<Linked>(this, "child");

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
				A(A&& _X) : b(std::forward<decltype(_X)>(_X).b) {}
				virtual ~A() {}
				A& operator=(A&& _X) { b = std::forward<decltype(_X)>(_X).b; return (*this); }
				A& operator=(const A& _X) { b = _X.b; return (*this); }

				int b = 3;
			};
			class B {
			public:
				static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
				static int foo2(mse::TRefCountingOfRegisteredPointer<A> A_refcountingofregistered_ptr) { return A_refcountingofregistered_ptr->b; }
			protected:
				~B() {}
			};

			A* A_native_ptr = nullptr;
			/* mse::TRefCountingOfRegisteredPointer<> is basically a slightly "safer" version of std::shared_ptr. */
			mse::TRefCountingOfRegisteredPointer<A> A_refcountingofregistered_ptr1;

			{
				A a;

				A_native_ptr = &a;
				A_refcountingofregistered_ptr1 = mse::make_refcountingofregistered<A>();
				assert(A_native_ptr->b == A_refcountingofregistered_ptr1->b);

				mse::TRefCountingOfRegisteredPointer<A> A_refcountingofregistered_ptr2 = A_refcountingofregistered_ptr1;
				A_refcountingofregistered_ptr2 = nullptr;
#ifndef MSE_REFCOUNTINGPOINTER_DISABLED
				bool expected_exception = false;
				MSE_TRY {
					int i = A_refcountingofregistered_ptr2->b; /* this is gonna throw an exception */
				}
				MSE_CATCH_ANY {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "refcountingofregistered pointer". */
				}
				assert(expected_exception);
#endif // !MSE_REFCOUNTINGPOINTER_DISABLED

				auto registeredfixedpointer1 = (&(*A_refcountingofregistered_ptr1));
				B::foo1(static_cast<A*>(registeredfixedpointer1));

				if (A_refcountingofregistered_ptr2) {
				}
				else if (A_refcountingofregistered_ptr2 != A_refcountingofregistered_ptr1) {
					A_refcountingofregistered_ptr2 = A_refcountingofregistered_ptr1;
					assert(A_refcountingofregistered_ptr2 == A_refcountingofregistered_ptr1);
				}

				mse::TRefCountingOfRegisteredConstPointer<A> rcp = A_refcountingofregistered_ptr1;
				mse::TRefCountingOfRegisteredConstPointer<A> rcp2 = rcp;
				rcp = mse::make_refcountingofregistered<A>();
				mse::TRefCountingOfRegisteredFixedConstPointer<A> rfcp = mse::make_refcountingofregistered<A>();
				{
					int i = rfcp->b;
				}
			}

			int i = A_refcountingofregistered_ptr1->b;
#endif // MSE_SELF_TESTS
		}
	};

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

}

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSEREFCOUNTINGOFREGISTERED_H_
