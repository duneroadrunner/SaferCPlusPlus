
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSETHREADLOCAL_H_
#define MSETHREADLOCAL_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED
#include "msenorad.h"
#include "mseany.h"
#endif // MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-value"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
#endif /*__GNUC__*/
#endif /*__clang__*/

/* Note that by default, MSE_THREADLOCALPOINTER_DISABLED is defined in non-debug builds. This is enacted in "msepointerbasics.h". */

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


namespace mse {
	namespace rsv {

		/* This macro roughly simulates constructor inheritance. */
#define MSE_THREADLOCAL_USING(Derived, Base) MSE_USING(Derived, Base)

		template<typename _Ty> class TThreadLocalID {};

		template<typename _Ty>
		class TPlaceHolder_msethreadlocal {};
		template<typename _Ty>
		class TPlaceHolder2_msethreadlocal {};

#ifdef MSE_THREADLOCALPOINTER_DISABLED
		//TThreadLocalID
		template<typename _Ty> using TThreadLocalPointer = _Ty * ;
		template<typename _Ty> using TThreadLocalConstPointer = const _Ty*;
		template<typename _Ty> using TThreadLocalNotNullPointer = _Ty * ;
		template<typename _Ty> using TThreadLocalNotNullConstPointer = const _Ty*;
		template<typename _Ty> using TThreadLocalFixedPointer = _Ty * /*const*/; /* Can't be const qualified because standard
																		   library containers don't support const elements. */
		template<typename _Ty> using TThreadLocalFixedConstPointer = const _Ty* /*const*/;
		template<typename _TROy> using TThreadLocalObjBase = _TROy;
		template<typename _TROy> using TThreadLocalObj = _TROy;

		template<typename _TROy> class TThreadLocalOwnerPointer;

		template<typename _Ty> auto thread_local_fptr_to(_Ty&& _X) { return std::addressof(_X); }
		template<typename _Ty> auto thread_local_fptr_to(const _Ty& _X) { return std::addressof(_X); }

#else /*MSE_THREADLOCALPOINTER_DISABLED*/

#ifdef MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED

		template<typename _TROz> using TThreadLocalObjBase = mse::TWNoradObj<_TROz>;
		template<typename _Ty> using TThreadLocalPointerBase = mse::us::impl::TAnyPointerBase<_Ty>;
		template<typename _Ty> using TThreadLocalConstPointerBase = mse::us::impl::TAnyConstPointerBase<_Ty>;
		template<typename _Ty> using Tthread_local_obj_base_ptr = mse::TWNoradFixedPointer<_Ty>;
		template<typename _Ty> using Tthread_local_obj_base_const_ptr = mse::TWNoradConstPointer<_Ty>;

#else // MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED

		template<typename _TROz>
		class TThreadLocalObjBase : public _TROz {
		public:
			MSE_THREADLOCAL_USING(TThreadLocalObjBase, _TROz);
			TThreadLocalObjBase(const TThreadLocalObjBase& _X) : _TROz(_X) {}
			TThreadLocalObjBase(TThreadLocalObjBase&& _X) : _TROz(std::forward<decltype(_X)>(_X)) {}

