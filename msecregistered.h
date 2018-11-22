
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* TCRegisteredPointers are basically just like TRegisteredPointers except that unlike TRegisteredPointers, they
are able to reference target object types before those target types are (fully) defined. This is required to support
"mutually referencing" objects or "cyclic references".

The implementation difference is that with TRegisteredPointers, the "pointer tracking registry" is located in the target
object, whereas TCRegisteredPointers use (thread local) global registries that track all the pointers targeting
objects of a given type. */

#pragma once
#ifndef MSECREGISTERED_H_
#define MSECREGISTERED_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_map>
#include <mutex>
#include <cassert>
//include <typeinfo>      // std::bad_cast
#include <stdexcept>

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_REGISTEREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

namespace mse {

	template<typename _Ty> class TWCRegisteredObj;
	template<typename _Ty> class TWCRegisteredPointer;
	template<typename _Ty> class TWCRegisteredConstPointer;
	template<typename _Ty> class TWCRegisteredNotNullPointer;
	template<typename _Ty> class TWCRegisteredNotNullConstPointer;
	template<typename _Ty> class TWCRegisteredFixedPointer;
	template<typename _Ty> class TWCRegisteredFixedConstPointer;

#ifdef MSE_REGISTEREDPOINTER_DISABLED
	template<typename _Ty> using TCRegisteredPointer = _Ty*;
	template<typename _Ty> using TCRegisteredConstPointer = const _Ty*;
	template<typename _Ty> using TCRegisteredNotNullPointer = _Ty*;
	template<typename _Ty> using TCRegisteredNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TCRegisteredFixedPointer = _Ty* /*const*/; /* Can't be const qualified because standard
																				  library containers don't support const elements. */
	template<typename _Ty> using TCRegisteredFixedConstPointer = const _Ty* /*const*/;
	template<typename _TROFLy> using TCRegisteredObj = _TROFLy;
	template <class _Ty, class... Args>
	TCRegisteredPointer<_Ty> cregistered_new(Args&&... args) {
		return new TCRegisteredObj<_Ty>(std::forward<Args>(args)...);
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TCRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TCRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	namespace us {
		template <class _Ty>
		void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
			mse::cregistered_delete(regPtrRef);
		}
		template <class _Ty>
		void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
			mse::cregistered_delete(regPtrRef);
		}
	}

	template<typename _Ty> auto cregistered_fptr_to(_Ty&& _X) { return std::addressof(_X); }
	template<typename _Ty> auto cregistered_fptr_to(const _Ty& _X) { return std::addressof(_X); }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	class cregistered_cannot_verify_cast_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

	template<typename _Ty> using TCRegisteredPointer = TWCRegisteredPointer<_Ty>;
	template<typename _Ty> using TCRegisteredConstPointer = TWCRegisteredConstPointer<_Ty>;
	template<typename _Ty> using TCRegisteredNotNullPointer = TWCRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using TCRegisteredNotNullConstPointer = TWCRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using TCRegisteredFixedPointer = TWCRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using TCRegisteredFixedConstPointer = TWCRegisteredFixedConstPointer<_Ty>;
	template<typename _TROFLy> using TCRegisteredObj = TWCRegisteredObj<_TROFLy>;

	template<typename _Ty>
	auto cregistered_fptr_to(_Ty&& _X) {
		return _X.mse_cregistered_fptr();
	}
	template<typename _Ty>
	auto cregistered_fptr_to(const _Ty& _X) {
		return _X.mse_cregistered_fptr();
	}

