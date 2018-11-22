
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREGISTERED_H_
#define MSEREGISTERED_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_REGISTEREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

namespace mse {

	template<typename _Ty> class TWRegisteredObj;
	template<typename _Ty> class TWRegisteredPointer;
	template<typename _Ty> class TWRegisteredConstPointer;
	template<typename _Ty> class TWRegisteredNotNullPointer;
	template<typename _Ty> class TWRegisteredNotNullConstPointer;
	template<typename _Ty> class TWRegisteredFixedPointer;
	template<typename _Ty> class TWRegisteredFixedConstPointer;

#ifdef MSE_REGISTEREDPOINTER_DISABLED
	template<typename _Ty> using TRegisteredPointer = _Ty*;
	template<typename _Ty> using TRegisteredConstPointer = const _Ty*;
	template<typename _Ty> using TRegisteredNotNullPointer = _Ty*;
	template<typename _Ty> using TRegisteredNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TRegisteredFixedPointer = _Ty* /*const*/; /* Can't be const qualified because standard
																											library containers don't support const elements. */
	template<typename _Ty> using TRegisteredFixedConstPointer = const _Ty* /*const*/;
	template<typename _TROy> using TRegisteredObj = _TROy;
	template <class _TRRWy> using TRegisteredRefWrapper = std::reference_wrapper<_TRRWy>;
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		return new TRegisteredObj<_Ty>(std::forward<Args>(args)...);
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	template <class _Ty>
	void registered_delete(const TRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	namespace us {
		template <class _Ty>
		void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
			mse::registered_delete(regPtrRef);
		}
		template <class _Ty>
		void registered_delete(const TRegisteredConstPointer<_Ty>& regPtrRef) {
			mse::registered_delete(regPtrRef);
		}
	}

	template<typename _Ty> auto registered_fptr_to(_Ty&& _X) { return std::addressof(_X); }
	template<typename _Ty> auto registered_fptr_to(const _Ty& _X) { return std::addressof(_X); }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	template<typename _Ty> using TRegisteredPointer = TWRegisteredPointer<_Ty>;
	template<typename _Ty> using TRegisteredConstPointer = TWRegisteredConstPointer<_Ty>;
	template<typename _Ty> using TRegisteredNotNullPointer = TWRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using TRegisteredNotNullConstPointer = TWRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using TRegisteredFixedPointer = TWRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using TRegisteredFixedConstPointer = TWRegisteredFixedConstPointer<_Ty>;
	template<typename _TROy> using TRegisteredObj = TWRegisteredObj<_TROy>;

	template<typename _Ty>
	auto registered_fptr_to(_Ty&& _X) {
		return _X.mse_registered_fptr();
	}
	template<typename _Ty>
	auto registered_fptr_to(const _Ty& _X) {
		return _X.mse_registered_fptr();
	}

#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

	namespace us {
		namespace impl {
			/* node of a (singly-linked) list of pointers */
			class CRegisteredNode {
			public:
				virtual void rn_set_pointer_to_null() const = 0;
				void set_next_ptr(const CRegisteredNode* next_ptr) const {
					m_next_ptr = next_ptr;
				}
				const CRegisteredNode* get_next_ptr() const {
					return m_next_ptr;
				}

			private:
				mutable const CRegisteredNode * m_next_ptr = nullptr;
			};
		}
	}

