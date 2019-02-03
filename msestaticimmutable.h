
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESTATIC_H_
#define MSESTATIC_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include "msemsearray.h"
//#include "mseasyncshared.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED
#include "msenorad.h"
#include <atomic>
#include <mutex>
#endif // MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED

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

/* Note that by default, MSE_STATICPOINTER_DISABLED is defined in non-debug builds. This is enacted in "msepointerbasics.h". */

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


namespace mse {
	namespace rsv {

		/* This macro roughly simulates constructor inheritance. */
#define MSE_STATIC_USING(Derived, Base) MSE_USING(Derived, Base)

		template<typename _Ty> class TStaticImmutableID {};

		template<typename _Ty>
		class TPlaceHolder_msestatic {};
		template<typename _Ty>
		class TPlaceHolder2_msestatic {};

#ifdef MSE_STATICPOINTER_DISABLED
		//TStaticImmutableID
		template<typename _Ty> using TStaticImmutableConstPointer = const _Ty*;
		template<typename _Ty> using TStaticImmutablePointer = TStaticImmutableConstPointer<_Ty>;
		template<typename _Ty> using TStaticImmutableNotNullConstPointer = const _Ty*;
		template<typename _Ty> using TStaticImmutableNotNullPointer = TStaticImmutableNotNullConstPointer<_Ty>;
		template<typename _Ty> using TStaticImmutableFixedConstPointer = const _Ty* /*const*/;
		template<typename _Ty> using TStaticImmutableFixedPointer = TStaticImmutableFixedConstPointer<_Ty>;
		template<typename _TROy> using TStaticImmutableObjBase = _TROy;
		template<typename _TROy> using TStaticImmutableObj = _TROy;

		template<typename _Ty> auto static_fptr_to(_Ty&& _X) { return std::addressof(_X); }
		template<typename _Ty> auto static_fptr_to(const _Ty& _X) { return std::addressof(_X); }

#else /*MSE_STATICPOINTER_DISABLED*/

#if defined(MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED)

		namespace impl {
			namespace cts {
#ifndef MSE_CHECKED_THREAD_SAFE_DO_NOT_USE_GNORAD
				class CNoOpCopyAtomicInt : public std::atomic<int> {
				public:
					typedef std::atomic<int> base_class;
					CNoOpCopyAtomicInt(int i) : base_class(i) {}
					CNoOpCopyAtomicInt(const CNoOpCopyAtomicInt&) : base_class(0) {}
				};
				typedef CNoOpCopyAtomicInt atomic_int_t;
				template<typename _Ty> using TCheckedThreadSafeObj = mse::impl::TGNoradObj<_Ty, atomic_int_t>;
				template<typename _Ty> using TCheckedThreadSafePointer = mse::impl::TGNoradPointer<_Ty, atomic_int_t>;
				template<typename _Ty> using TCheckedThreadSafeConstPointer = mse::impl::TGNoradConstPointer<_Ty, atomic_int_t>;
				template<typename _Ty> using TCheckedThreadSafeFixedPointer = mse::impl::TGNoradFixedPointer<_Ty, atomic_int_t>;
				template<typename _Ty> using TCheckedThreadSafeFixedConstPointer = mse::impl::TGNoradFixedConstPointer<_Ty, atomic_int_t>;
#else // !MSE_CHECKED_THREAD_SAFE_DO_NOT_USE_GNORAD
				/* TCheckedThreadSafePointer<> is essentially just a simplified TNoradPointer<> with an atomic refcounter. */
				template<typename _Ty> class TCheckedThreadSafeObj;
				template<typename _Ty> class TCheckedThreadSafePointer;
				template<typename _Ty> class TCheckedThreadSafeConstPointer;
				template<typename _Ty> using TCheckedThreadSafeFixedPointer = TCheckedThreadSafePointer<_Ty>;
				template<typename _Ty> using TCheckedThreadSafeFixedConstPointer = TCheckedThreadSafeConstPointer<_Ty>;