#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

	namespace us {
		namespace impl {
			class CCRegisteredNode;

			class CCRNMutablePointer {
			public:
				CCRNMutablePointer(const CCRegisteredNode* ptr) : m_ptr(ptr) {}
				/* explicit */CCRNMutablePointer(const CCRNMutablePointer& src_cref) : m_ptr(src_cref.m_ptr) {}

				auto& operator*() const {
					return (*m_ptr);
				}
				auto operator->() const {
					return m_ptr;
				}
				/* The point of this class is the const at the end. */
				auto& operator=(const CCRNMutablePointer& src_cref) const {
					m_ptr = src_cref.m_ptr;
					return (*this);
				}
				operator bool() const {
					return (m_ptr != nullptr);
				}

			private:
				mutable const CCRegisteredNode* m_ptr = nullptr;
			};

			/* node of a (doubly-linked) list of pointers */
			class CCRegisteredNode {
			public:
				virtual void rn_set_pointer_to_null() const = 0;
				void set_next_ptr(mse::us::impl::CCRNMutablePointer next_ptr) const {
					m_next_ptr = next_ptr;
				}
				mse::us::impl::CCRNMutablePointer get_next_ptr() const {
					return m_next_ptr;
				}
				void set_prev_next_ptr_ptr(const mse::us::impl::CCRNMutablePointer* prev_next_ptr_ptr) const {
					m_prev_next_ptr_ptr = prev_next_ptr_ptr;
				}
				const mse::us::impl::CCRNMutablePointer* get_prev_next_ptr_ptr() const {
					return m_prev_next_ptr_ptr;
				}
				const mse::us::impl::CCRNMutablePointer* get_address_of_my_next_ptr() const {
					return &m_next_ptr;
				}

			private:
				mutable mse::us::impl::CCRNMutablePointer m_next_ptr = nullptr;
				mutable const mse::us::impl::CCRNMutablePointer* m_prev_next_ptr_ptr = nullptr;
			};
		}
	}

	/* TWCRegisteredPointer is similar to TWRegisteredPointer but uses a different implementation that allows it to be be declared
	before its target type is fully defined. (This is necessary to support mutual and cyclic references.) It's also generally more
	memory efficient. But maybe a bit slower in some cases. */
	template<typename _Ty>
	class TWCRegisteredPointer : public mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>, public mse::us::impl::CCRegisteredNode {
	public:
		TWCRegisteredPointer() : mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>() {}
		TWCRegisteredPointer(const TWCRegisteredPointer& src_cref) : mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		TWCRegisteredPointer(std::nullptr_t) : mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>(nullptr) {}
		virtual ~TWCRegisteredPointer() {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
		}
		TWCRegisteredPointer<_Ty>& operator=(const TWCRegisteredPointer<_Ty>& _Right_cref) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
			mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>::operator=(_Right_cref);
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredPointer<_Ty>& operator=(const TWCRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWCRegisteredPointer(_Right_cref));
		}
		operator bool() const { return !(!((*this).m_ptr)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return (*this).m_ptr;
		}

		/* In C++, if an object is deleted via a pointer to its base class and the base class' destructor is not virtual,
		then the (derived) object's destructor won't be called possibly resulting in resource leaks. With registered
		objects, the destructor not being called also circumvents their memory safety mechanism. */
		void cregistered_delete() const {
			auto a = asANativePointerToTWCRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

		/* todo: make this private */
		void rn_set_pointer_to_null() const override { (*this).spb_set_to_null(); }

	private:
		TWCRegisteredPointer(TWCRegisteredObj<_Ty>* ptr) : mse::us::TSaferPtr<TWCRegisteredObj<_Ty>>(ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by cregistered_delete<>(). */
		TWCRegisteredObj<_Ty>* asANativePointerToTWCRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TWCRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TWCRegisteredPointer;
		template <class Y> friend class TWCRegisteredConstPointer;
		friend class TWCRegisteredNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TWCRegisteredConstPointer : public mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>, public mse::us::impl::CCRegisteredNode {
	public:
		TWCRegisteredConstPointer() : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>() {}
		TWCRegisteredConstPointer(const TWCRegisteredConstPointer& src_cref) : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredConstPointer(const TWCRegisteredConstPointer<_Ty2>& src_cref) : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		TWCRegisteredConstPointer(const TWCRegisteredPointer<_Ty>& src_cref) : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredConstPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		TWCRegisteredConstPointer(std::nullptr_t) : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>(nullptr) {}
		virtual ~TWCRegisteredConstPointer() {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
		}
		TWCRegisteredConstPointer<_Ty>& operator=(const TWCRegisteredConstPointer<_Ty>& _Right_cref) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
			mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>::operator=(_Right_cref);
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredConstPointer<_Ty>& operator=(const TWCRegisteredConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWCRegisteredConstPointer(_Right_cref));
		}

		operator bool() const { return !(!((*this).m_ptr)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator const _Ty*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return (*this).m_ptr;
		}

		/* In C++, if an object is deleted via a pointer to its base class and the base class' destructor is not virtual,
		then the (derived) object's destructor won't be called possibly resulting in resource leaks. With registered
		objects, the destructor not being called also circumvents their memory safety mechanism. */
		void cregistered_delete() const {
			auto a = asANativePointerToTWCRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

		/* todo: make this private */
		void rn_set_pointer_to_null() const override { (*this).spb_set_to_null(); }

	private:
		TWCRegisteredConstPointer(const TWCRegisteredObj<_Ty>* ptr) : mse::us::TSaferPtr<const TWCRegisteredObj<_Ty>>(ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by cregistered_delete<>(). */
		const TWCRegisteredObj<_Ty>* asANativePointerToTWCRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TWCRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TWCRegisteredConstPointer;
		friend class TWCRegisteredNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TWCRegisteredNotNullPointer : public TWCRegisteredPointer<_Ty> {
	public:
		TWCRegisteredNotNullPointer(const TWCRegisteredNotNullPointer& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredNotNullPointer() {}
		/*
		TWCRegisteredNotNullPointer<_Ty>& operator=(const TWCRegisteredNotNullPointer<_Ty>& _Right_cref) {
			TWCRegisteredPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		*/
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWCRegisteredPointer<_Ty>::operator _Ty*(); }
		explicit operator TWCRegisteredObj<_Ty>*() const { return TWCRegisteredPointer<_Ty>::operator TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredNotNullPointer(TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWCRegisteredNotNullPointer(const  TWCRegisteredPointer<_Ty>& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		/* If you want a pointer to a TWCRegisteredNotNullPointer<_Ty>, declare the TWCRegisteredNotNullPointer<_Ty> as a
		TWCRegisteredObj<TWCRegisteredNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWCRegisteredObj<TWCRegisteredNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWCRegisteredFixedPointer<_Ty>;
		template<typename _Ty2>
		friend TWCRegisteredNotNullPointer<_Ty2> not_null_from_nullable(const TWCRegisteredPointer<_Ty2>& src);
	};

	template<typename _Ty>
	class TWCRegisteredNotNullConstPointer : public TWCRegisteredConstPointer<_Ty> {
	public:
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullConstPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWCRegisteredConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWCRegisteredObj<_Ty>*() const { return TWCRegisteredConstPointer<_Ty>::operator const TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredNotNullConstPointer(const TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredConstPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWCRegisteredNotNullConstPointer(const TWCRegisteredPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TWCRegisteredNotNullConstPointer(const TWCRegisteredConstPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredConstPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWCRegisteredFixedConstPointer<_Ty>;
		template<typename _Ty2>
		friend TWCRegisteredNotNullConstPointer<_Ty2> not_null_from_nullable(const TWCRegisteredConstPointer<_Ty2>& src);
	};

	template<typename _Ty>
	TWCRegisteredNotNullPointer<_Ty> not_null_from_nullable(const TWCRegisteredPointer<_Ty>& src) {
		return src;
	}
	template<typename _Ty>
	TWCRegisteredNotNullConstPointer<_Ty> not_null_from_nullable(const TWCRegisteredConstPointer<_Ty>& src) {
		return src;
	}

	/* TWCRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TWCRegisteredFixedPointer : public TWCRegisteredNotNullPointer<_Ty> {
	public:
		TWCRegisteredFixedPointer(const TWCRegisteredFixedPointer& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedPointer(const TWCRegisteredFixedPointer<_Ty2>& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}

		TWCRegisteredFixedPointer(const TWCRegisteredNotNullPointer<_Ty>& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWCRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TWCRegisteredObj<_Ty>*() const { return TWCRegisteredNotNullPointer<_Ty>::operator TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredFixedPointer(TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredNotNullPointer<_Ty>(ptr) {}
		TWCRegisteredFixedPointer<_Ty>& operator=(const TWCRegisteredFixedPointer<_Ty>& _Right_cref) = delete;

		/* If you want a pointer to a TWCRegisteredFixedPointer<_Ty>, declare the TWCRegisteredFixedPointer<_Ty> as a
		TWCRegisteredObj<TWCRegisteredFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWCRegisteredObj<TWCRegisteredFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWCRegisteredObj<_Ty>;
	};

	template<typename _Ty>
	class TWCRegisteredFixedConstPointer : public TWCRegisteredNotNullConstPointer<_Ty> {
	public:
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedConstPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedConstPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullConstPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWCRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWCRegisteredObj<_Ty>*() const { return TWCRegisteredNotNullConstPointer<_Ty>::operator const TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredFixedConstPointer(const TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredNotNullConstPointer<_Ty>(ptr) {}
		TWCRegisteredFixedConstPointer<_Ty>& operator=(const TWCRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWCRegisteredObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_CREGISTERED_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	/* TWCRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TWCRegisteredPointers will avoid referencing destroyed objects. Note that TWCRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TWCRegisteredObj : public _TROFLy, public std::conditional<(!std::is_convertible<_TROFLy*, mse::us::impl::NotAsyncShareableTagBase*>::value) && (!std::is_base_of<mse::us::impl::NotAsyncShareableTagBase, _TROFLy>::value)
		, mse::us::impl::NotAsyncShareableTagBase, impl::TPlaceHolder_msepointerbasics<TWCRegisteredObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_CREGISTERED_OBJ_USING(TWCRegisteredObj, _TROFLy);
		TWCRegisteredObj(const TWCRegisteredObj& _X) : _TROFLy(_X) {}
		TWCRegisteredObj(TWCRegisteredObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TWCRegisteredObj() {
			unregister_and_set_outstanding_pointers_to_null();
		}

		template<class _Ty2>
		TWCRegisteredObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TWCRegisteredObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TWCRegisteredFixedPointer<_TROFLy> operator&() {
			return TWCRegisteredFixedPointer<_TROFLy>(this);
		}
		TWCRegisteredFixedConstPointer<_TROFLy> operator&() const {
			return TWCRegisteredFixedConstPointer<_TROFLy>(this);
		}
		TWCRegisteredFixedPointer<_TROFLy> mse_cregistered_fptr() { return TWCRegisteredFixedPointer<_TROFLy>(this); }
		TWCRegisteredFixedConstPointer<_TROFLy> mse_cregistered_fptr() const { return TWCRegisteredFixedConstPointer<_TROFLy>(this); }

		/* todo: make these private */
		void register_pointer(const mse::us::impl::CCRegisteredNode& node_cref) const {
			if (m_head_ptr) {
				m_head_ptr->set_prev_next_ptr_ptr(node_cref.get_address_of_my_next_ptr());
			}
			node_cref.set_next_ptr(m_head_ptr);
			node_cref.set_prev_next_ptr_ptr(&m_head_ptr);
			m_head_ptr = &node_cref;
		}
		static void unregister_pointer(const mse::us::impl::CCRegisteredNode& node_cref) {
			assert(node_cref.get_prev_next_ptr_ptr());
			(*(node_cref.get_prev_next_ptr_ptr())) = node_cref.get_next_ptr();
			if (node_cref.get_next_ptr()) {
				node_cref.get_next_ptr()->set_prev_next_ptr_ptr(node_cref.get_prev_next_ptr_ptr());
			}
			node_cref.set_prev_next_ptr_ptr(nullptr);
			node_cref.set_next_ptr(nullptr);
		}

	private:
		void unregister_and_set_outstanding_pointers_to_null() const {
			auto current_node_ptr = m_head_ptr;
			while (current_node_ptr) {
				current_node_ptr->rn_set_pointer_to_null();
				current_node_ptr->set_prev_next_ptr_ptr(nullptr);
				auto next_ptr = current_node_ptr->get_next_ptr();
				current_node_ptr->set_next_ptr(nullptr);
				current_node_ptr = next_ptr;
			}
		}

		/* first node in a (doubly-linked) list of pointers targeting this object */
		mutable mse::us::impl::CCRNMutablePointer m_head_ptr = nullptr;
	};


#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TCRegisteredPointer<_Ty> cregistered_new(Args&&... args) {
		auto a = new TCRegisteredObj<_Ty>(std::forward<Args>(args)...);
		mse::us::impl::tlSAllocRegistry_ref<TCRegisteredObj<_Ty> >().registerPointer(a);
		return &(*a);
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TCRegisteredObj<_Ty>*>(regPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TCRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::cregistered_delete() \n- tip: If deleting via base class pointer, use mse::us::cregistered_delete() instead. ")); }
		regPtrRef.cregistered_delete();
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TCRegisteredObj<_Ty>*>(regPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TCRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::cregistered_delete() \n- tip: If deleting via base class pointer, use mse::us::cregistered_delete() instead. ")); }
		regPtrRef.cregistered_delete();
	}
	namespace us {
		template <class _Ty>
		void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
			regPtrRef.cregistered_delete();
		}
		template <class _Ty>
		void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
			regPtrRef.cregistered_delete();
		}
	}
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

}

namespace std {
	template<class _Ty>
	struct hash<mse::TWCRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TWCRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	/* template specializations */

	template<typename _Ty>
	class TWCRegisteredObj<_Ty*> : public TWCRegisteredObj<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredObj<_Ty* const> : public TWCRegisteredObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredObj<const _Ty *> : public TWCRegisteredObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredObj<const _Ty * const> : public TWCRegisteredObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};

	template<typename _Ty>
	class TWCRegisteredPointer<_Ty*> : public TWCRegisteredPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredPointer<_Ty* const> : public TWCRegisteredPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredPointer<const _Ty *> : public TWCRegisteredPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredPointer<const _Ty * const> : public TWCRegisteredPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};

	template<typename _Ty>
	class TWCRegisteredConstPointer<_Ty*> : public TWCRegisteredConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredConstPointer<_Ty* const> : public TWCRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredConstPointer<const _Ty *> : public TWCRegisteredConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredConstPointer<const _Ty * const> : public TWCRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TWCRegisteredObj<int> : public TWCRegisteredObj<mse::TInt<int>> {
	public:
		typedef TWCRegisteredObj<mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredObj<const int> : public TWCRegisteredObj<const mse::TInt<int>> {
	public:
		typedef TWCRegisteredObj<const mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredPointer<int> : public TWCRegisteredPointer<mse::TInt<int>> {
	public:
		typedef TWCRegisteredPointer<mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredPointer<const int> : public TWCRegisteredPointer<const mse::TInt<int>> {
	public:
		typedef TWCRegisteredPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<int> : public TWCRegisteredConstPointer<mse::TInt<int>> {
	public:
		typedef TWCRegisteredConstPointer<mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<const int> : public TWCRegisteredConstPointer<const mse::TInt<int>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};

	template<>
	class TWCRegisteredObj<size_t> : public TWCRegisteredObj<mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredObj<mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredObj<const size_t> : public TWCRegisteredObj<const mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredPointer<size_t> : public TWCRegisteredPointer<mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredPointer<const size_t> : public TWCRegisteredPointer<const mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<size_t> : public TWCRegisteredConstPointer<mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredConstPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<const size_t> : public TWCRegisteredConstPointer<const mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

#if defined(MSE_REGISTEREDPOINTER_DISABLED)
	/* Omit definition of make_pointer_to_member() as it would clash with the one already defined in mseregistered.h. */
#define MSE_CREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER
#endif // defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_CREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, TCRegisteredPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TCRegisteredPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, TCRegisteredPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, TCRegisteredConstPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TCRegisteredConstPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, TCRegisteredConstPointer<_Ty>>::make(target, lease_pointer);
	}
#endif // !defined(MSE_CREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)

	/* shorter aliases */
	template<typename _Ty> using rrp = TCRegisteredPointer<_Ty>;
	template<typename _Ty> using rrcp = TCRegisteredConstPointer<_Ty>;
	template<typename _Ty> using rrnnp = TCRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using rrnncp = TCRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using rrfp = TCRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using rrfcp = TCRegisteredFixedConstPointer<_Ty>;
	template<typename _TROFLy> using rro = TCRegisteredObj<_TROFLy>;
	template <class _Ty, class... Args>
	TCRegisteredPointer<_Ty> rrnew(Args&&... args) { return cregistered_new<_Ty>(std::forward<Args>(args)...); }
	template <class _Ty>
	void rrdelete(const TCRegisteredPointer<_Ty>& regPtrRef) { cregistered_delete<_Ty>(regPtrRef); }


#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

	namespace self_test {
		class CCRegPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class C;

				class D {
				public:
					virtual ~D() {}
					mse::TCRegisteredPointer<C> m_c_ptr = nullptr;
				};

				class C {
				public:
					C() {}
					mse::TCRegisteredPointer<D> m_d_ptr = nullptr;
				};

				mse::TCRegisteredObj<C> regobjfl_c;
				mse::TCRegisteredPointer<D> d_ptr = mse::cregistered_new<D>();

				regobjfl_c.m_d_ptr = d_ptr;
				d_ptr->m_c_ptr = &regobjfl_c;

				mse::TCRegisteredConstPointer<C> rrcp = d_ptr->m_c_ptr;
				mse::TCRegisteredConstPointer<C> rrcp2 = rrcp;
				const mse::TCRegisteredObj<C> regobjfl_e;
				rrcp = &regobjfl_e;
				mse::TCRegisteredFixedConstPointer<C> rrfcp = &regobjfl_e;
				rrcp = mse::cregistered_new<C>();
				mse::cregistered_delete<C>(rrcp);

				mse::cregistered_delete<D>(d_ptr);

				{
					/* Polymorphic conversions. */
					class FD : public mse::TCRegisteredObj<D> {};
					mse::TCRegisteredObj<FD> cregistered_fd;
					mse::TCRegisteredPointer<FD> FD_cregistered_ptr1 = &cregistered_fd;
					mse::TCRegisteredPointer<D> D_cregistered_ptr4 = FD_cregistered_ptr1;
					D_cregistered_ptr4 = &cregistered_fd;
					mse::TCRegisteredFixedPointer<D> D_cregistered_fptr1 = &cregistered_fd;
					mse::TCRegisteredFixedConstPointer<D> D_cregistered_fcptr1 = &cregistered_fd;
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

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

}

#undef MSE_THROW

#endif // MSECREGISTERED_H_