	/* "Registered" pointers are intended to behave just like native C++ pointers, except that their value is (automatically)
	set to nullptr when the target object is destroyed. And by default they will throw an exception upon any attempt to
	dereference a nullptr. Because they don't take ownership like some other smart pointers, they can point to objects
	allocated on the stack as well as the heap. */
	template<typename _Ty>
	class TWRegisteredPointer : public mse::us::TSaferPtr<TWRegisteredObj<_Ty>>, public mse::us::impl::CRegisteredNode {
	public:
		TWRegisteredPointer() : mse::us::TSaferPtr<TWRegisteredObj<_Ty>>() {}
		TWRegisteredPointer(const TWRegisteredPointer& src_cref) : mse::us::TSaferPtr<TWRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredPointer(const TWRegisteredPointer<_Ty2>& src_cref) : mse::us::TSaferPtr<TWRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		TWRegisteredPointer(std::nullptr_t) : mse::us::TSaferPtr<TWRegisteredObj<_Ty>>(nullptr) {}
		virtual ~TWRegisteredPointer() {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
		}
		TWRegisteredPointer<_Ty>& operator=(const TWRegisteredPointer<_Ty>& _Right_cref) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
			mse::us::TSaferPtr<TWRegisteredObj<_Ty>>::operator=(_Right_cref);
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredPointer<_Ty>& operator=(const TWRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWRegisteredPointer(_Right_cref));
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
		void registered_delete() const {
			auto a = asANativePointerToTWRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

		/* todo: make this private */
		void rn_set_pointer_to_null() const override { (*this).spb_set_to_null(); }

	private:
		TWRegisteredPointer(TWRegisteredObj<_Ty>* ptr) : mse::us::TSaferPtr<TWRegisteredObj<_Ty>>(ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by registered_delete<>(). */
		TWRegisteredObj<_Ty>* asANativePointerToTWRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TWRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TWRegisteredPointer;
		template <class Y> friend class TWRegisteredConstPointer;
		friend class TWRegisteredNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TWRegisteredConstPointer : public mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>, public mse::us::impl::CRegisteredNode {
	public:
		TWRegisteredConstPointer() : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>() {}
		TWRegisteredConstPointer(const TWRegisteredConstPointer& src_cref) : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredConstPointer(const TWRegisteredConstPointer<_Ty2>& src_cref) : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		TWRegisteredConstPointer(const TWRegisteredPointer<_Ty>& src_cref) : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredConstPointer(const TWRegisteredPointer<_Ty2>& src_cref) : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>(src_cref.m_ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}
		TWRegisteredConstPointer(std::nullptr_t) : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>(nullptr) {}
		virtual ~TWRegisteredConstPointer() {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
		}
		TWRegisteredConstPointer<_Ty>& operator=(const TWRegisteredConstPointer<_Ty>& _Right_cref) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).unregister_pointer(*this);
			}
			mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>::operator=(_Right_cref);
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredConstPointer<_Ty>& operator=(const TWRegisteredConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWRegisteredConstPointer(_Right_cref));
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
		void registered_delete() const {
			auto a = asANativePointerToTWRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

		/* todo: make this private */
		void rn_set_pointer_to_null() const override { (*this).spb_set_to_null(); }

	private:
		TWRegisteredConstPointer(const TWRegisteredObj<_Ty>* ptr) : mse::us::TSaferPtr<const TWRegisteredObj<_Ty>>(ptr) {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).register_pointer(*this);
			}
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by registered_delete<>(). */
		const TWRegisteredObj<_Ty>* asANativePointerToTWRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TWRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TWRegisteredConstPointer;
		friend class TWRegisteredNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TWRegisteredNotNullPointer : public TWRegisteredPointer<_Ty> {
	public:
		TWRegisteredNotNullPointer(const TWRegisteredNotNullPointer& src_cref) : TWRegisteredPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredNotNullPointer(const TWRegisteredNotNullPointer<_Ty2>& src_cref) : TWRegisteredPointer<_Ty>(src_cref) {}

		virtual ~TWRegisteredNotNullPointer() {}
		/*
		TWRegisteredNotNullPointer<_Ty>& operator=(const TWRegisteredNotNullPointer<_Ty>& _Right_cref) {
		TWRegisteredPointer<_Ty>::operator=(_Right_cref);
		return (*this);
		}
		*/
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWRegisteredPointer<_Ty>::operator _Ty*(); }
		explicit operator TWRegisteredObj<_Ty>*() const { return TWRegisteredPointer<_Ty>::operator TWRegisteredObj<_Ty>*(); }

	private:
		TWRegisteredNotNullPointer(TWRegisteredObj<_Ty>* ptr) : TWRegisteredPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWRegisteredNotNullPointer(const  TWRegisteredPointer<_Ty>& src_cref) : TWRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredNotNullPointer(const TWRegisteredPointer<_Ty2>& src_cref) : TWRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		/* If you want a pointer to a TWRegisteredNotNullPointer<_Ty>, declare the TWRegisteredNotNullPointer<_Ty> as a
		TWRegisteredObj<TWRegisteredNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWRegisteredObj<TWRegisteredNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWRegisteredFixedPointer<_Ty>;
		template<typename _Ty2>
		friend TWRegisteredNotNullPointer<_Ty2> not_null_from_nullable(const TWRegisteredPointer<_Ty2>& src);
	};

	template<typename _Ty>
	class TWRegisteredNotNullConstPointer : public TWRegisteredConstPointer<_Ty> {
	public:
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullPointer<_Ty>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullPointer<_Ty2>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {}
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullConstPointer<_Ty>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {}

		virtual ~TWRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWRegisteredConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWRegisteredObj<_Ty>*() const { return TWRegisteredConstPointer<_Ty>::operator const TWRegisteredObj<_Ty>*(); }

	private:
		TWRegisteredNotNullConstPointer(const TWRegisteredObj<_Ty>* ptr) : TWRegisteredConstPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWRegisteredNotNullConstPointer(const TWRegisteredPointer<_Ty>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredPointer<_Ty2>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TWRegisteredNotNullConstPointer(const TWRegisteredConstPointer<_Ty>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredConstPointer<_Ty2>& src_cref) : TWRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWRegisteredFixedConstPointer<_Ty>;
		template<typename _Ty2>
		friend TWRegisteredNotNullConstPointer<_Ty2> not_null_from_nullable(const TWRegisteredConstPointer<_Ty2>& src);
	};

	template<typename _Ty>
	TWRegisteredNotNullPointer<_Ty> not_null_from_nullable(const TWRegisteredPointer<_Ty>& src) {
		return src;
	}
	template<typename _Ty>
	TWRegisteredNotNullConstPointer<_Ty> not_null_from_nullable(const TWRegisteredConstPointer<_Ty>& src) {
		return src;
	}

	/* TWRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TWRegisteredFixedPointer : public TWRegisteredNotNullPointer<_Ty> {
	public:
		TWRegisteredFixedPointer(const TWRegisteredFixedPointer& src_cref) : TWRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredFixedPointer(const TWRegisteredFixedPointer<_Ty2>& src_cref) : TWRegisteredNotNullPointer<_Ty>(src_cref) {}

		TWRegisteredFixedPointer(const TWRegisteredNotNullPointer<_Ty>& src_cref) : TWRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredFixedPointer(const TWRegisteredNotNullPointer<_Ty2>& src_cref) : TWRegisteredNotNullPointer<_Ty>(src_cref) {}

		virtual ~TWRegisteredFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TWRegisteredObj<_Ty>*() const { return TWRegisteredNotNullPointer<_Ty>::operator TWRegisteredObj<_Ty>*(); }

	private:
		TWRegisteredFixedPointer(TWRegisteredObj<_Ty>* ptr) : TWRegisteredNotNullPointer<_Ty>(ptr) {}
		TWRegisteredFixedPointer<_Ty>& operator=(const TWRegisteredFixedPointer<_Ty>& _Right_cref) = delete;

		/* If you want a pointer to a TWRegisteredFixedPointer<_Ty>, declare the TWRegisteredFixedPointer<_Ty> as a
		TWRegisteredObj<TWRegisteredFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWRegisteredObj<TWRegisteredFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWRegisteredObj<_Ty>;
	};

	template<typename _Ty>
	class TWRegisteredFixedConstPointer : public TWRegisteredNotNullConstPointer<_Ty> {
	public:
		TWRegisteredFixedConstPointer(const TWRegisteredFixedPointer<_Ty>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredFixedPointer<_Ty2>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWRegisteredFixedConstPointer(const TWRegisteredFixedConstPointer<_Ty>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredFixedConstPointer<_Ty2>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		TWRegisteredFixedConstPointer(const TWRegisteredNotNullPointer<_Ty>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredNotNullPointer<_Ty2>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWRegisteredFixedConstPointer(const TWRegisteredNotNullConstPointer<_Ty>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		virtual ~TWRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWRegisteredObj<_Ty>*() const { return TWRegisteredNotNullConstPointer<_Ty>::operator const TWRegisteredObj<_Ty>*(); }

	private:
		TWRegisteredFixedConstPointer(const TWRegisteredObj<_Ty>* ptr) : TWRegisteredNotNullConstPointer<_Ty>(ptr) {}
		TWRegisteredFixedConstPointer<_Ty>& operator=(const TWRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWRegisteredObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_CREGISTERED_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	/* TWRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TWRegisteredPointers will avoid referencing destroyed objects. Note that TWRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TWRegisteredObj : public _TROFLy, public std::conditional<(!std::is_convertible<_TROFLy*, mse::us::impl::NotAsyncShareableTagBase*>::value) && (!std::is_base_of<mse::us::impl::NotAsyncShareableTagBase, _TROFLy>::value)
		, mse::us::impl::NotAsyncShareableTagBase, impl::TPlaceHolder_msepointerbasics<TWRegisteredObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_CREGISTERED_OBJ_USING(TWRegisteredObj, _TROFLy);
		TWRegisteredObj(const TWRegisteredObj& _X) : _TROFLy(_X) {}
		TWRegisteredObj(TWRegisteredObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TWRegisteredObj() {
			unregister_and_set_outstanding_pointers_to_null();
		}

		template<class _Ty2>
		TWRegisteredObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TWRegisteredObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TWRegisteredFixedPointer<_TROFLy> operator&() {
			return TWRegisteredFixedPointer<_TROFLy>(this);
		}
		TWRegisteredFixedConstPointer<_TROFLy> operator&() const {
			return TWRegisteredFixedConstPointer<_TROFLy>(this);
		}
		TWRegisteredFixedPointer<_TROFLy> mse_registered_fptr() { return TWRegisteredFixedPointer<_TROFLy>(this); }
		TWRegisteredFixedConstPointer<_TROFLy> mse_registered_fptr() const { return TWRegisteredFixedConstPointer<_TROFLy>(this); }

		/* todo: make these private */
		void register_pointer(const mse::us::impl::CRegisteredNode& node_cref) const {
			node_cref.set_next_ptr(m_head_ptr);
			m_head_ptr = &node_cref;
		}
		void unregister_pointer(const mse::us::impl::CRegisteredNode& node_cref) const {
			const auto target_node_ptr = &node_cref;
			if (target_node_ptr == m_head_ptr) {
				m_head_ptr = target_node_ptr->get_next_ptr();
				node_cref.set_next_ptr(nullptr);
				return;
			}
			if (!m_head_ptr) {
				assert(false);
				return;
			}
			auto current_node_ptr = m_head_ptr;
			while (target_node_ptr != current_node_ptr->get_next_ptr()) {
				current_node_ptr = current_node_ptr->get_next_ptr();
				if (!current_node_ptr) {
					assert(false);
					return;
				}
			}
			current_node_ptr->set_next_ptr(target_node_ptr->get_next_ptr());
			node_cref.set_next_ptr(nullptr);
		}

	private:
		void unregister_and_set_outstanding_pointers_to_null() const {
			auto current_node_ptr = m_head_ptr;
			while (current_node_ptr) {
				current_node_ptr->rn_set_pointer_to_null();
				auto next_ptr = current_node_ptr->get_next_ptr();
				current_node_ptr->set_next_ptr(nullptr);
				current_node_ptr = next_ptr;
			}
		}

		/* first node in a (singly-linked) list of pointers targeting this object */
		mutable const mse::us::impl::CRegisteredNode * m_head_ptr = nullptr;
	};


#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		auto a = new TRegisteredObj<_Ty>(std::forward<Args>(args)...);
		mse::us::impl::tlSAllocRegistry_ref<TRegisteredObj<_Ty> >().registerPointer(a);
		return &(*a);
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TRegisteredObj<_Ty>*>(regPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::registered_delete() \n- tip: If deleting via base class pointer, use mse::us::registered_delete() instead. ")); }
		regPtrRef.registered_delete();
}
	template <class _Ty>
	void registered_delete(const TRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TRegisteredObj<_Ty>*>(regPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::registered_delete() \n- tip: If deleting via base class pointer, use mse::us::registered_delete() instead. ")); }
		regPtrRef.registered_delete();
	}
	namespace us {
		template <class _Ty>
		void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
			regPtrRef.registered_delete();
		}
		template <class _Ty>
		void registered_delete(const TRegisteredConstPointer<_Ty>& regPtrRef) {
			regPtrRef.registered_delete();
		}
	}
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

}

namespace std {
	template<class _Ty>
	struct hash<mse::TWRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TWRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
	class TWRegisteredObj<_Ty*> : public TWRegisteredObj<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWRegisteredObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWRegisteredObj<_Ty* const> : public TWRegisteredObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWRegisteredObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWRegisteredObj<const _Ty *> : public TWRegisteredObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWRegisteredObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWRegisteredObj<const _Ty * const> : public TWRegisteredObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWRegisteredObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};