				template<typename _Ty>
				class TCheckedThreadSafePointer : public mse::us::impl::TPointer<TCheckedThreadSafeObj<_Ty> >, public mse::us::impl::StrongPointerTagBase {
				public:
					TCheckedThreadSafePointer(const TCheckedThreadSafePointer& src_cref) : mse::us::impl::TPointer<TCheckedThreadSafeObj<_Ty>>(src_cref.m_ptr) {
						if (*this) { (*(*this)).increment_refcount(); }
					}
					template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
					TCheckedThreadSafePointer(const TCheckedThreadSafePointer<_Ty2>& src_cref) : mse::us::impl::TPointer<TCheckedThreadSafeObj<_Ty>>(src_cref.m_ptr) {
						if (*this) { (*(*this)).increment_refcount(); }
					}
					TCheckedThreadSafePointer(TCheckedThreadSafePointer&& src_ref) : mse::us::impl::TPointer<TCheckedThreadSafeObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
						src_ref.m_ptr = nullptr;
					}
					virtual ~TCheckedThreadSafePointer() {
						if (*this) { (*(*this)).decrement_refcount(); }
					}
					TCheckedThreadSafePointer<_Ty>& operator=(const TCheckedThreadSafePointer<_Ty>& _Right_cref) {
						if (*this) { (*(*this)).decrement_refcount(); }
						mse::us::impl::TPointer<TCheckedThreadSafeObj<_Ty>>::operator=(_Right_cref);
						if (*this) { (*(*this)).increment_refcount(); }
						return (*this);
					}
					template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
					TCheckedThreadSafePointer<_Ty>& operator=(const TCheckedThreadSafePointer<_Ty2>& _Right_cref) {
						return (*this).operator=(TCheckedThreadSafePointer(_Right_cref));
					}

					TCheckedThreadSafeObj<_Ty>& operator*() const {
						if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TTSNoradPointer")); }
						return *((*this).m_ptr);
					}
					TCheckedThreadSafeObj<_Ty>* operator->() const {
						if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TTSNoradPointer")); }
						return (*this).m_ptr;
					}

					operator bool() const { return !(!((*this).m_ptr)); }

				private:
					TCheckedThreadSafePointer(TCheckedThreadSafeObj<_Ty>* ptr) : mse::us::impl::TPointer<TCheckedThreadSafeObj<_Ty>>(ptr) {
						assert(*this);
						(*(*this)).increment_refcount();
					}

					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					friend class TCheckedThreadSafeObj<_Ty>;
				};

				template<typename _Ty>
				class TCheckedThreadSafeConstPointer : public mse::us::impl::TPointer<const TCheckedThreadSafeObj<_Ty> >, public mse::us::impl::StrongPointerTagBase {
				public:
					TCheckedThreadSafeConstPointer(const TCheckedThreadSafeConstPointer& src_cref) : mse::us::impl::TPointer<const TCheckedThreadSafeObj<_Ty>>(src_cref.m_ptr) {
						if (*this) { (*(*this)).increment_refcount(); }
					}
					template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
					TCheckedThreadSafeConstPointer(const TCheckedThreadSafeConstPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<const TCheckedThreadSafeObj<_Ty>>(src_cref.m_ptr) {
						if (*this) { (*(*this)).increment_refcount(); }
					}
					TCheckedThreadSafeConstPointer(TCheckedThreadSafeConstPointer&& src_ref) : mse::us::impl::TPointer<const TCheckedThreadSafeObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
						src_ref.m_ptr = nullptr;
					}
					virtual ~TCheckedThreadSafeConstPointer() {
						if (*this) { (*(*this)).decrement_refcount(); }
					}
					TCheckedThreadSafeConstPointer<_Ty>& operator=(const TCheckedThreadSafeConstPointer<_Ty>& _Right_cref) {
						if (*this) { (*(*this)).decrement_refcount(); }
						mse::us::impl::TPointer<const TCheckedThreadSafeObj<_Ty>>::operator=(_Right_cref);
						if (*this) { (*(*this)).increment_refcount(); }
						return (*this);
					}
					template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
					TCheckedThreadSafeConstPointer<_Ty>& operator=(const TCheckedThreadSafeConstPointer<_Ty2>& _Right_cref) {
						return (*this).operator=(TCheckedThreadSafeConstPointer(_Right_cref));
					}

