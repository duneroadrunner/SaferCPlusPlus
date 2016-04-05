
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTING_H_
#define MSEREFCOUNTING_H_

//include "mseprimitives.h"
#include <memory>
#include <iostream>
#include <utility>
#include <cassert>

/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTINGPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

	template<typename _Ty> class TRefCountingNotNullConstPointer;
	template<typename _Ty> class TRefCountingFixedConstPointer;

#ifdef MSE_REFCOUNTINGPOINTER_DISABLED
	template <class X> using TRefCountingPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingNotNullPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingFixedPointer = std::shared_ptr<X>;

	template <class X, class... Args>
	TRefCountingPointer<X> make_refcounting(Args&&... args) {
		return std::make_shared<X>(args...);
	}
#else /*MSE_REFCOUNTINGPOINTER_DISABLED*/

	template<typename _Ty> class TRefCountingNotNullPointer;
	template<typename _Ty> class TRefCountingFixedPointer;

	class CRefCounter {
	private:
		int m_counter;

	public:
		CRefCounter() : m_counter(1) {}
		virtual ~CRefCounter() {}
		void increment() { m_counter++; }
		void decrement() { assert(0 <= m_counter); m_counter--; }
		int use_count() const { return m_counter; }
	};

	template<class Y>
	class TRefWithTargetObj : public CRefCounter {
	public:
		Y m_object;

		template<class ... Args>
		TRefWithTargetObj(Args && ...args) : m_object(args...) {}
	};

	/* Some code originally came from this stackoverflow post:
	http://stackoverflow.com/questions/6593770/creating-a-non-thread-safe-shared-ptr */

	template <class X> class TRefCountingConstPointer;

	/* TRefCountingPointer behaves similar to an std::shared_ptr. Some differences being that it foregoes any thread safety
	mechanisms, it does not accept raw pointer assignment or construction (use make_refcounting<>() instead), and it will throw
	an exception on attempted nullptr dereference. And it's faster. */
	template <class X>
	class TRefCountingPointer {
	public:
		TRefCountingPointer() : m_ref_with_target_obj_ptr(nullptr), m_target_ptr(nullptr) {}
		TRefCountingPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr), m_target_ptr(nullptr) {}
		~TRefCountingPointer() {
			release();
		}
		TRefCountingPointer(const TRefCountingPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			m_target_ptr = r.m_target_ptr;
		}
		operator bool() const { return nullptr != get(); }
		void clear() { (*this) = TRefCountingPointer<X>(nullptr); }
		TRefCountingPointer& operator=(const TRefCountingPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				m_target_ptr = r.m_target_ptr;
			}
			return *this;
		}
		bool operator<(const TRefCountingPointer& r) const {
			return get() < r.get();
		}
		bool operator==(const TRefCountingPointer& r) const {
			return get() == r.get();
		}
		bool operator!=(const TRefCountingPointer& r) const {
			return get() != r.get();
		}

#ifndef MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES
		/* Apparently msvc2015 requires that templated member functions come before regular ones.
		From this webpage regarding compiler error C2668 - https://msdn.microsoft.com/en-us/library/da60x087.aspx:
		"If, in the same class, you have a regular member function and a templated member function with the same
		signature, the templated one must come first. This is a limitation of the current implementation of Visual C++."
		*/
		//  template <class Y> friend class TRefCountingPointer<Y>;
		template <class Y> friend class TRefCountingPointer;
		template <class Y> TRefCountingPointer(const TRefCountingPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			m_target_ptr = r.m_target_ptr;
		}
		template <class Y> TRefCountingPointer& operator=(const TRefCountingPointer<Y>& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				m_target_ptr = r.m_target_ptr;
			}
			return *this;
		}
		template <class Y> bool operator<(const TRefCountingPointer<Y>& r) const {
			return get() < r.get();
		}
		template <class Y> bool operator==(const TRefCountingPointer<Y>& r) const {
			return get() == r.get();
		}
		template <class Y> bool operator!=(const TRefCountingPointer<Y>& r) const {
			return get() != r.get();
		}