			TThreadLocalObjBase& operator=(TThreadLocalObjBase&& _X) { _TROz::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			TThreadLocalObjBase& operator=(const TThreadLocalObjBase& _X) { _TROz::operator=(_X); return (*this); }
			template<class _Ty2>
			TThreadLocalObjBase& operator=(_Ty2&& _X) { _TROz::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			template<class _Ty2>
			TThreadLocalObjBase& operator=(const _Ty2& _X) { _TROz::operator=(_X); return (*this); }

			auto operator&() {
				return this;
			}
			auto operator&() const {
				return this;
			}
		};

		template<typename _Ty> using TThreadLocalPointerBase = mse::us::impl::TPointerForLegacy<_Ty, TThreadLocalID<const _Ty>>;
		template<typename _Ty> using TThreadLocalConstPointerBase = mse::us::impl::TPointerForLegacy<const _Ty, TThreadLocalID<const _Ty>>;
		template<typename _Ty> using Tthread_local_obj_base_ptr = TThreadLocalObjBase<_Ty>*;
		template<typename _Ty> using Tthread_local_obj_base_const_ptr = TThreadLocalObjBase<_Ty> const*;

#endif // MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED

		template <class _Ty, class _Ty2, class = typename std::enable_if<
			(!std::is_same<_Ty&&, _Ty2>::value) || (!std::is_rvalue_reference<_Ty2>::value)
			, void>::type>
			static void valid_if_not_rvalue_reference_of_given_type_msethreadlocal(_Ty2 src) {}

		template<typename _Ty> class TThreadLocalObj;
		template<typename _Ty> class TThreadLocalNotNullPointer;
		template<typename _Ty> class TThreadLocalNotNullConstPointer;
		template<typename _Ty> class TThreadLocalFixedPointer;
		template<typename _Ty> class TThreadLocalFixedConstPointer;
	}

	namespace us {
		namespace impl {
			template <typename _Ty, typename _TConstPointer1> class TCommonizedPointer;
			template <typename _Ty, typename _TConstPointer1> class TCommonizedConstPointer;
		}
	}

	namespace rsv {

		/* Use TThreadLocalFixedPointer instead. */
		template<typename _Ty>
		class TThreadLocalPointer : public TThreadLocalPointerBase<_Ty>, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
		public:
			typedef Tthread_local_obj_base_ptr<_Ty> scope_obj_base_ptr_t;
			virtual ~TThreadLocalPointer() {}
		private:
			TThreadLocalPointer() : TThreadLocalPointerBase<_Ty>() {}
			TThreadLocalPointer(scope_obj_base_ptr_t ptr) : TThreadLocalPointerBase<_Ty>(ptr) {}
			TThreadLocalPointer(const TThreadLocalPointer& src_cref) : TThreadLocalPointerBase<_Ty>(
				static_cast<const TThreadLocalPointerBase<_Ty>&>(src_cref)) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalPointer(const TThreadLocalPointer<_Ty2>& src_cref) : TThreadLocalPointerBase<_Ty>(TThreadLocalPointerBase<_Ty2>(src_cref)) {}
			TThreadLocalPointer<_Ty>& operator=(TThreadLocalObj<_Ty>* ptr) {
				return TThreadLocalPointerBase<_Ty>::operator=(ptr);
			}
			TThreadLocalPointer<_Ty>& operator=(const TThreadLocalPointer<_Ty>& _Right_cref) {
				return TThreadLocalPointerBase<_Ty>::operator=(_Right_cref);
			}
			operator bool() const {
				bool retval = (bool(*static_cast<const TThreadLocalPointerBase<_Ty>*>(this)));
				return retval;
			}
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator _Ty*() const {
				_Ty* retval = std::addressof(*(*this))/*(*static_cast<const TThreadLocalPointerBase<_Ty>*>(this))*/;
				return retval;
			}
			explicit operator TThreadLocalObj<_Ty>*() const {
				TThreadLocalObj<_Ty>* retval = (*static_cast<const TThreadLocalPointerBase<_Ty>*>(this));
				return retval;
			}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TThreadLocalNotNullPointer<_Ty>;
			friend class mse::us::impl::TCommonizedPointer<_Ty, TThreadLocalPointer<_Ty> >;
			friend class mse::us::impl::TCommonizedConstPointer<const _Ty, TThreadLocalPointer<_Ty> >;
		};