					const TCheckedThreadSafeObj<_Ty>& operator*() const {
						if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TTSNoradPointer")); }
						return *((*this).m_ptr);
					}
					const TCheckedThreadSafeObj<_Ty>* operator->() const {
						if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TTSNoradPointer")); }
						return (*this).m_ptr;
					}

					operator bool() const { return !(!((*this).m_ptr)); }

				private:
					TCheckedThreadSafeConstPointer(const TCheckedThreadSafeObj<_Ty>* ptr) : mse::us::impl::TPointer<const TCheckedThreadSafeObj<_Ty>>(ptr) {
						assert(*this);
						(*(*this)).increment_refcount();
					}

					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					friend class TCheckedThreadSafeObj<_Ty>;
				};

				template<typename _TROFLy>
				class TCheckedThreadSafeObj : public _TROFLy
				{
				public:
					MSE_USING(TCheckedThreadSafeObj, _TROFLy);
					TCheckedThreadSafeObj(const TCheckedThreadSafeObj& _X) : _TROFLy(_X) {}
					TCheckedThreadSafeObj(TCheckedThreadSafeObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)) {}
					virtual ~TCheckedThreadSafeObj() {
						std::lock_guard<decltype(m_mutex1)> lock1(m_mutex1);
						if (0 != m_counter) {
							/* It would be unsafe to allow this object to be destroyed as there are outstanding references to this object. */
							std::cerr << "\n\nFatal Error: mse::TCheckedThreadSafeObj<> destructed with outstanding references \n\n";
							std::terminate();
						}
					}

					template<class _Ty2>
					TCheckedThreadSafeObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
					template<class _Ty2>
					TCheckedThreadSafeObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

					TCheckedThreadSafeFixedPointer<_TROFLy> operator&() {
						return TCheckedThreadSafeFixedPointer<_TROFLy>(this);
					}
					TCheckedThreadSafeFixedConstPointer<_TROFLy> operator&() const {
						return TCheckedThreadSafeFixedConstPointer<_TROFLy>(this);
					}

					/* todo: make these private */
					void increment_refcount() const { std::lock_guard<decltype(m_mutex1)> lock1(m_mutex1); m_counter += 1; }
					void decrement_refcount() const { std::lock_guard<decltype(m_mutex1)> lock1(m_mutex1); m_counter -= 1; }

				private:
					mutable std::mutex m_mutex1;
					mutable int m_counter = 0;
				};
#endif // !MSE_CHECKED_THREAD_SAFE_DO_NOT_USE_GNORAD
			}
		}

		template<typename _TROz> using TStaticImmutableObjBase = mse::rsv::impl::cts::TCheckedThreadSafeObj<_TROz>;
		template<typename _Ty> using TStaticImmutableConstPointerBase = mse::rsv::impl::cts::TCheckedThreadSafeConstPointer<_Ty>;