#endif // !MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES

		X& operator*() const {
			if (!m_target_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TRefCountingPointer")); }
			return (*m_target_ptr);
		}
		X* operator->() const {
			if (!m_target_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TRefCountingPointer")); }
			return m_target_ptr;
		}
		X* get() const { return m_target_ptr; }
		bool unique() const {
			return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
		}

		template <class... Args>
		static TRefCountingPointer make_refcounting(Args&&... args) {
			auto new_ptr = new CRefCounter(args...);
			TRefCountingPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountingPointer(TRefWithTargetObj<X>* p/* = nullptr*/) : m_ref_with_target_obj_ptr(nullptr) {
			m_ref_with_target_obj_ptr = p;
			if (p) {
				m_target_ptr = std::addressof(p->m_object);
			}
			else {
				m_target_ptr = nullptr;
			}
		}

		void acquire(CRefCounter* c) {
			m_ref_with_target_obj_ptr = c;
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
		}

		struct auto_release {
			auto_release(CRefCounter* c) : m_ref_with_target_obj_ptr(c) {}
			~auto_release() { dorelease(m_ref_with_target_obj_ptr); }
			CRefCounter* m_ref_with_target_obj_ptr;
		};

		void static dorelease(CRefCounter* ref_with_target_obj_ptr) {
			// decrement the count, delete if it is nullptr
			if (ref_with_target_obj_ptr) {
				if (1 == ref_with_target_obj_ptr->use_count()) {
					delete ref_with_target_obj_ptr;
				}
				else {
					ref_with_target_obj_ptr->decrement();
				}
				ref_with_target_obj_ptr = nullptr;
			}
		}

		X* m_target_ptr;
		CRefCounter* m_ref_with_target_obj_ptr;

		friend class TRefCountingNotNullPointer<X>;
		friend class TRefCountingConstPointer<X>;
	};

	template<typename _Ty>
	class TRefCountingNotNullPointer : public TRefCountingPointer<_Ty> {
	public:
		TRefCountingNotNullPointer(const TRefCountingNotNullPointer& src_cref) : TRefCountingPointer<_Ty>(src_cref) {}
		virtual ~TRefCountingNotNullPointer() {}
		TRefCountingNotNullPointer<_Ty>& operator=(const TRefCountingNotNullPointer<_Ty>& _Right_cref) {
			TRefCountingPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRefCountingPointer<_Ty>::operator _Ty*(); }

	private:
		explicit TRefCountingNotNullPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountingPointer<_Ty>(p) {}

		//TRefCountingNotNullPointer<_Ty>* operator&() { return this; }
		//const TRefCountingNotNullPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingFixedPointer<_Ty>;
	};

	/* TRefCountingFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRefCountingFixedPointer : public TRefCountingNotNullPointer<_Ty> {
	public:
		TRefCountingFixedPointer(const TRefCountingFixedPointer& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}
		virtual ~TRefCountingFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRefCountingNotNullPointer<_Ty>::operator _Ty*(); }

		template <class... Args>
		static TRefCountingFixedPointer make_refcounting(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<_Ty>(args...);
			TRefCountingFixedPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountingFixedPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountingNotNullPointer<_Ty>(p) {}
		TRefCountingFixedPointer<_Ty>& operator=(const TRefCountingFixedPointer<_Ty>& _Right_cref) = delete;

		//TRefCountingFixedPointer<_Ty>* operator&() { return this; }
		//const TRefCountingFixedPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingConstPointer<_Ty>;
	};

	template <class X, class... Args>
	TRefCountingFixedPointer<X> make_refcounting(Args&&... args) {
		return TRefCountingFixedPointer<X>::make_refcounting(args...);
	}

#endif /*MSE_REFCOUNTINGPOINTER_DISABLED*/


	template <class X>
	class TRefCountingConstPointer {
	public:
		TRefCountingConstPointer() : m_ref_with_target_obj_ptr(nullptr), m_target_ptr(nullptr) {}
		TRefCountingConstPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr), m_target_ptr(nullptr) {}
		~TRefCountingConstPointer() {
			release();
		}
		TRefCountingConstPointer(const TRefCountingConstPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			m_target_ptr = r.m_target_ptr;
		}
		TRefCountingConstPointer(const TRefCountingPointer<X>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			m_target_ptr = r.m_target_ptr;
		}
		operator bool() const { return nullptr != get(); }
		void clear() { (*this) = TRefCountingConstPointer<X>(nullptr); }
		TRefCountingConstPointer& operator=(const TRefCountingConstPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				m_target_ptr = r.m_target_ptr;
			}
			return *this;
		}
		TRefCountingConstPointer& operator=(const TRefCountingPointer<X>& r) {
			/*if (this != &r) */{
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				m_target_ptr = r.m_target_ptr;
			}
			return *this;
		}
		bool operator<(const TRefCountingConstPointer& r) const {
			return get() < r.get();
		}
		bool operator==(const TRefCountingConstPointer& r) const {
			return get() == r.get();
		}
		bool operator!=(const TRefCountingConstPointer& r) const {
			return get() != r.get();
		}

#ifndef MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES
		/* Apparently msvc2015 requires that templated member functions come before regular ones.
		From this webpage regarding compiler error C2668 - https://msdn.microsoft.com/en-us/library/da60x087.aspx:
		"If, in the same class, you have a regular member function and a templated member function with the same
		signature, the templated one must come first. This is a limitation of the current implementation of Visual C++."
		*/
		//  template <class Y> friend class TRefCountingConstPointer<Y>;
		template <class Y> friend class TRefCountingConstPointer;
		template <class Y> TRefCountingConstPointer(const TRefCountingConstPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			m_target_ptr = r.m_target_ptr;
		}
		template <class Y> TRefCountingConstPointer& operator=(const TRefCountingConstPointer<Y>& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				m_target_ptr = r.m_target_ptr;
			}
			return *this;
		}
		template <class Y> bool operator<(const TRefCountingConstPointer<Y>& r) const {
			return get() < r.get();
		}
		template <class Y> bool operator==(const TRefCountingConstPointer<Y>& r) const {
			return get() == r.get();
		}
		template <class Y> bool operator!=(const TRefCountingConstPointer<Y>& r) const {
			return get() != r.get();
		}