	template<typename _Ty>
	class TWRegisteredPointer<_Ty*> : public TWRegisteredPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWRegisteredPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWRegisteredPointer<_Ty* const> : public TWRegisteredPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWRegisteredPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWRegisteredPointer<const _Ty *> : public TWRegisteredPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWRegisteredPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWRegisteredPointer<const _Ty * const> : public TWRegisteredPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWRegisteredPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};

	template<typename _Ty>
	class TWRegisteredConstPointer<_Ty*> : public TWRegisteredConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWRegisteredConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWRegisteredConstPointer<_Ty* const> : public TWRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWRegisteredConstPointer<const _Ty *> : public TWRegisteredConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWRegisteredConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWRegisteredConstPointer<const _Ty * const> : public TWRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWRegisteredConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TWRegisteredObj<int> : public TWRegisteredObj<mse::TInt<int>> {
	public:
		typedef TWRegisteredObj<mse::TInt<int>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<>
	class TWRegisteredObj<const int> : public TWRegisteredObj<const mse::TInt<int>> {
	public:
		typedef TWRegisteredObj<const mse::TInt<int>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<>
	class TWRegisteredPointer<int> : public TWRegisteredPointer<mse::TInt<int>> {
	public:
		typedef TWRegisteredPointer<mse::TInt<int>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<>
	class TWRegisteredPointer<const int> : public TWRegisteredPointer<const mse::TInt<int>> {
	public:
		typedef TWRegisteredPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<>
	class TWRegisteredConstPointer<int> : public TWRegisteredConstPointer<mse::TInt<int>> {
	public:
		typedef TWRegisteredConstPointer<mse::TInt<int>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<>
	class TWRegisteredConstPointer<const int> : public TWRegisteredConstPointer<const mse::TInt<int>> {
	public:
		typedef TWRegisteredConstPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};

	template<>
	class TWRegisteredObj<size_t> : public TWRegisteredObj<mse::TInt<size_t>> {
	public:
		typedef TWRegisteredObj<mse::TInt<size_t>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<>
	class TWRegisteredObj<const size_t> : public TWRegisteredObj<const mse::TInt<size_t>> {
	public:
		typedef TWRegisteredObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<>
	class TWRegisteredPointer<size_t> : public TWRegisteredPointer<mse::TInt<size_t>> {
	public:
		typedef TWRegisteredPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<>
	class TWRegisteredPointer<const size_t> : public TWRegisteredPointer<const mse::TInt<size_t>> {
	public:
		typedef TWRegisteredPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<>
	class TWRegisteredConstPointer<size_t> : public TWRegisteredConstPointer<mse::TInt<size_t>> {
	public:
		typedef TWRegisteredConstPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<>
	class TWRegisteredConstPointer<const size_t> : public TWRegisteredConstPointer<const mse::TInt<size_t>> {
	public:
		typedef TWRegisteredConstPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

#ifdef _MSC_VER
#if (1900 <= _MSC_VER)
#define MSEREGISTEREDREFWRAPPER 1
#endif // (1900 <= _MSC_VER)
#else /*_MSC_VER*/
#define MSEREGISTEREDREFWRAPPER 1
#if (defined(__GNUC__) || defined(__GNUG__))
#define GPP_COMPATIBLE 1
#else /*(defined(__GNUC__) || defined(__GNUG__))*/
#ifdef __clang__
#define CLANG_COMPATIBLE 1
#endif // __clang__
#endif /*(defined(__GNUC__) || defined(__GNUG__))*/
#endif /*_MSC_VER*/

#ifdef MSEREGISTEREDREFWRAPPER
	template <class _TRRWy>
	class TRegisteredRefWrapper : public mse::us::impl::NotAsyncShareableTagBase {
	public:
		// types
		typedef TRegisteredObj<_TRRWy> type;

		// construct/copy/destroy
		TRegisteredRefWrapper(TRegisteredObj<_TRRWy>& ref) : _ptr(&ref) {}
		TRegisteredRefWrapper(TRegisteredObj<_TRRWy>&&) = delete;
		TRegisteredRefWrapper(const TRegisteredRefWrapper&) = default;

		// assignment
		TRegisteredRefWrapper& operator=(const TRegisteredRefWrapper& x) = default;

		// access
		operator TRegisteredObj<_TRRWy>& () const { return *_ptr; }
		TRegisteredObj<_TRRWy>& get() const { return *_ptr; }

		template< class... ArgTypes >
		typename std::result_of<TRegisteredObj<_TRRWy>&(ArgTypes&&...)>::type
			operator() (ArgTypes&&... args) const {
#if defined(GPP_COMPATIBLE) || defined(CLANG_COMPATIBLE)
			return __invoke(get(), std::forward<ArgTypes>(args)...);
#else // defined(GPP_COMPATIBLE) || definded(CLANG_COMPATIBLE)
			return std::invoke(get(), std::forward<ArgTypes>(args)...);
#endif // defined(GPP_COMPATIBLE) || definded(CLANG_COMPATIBLE)
		}

	private:
		TRegisteredPointer<_TRRWy> _ptr;
	};
#endif // MSEREGISTEREDREFWRAPPER

#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* shorter aliases */
	template<typename _Ty> using rp = TRegisteredPointer<_Ty>;
	template<typename _Ty> using rcp = TRegisteredConstPointer<_Ty>;
	template<typename _Ty> using rnnp = TRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using rnncp = TRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using rfp = TRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using rfcp = TRegisteredFixedConstPointer<_Ty>;
	template<typename _TROy> using ro = TRegisteredObj<_TROy>;
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> rnew(Args&&... args) { return registered_new<_Ty>(std::forward<Args>(args)...); }
	template <class _Ty>
	void rdelete(const TRegisteredPointer<_Ty>& regPtrRef) { registered_delete<_Ty>(regPtrRef); }

	/* These functions create a registered object assuming the type is the same as the lone parameter, so you don't need
	to explicitly specify it. */
	template <typename _TLoneParam>
	auto mkrolp(const _TLoneParam& lone_param) {
		return TRegisteredObj<_TLoneParam>(lone_param);
	}
	template <typename _TLoneParam>
	auto mkrolp(_TLoneParam&& lone_param) {
		return TRegisteredObj<_TLoneParam>(std::forward<decltype(lone_param)>(lone_param));
	}

	/* deprecated aliases */
	template<class _TTargetType, class _TLeasePointerType> using swkfp = TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>;
	template<class _TTargetType, class _TLeasePointerType> using swkfcp = TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType>;


#ifdef MSEREGISTEREDREFWRAPPER
	template <class _TRRWy> using rrw = TRegisteredRefWrapper<_TRRWy>;

	// TEMPLATE FUNCTIONS ref AND cref
	template<class _TRRy> inline
		TRegisteredRefWrapper<_TRRy>
		registered_ref(TRegisteredObj<_TRRy>& _Val)
	{	// create TRegisteredRefWrapper<_TRRy> object
		return (TRegisteredRefWrapper<_TRRy>(_Val));
	}

	template<class _TRRy>
	void registered_ref(const TRegisteredObj<_TRRy>&&) = delete;

	template<class _TRRy> inline
		TRegisteredRefWrapper<_TRRy>
		registered_ref(TRegisteredRefWrapper<_TRRy> _Val)
	{	// create TRegisteredRefWrapper<_TRRy> object
		return (registered_ref(_Val.get()));
	}

	template<class _TRCRy> inline
		TRegisteredRefWrapper<const _TRCRy>
		registered_cref(const TRegisteredObj<_TRCRy>& _Val)
	{	// create TRegisteredRefWrapper<const _TRCRy> object
		return (TRegisteredRefWrapper<const _TRCRy>(_Val));
	}

	template<class _TRCRy>
	void registered_cref(const TRegisteredObj<_TRCRy>&&) = delete;

	template<class _TRCRy> inline
		TRegisteredRefWrapper<const _TRCRy>
		registered_cref(TRegisteredRefWrapper<_TRCRy> _Val)
	{	// create TRegisteredRefWrapper<const _TRCRy> object
		return (registered_cref(_Val.get()));
	}
#endif // MSEREGISTEREDREFWRAPPER


#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

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
		class CRegPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class A {
				public:
					A() {}
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
					static int foo2(mse::TRegisteredPointer<A> A_registered_ptr) { return A_registered_ptr->b; }
				protected:
					~B() {}
				};

				A* A_native_ptr = nullptr;
				/* mse::TRegisteredPointer<> is basically a "safe" version of the native pointer. */
				mse::TRegisteredPointer<A> A_registered_ptr1;

				{
					A a;
					mse::TRegisteredObj<A> registered_a;
					/* mse::TRegisteredObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
				in almost all cases. */

					assert(a.b == registered_a.b);
					A_native_ptr = &a;
					A_registered_ptr1 = &registered_a;
					assert(A_native_ptr->b == A_registered_ptr1->b);

					mse::TRegisteredPointer<A> A_registered_ptr2 = &registered_a;
					A_registered_ptr2 = nullptr;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
					bool expected_exception = false;
					try {
						int i = A_registered_ptr2->b; /* this is gonna throw an exception */
					}
					catch (...) {
						//std::cerr << "expected exception" << std::endl;
						expected_exception = true;
						/* The exception is triggered by an attempt to dereference a null "registered pointer". */
					}
					assert(expected_exception);
#endif // !MSE_REGISTEREDPOINTER_DISABLED

					/* mse::TRegisteredPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<A*>(A_registered_ptr1));

					if (A_registered_ptr2) {
						assert(false);
					}
					else if (A_registered_ptr2 != A_registered_ptr1) {
						A_registered_ptr2 = A_registered_ptr1;
						assert(A_registered_ptr2 == A_registered_ptr1);
					}
					else {
						assert(false);
					}

					A a2 = a;
					mse::TRegisteredObj<A> registered_a2 = registered_a;

					a2 = A();
					registered_a2 = mse::TRegisteredObj<A>();

					A a3((A()));
					mse::TRegisteredObj<A> registered_a3((A()));
					{
						mse::TRegisteredObj<A> registered_a4((mse::TRegisteredObj<A>()));
					}

					mse::TRegisteredConstPointer<A> rcp = A_registered_ptr1;
					mse::TRegisteredConstPointer<A> rcp2 = rcp;
					const mse::TRegisteredObj<A> cregistered_a;
					rcp = &cregistered_a;
					mse::TRegisteredFixedConstPointer<A> rfcp = &cregistered_a;
					rcp = mse::registered_new<A>();
					mse::registered_delete<A>(rcp);
				}

				bool expected_exception = false;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
				try {
					/* A_registered_ptr1 "knows" that the (registered) object it was pointing to has now been deallocated. */
					int i = A_registered_ptr1->b; /* So this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
				}
				assert(expected_exception);
#endif // !MSE_REGISTEREDPOINTER_DISABLED

				{
					/* For heap allocations mse::registered_new is kind of analagous to std::make_shared, but again,
				mse::TRegisteredPointers don't take ownership so you are responsible for deallocation. */
					auto A_registered_ptr3 = mse::registered_new<A>();
					assert(3 == A_registered_ptr3->b);
					mse::registered_delete<A>(A_registered_ptr3);
					bool expected_exception = false;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
					try {
						/* A_registered_ptr3 "knows" that the (registered) object it was pointing to has now been deallocated. */
						int i = A_registered_ptr3->b; /* So this is gonna throw an exception */
					}
					catch (...) {
						//std::cerr << "expected exception" << std::endl;
						expected_exception = true;
					}
					assert(expected_exception);
#endif // !MSE_REGISTEREDPOINTER_DISABLED
				}

				{
					/* Remember that registered pointers can only point to registered objects. So, for example, if you want
				a registered pointer to an object's base class object, that base class object has to be a registered
				object. */
					class DA : public mse::TRegisteredObj<A> {};
					mse::TRegisteredObj<DA> registered_da;
					mse::TRegisteredPointer<DA> DA_registered_ptr1 = &registered_da;
					mse::TRegisteredPointer<A> A_registered_ptr4 = DA_registered_ptr1;
					A_registered_ptr4 = &registered_da;
					mse::TRegisteredFixedPointer<A> A_registered_fptr1 = &registered_da;
					mse::TRegisteredFixedConstPointer<A> A_registered_fcptr1 = &registered_da;
				}

				{
					/* Obtaining safe pointers to members of registered objects: */
					class E {
					public:
						virtual ~E() {}
						mse::TRegisteredObj<std::string> reg_s = "some text ";
						std::string s2 = "some other text ";
					};

					mse::TRegisteredObj<E> registered_e;
					mse::TRegisteredPointer<E> E_registered_ptr1 = &registered_e;

					/* To obtain a safe pointer to a member of a registered object you could just make the
				member itself a registered object. */
					mse::TRegisteredPointer<std::string> reg_s_registered_ptr1 = &(E_registered_ptr1->reg_s);

					/* Or you can use the "mse::make_pointer_to_member_v2()" function. */
					auto s2_safe_ptr1 = mse::make_pointer_to_member_v2(E_registered_ptr1, &E::s2);
					(*s2_safe_ptr1) = "some new text";
					auto s2_safe_const_ptr1 = mse::make_const_pointer_to_member_v2(E_registered_ptr1, &E::s2);

					/* Just testing the convertibility of mse::TSyncWeakFixedPointers. */
					auto E_registered_fixed_ptr1 = &registered_e;
					auto swfptr1 = mse::make_syncweak<std::string>(E_registered_fixed_ptr1->s2, E_registered_fixed_ptr1);
					mse::TSyncWeakFixedPointer<std::string, mse::TRegisteredPointer<E>> swfptr2 = swfptr1;
					mse::TSyncWeakFixedConstPointer<std::string, mse::TRegisteredFixedPointer<E>> swfcptr1 = swfptr1;
					mse::TSyncWeakFixedConstPointer<std::string, mse::TRegisteredPointer<E>> swfcptr2 = swfcptr1;
					if (swfcptr1 == swfptr1) {
						int q = 7;
					}
					if (swfptr1 == swfcptr1) {
						int q = 7;
					}
					if (swfptr1) {
						int q = 7;
					}
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

#undef MSE_THROW

#endif // MSEREGISTERED_H_