		/* Use TThreadLocalFixedConstPointer instead. */
		template<typename _Ty>
		class TThreadLocalConstPointer : public TThreadLocalConstPointerBase<const _Ty>, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
		public:
			typedef Tthread_local_obj_base_const_ptr<_Ty> scope_obj_base_const_ptr_t;
			virtual ~TThreadLocalConstPointer() {}
		private:
			TThreadLocalConstPointer() : TThreadLocalConstPointerBase<const _Ty>() {}
			TThreadLocalConstPointer(scope_obj_base_const_ptr_t ptr) : TThreadLocalConstPointerBase<const _Ty>(ptr) {}
			TThreadLocalConstPointer(const TThreadLocalConstPointer& src_cref) : TThreadLocalConstPointerBase<const _Ty>(static_cast<const TThreadLocalConstPointerBase<const _Ty>&>(src_cref)) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalConstPointer(const TThreadLocalConstPointer<_Ty2>& src_cref) : TThreadLocalConstPointerBase<const _Ty>(src_cref) {}
			TThreadLocalConstPointer(const TThreadLocalPointer<_Ty>& src_cref) : TThreadLocalConstPointerBase<const _Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalConstPointer(const TThreadLocalPointer<_Ty2>& src_cref) : TThreadLocalConstPointerBase<const _Ty>(TThreadLocalConstPointerBase<_Ty2>(src_cref)) {}
			TThreadLocalConstPointer<_Ty>& operator=(const TThreadLocalObj<_Ty>* ptr) {
				return TThreadLocalConstPointerBase<_Ty>::operator=(ptr);
			}
			TThreadLocalConstPointer<_Ty>& operator=(const TThreadLocalConstPointer<_Ty>& _Right_cref) {
				return TThreadLocalConstPointerBase<_Ty>::operator=(_Right_cref);
			}
			operator bool() const {
				bool retval = (bool(*static_cast<const TThreadLocalConstPointerBase<const _Ty>*>(this)));
				return retval;
			}
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const {
				const _Ty* retval = (*static_cast<const TThreadLocalConstPointerBase<const _Ty>*>(this));
				return retval;
			}
			explicit operator const TThreadLocalObj<_Ty>*() const {
				const TThreadLocalObj<_Ty>* retval = (*static_cast<const TThreadLocalConstPointerBase<const _Ty>*>(this));
				return retval;
			}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TThreadLocalNotNullConstPointer<_Ty>;
			friend class mse::us::impl::TCommonizedConstPointer<const _Ty, TThreadLocalConstPointer<_Ty> >;
		};

		/* Use TThreadLocalFixedPointer instead. */
		template<typename _Ty>
		class TThreadLocalNotNullPointer : public TThreadLocalPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
		public:
			virtual ~TThreadLocalNotNullPointer() {}
		private:
			TThreadLocalNotNullPointer(typename TThreadLocalPointer<_Ty>::scope_obj_base_ptr_t src_cref) : TThreadLocalPointer<_Ty>(src_cref) {}
			TThreadLocalNotNullPointer(TThreadLocalObj<_Ty>* ptr) : TThreadLocalPointer<_Ty>(ptr) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalNotNullPointer(const TThreadLocalNotNullPointer<_Ty2>& src_cref) : TThreadLocalPointer<_Ty>(src_cref) {}
			TThreadLocalNotNullPointer<_Ty>& operator=(const TThreadLocalPointer<_Ty>& _Right_cref) {
				TThreadLocalPointer<_Ty>::operator=(_Right_cref);
				return (*this);
			}
			operator bool() const { return (*static_cast<const TThreadLocalPointer<_Ty>*>(this)); }
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator _Ty*() const { return TThreadLocalPointer<_Ty>::operator _Ty*(); }
			explicit operator TThreadLocalObj<_Ty>*() const { return TThreadLocalPointer<_Ty>::operator TThreadLocalObj<_Ty>*(); }

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TThreadLocalFixedPointer<_Ty>;
		};