#endif // !MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES

		const X& operator*() const {
			if (!m_target_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TRefCountingConstPointer")); }
			return (*m_target_ptr);
		}
		const X* operator->() const {
			if (!m_target_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TRefCountingConstPointer")); }
			return m_target_ptr;
		}
		const X* get() const { return m_target_ptr; }
		bool unique() const {
			return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
		}

	private:
		explicit TRefCountingConstPointer(TRefWithTargetObj<X>* p/* = nullptr*/) : m_ref_with_target_obj_ptr(nullptr) {
			m_ref_with_target_obj_ptr = p;
			if (p) {
				m_target_ptr = std::addressof(p->m_object);
			}
			else {
				m_target_ptr = nullptr;
			}
		}

		void acquire(CRefCounter* c) {
			m_ref_with_target_obj_ptr = c;
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
		}

		struct auto_release {
			auto_release(CRefCounter* c) : m_ref_with_target_obj_ptr(c) {}
			~auto_release() { dorelease(m_ref_with_target_obj_ptr); }
			CRefCounter* m_ref_with_target_obj_ptr;
		};

		void static dorelease(CRefCounter* ref_with_target_obj_ptr) {
			// decrement the count, delete if it is nullptr
			if (ref_with_target_obj_ptr) {
				if (1 == ref_with_target_obj_ptr->use_count()) {
					delete ref_with_target_obj_ptr;
				}
				else {
					ref_with_target_obj_ptr->decrement();
				}
				ref_with_target_obj_ptr = nullptr;
			}
		}

		X* m_target_ptr;
		CRefCounter* m_ref_with_target_obj_ptr;

		friend class TRefCountingNotNullConstPointer<X>;
	};

	template<typename _Ty>
	class TRefCountingNotNullConstPointer : public TRefCountingConstPointer<_Ty> {
	public:
		TRefCountingNotNullConstPointer(const TRefCountingNotNullConstPointer& src_cref) : TRefCountingConstPointer<_Ty>(src_cref) {}
		TRefCountingNotNullConstPointer(const TRefCountingNotNullPointer<_Ty>& src_cref) : TRefCountingConstPointer<_Ty>(src_cref) {}
		virtual ~TRefCountingNotNullConstPointer() {}
		TRefCountingNotNullConstPointer<_Ty>& operator=(const TRefCountingNotNullConstPointer<_Ty>& _Right_cref) {
			TRefCountingConstPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRefCountingConstPointer<_Ty>::operator _Ty*(); }

	private:
		//TRefCountingNotNullConstPointer<_Ty>* operator&() { return this; }
		//const TRefCountingNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingFixedConstPointer<_Ty>;
	};

	/* TRefCountingFixedConstPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRefCountingFixedConstPointer : public TRefCountingNotNullConstPointer<_Ty> {
	public:
		TRefCountingFixedConstPointer(const TRefCountingFixedConstPointer& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingFixedPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		virtual ~TRefCountingFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRefCountingNotNullConstPointer<_Ty>::operator _Ty*(); }

	private:
		TRefCountingFixedConstPointer<_Ty>& operator=(const TRefCountingFixedConstPointer<_Ty>& _Right_cref) = delete;

		//TRefCountingFixedConstPointer<_Ty>* operator&() { return this; }
		//const TRefCountingFixedConstPointer<_Ty>* operator&() const { return this; }
	};


	class TRefCountingPointer_test {
	public:
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountingPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			const std::string _id;
			TRefCountingPointer_test* m_state_ptr;
		};

		typedef TRefCountingPointer<Trackable> target_t;


#define MTXASSERT_EQ(a, b, c) a &= (b==c)
#define MTXASSERT(a, b) a &= (bool)(b)
		bool testBehaviour()
		{
			static const TRefCountingPointer<Trackable> Nil = target_t(nullptr);
			bool ok = true;

			constructions.clear();
			destructions.clear();

			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t a = make_refcounting<Trackable>(this, "aap");

			MTXASSERT_EQ(ok, 1ul, constructions.size());
			MTXASSERT_EQ(ok, 1, constructions["aap"]);
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			MTXASSERT_EQ(ok, 0, constructions["noot"]);
			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t hold;
			{
				target_t b = make_refcounting<Trackable>(this, "noot"),
					c = make_refcounting<Trackable>(this, "mies"),
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
			Linked(TRefCountingPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
			TRefCountingPointer<Linked> next;
		};

		bool testLinked()
		{
			bool ok = true;

			constructions.clear();
			destructions.clear();
			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			TRefCountingPointer<Linked> node = make_refcounting<Linked>(this, "parent");
			MTXASSERT(ok, node.get());
			node->next = make_refcounting<Linked>(this, "child");

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
				static int foo2(mse::TRefCountingPointer<A> A_refcounting_ptr) { return A_refcounting_ptr->b; }
			protected:
				~B() {}
			};

			A* A_native_ptr = nullptr;
			/* mse::TRefCountingPointer<> is basically a slightly "safer" version of std::shared_ptr. */
			mse::TRefCountingPointer<A> A_refcounting_ptr1;

			{
				A a;

				A_native_ptr = &a;
				A_refcounting_ptr1 = mse::make_refcounting<A>();
				assert(A_native_ptr->b == A_refcounting_ptr1->b);

				mse::TRefCountingPointer<A> A_refcounting_ptr2 = A_refcounting_ptr1;
				A_refcounting_ptr2 = nullptr;
				bool expected_exception = false;
				try {
					int i = A_refcounting_ptr2->b; /* this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "refcounting pointer". */
				}
				assert(expected_exception);

				B::foo1(&(*A_refcounting_ptr1));

				if (A_refcounting_ptr2) {
				}
				else if (A_refcounting_ptr2 != A_refcounting_ptr1) {
					A_refcounting_ptr2 = A_refcounting_ptr1;
					assert(A_refcounting_ptr2 == A_refcounting_ptr1);
				}

				mse::TRefCountingConstPointer<A> rcp = A_refcounting_ptr1;
				mse::TRefCountingConstPointer<A> rcp2 = rcp;
				rcp = mse::make_refcounting<A>();
				mse::TRefCountingFixedConstPointer<A> rfcp = mse::make_refcounting<A>();
				{
					int i = rfcp->b;
				}
			}

			int i = A_refcounting_ptr1->b;

			{
				class D : public A {};
				mse::TRefCountingFixedPointer<D> D_refcountingfixed_ptr1 = mse::make_refcounting<D>();
				//mse::TRefCountingFixedPointer<const D> constD_refcountingfixed_ptr1 = D_refcountingfixed_ptr1;
				mse::TRefCountingPointer<A> A_refcountingfixed_ptr2 = D_refcountingfixed_ptr1;
			}
		}

	};
}

#endif // MSEREFCOUNTING_H_
