
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESCOPE_H_
#define MSESCOPE_H_

#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

/* Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause relaxed registered pointers to be used to help catch
misuse of scope pointers in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause
mse::TRelaxedRegisteredObj to be used in non-debug modes as well. */
#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED
#include "mserelaxedregistered.h"
#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_SCOPEPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef NDEBUG
#ifndef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#define MSE_SCOPEPOINTER_DISABLED
#endif // !MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // NDEBUG


namespace mse {

#ifdef MSE_SCOPEPOINTER_DISABLED
	template<typename _Ty> using TScopePointer = _Ty*;
	template<typename _Ty> using TScopeConstPointer = const _Ty*;
	template<typename _Ty> using TScopeNotNullPointer = _Ty*;
	template<typename _Ty> using TScopeNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TScopeFixedPointer = _Ty*;
	template<typename _Ty> using TScopeFixedConstPointer = const _Ty*;
	template<typename _TROy> using TScopeObj = _TROy;

#else /*MSE_SCOPEPOINTER_DISABLED*/

	/* This macro roughly simulates constructor inheritance. Originally it was used when some compilers didn't support
	constructor inheritance, but now we use it because of it's differences with standard constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base)                                 \
    template<typename ...Args,                               \
             typename = typename std::enable_if              \
             <                                               \
                std::is_constructible<Base, Args...>::value  \
			 			 			 			              >::type>                                        \
    Derived(Args &&...args)                                  \
        : Base(std::forward<Args>(args)...) { }              \

#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> using TScopePointerBase = mse::TRelaxedRegisteredPointer<_Ty>;
	template<typename _Ty> using TScopeConstPointerBase = mse::TRelaxedRegisteredConstPointer<_Ty>;
	template<typename _TROz> using TScopeObjBase = mse::TRelaxedRegisteredObj<_TROz>;

#else // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> using TScopePointerBase = TSaferPtrForLegacy<_Ty>;
	template<typename _Ty> using TScopeConstPointerBase = TSaferPtrForLegacy<const _Ty>;

	template<typename _TROz>
	class TScopeObjBase : public _TROz {
	public:
		MSE_SCOPE_USING(TScopeObjBase, _TROz);
	};

#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> class TScopeObj;
	template<typename _Ty> class TScopeNotNullPointer;
	template<typename _Ty> class TScopeNotNullConstPointer;
	template<typename _Ty> class TScopeFixedPointer;
	template<typename _Ty> class TScopeFixedConstPointer;

	/* Use TScopeFixedPointer instead. */
	template<typename _Ty>
	class TScopePointer : public TScopePointerBase<_Ty> {
	public:
	private:
		TScopePointer() : TScopePointerBase<_Ty>() {}
		TScopePointer(TScopeObj<_Ty>* ptr) : TScopePointerBase<_Ty>(ptr) {}
		TScopePointer(const TScopePointer& src_cref) : TScopePointerBase<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopePointer(const TScopePointer<_Ty2>& src_cref) : TScopePointerBase<_Ty>(src_cref) {}
		virtual ~TScopePointer() {}
		TScopePointer<_Ty>& operator=(TScopeObj<_Ty>* ptr) {
			return TScopePointerBase<_Ty>::operator=(ptr);
		}
		TScopePointer<_Ty>& operator=(const TScopePointer<_Ty>& _Right_cref) {
			return TScopePointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = (*static_cast<const TScopePointerBase<_Ty>*>(this));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
			_Ty* retval = (*static_cast<const TScopePointerBase<_Ty>*>(this));
			return retval;
		}
		explicit operator TScopeObj<_Ty>*() const {
			TScopeObj<_Ty>* retval = (*static_cast<const TScopePointerBase<_Ty>*>(this));
			return retval;
		}

		TScopePointer<_Ty>* operator&() { return this; }
		const TScopePointer<_Ty>* operator&() const { return this; }

		friend class TScopeNotNullPointer<_Ty>;
	};