		/* Use TThreadLocalFixedConstPointer instead. */
		template<typename _Ty>
		class TThreadLocalNotNullConstPointer : public TThreadLocalConstPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
		public:
			virtual ~TThreadLocalNotNullConstPointer() {}
		private:
			TThreadLocalNotNullConstPointer(const TThreadLocalNotNullConstPointer<_Ty>& src_cref) : TThreadLocalConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalNotNullConstPointer(const TThreadLocalNotNullConstPointer<_Ty2>& src_cref) : TThreadLocalConstPointer<_Ty>(src_cref) {}
			TThreadLocalNotNullConstPointer(const TThreadLocalNotNullPointer<_Ty>& src_cref) : TThreadLocalConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalNotNullConstPointer(const TThreadLocalNotNullPointer<_Ty2>& src_cref) : TThreadLocalConstPointer<_Ty>(src_cref) {}
			operator bool() const { return (*static_cast<const TThreadLocalConstPointer<_Ty>*>(this)); }
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const { return TThreadLocalConstPointer<_Ty>::operator const _Ty*(); }
			explicit operator const TThreadLocalObj<_Ty>*() const { return TThreadLocalConstPointer<_Ty>::operator const TThreadLocalObj<_Ty>*(); }
			TThreadLocalNotNullConstPointer(typename TThreadLocalConstPointer<_Ty>::scope_obj_base_const_ptr_t ptr) : TThreadLocalConstPointer<_Ty>(ptr) {}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TThreadLocalFixedConstPointer<_Ty>;
		};

		template<typename _Ty>
		class TThreadLocalFixedPointer : public TThreadLocalNotNullPointer<_Ty> {
		public:
			TThreadLocalFixedPointer(const TThreadLocalFixedPointer& src_cref) : TThreadLocalNotNullPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalFixedPointer(const TThreadLocalFixedPointer<_Ty2>& src_cref) : TThreadLocalNotNullPointer<_Ty>(src_cref) {}
			virtual ~TThreadLocalFixedPointer() {}
			operator bool() const { return (*static_cast<const TThreadLocalNotNullPointer<_Ty>*>(this)); }
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator _Ty*() const { return TThreadLocalNotNullPointer<_Ty>::operator _Ty*(); }
			explicit operator TThreadLocalObj<_Ty>*() const { return TThreadLocalNotNullPointer<_Ty>::operator TThreadLocalObj<_Ty>*(); }
			void thread_local_tag() const {}

		private:
			TThreadLocalFixedPointer(typename TThreadLocalPointer<_Ty>::scope_obj_base_ptr_t ptr) : TThreadLocalNotNullPointer<_Ty>(ptr) {}
#ifdef MSE_THREADLOCAL_DISABLE_MOVE_RESTRICTIONS
			TThreadLocalFixedPointer(TThreadLocalFixedPointer&& src_ref) : TThreadLocalNotNullPointer<_Ty>(src_ref) {
				int q = 5;
		}
#endif // !MSE_THREADLOCAL_DISABLE_MOVE_RESTRICTIONS
			TThreadLocalFixedPointer<_Ty>& operator=(const TThreadLocalFixedPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TThreadLocalObj<_Ty>;
	};

		template<typename _Ty>
		class TThreadLocalFixedConstPointer : public TThreadLocalNotNullConstPointer<_Ty> {
		public:
			TThreadLocalFixedConstPointer(const TThreadLocalFixedConstPointer<_Ty>& src_cref) : TThreadLocalNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalFixedConstPointer(const TThreadLocalFixedConstPointer<_Ty2>& src_cref) : TThreadLocalNotNullConstPointer<_Ty>(src_cref) {}
			TThreadLocalFixedConstPointer(const TThreadLocalFixedPointer<_Ty>& src_cref) : TThreadLocalNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TThreadLocalFixedConstPointer(const TThreadLocalFixedPointer<_Ty2>& src_cref) : TThreadLocalNotNullConstPointer<_Ty>(src_cref) {}
			virtual ~TThreadLocalFixedConstPointer() {}
			operator bool() const { return (*static_cast<const TThreadLocalNotNullConstPointer<_Ty>*>(this)); }
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const { return TThreadLocalNotNullConstPointer<_Ty>::operator const _Ty*(); }
			explicit operator const TThreadLocalObj<_Ty>*() const { return TThreadLocalNotNullConstPointer<_Ty>::operator const TThreadLocalObj<_Ty>*(); }
			void thread_local_tag() const {}

		private:
			TThreadLocalFixedConstPointer(typename TThreadLocalConstPointer<_Ty>::scope_obj_base_const_ptr_t ptr) : TThreadLocalNotNullConstPointer<_Ty>(ptr) {}
			TThreadLocalFixedConstPointer<_Ty>& operator=(const TThreadLocalFixedConstPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TThreadLocalObj<_Ty>;
		};

		template<typename _TROy>
		class TThreadLocalObj : public TThreadLocalObjBase<_TROy> {
		public:
			TThreadLocalObj(const TThreadLocalObj& _X) : TThreadLocalObjBase<_TROy>(_X) {}

#ifdef MSE_THREADLOCAL_DISABLE_MOVE_RESTRICTIONS
			explicit TThreadLocalObj(TThreadLocalObj&& _X) : TThreadLocalObjBase<_TROy>(std::forward<decltype(_X)>(_X)) {}
#endif // !MSE_THREADLOCAL_DISABLE_MOVE_RESTRICTIONS

			MSE_THREADLOCAL_USING(TThreadLocalObj, TThreadLocalObjBase<_TROy>);
			virtual ~TThreadLocalObj() {}

			TThreadLocalObj& operator=(TThreadLocalObj&& _X) {
				valid_if_not_rvalue_reference_of_given_type_msethreadlocal<TThreadLocalObj, decltype(_X)>(_X);
				TThreadLocalObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
				return (*this);
			}
			TThreadLocalObj& operator=(const TThreadLocalObj& _X) { TThreadLocalObjBase<_TROy>::operator=(_X); return (*this); }
			template<class _Ty2>
			TThreadLocalObj& operator=(_Ty2&& _X) {
				TThreadLocalObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
				return (*this);
			}
			template<class _Ty2>
			TThreadLocalObj& operator=(const _Ty2& _X) { TThreadLocalObjBase<_TROy>::operator=(_X); return (*this); }

			const TThreadLocalFixedPointer<_TROy> operator&() & {
				return &(*static_cast<TThreadLocalObjBase<_TROy>*>(this));
			}
			const TThreadLocalFixedConstPointer<_TROy> operator&() const & {
				return &(*static_cast<const TThreadLocalObjBase<_TROy>*>(this));
			}
			const TThreadLocalFixedPointer<_TROy> mse_thread_local_fptr() & { return &(*this); }
			const TThreadLocalFixedConstPointer<_TROy> mse_thread_local_fptr() const & { return &(*this); }

			void operator&() && = delete;
			void operator&() const && = delete;
			void mse_thread_local_fptr() && = delete;
			void mse_thread_local_fptr() const && = delete;

			void thread_local_tag() const {}

		private:
			void* operator new(size_t size) { return ::operator new(size); }
		};

		template<typename _Ty>
		auto thread_local_fptr_to(_Ty&& _X) {
			return _X.mse_thread_local_fptr();
		}
		template<typename _Ty>
		auto thread_local_fptr_to(const _Ty& _X) {
			return _X.mse_thread_local_fptr();
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::rsv::TThreadLocalFixedPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TThreadLocalFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TThreadLocalFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::rsv::TThreadLocalFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TThreadLocalFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TThreadLocalFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {
	namespace rsv {

		/* template specializations */

		template<typename _Ty>
		class TThreadLocalObj<_Ty*> : public TThreadLocalObj<mse::us::impl::TPointerForLegacy<_Ty>> {
		public:
			typedef TThreadLocalObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};
		template<typename _Ty>
		class TThreadLocalObj<_Ty* const> : public TThreadLocalObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
		public:
			typedef TThreadLocalObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};
		template<typename _Ty>
		class TThreadLocalObj<const _Ty *> : public TThreadLocalObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
		public:
			typedef TThreadLocalObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};
		template<typename _Ty>
		class TThreadLocalObj<const _Ty * const> : public TThreadLocalObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
		public:
			typedef TThreadLocalObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};

#ifdef MSEPRIMITIVES_H
		template<>
		class TThreadLocalObj<int> : public TThreadLocalObj<mse::TInt<int>> {
		public:
			typedef TThreadLocalObj<mse::TInt<int>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};
		template<>
		class TThreadLocalObj<const int> : public TThreadLocalObj<const mse::TInt<int>> {
		public:
			typedef TThreadLocalObj<const mse::TInt<int>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};

		template<>
		class TThreadLocalObj<size_t> : public TThreadLocalObj<mse::TInt<size_t>> {
		public:
			typedef TThreadLocalObj<mse::TInt<size_t>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};
		template<>
		class TThreadLocalObj<const size_t> : public TThreadLocalObj<const mse::TInt<size_t>> {
		public:
			typedef TThreadLocalObj<const mse::TInt<size_t>> base_class;
			MSE_USING(TThreadLocalObj, base_class);
		};
#endif /*MSEPRIMITIVES_H*/

		/* end of template specializations */

#endif /*MSE_THREADLOCALPOINTER_DISABLED*/
	}

#define MSE_DECLARE_THREAD_LOCAL(type) thread_local mse::rsv::TThreadLocalObj<type> 
#define MSE_DECLARE_THREAD_LOCAL_CONST(type) thread_local const mse::rsv::TThreadLocalObj<type> 
#define MSE_DECLARE_THREAD_LOCAL_GLOBAL(type) MSE_DECLARE_THREAD_LOCAL(type) 
#define MSE_DECLARE_THREAD_LOCAL_GLOBAL_CONST(type) MSE_DECLARE_THREAD_LOCAL_CONST(type) 


	namespace self_test {
		class CThreadLocalPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class A {
				public:
					A(int x) : b(x) {}
					A(const A& _X) : b(_X.b) {}
					A(A&& _X) : b(std::forward<decltype(_X.b)>(_X.b)) {}
					virtual ~A() {}
					A& operator=(A&& _X) { b = std::forward<decltype(_X.b)>(_X.b); return (*this); }
					A& operator=(const A& _X) { b = _X.b; return (*this); }

					int b = 3;
				};
				class B {
				public:
					static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
					static int foo2(mse::rsv::TThreadLocalFixedPointer<A> A_thread_local_ptr) { return A_thread_local_ptr->b; }
				protected:
					~B() {}
				};

				A* A_native_ptr = nullptr;

				{
					A a(7);
					MSE_DECLARE_THREAD_LOCAL(A) thread_local_a(7);

					assert(a.b == thread_local_a.b);
					A_native_ptr = &a;

					mse::rsv::TThreadLocalFixedPointer<A> A_thread_local_ptr1(&thread_local_a);
					assert(A_native_ptr->b == A_thread_local_ptr1->b);
					mse::rsv::TThreadLocalFixedPointer<A> A_thread_local_ptr2 = &thread_local_a;

					/* mse::rsv::TThreadLocalFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<A*>(A_thread_local_ptr1));

					if (!A_thread_local_ptr2) {
						assert(false);
					}
					else if (!(A_thread_local_ptr2 != A_thread_local_ptr1)) {
						int q = B::foo2(A_thread_local_ptr2);
					}
					else {
						assert(false);
					}

					A a2 = a;
					MSE_DECLARE_THREAD_LOCAL(A) thread_local_a2 = thread_local_a;
					thread_local_a2 = a;
					thread_local_a2 = thread_local_a;

					mse::rsv::TThreadLocalFixedConstPointer<A> rcp = A_thread_local_ptr1;
					mse::rsv::TThreadLocalFixedConstPointer<A> rcp2 = rcp;
					MSE_DECLARE_THREAD_LOCAL_CONST(A) cthread_local_a(11);
					mse::rsv::TThreadLocalFixedConstPointer<A> rfcp = &cthread_local_a;
				}

				{
					/* Polymorphic conversions. */
					class E {
					public:
						int m_b = 5;
					};

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					MSE_DECLARE_THREAD_LOCAL(GE) thread_local_gd;
					mse::rsv::TThreadLocalFixedPointer<GE> GE_thread_local_ifptr1 = &thread_local_gd;
					mse::rsv::TThreadLocalFixedPointer<E> E_thread_local_ifptr5 = GE_thread_local_ifptr1;
					mse::rsv::TThreadLocalFixedPointer<E> E_thread_local_fptr2(&thread_local_gd);
					mse::rsv::TThreadLocalFixedPointer<E> E_thread_local_ifptr2(&thread_local_gd);
					mse::rsv::TThreadLocalFixedConstPointer<E> E_thread_local_fcptr2 = &thread_local_gd;
				}

				{
					class A {
					public:
						A(int x) : b(x) {}
						virtual ~A() {}

						int b = 3;
						std::string s = "some text ";
					};
					class B {
					public:
						static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
						static int foo2(mse::rsv::TThreadLocalFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
						static int foo3(mse::rsv::TThreadLocalFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
					protected:
						~B() {}
					};

					MSE_DECLARE_THREAD_LOCAL(A) a_scpobj(5);
					int res1 = (&a_scpobj)->b;
					int res2 = B::foo2(&a_scpobj);
					int res3 = B::foo3(&a_scpobj);

					/* You can use the "mse::make_pointer_to_member_v2()" function to obtain a safe pointer to a member of
					an thread_local object. */
					auto s_safe_ptr1 = mse::make_pointer_to_member_v2((&a_scpobj), &A::s);
					(*s_safe_ptr1) = "some new text";
					auto s_safe_const_ptr1 = mse::make_const_pointer_to_member_v2((&a_scpobj), &A::s);
				}

				{
					A a(7);
					MSE_DECLARE_THREAD_LOCAL(A) thread_local_a(7);

					assert(a.b == thread_local_a.b);
					A_native_ptr = &a;

					mse::rsv::TThreadLocalFixedPointer<A> A_thread_local_ptr1 = &thread_local_a;
					assert(A_native_ptr->b == A_thread_local_ptr1->b);
					mse::rsv::TThreadLocalFixedPointer<A> A_thread_local_ptr2 = &thread_local_a;

					/* mse::rsv::TThreadLocalFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<A*>(A_thread_local_ptr1));

					if (!A_thread_local_ptr2) {
						assert(false);
					}
					else if (!(A_thread_local_ptr2 != A_thread_local_ptr1)) {
						int q = B::foo2(A_thread_local_ptr2);
					}
					else {
						assert(false);
					}

					A a2 = a;
					MSE_DECLARE_THREAD_LOCAL(A) thread_local_a2 = thread_local_a;
					thread_local_a2 = a;
					thread_local_a2 = thread_local_a;

					mse::rsv::TThreadLocalFixedConstPointer<A> rcp = A_thread_local_ptr1;
					mse::rsv::TThreadLocalFixedConstPointer<A> rcp2 = rcp;
					MSE_DECLARE_THREAD_LOCAL_CONST(A) cthread_local_a(11);
					mse::rsv::TThreadLocalFixedConstPointer<A> rfcp = &cthread_local_a;
				}

				{
					/* Polymorphic conversions. */
					class E {
					public:
						int m_b = 5;
					};

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					MSE_DECLARE_THREAD_LOCAL(GE) thread_local_gd;
					mse::rsv::TThreadLocalFixedPointer<GE> GE_thread_local_ifptr1 = &thread_local_gd;
					mse::rsv::TThreadLocalFixedPointer<E> E_thread_local_ptr5(GE_thread_local_ifptr1);
					mse::rsv::TThreadLocalFixedPointer<E> E_thread_local_ifptr2(&thread_local_gd);
					mse::rsv::TThreadLocalFixedConstPointer<E> E_thread_local_fcptr2 = &thread_local_gd;
				}

#endif // MSE_SELF_TESTS
			}
		};
	}

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

#endif // MSETHREADLOCAL_H_