#else // MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED

		template<typename _TROz>
		class TStaticImmutableObjBase : public std::add_const<_TROz>::type {
		public:
			typedef typename std::add_const<_TROz>::type base_class;
			MSE_STATIC_USING(TStaticImmutableObjBase, base_class);
			TStaticImmutableObjBase(const TStaticImmutableObjBase& _X) : base_class(_X) {}
			TStaticImmutableObjBase(TStaticImmutableObjBase&& _X) : base_class(std::forward<decltype(_X)>(_X)) {}

			TStaticImmutableObjBase& operator=(TStaticImmutableObjBase&& _X) { base_class::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			TStaticImmutableObjBase& operator=(const TStaticImmutableObjBase& _X) { base_class::operator=(_X); return (*this); }
			template<class _Ty2>
			TStaticImmutableObjBase& operator=(_Ty2&& _X) { base_class::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			template<class _Ty2>
			TStaticImmutableObjBase& operator=(const _Ty2& _X) { base_class::operator=(_X); return (*this); }

			auto operator&() {
				return this;
			}
			auto operator&() const {
				return this;
			}
		};

		template<typename _Ty> using TStaticImmutableConstPointerBase = mse::us::impl::TPointerForLegacy<const _Ty, TStaticImmutableID<const _Ty>>;

#endif // MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED

		template <class _Ty, class _Ty2, class = typename std::enable_if<
			(!std::is_same<_Ty&&, _Ty2>::value) || (!std::is_rvalue_reference<_Ty2>::value)
			, void>::type>
			static void valid_if_not_rvalue_reference_of_given_type_msestatic(_Ty2 src) {}

		template<typename _Ty> class TStaticImmutableObj;
		template<typename _Ty> class TStaticImmutableNotNullPointer;
		template<typename _Ty> class TStaticImmutableNotNullConstPointer;
		template<typename _Ty> class TStaticImmutableFixedConstPointer;
		template<typename _Ty> using TStaticImmutableFixedPointer = TStaticImmutableFixedConstPointer<_Ty>;
	}

	namespace us {
		namespace impl {
			template <typename _Ty, typename _TConstPointer1> class TCommonizedPointer;
			template <typename _Ty, typename _TConstPointer1> class TCommonizedConstPointer;
		}
	}

	namespace rsv {

		/* Use TStaticImmutableFixedConstPointer instead. */
		template<typename _Ty>
		class TStaticImmutableConstPointer : public TStaticImmutableConstPointerBase<_Ty>
			, public std::conditional<std::is_base_of<mse::us::impl::StrongPointerTagBase, TStaticImmutableConstPointerBase<_Ty> >::value, mse::impl::TPlaceHolder_msescope<TStaticImmutableConstPointer<_Ty> >, mse::us::impl::StrongPointerTagBase>::type
		{
		public:
			typedef TStaticImmutableConstPointerBase<_Ty> base_class;
			virtual ~TStaticImmutableConstPointer() {}
		private:
			TStaticImmutableConstPointer() : base_class() {}
			TStaticImmutableConstPointer(const base_class& ptr) : base_class(ptr) {}
			TStaticImmutableConstPointer(const TStaticImmutableConstPointer& src_cref) : base_class(static_cast<const base_class&>(src_cref)) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TStaticImmutableConstPointer(const TStaticImmutableConstPointer<_Ty2>& src_cref) : base_class(src_cref) {}
			//TStaticImmutableConstPointer(const TStaticImmutablePointer<_Ty>& src_cref) : base_class(src_cref) {}
			//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			//TStaticImmutableConstPointer(const TStaticImmutablePointer<_Ty2>& src_cref) : base_class(TStaticImmutableConstPointerBase<_Ty2>(src_cref)) {}
			TStaticImmutableConstPointer<_Ty>& operator=(const TStaticImmutableObj<_Ty>* ptr) {
				return TStaticImmutableConstPointerBase<_Ty>::operator=(ptr);
			}
			TStaticImmutableConstPointer<_Ty>& operator=(const TStaticImmutableConstPointer<_Ty>& _Right_cref) {
				return TStaticImmutableConstPointerBase<_Ty>::operator=(_Right_cref);
			}
			operator bool() const {
				bool retval = (bool(*static_cast<const base_class*>(this)));
				return retval;
			}
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const {
				const _Ty* retval = std::addressof(*(*static_cast<const base_class*>(this)));
				return retval;
			}
			explicit operator const TStaticImmutableObj<_Ty>*() const {
				const TStaticImmutableObj<_Ty>* retval = std::addressof(*(*static_cast<const base_class*>(this)));
				return retval;
			}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TStaticImmutableNotNullConstPointer<_Ty>;
			friend class mse::us::impl::TCommonizedConstPointer<const _Ty, TStaticImmutableConstPointer<_Ty> >;
		};

		/* Use TStaticImmutableFixedConstPointer instead. */
		template<typename _Ty>
		class TStaticImmutableNotNullConstPointer : public TStaticImmutableConstPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
		public:
			virtual ~TStaticImmutableNotNullConstPointer() {}
		private:
			TStaticImmutableNotNullConstPointer(const TStaticImmutableNotNullConstPointer<_Ty>& src_cref) : TStaticImmutableConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TStaticImmutableNotNullConstPointer(const TStaticImmutableNotNullConstPointer<_Ty2>& src_cref) : TStaticImmutableConstPointer<_Ty>(src_cref) {}
			//TStaticImmutableNotNullConstPointer(const TStaticImmutableNotNullPointer<_Ty>& src_cref) : TStaticImmutableConstPointer<_Ty>(src_cref) {}
			//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			//TStaticImmutableNotNullConstPointer(const TStaticImmutableNotNullPointer<_Ty2>& src_cref) : TStaticImmutableConstPointer<_Ty>(src_cref) {}
			operator bool() const { return (*static_cast<const TStaticImmutableConstPointer<_Ty>*>(this)); }
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const { return TStaticImmutableConstPointer<_Ty>::operator const _Ty*(); }
			explicit operator const TStaticImmutableObj<_Ty>*() const { return TStaticImmutableConstPointer<_Ty>::operator const TStaticImmutableObj<_Ty>*(); }
			TStaticImmutableNotNullConstPointer(const typename TStaticImmutableConstPointer<_Ty>::base_class& ptr) : TStaticImmutableConstPointer<_Ty>(ptr) {}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TStaticImmutableFixedConstPointer<_Ty>;
		};

		template<typename _Ty>
		class TStaticImmutableFixedConstPointer : public TStaticImmutableNotNullConstPointer<_Ty> {
		public:
			TStaticImmutableFixedConstPointer(const TStaticImmutableFixedConstPointer<_Ty>& src_cref) : TStaticImmutableNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TStaticImmutableFixedConstPointer(const TStaticImmutableFixedConstPointer<_Ty2>& src_cref) : TStaticImmutableNotNullConstPointer<_Ty>(src_cref) {}
			//TStaticImmutableFixedConstPointer(const TStaticImmutableFixedPointer<_Ty>& src_cref) : TStaticImmutableNotNullConstPointer<_Ty>(src_cref) {}
			//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			//TStaticImmutableFixedConstPointer(const TStaticImmutableFixedPointer<_Ty2>& src_cref) : TStaticImmutableNotNullConstPointer<_Ty>(src_cref) {}
			virtual ~TStaticImmutableFixedConstPointer() {}
			operator bool() const { return (*static_cast<const TStaticImmutableNotNullConstPointer<_Ty>*>(this)); }
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const { return TStaticImmutableNotNullConstPointer<_Ty>::operator const _Ty*(); }
			explicit operator const TStaticImmutableObj<_Ty>*() const { return TStaticImmutableNotNullConstPointer<_Ty>::operator const TStaticImmutableObj<_Ty>*(); }
			void static_tag() const {}
			void async_passable_tag() const {} /* Indication that this type is eligible to be passed between threads. */

		private:
			TStaticImmutableFixedConstPointer(const typename TStaticImmutableConstPointer<_Ty>::base_class& ptr) : TStaticImmutableNotNullConstPointer<_Ty>(ptr) {}
			TStaticImmutableFixedConstPointer<_Ty>& operator=(const TStaticImmutableFixedConstPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TStaticImmutableObj<_Ty>;
		};

		template<typename _TROy>
		class TStaticImmutableObj : public TStaticImmutableObjBase<_TROy> {
		public:
			typedef TStaticImmutableObjBase<_TROy> base_class;
			TStaticImmutableObj(const TStaticImmutableObj& _X) : TStaticImmutableObjBase<_TROy>(_X) {}

#ifdef MSE_STATIC_DISABLE_MOVE_RESTRICTIONS
			explicit TStaticImmutableObj(TStaticImmutableObj&& _X) : TStaticImmutableObjBase<_TROy>(std::forward<decltype(_X)>(_X)) {}
#endif // !MSE_STATIC_DISABLE_MOVE_RESTRICTIONS

			MSE_STATIC_USING(TStaticImmutableObj, TStaticImmutableObjBase<_TROy>);
			virtual ~TStaticImmutableObj() {
				mse::impl::T_valid_if_is_marked_as_xscope_shareable_msemsearray<_TROy>();
			}

			TStaticImmutableObj& operator=(TStaticImmutableObj&& _X) {
				valid_if_not_rvalue_reference_of_given_type_msestatic<TStaticImmutableObj, decltype(_X)>(_X);
				TStaticImmutableObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
				return (*this);
			}
			TStaticImmutableObj& operator=(const TStaticImmutableObj& _X) { TStaticImmutableObjBase<_TROy>::operator=(_X); return (*this); }
			template<class _Ty2>
			TStaticImmutableObj& operator=(_Ty2&& _X) {
				TStaticImmutableObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
				return (*this);
			}
			template<class _Ty2>
			TStaticImmutableObj& operator=(const _Ty2& _X) { TStaticImmutableObjBase<_TROy>::operator=(_X); return (*this); }
			const TStaticImmutableFixedConstPointer<_TROy> operator&() const & {
				return &(*static_cast<const TStaticImmutableObjBase<_TROy>*>(this));
			}
			const TStaticImmutableFixedConstPointer<_TROy> mse_static_fptr() const & { return &(*this); }

			void operator&() && = delete;
			void operator&() const && = delete;
			void mse_static_fptr() && = delete;
			void mse_static_fptr() const && = delete;

			void static_tag() const {}

		private:

			void operator&() & {
				/* This object does not seem to be declared const, which is not valid. Objects of this type should only be
				declared via the provided macro that declares them (static and) const. */
			}
			void mse_static_fptr() & { &(*this); }

			void* operator new(size_t size) { return ::operator new(size); }
		};

		template<typename _Ty>
		auto static_fptr_to(_Ty&& _X) {
			return _X.mse_static_fptr();
		}
		template<typename _Ty>
		auto static_fptr_to(const _Ty& _X) {
			return _X.mse_static_fptr();
		}
	}
}

