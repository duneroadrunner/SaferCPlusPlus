
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTED_H_
#define MSEREFCOUNTED_H_

#include "mseprimitives.h"
#include <memory>
#include <iostream>

/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTEDPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_REFCOUNTEDPOINTER_DISABLED
	template <class X> using TRefCountedPointer = std::shared_ptr<X>;
	template <class X> using TRefCountedNotNullPointer = std::shared_ptr<X>;
	template <class X> using TRefCountedFixedPointer = std::shared_ptr<X>;

	template <class X, class... Args>
	TRefCountedPointer<X> make_refcounted(Args&&... args) {
		return std::make_shared<X>(args...);
	}
#else /*MSE_REFCOUNTEDPOINTER_DISABLED*/

	template<typename _Ty> class TRefCountedNotNullPointer;
	template<typename _Ty> class TRefCountedFixedPointer;

	class CRefCounter {
	private:
		int m_counter;

	public:
		CRefCounter() : m_counter(1) {}
		//virtual ~CRefCounter() {}
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

	template <class X>
	class TRefCountedPointer {
	public:
		TRefCountedPointer() : m_ref_with_target_obj_ptr(nullptr) {}
		TRefCountedPointer(nullptr_t) : m_ref_with_target_obj_ptr(nullptr) {}
		~TRefCountedPointer() {
			release();
		}
		TRefCountedPointer(const TRefCountedPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		operator bool() const { return nullptr != get(); }
		void clear() { (*this) = TRefCountedPointer<X>(nullptr); }
		TRefCountedPointer& operator=(const TRefCountedPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		bool operator<(const TRefCountedPointer& r) const {
			return get() < r.get();
		}
		bool operator==(const TRefCountedPointer& r) const {
			return get() == r.get();
		}
		bool operator!=(const TRefCountedPointer& r) const {
			return get() != r.get();
		}

#ifndef MSE_REFCOUNTEDPOINTER_DISABLE_MEMBER_TEMPLATES
		/* Apparently msvc2015 requires that templated member functions come before regular ones.
		From this webpage regarding compiler error C2668 - https://msdn.microsoft.com/en-us/library/da60x087.aspx:
		"If, in the same class, you have a regular member function and a templated member function with the same
		signature, the templated one must come first. This is a limitation of the current implementation of Visual C++."
		*/
		//  template <class Y> friend class TRefCountedPointer<Y>;
		template <class Y> TRefCountedPointer(const TRefCountedPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		template <class Y> TRefCountedPointer& operator=(const TRefCountedPointer<Y>& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		template <class Y> bool operator<(const TRefCountedPointer<Y>& r) const {
			return get() < r.get();
		}
		template <class Y> bool operator==(const TRefCountedPointer<Y>& r) const {
			return get() == r.get();
		}
		template <class Y> bool operator!=(const TRefCountedPointer<Y>& r) const {
			return get() != r.get();
		}
#endif // !MSE_REFCOUNTEDPOINTER_DISABLE_MEMBER_TEMPLATES

		X& operator*()  const {
			if (!m_ref_with_target_obj_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TRefCountedPointer")); }
			return (m_ref_with_target_obj_ptr->m_object);
		}
		X* operator->() const {
			if (!m_ref_with_target_obj_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TRefCountedPointer")); }
			return &(m_ref_with_target_obj_ptr->m_object);
		}
		X* get()        const { return m_ref_with_target_obj_ptr ? &(m_ref_with_target_obj_ptr->m_object) : nullptr; }
		bool unique()   const {
			return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
		}

		template <class... Args>
		static TRefCountedPointer make_refcounted(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<X>(args...);
			TRefCountedPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountedPointer(TRefWithTargetObj<X>* p/* = nullptr*/) : m_ref_with_target_obj_ptr(nullptr) {
			if (p) { m_ref_with_target_obj_ptr = p; }
		}

		void acquire(TRefWithTargetObj<X>* c) {
			m_ref_with_target_obj_ptr = c;
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
		}

		struct auto_release {
			auto_release(TRefWithTargetObj<X>* c) : m_ref_with_target_obj_ptr(c) {}
			~auto_release() { dorelease(m_ref_with_target_obj_ptr); }
			TRefWithTargetObj<X>* m_ref_with_target_obj_ptr;
		};

		void static dorelease(TRefWithTargetObj<X>* ref_with_target_obj_ptr) {
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

		TRefWithTargetObj<X>* m_ref_with_target_obj_ptr;

		friend class TRefCountedNotNullPointer<X>;
	};

	template<typename _Ty>
	class TRefCountedNotNullPointer : public TRefCountedPointer<_Ty> {
	public:
		TRefCountedNotNullPointer(const TRefCountedNotNullPointer& src_cref) : TRefCountedPointer<_Ty>(src_cref) {}
		virtual ~TRefCountedNotNullPointer() {}
		TRefCountedNotNullPointer<_Ty>& operator=(const TRefCountedNotNullPointer<_Ty>& _Right_cref) {
			TRefCountedPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRefCountedPointer<_Ty>::operator _Ty*(); }
		//explicit operator TRegisteredObj<_Ty, _Tn>*() const { return TRefCountedPointer<_Ty>::operator TRegisteredObj<_Ty, _Tn>*(); }

	private:
		explicit TRefCountedNotNullPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountedPointer<_Ty>(p) {}

		//TRefCountedNotNullPointer(TRegisteredObj<_Ty, _Tn>* ptr) : TRefCountedPointer<_Ty>(ptr) {}

		TRefCountedNotNullPointer<_Ty>* operator&() { return this; }
		const TRefCountedNotNullPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountedFixedPointer<_Ty>;
	};

	/* TRefCountedFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRefCountedFixedPointer : public TRefCountedNotNullPointer<_Ty> {
	public:
		TRefCountedFixedPointer(const TRefCountedFixedPointer& src_cref) : TRefCountedNotNullPointer<_Ty>(src_cref) {}
		virtual ~TRefCountedFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRefCountedNotNullPointer<_Ty>::operator _Ty*(); }
		//explicit operator TRegisteredObj<_Ty, _Tn>*() const { return TRefCountedNotNullPointer<_Ty>::operator TRegisteredObj<_Ty, _Tn>*(); }

		template <class... Args>
		static TRefCountedFixedPointer make_refcounted(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<_Ty>(args...);
			TRefCountedFixedPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountedFixedPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountedNotNullPointer<_Ty>(p) {}
		TRefCountedFixedPointer<_Ty>& operator=(const TRefCountedFixedPointer<_Ty>& _Right_cref) {}

		//TRefCountedFixedPointer(TRegisteredObj<_Ty, _Tn>* ptr) : TRefCountedNotNullPointer<_Ty>(ptr) {}

		TRefCountedFixedPointer<_Ty>* operator&() { return this; }
		const TRefCountedFixedPointer<_Ty>* operator&() const { return this; }

		//friend class TRegisteredObj<_Ty, _Tn>;
	};

	template <class X, class... Args>
	TRefCountedFixedPointer<X> make_refcounted(Args&&... args) {
		return TRefCountedFixedPointer<X>::make_refcounted(args...);
	}

#endif /*MSE_REFCOUNTEDPOINTER_DISABLED*/


	template <class X>
	class TRefCountedConstPointer {
	public:
		TRefCountedConstPointer() {}
		TRefCountedConstPointer(nullptr_t) {}
		//~TRefCountedConstPointer() {}
		TRefCountedConstPointer(const TRefCountedConstPointer& r) = default;
		TRefCountedConstPointer(const TRefCountedPointer<X>& r) : m_refcounted_ptr(r) {}
		operator bool() const { return m_refcounted_ptr; }
		void clear() { (*this) = TRefCountedConstPointer<X>(nullptr); }
		TRefCountedConstPointer& operator=(const TRefCountedConstPointer& r) = default;
		TRefCountedConstPointer& operator=(const TRefCountedPointer<X>& r) {
			m_refcounted_ptr = r;
		}
		bool operator<(const TRefCountedConstPointer& r) const {
			return m_refcounted_ptr < r.m_refcounted_ptr;
		}
		bool operator==(const TRefCountedConstPointer& r) const {
			return m_refcounted_ptr == r.m_refcounted_ptr;
		}
		bool operator!=(const TRefCountedConstPointer& r) const {
			return m_refcounted_ptr != r.m_refcounted_ptr;
		}

#ifndef MSE_REFCOUNTEDPOINTER_DISABLE_MEMBER_TEMPLATES
		//  template <class Y> friend class TRefCountedConstPointer<Y>;
		template <class Y> TRefCountedConstPointer(const TRefCountedConstPointer<Y>& r) {
			m_refcounted_ptr = r.m_refcounted_ptr;
		}
		template <class Y> TRefCountedConstPointer& operator=(const TRefCountedConstPointer<Y>& r) {
			m_refcounted_ptr = r.m_refcounted_ptr;
			return *this;
		}
		template <class Y> bool operator<(const TRefCountedConstPointer<Y>& r) const {
			return m_refcounted_ptr < r.m_refcounted_ptr;
		}
		template <class Y> bool operator==(const TRefCountedConstPointer<Y>& r) const {
			return m_refcounted_ptr == r.m_refcounted_ptr;
		}
		template <class Y> bool operator!=(const TRefCountedConstPointer<Y>& r) const {
			return m_refcounted_ptr != r.m_refcounted_ptr;
		}
#endif // !MSE_REFCOUNTEDPOINTER_DISABLE_MEMBER_TEMPLATES

		const X& operator*()  const {
			return (*m_refcounted_ptr);
		}
		const X* operator->() const {
			return m_refcounted_ptr.operator->();
		}
		const X* get()        const { return m_refcounted_ptr.get(); }
		bool unique()   const {
			return m_refcounted_ptr.unique();
		}

	private:

		TRefCountedPointer<X> m_refcounted_ptr;
	};

	class TRefCountedPointer_test {
	public:
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountedPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			const std::string _id;
			TRefCountedPointer_test* m_state_ptr;
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
			Linked(TRefCountedPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
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
			mse::TRefCountedPointer<int> rcp1 = mse::make_refcounted<int>(7);
			(*rcp1) = 11;
			mse::TRefCountedConstPointer<int> rccp1(rcp1);
			mse::TRefCountedConstPointer<int> rccp2;
			rccp2 = rccp1;
			mse::TRefCountedPointer<int> rccp3 = mse::make_refcounted<int>(17);
		}

	};
}

#endif // MSEREFCOUNTED_H_