	/* Use TScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TScopeConstPointer : public TScopeConstPointerBase<const _Ty> {
	public:
	private:
		TScopeConstPointer() : TScopeConstPointerBase<const _Ty>() {}
		TScopeConstPointer(const TScopeObj<_Ty>* ptr) : TScopeConstPointerBase<const _Ty>(ptr) {}
		TScopeConstPointer(const TScopeConstPointer& src_cref) : TScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeConstPointer(const TScopeConstPointer<_Ty2>& src_cref) : TScopeConstPointerBase<const _Ty>(src_cref) {}
		TScopeConstPointer(const TScopePointer<_Ty>& src_cref) : TScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeConstPointer(const TScopePointer<_Ty2>& src_cref) : TScopeConstPointerBase<const _Ty>(src_cref) {}
		virtual ~TScopeConstPointer() {}
		TScopeConstPointer<_Ty>& operator=(const TScopeObj<_Ty>* ptr) {
			return TScopeConstPointerBase<_Ty>::operator=(ptr);
		}
		TScopeConstPointer<_Ty>& operator=(const TScopeConstPointer<_Ty>& _Right_cref) {
			return TScopeConstPointerBase<_Ty>::operator=(_Right_cref);
		}
		TScopeConstPointer<_Ty>& operator=(const TScopePointer<_Ty>& _Right_cref) { return (*this).operator=(TScopeConstPointer(_Right_cref)); }
		operator bool() const {
			bool retval = (*static_cast<const TScopeConstPointerBase<_Ty>*>(this));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const {
			const _Ty* retval = (*static_cast<const TScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}
		explicit operator const TScopeObj<_Ty>*() const {
			const TScopeObj<_Ty>* retval = (*static_cast<const TScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}

		TScopeConstPointer<_Ty>* operator&() { return this; }
		const TScopeConstPointer<_Ty>* operator&() const { return this; }

		friend class TScopeNotNullConstPointer<_Ty>;
	};

	/* Use TScopeFixedPointer instead. */
	template<typename _Ty>
	class TScopeNotNullPointer : public TScopePointer<_Ty> {
	public:
	private:
		TScopeNotNullPointer(TScopeObj<_Ty>* ptr) : TScopePointer<_Ty>(ptr) {}
		TScopeNotNullPointer(const TScopeNotNullPointer& src_cref) : TScopePointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeNotNullPointer(const TScopeNotNullPointer<_Ty2>& src_cref) : TScopePointer<_Ty>(src_cref) {}
		virtual ~TScopeNotNullPointer() {}
		TScopeNotNullPointer<_Ty>& operator=(const TScopePointer<_Ty>& _Right_cref) {
			TScopePointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		operator bool() const { return (*static_cast<const TScopePointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TScopePointer<_Ty>::operator _Ty*(); }
		explicit operator TScopeObj<_Ty>*() const { return TScopePointer<_Ty>::operator TScopeObj<_Ty>*(); }

		TScopeNotNullPointer<_Ty>* operator&() { return this; }
		const TScopeNotNullPointer<_Ty>* operator&() const { return this; }

		friend class TScopeFixedPointer<_Ty>;
	};

	/* Use TScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TScopeNotNullConstPointer : public TScopeConstPointer<_Ty> {
	public:
	private:
		TScopeNotNullConstPointer(const TScopeNotNullConstPointer<_Ty>& src_cref) : TScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeNotNullConstPointer(const TScopeNotNullConstPointer<_Ty2>& src_cref) : TScopeConstPointer<_Ty>(src_cref) {}
		TScopeNotNullConstPointer(const TScopeNotNullPointer<_Ty>& src_cref) : TScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeNotNullConstPointer(const TScopeNotNullPointer<_Ty2>& src_cref) : TScopeConstPointer<_Ty>(src_cref) {}
		virtual ~TScopeNotNullConstPointer() {}
		operator bool() const { return (*static_cast<const TScopeConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TScopeConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TScopeObj<_Ty>*() const { return TScopeConstPointer<_Ty>::operator const TScopeObj<_Ty>*(); }
		TScopeNotNullConstPointer(const TScopeObj<_Ty>* ptr) : TScopeConstPointer<_Ty>(ptr) {}

		TScopeNotNullConstPointer<_Ty>* operator&() { return this; }
		const TScopeNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TScopeFixedConstPointer<_Ty>;
	};

	/* A TScopeFixedPointer points to a TScopeObj. Its intended for very limited use. Basically just to pass a TScopeObj
	by reference as a function parameter. TScopeFixedPointers can be obtained from TScopeObj's "&" (address of) operator. */
	template<typename _Ty>
	class TScopeFixedPointer : public TScopeNotNullPointer<_Ty> {
	public:
		TScopeFixedPointer(const TScopeFixedPointer& src_cref) : TScopeNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeFixedPointer(const TScopeFixedPointer<_Ty2>& src_cref) : TScopeNotNullPointer<_Ty>(src_cref) {}
		virtual ~TScopeFixedPointer() {}
		operator bool() const { return (*static_cast<const TScopeNotNullPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TScopeNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TScopeObj<_Ty>*() const { return TScopeNotNullPointer<_Ty>::operator TScopeObj<_Ty>*(); }

	private:
		TScopeFixedPointer(TScopeObj<_Ty>* ptr) : TScopeNotNullPointer<_Ty>(ptr) {}
		TScopeFixedPointer<_Ty>& operator=(const TScopeFixedPointer<_Ty>& _Right_cref) = delete;

		TScopeFixedPointer<_Ty>* operator&() { return this; }
		const TScopeFixedPointer<_Ty>* operator&() const { return this; }

		friend class TScopeObj<_Ty>;
	};

	template<typename _Ty>
	class TScopeFixedConstPointer : public TScopeNotNullConstPointer<_Ty> {
	public:
		TScopeFixedConstPointer(const TScopeFixedConstPointer<_Ty>& src_cref) : TScopeNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeFixedConstPointer(const TScopeFixedConstPointer<_Ty2>& src_cref) : TScopeNotNullConstPointer<_Ty>(src_cref) {}
		TScopeFixedConstPointer(const TScopeFixedPointer<_Ty>& src_cref) : TScopeNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TScopeFixedConstPointer(const TScopeFixedPointer<_Ty2>& src_cref) : TScopeNotNullConstPointer<_Ty>(src_cref) {}
		virtual ~TScopeFixedConstPointer() {}
		operator bool() const { return (*static_cast<const TScopeNotNullConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TScopeNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TScopeObj<_Ty>*() const { return TScopeNotNullConstPointer<_Ty>::operator const TScopeObj<_Ty>*(); }

	private:
		TScopeFixedConstPointer(const TScopeObj<_Ty>* ptr) : TScopeNotNullConstPointer<_Ty>(ptr) {}
		TScopeFixedConstPointer<_Ty>& operator=(const TScopeFixedConstPointer<_Ty>& _Right_cref) = delete;

		TScopeFixedConstPointer<_Ty>* operator&() { return this; }
		const TScopeFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TScopeObj<_Ty>;
	};

	/* TScopeObj is intended as a transparent wrapper for other classes/objects with "scope lifespans". That is, objects
	that are either allocated on the stack, or whose "owning" pointer is allocated on the stack. Unfortunately it's not
	really possible to prevent misuse. For example, auto x = new TScopeObj<mse::CInt> is an improper, and dangerous, use
	of TScopeObj<>. So we provide the option of using an mse::TRelaxedRegisteredObj as TScopeObj's base class for
	enhanced safety and to help catch misuse. Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause
	mse::TRelaxedRegisteredObj to be used in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	will cause mse::TRelaxedRegisteredObj to be used in non-debug modes as well. */
	template<typename _TROy>
	class TScopeObj : public TScopeObjBase<_TROy> {
	public:
		MSE_SCOPE_USING(TScopeObj, TScopeObjBase<_TROy>);
		//TScopeObj(const TScopeObj& _X) : TScopeObjBase<_TROy>(_X) {}
		//TScopeObj(TScopeObj&& _X) : TScopeObjBase<_TROy>(std::move(_X)) {}
		virtual ~TScopeObj() {}
		TScopeObj& operator=(const TScopeObj& _X) { TScopeObjBase<_TROy>::operator=(_X); return (*this); }
		TScopeFixedPointer<_TROy> operator&() {
			return this;
		}
		TScopeFixedConstPointer<_TROy> operator&() const {
			return this;
		}
		//TScpPTracker<>& mseRPManager() const { return m_mseRPManager; }

	private:
		TScopeObj& operator=(TScopeObj&& _X) = delete;
		//mutable TScpPTracker<> m_mseRPManager;
	};

#endif /*MSE_SCOPEPOINTER_DISABLED*/

	/* TScopeOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
	TScopeOwnerPointer just forwards it's constructor arguments to the constructor of the TScopeObj<_Ty>.
	TScopeOwnerPointers are meant to be allocated on the stack only. Unfortunately there's really no way to
	enforce this, which makes this data type less intrinsically safe than say, "reference counting" pointers.
	Because of this, in debug mode, we employ the same comprehensive safety mechanisms that "registered
	pointers" use. */
	template<typename _Ty>
	class TScopeOwnerPointer {
	public:
		template <class... Args>
		TScopeOwnerPointer(Args&&... args) {
			TScopeObj<_Ty>* new_ptr = new TScopeObj<_Ty>(args...);
			m_ptr = new_ptr;
		}
		virtual ~TScopeOwnerPointer() {
			assert(m_ptr);
			delete m_ptr;
		}

		TScopeObj<_Ty>& operator*() const {
			return (*m_ptr);
		}
		TScopeObj<_Ty>* operator->() const {
			return m_ptr;
		}

	private:
		TScopeOwnerPointer(TScopeOwnerPointer<_Ty>& src_cref) = delete;
		TScopeOwnerPointer<_Ty>& operator=(const TScopeOwnerPointer<_Ty>& _Right_cref) = delete;

		TScopeObj<_Ty>* m_ptr = nullptr;
	};

	/* shorter aliases */
	//template<typename _Ty> using scpp = TScopePointer<_Ty>;
	//template<typename _Ty> using scpcp = TScopeConstPointer<_Ty>;
	//template<typename _Ty> using scpnnp = TScopeNotNullPointer<_Ty>;
	//template<typename _Ty> using scpnncp = TScopeNotNullConstPointer<_Ty>;
	template<typename _Ty> using scpfp = TScopeFixedPointer<_Ty>;
	template<typename _Ty> using scpfcp = TScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using scpo = TScopeObj<_TROy>;


	static void s_scpptr_test1() {
		class A {
		public:
			A(int x) : b(x) {}
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
			static int foo2(mse::TScopeFixedPointer<A> A_scope_ptr) { return A_scope_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;

		{
			A a(7);
			mse::TScopeObj<A> scope_a(7);
			/* mse::TScopeObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == scope_a.b);
			A_native_ptr = &a;
			mse::TScopeFixedPointer<A> A_scope_ptr1 = &scope_a;
			assert(A_native_ptr->b == A_scope_ptr1->b);

			mse::TScopeFixedPointer<A> A_scope_ptr2 = &scope_a;

			/* mse::TScopeFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_scope_ptr1);

			if (A_scope_ptr2) {
			}
			else if (A_scope_ptr2 != A_scope_ptr1) {
				int q = B::foo2(A_scope_ptr2);
			}

			A a2 = a;
			mse::TScopeObj<A> scope_a2 = scope_a;

			mse::TScopeFixedConstPointer<A> rcp = A_scope_ptr1;
			mse::TScopeFixedConstPointer<A> rcp2 = rcp;
			const mse::TScopeObj<A> cscope_a(11);
			mse::TScopeFixedConstPointer<A> rfcp = &cscope_a;

			mse::TScopeOwnerPointer<A> A_scpoptr(11);
			B::foo2(&*A_scpoptr);
			if (A_scpoptr->b == (&*A_scpoptr)->b) {
			}
		}

		{
			/* Polymorphic conversions. */
			class E {
			public:
				int m_b = 5;
			};
			class FE : public mse::TScopeObj<E> {};
			mse::TScopeObj<FE> scope_fd;
			mse::TScopeFixedPointer<FE> FE_scope_fptr1 = &scope_fd;
			mse::TScopeFixedPointer<E> E_scope_ptr4 = FE_scope_fptr1;
			mse::TScopeFixedPointer<E> E_scope_fptr1 = &scope_fd;
			mse::TScopeFixedConstPointer<E> E_scope_fcptr1 = &scope_fd;

			/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
			class GE : public E {};
			mse::TScopeObj<GE> scope_gd;
			mse::TScopeFixedPointer<GE> GE_scope_fptr1 = &scope_gd;
			mse::TScopeFixedPointer<E> E_scope_ptr5 = GE_scope_fptr1;
			mse::TScopeFixedPointer<E> E_scope_fptr2 = &scope_gd;
			mse::TScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_gd;
		}
	}
}

#endif // MSESCOPE_H_