namespace std {

	template<class _Ty>
	struct hash<mse::rsv::TStaticImmutableFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TStaticImmutableFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TStaticImmutableFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
		class TStaticImmutableObj<_Ty*> : public TStaticImmutableObj<mse::us::impl::TPointerForLegacy<_Ty>> {
		public:
			typedef TStaticImmutableObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};
		template<typename _Ty>
		class TStaticImmutableObj<_Ty* const> : public TStaticImmutableObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
		public:
			typedef TStaticImmutableObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};
		template<typename _Ty>
		class TStaticImmutableObj<const _Ty *> : public TStaticImmutableObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
		public:
			typedef TStaticImmutableObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};
		template<typename _Ty>
		class TStaticImmutableObj<const _Ty * const> : public TStaticImmutableObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
		public:
			typedef TStaticImmutableObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};

#ifdef MSEPRIMITIVES_H
		template<>
		class TStaticImmutableObj<int> : public TStaticImmutableObj<mse::TInt<int>> {
		public:
			typedef TStaticImmutableObj<mse::TInt<int>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};
		template<>
		class TStaticImmutableObj<const int> : public TStaticImmutableObj<const mse::TInt<int>> {
		public:
			typedef TStaticImmutableObj<const mse::TInt<int>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};

		template<>
		class TStaticImmutableObj<size_t> : public TStaticImmutableObj<mse::TInt<size_t>> {
		public:
			typedef TStaticImmutableObj<mse::TInt<size_t>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};
		template<>
		class TStaticImmutableObj<const size_t> : public TStaticImmutableObj<const mse::TInt<size_t>> {
		public:
			typedef TStaticImmutableObj<const mse::TInt<size_t>> base_class;
			MSE_USING(TStaticImmutableObj, base_class);
		};
#endif /*MSEPRIMITIVES_H*/

		/* end of template specializations */

#endif /*MSE_STATICPOINTER_DISABLED*/
	}

#define MSE_DECLARE_STATIC_IMMUTABLE(type) static const mse::rsv::TStaticImmutableObj<type> 
#define MSE_RSV_DECLARE_GLOBAL_IMMUTABLE(type) const mse::rsv::TStaticImmutableObj<type> 


	namespace self_test {
		class CStaticImmutablePtrTest1 {
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
				typedef mse::rsv::TAsyncShareableObj<A> shareable_A;
				class B {
				public:
					static int foo1(const shareable_A* a_native_ptr) { return a_native_ptr->b; }
					static int foo2(mse::rsv::TStaticImmutableFixedPointer<shareable_A> shareable_A_static_ptr) { return shareable_A_static_ptr->b; }
				protected:
					~B() {}
				};

				shareable_A* shareable_A_native_ptr = nullptr;

				{
					shareable_A a(7);
					MSE_DECLARE_STATIC_IMMUTABLE(shareable_A) static_a(7);

					assert(a.b == static_a.b);
					shareable_A_native_ptr = std::addressof(a);

					mse::rsv::TStaticImmutableFixedPointer<shareable_A> shareable_A_static_ptr1(&static_a);
					assert(shareable_A_native_ptr->b == shareable_A_static_ptr1->b);
					mse::rsv::TStaticImmutableFixedPointer<shareable_A> shareable_A_static_ptr2 = &static_a;

					/* mse::rsv::TStaticImmutableFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<const shareable_A*>(shareable_A_static_ptr1));

					if (!shareable_A_static_ptr2) {
						assert(false);
					}
					else if (!(shareable_A_static_ptr2 != shareable_A_static_ptr1)) {
						int q = B::foo2(shareable_A_static_ptr2);
					}
					else {
						assert(false);
					}

					shareable_A a2 = a;
					MSE_DECLARE_STATIC_IMMUTABLE(shareable_A) static_a2 = static_a;

					mse::rsv::TStaticImmutableFixedConstPointer<shareable_A> rcp = shareable_A_static_ptr1;
					mse::rsv::TStaticImmutableFixedConstPointer<shareable_A> rcp2 = rcp;
				}

				{
					class A {
					public:
						A(int x) : b(x) {}
						virtual ~A() {}

						int b = 3;
						std::string s = "some text ";
					};
					typedef mse::rsv::TAsyncShareableObj<A> shareable_A;
					class B {
					public:
						static int foo1(shareable_A* a_native_ptr) { return a_native_ptr->b; }
						static int foo2(mse::rsv::TStaticImmutableFixedPointer<shareable_A> shareable_A_scpfptr) { return shareable_A_scpfptr->b; }
						static int foo3(mse::rsv::TStaticImmutableFixedConstPointer<shareable_A> shareable_A_scpfcptr) { return shareable_A_scpfcptr->b; }
					protected:
						~B() {}
					};

					MSE_DECLARE_STATIC_IMMUTABLE(shareable_A) a_scpobj(5);
					int res1 = (&a_scpobj)->b;
					int res2 = B::foo2(&a_scpobj);
					int res3 = B::foo3(&a_scpobj);

					/* You can use the "mse::make_pointer_to_member_v2()" function to obtain a safe pointer to a member of
					an static object. */
					auto s_safe_ptr1 = mse::make_pointer_to_member_v2((&a_scpobj), &shareable_A::s);
					auto s_safe_const_ptr1 = mse::make_const_pointer_to_member_v2((&a_scpobj), &shareable_A::s);
				}

				{
					shareable_A a(7);
					MSE_DECLARE_STATIC_IMMUTABLE(shareable_A) static_a(7);

					assert(a.b == static_a.b);
					shareable_A_native_ptr = std::addressof(a);

					mse::rsv::TStaticImmutableFixedPointer<shareable_A> shareable_A_static_ptr1 = &static_a;
					assert(shareable_A_native_ptr->b == shareable_A_static_ptr1->b);
					mse::rsv::TStaticImmutableFixedPointer<shareable_A> shareable_A_static_ptr2 = &static_a;

					/* mse::rsv::TStaticImmutableFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<const shareable_A*>(shareable_A_static_ptr1));

					if (!shareable_A_static_ptr2) {
						assert(false);
					}
					else if (!(shareable_A_static_ptr2 != shareable_A_static_ptr1)) {
						int q = B::foo2(shareable_A_static_ptr2);
					}
					else {
						assert(false);
					}

					shareable_A a2 = a;
					MSE_DECLARE_STATIC_IMMUTABLE(shareable_A) static_a2 = static_a;

					mse::rsv::TStaticImmutableFixedConstPointer<shareable_A> rcp = shareable_A_static_ptr1;
					mse::rsv::TStaticImmutableFixedConstPointer<shareable_A> rcp2 = rcp;
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

#endif // MSESTATIC_H_
