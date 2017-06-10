
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPOLY_H_
#define MSEPOLY_H_

//include "mseprimitives.h"
#include "msepointerbasics.h"
#include "mseregistered.h"
#include "mserelaxedregistered.h"
#include "mserefcounting.h"
#include "msescope.h"
#include "msemstdvector.h"
#include "mseasyncshared.h"
#include "mseany.h"
#include "msemstdarray.h"
#include <memory>
#include <iostream>
#include <utility>
#include <cassert>

#include <typeinfo>
#include <type_traits>
#include <new>

/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_POLYPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_POLYPOINTER_DISABLED
#else /*MSE_POLYPOINTER_DISABLED*/
#endif /*MSE_POLYPOINTER_DISABLED*/

#if defined(_MSC_VER)
#pragma warning(disable:4503)
#endif

	/* The original variant code came from: https://gist.github.com/tibordp/6909880 */
	template <size_t arg1, size_t ... others>
	struct static_max;

	template <size_t arg>
	struct static_max<arg>
	{
		static const size_t value = arg;
	};

	template <size_t arg1, size_t arg2, size_t ... others>
	struct static_max<arg1, arg2, others...>
	{
		static const size_t value = arg1 >= arg2 ? static_max<arg1, others...>::value :
			static_max<arg2, others...>::value;
	};

	template<typename... Ts>
	struct tdp_variant_helper;

	template<typename F, typename... Ts>
	struct tdp_variant_helper<F, Ts...> {
		inline static void destroy(size_t id, void * data)
		{
			if (id == typeid(F).hash_code())
				reinterpret_cast<F*>(data)->~F();
			else
				tdp_variant_helper<Ts...>::destroy(id, data);
		}

		inline static void move(size_t old_t, void * old_v, void * new_v)
		{
			if (old_t == typeid(F).hash_code())
				::new (new_v) F(std::move(*reinterpret_cast<F*>(old_v)));
			else
				tdp_variant_helper<Ts...>::move(old_t, old_v, new_v);
		}

		inline static void copy(size_t old_t, const void * old_v, void * new_v)
		{
			if (old_t == typeid(F).hash_code())
				::new (new_v) F(*reinterpret_cast<const F*>(old_v));
			else
				tdp_variant_helper<Ts...>::copy(old_t, old_v, new_v);
		}
	};

	template<> struct tdp_variant_helper<> {
		inline static void destroy(size_t id, void * data) { }
		inline static void move(size_t old_t, void * old_v, void * new_v) { }
		inline static void copy(size_t old_t, const void * old_v, void * new_v) { }
	};

	template<typename... Ts>
	struct tdp_variant {
	protected:
		static const size_t data_size = static_max<sizeof(Ts)...>::value;
		static const size_t data_align = static_max<alignof(Ts)...>::value;

		using data_t = typename std::aligned_storage<data_size, data_align>::type;

		using helper_t = tdp_variant_helper<Ts...>;

		static inline size_t invalid_type() {
			return typeid(void).hash_code();
		}

		size_t type_id;
		data_t data;
	public:
		tdp_variant() : type_id(invalid_type()) {   }

		tdp_variant(const tdp_variant<Ts...>& old) : type_id(old.type_id)
		{
			helper_t::copy(old.type_id, &old.data, &data);
		}

		tdp_variant(tdp_variant<Ts...>&& old) : type_id(old.type_id)
		{
			helper_t::move(old.type_id, &old.data, &data);
		}

#ifdef MSE_TDP_VARIANT_ASSIGNMENT_OPERATOR_USE_NON_TYPESAFE_SWAP
		// Serves as both the move and the copy asignment operator.
		tdp_variant<Ts...>& operator= (tdp_variant<Ts...> old)
		{
			std::swap(type_id, old.type_id);
			std::swap(data, old.data);

			return *this;
		}
#else // MSE_TDP_VARIANT_ASSIGNMENT_OPERATOR_USE_NON_TYPESAFE_SWAP
		tdp_variant<Ts...>& operator= (const tdp_variant<Ts...>& old)
		{
			/* The original implementation seemed to assume a bitwise swap was valid, which
			isn't always the case. This implementation doesn't rely on swap functionality, but
			also doesn't benefit from the inherent exception safety of the swap implementation. */
			auto held_type_id = type_id;
			type_id = invalid_type();
			helper_t::destroy(held_type_id, &data);
			helper_t::copy(old.type_id, &old.data, &data);
			type_id = old.type_id;
			return *this;
		}
#endif // MSE_TDP_VARIANT_ASSIGNMENT_OPERATOR_USE_NON_TYPESAFE_SWAP

		template<typename T>
		bool is() const {
			return (type_id == typeid(T).hash_code());
		}

		bool valid() const {
			return (type_id != invalid_type());
		}

		template<typename T, typename... Args>
		void set(Args&&... args)
		{
			// First we destroy the current contents    
			auto held_type_id = type_id;
			type_id = invalid_type();
			helper_t::destroy(held_type_id, &data);
			::new (&data) T(std::forward<Args>(args)...);
			type_id = typeid(T).hash_code();
		}

		template<typename T>
		const T& get() const
		{
			// It is a dynamic_cast-like behaviour
			if (type_id == typeid(T).hash_code())
				return *reinterpret_cast<const T*>(&data);
			else
				MSE_THROW(std::bad_cast());
		}

		template<typename T>
		T& get()
		{
			// It is a dynamic_cast-like behaviour
			if (type_id == typeid(T).hash_code())
				return *reinterpret_cast<T*>(&data);
			else
				MSE_THROW(std::bad_cast());
		}

		~tdp_variant() {
			auto held_type_id = type_id;
			type_id = invalid_type();
			helper_t::destroy(held_type_id, &data);
		}
	};

	template<typename... Ts>
	struct tdp_pointer_variant_helper;

	template<typename F, typename... Ts>
	struct tdp_pointer_variant_helper<F, Ts...> {
		inline static void* arrow_operator(size_t id, const void * data) {
			if (id == typeid(F).hash_code()) {
				return (reinterpret_cast<const F*>(data))->operator->();
			}
			else {
				return tdp_pointer_variant_helper<Ts...>::arrow_operator(id, data);
			}
		}

		inline static const void* const_arrow_operator(size_t id, const void * data) {
			if (id == typeid(F).hash_code()) {
				return (reinterpret_cast<const F*>(data))->operator->();
			}
			else {
				return tdp_pointer_variant_helper<Ts...>::const_arrow_operator(id, data);
			}
		}
	};

	template<> struct tdp_pointer_variant_helper<> {
		inline static void* arrow_operator(size_t id, const void * data) { return nullptr; }
		inline static const void* const_arrow_operator(size_t id, const void * data) { return nullptr; }
	};

	template<typename... Ts>
	struct tdp_pointer_variant : public tdp_variant<Ts...> {
	protected:
		using pointer_helper_t = tdp_pointer_variant_helper<Ts...>;
	public:
		using tdp_variant<Ts...>::tdp_variant;

		void* arrow_operator() const {
			return pointer_helper_t::arrow_operator((*this).type_id, &((*this).data));
		}
		const void* const_arrow_operator() const {
			return pointer_helper_t::const_arrow_operator((*this).type_id, &((*this).data));
		}
	};

	template <typename _Ty>
	class TAnyPointer;
	template <typename _Ty>
	class TXScopeAnyConstPointer;
	template <typename _Ty>
	class TAnyConstPointer;

	template <typename _Ty>
	class TCommonPointerInterface {
	public:
		virtual ~TCommonPointerInterface() {}
		virtual _Ty& operator*() const = 0;
		virtual _Ty* operator->() const = 0;
	};

	template <typename _Ty, typename _TPointer1>
	class TCommonizedPointer : public TCommonPointerInterface<_Ty> {
	public:
		TCommonizedPointer(const _TPointer1& pointer) : m_pointer(pointer) {}
		virtual ~TCommonizedPointer() {}

		_Ty& operator*() const {
			return (*m_pointer);
		}
		_Ty* operator->() const {
			//return m_pointer.operator->();
			return std::addressof(*m_pointer);
		}

		_TPointer1 m_pointer;
	};

	template <typename _Ty>
	class TXScopeAnyPointer {
	public:
		TXScopeAnyPointer(const TXScopeAnyPointer& src) : m_any_pointer(src.m_any_pointer) {}
		TXScopeAnyPointer(const TAnyPointer<_Ty>& src) : m_any_pointer(src.m_any_pointer) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TXScopeAnyPointer<_Ty>>::value)
			&& (!std::is_same<_TPointer1, TXScopeAnyConstPointer<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyConstPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_same<_TPointer1, TAnyPointer<_Ty>>::value)
			&& (!std::is_base_of<TAnyPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_same<_TPointer1, TAnyConstPointer<_Ty>>::value)
			&& (!std::is_base_of<TAnyConstPointer<_Ty>, _TPointer1>::value)
			, void>::type>
		TXScopeAnyPointer(const _TPointer1& pointer) : m_any_pointer(TCommonizedPointer<_Ty, _TPointer1>(pointer)) {}

		_Ty& operator*() const {
			return (*(*common_pointer_interface_const_ptr()));
		}
		_Ty* operator->() const {
			return common_pointer_interface_const_ptr()->operator->();
		}
		template <typename _Ty2>
		bool operator ==(const _Ty2& _Right_cref) const {
			return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
		}
		template <typename _Ty2>
		bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }

	protected:
		TXScopeAnyPointer<_Ty>& operator=(const TXScopeAnyPointer<_Ty>& _Right_cref) {
			m_any_pointer = _Right_cref.m_any_pointer;
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyPointer<_Ty>* operator&() { return this; }
		const TXScopeAnyPointer<_Ty>* operator&() const { return this; }

		const TCommonPointerInterface<_Ty>* common_pointer_interface_const_ptr() const {
			auto retval = reinterpret_cast<const TCommonPointerInterface<_Ty>*>(m_any_pointer.storage_address());
			assert(nullptr != retval);
			return retval;
		}

		mse::any m_any_pointer;
	};

	template <typename _Ty>
	class TAnyPointer : public TXScopeAnyPointer<_Ty> {
	public:
		TAnyPointer(const TAnyPointer& src) : TXScopeAnyPointer<_Ty>(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyPointer>::value)
			&& (!std::is_same<_TPointer1, TXScopeAnyPointer<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_same<_TPointer1, TXScopeAnyConstPointer<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyConstPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_same<_TPointer1, TAnyConstPointer<_Ty>>::value)
			&& (!std::is_base_of<TAnyConstPointer<_Ty>, _TPointer1>::value)
#ifndef MSE_SCOPEPOINTER_DISABLED
			&& (!std::is_convertible<_TPointer1, TXScopeFixedPointer<_Ty>>::value) 
			&& (!std::is_convertible<_TPointer1, TXScopeFixedConstPointer<_Ty>>::value)
#endif // !MSE_SCOPEPOINTER_DISABLED
			, void>::type>
			TAnyPointer(const _TPointer1& pointer) : TXScopeAnyPointer<_Ty>(pointer) {}

		TAnyPointer<_Ty>& operator=(const TAnyPointer<_Ty>& _Right_cref) {
			TXScopeAnyPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

	protected:
		TAnyPointer<_Ty>* operator&() { return this; }
		const TAnyPointer<_Ty>* operator&() const { return this; }
	};

	template <typename _Ty>
	class TCommonConstPointerInterface {
	public:
		virtual ~TCommonConstPointerInterface() {}
		virtual const _Ty& operator*() const = 0;
		virtual const _Ty* operator->() const = 0;
	};

	template <typename _Ty, typename _TConstPointer1>
	class TCommonizedConstPointer : public TCommonConstPointerInterface<_Ty> {
	public:
		TCommonizedConstPointer(const _TConstPointer1& const_pointer) : m_const_pointer(const_pointer) {}
		virtual ~TCommonizedConstPointer() {}

		const _Ty& operator*() const {
			return (*m_const_pointer);
		}
		const _Ty* operator->() const {
			//return m_const_pointer.operator->();
			return std::addressof(*m_const_pointer);
		}

		_TConstPointer1 m_const_pointer;
	};

	template <typename _Ty>
	class TXScopeAnyConstPointer {
	public:
		TXScopeAnyConstPointer(const TXScopeAnyConstPointer& src) : m_any_const_pointer(src.m_any_const_pointer) {}
		TXScopeAnyConstPointer(const TAnyConstPointer<_Ty>& src) : m_any_const_pointer(src.m_any_const_pointer) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TXScopeAnyConstPointer<_Ty>>::value)
			&& (!std::is_same<_TPointer1, TAnyConstPointer<_Ty>>::value)
			&& (!std::is_base_of<TAnyConstPointer<_Ty>, _TPointer1>::value)
			, void>::type>
		TXScopeAnyConstPointer(const _TPointer1& pointer) : m_any_const_pointer(TCommonizedConstPointer<_Ty, _TPointer1>(pointer)) {}

		const _Ty& operator*() const {
			return (*(*common_pointer_interface_const_ptr()));
		}
		const _Ty* operator->() const {
			return common_pointer_interface_const_ptr()->operator->();
		}
		template <typename _Ty2>
		bool operator ==(const _Ty2& _Right_cref) const {
			return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
		}
		template <typename _Ty2>
		bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }

	protected:
		TXScopeAnyConstPointer<_Ty>& operator=(const TXScopeAnyConstPointer<_Ty>& _Right_cref) {
			m_any_const_pointer = _Right_cref.m_any_const_pointer;
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyConstPointer<_Ty>* operator&() { return this; }
		const TXScopeAnyConstPointer<_Ty>* operator&() const { return this; }

		const TCommonPointerInterface<_Ty>* common_pointer_interface_const_ptr() const {
			auto retval = reinterpret_cast<const TCommonPointerInterface<_Ty>*>(m_any_const_pointer.storage_address());
			assert(nullptr != retval);
			return retval;
		}

		mse::any m_any_const_pointer;
	};

	template <typename _Ty>
	class TAnyConstPointer : public TXScopeAnyConstPointer<_Ty> {
	public:
		TAnyConstPointer(const TAnyConstPointer& src) : TXScopeAnyConstPointer<_Ty>(src) {}
		TAnyConstPointer(const TAnyPointer<_Ty>& src) : TXScopeAnyConstPointer<_Ty>(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyConstPointer<_Ty>>::value)
			&& (!std::is_same<_TPointer1, TXScopeAnyPointer<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_same<_TPointer1, TXScopeAnyConstPointer<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyConstPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_same<_TPointer1, TAnyPointer<_Ty>>::value)
			&& (!std::is_base_of<TAnyPointer<_Ty>, _TPointer1>::value)
#ifndef MSE_SCOPEPOINTER_DISABLED
			&& (!std::is_convertible<_TPointer1, TXScopeFixedPointer<_Ty>>::value)
			&& (!std::is_convertible<_TPointer1, TXScopeFixedConstPointer<_Ty>>::value)
#endif // !MSE_SCOPEPOINTER_DISABLED
			, void>::type>
			TAnyConstPointer(const _TPointer1& pointer) : TXScopeAnyConstPointer<_Ty>(pointer) {}

		TAnyConstPointer<_Ty>& operator=(const TAnyConstPointer<_Ty>& _Right_cref) {
			TXScopeAnyConstPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

	protected:
		TAnyConstPointer<_Ty>* operator&() { return this; }
		const TAnyConstPointer<_Ty>* operator&() const { return this; }
	};


	template<typename _Ty>
	class TPolyPointerID {};

	template<typename _Ty>
	class TXScopePolyPointer {
	public:
		using poly_variant = tdp_pointer_variant<
#if !defined(MSE_SCOPEPOINTER_DISABLED)
			mse::TXScopeFixedPointer<_Ty>,
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
			mse::TRegisteredPointer<_Ty>,
			mse::TRelaxedRegisteredPointer<_Ty>,
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
			mse::TRefCountingPointer<_Ty>,
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
			typename mse::mstd::vector<_Ty>::iterator,
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
			typename mse::msevector<_Ty>::iterator,
			typename mse::msevector<_Ty>::ipointer,
			typename mse::msevector<_Ty>::ss_iterator_type,
			mse::TAsyncSharedReadWritePointer<_Ty>,
			mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>,
			std::shared_ptr<_Ty>,
			mse::TXScopeAnyPointer<_Ty>,
			//mse::TAnyPointer<_Ty>,

			mse::TPointer<_Ty, TPolyPointerID<const _Ty>>
		>;

		TXScopePolyPointer(const TXScopePolyPointer<_Ty>& p) : m_pointer(p.m_pointer) {}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TXScopePolyPointer(const mse::TXScopeFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyPointer(const mse::TXScopeFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedPointer<_Ty>>(p); }
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
		TXScopePolyPointer(const mse::TRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<
			std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value || std::is_same<const _Ty2, _Ty>::value
			, void>::type>
		TXScopePolyPointer(const mse::TRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredPointer<_Ty>>(p); }

		TXScopePolyPointer(const mse::TRelaxedRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRelaxedRegisteredPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyPointer(const mse::TRelaxedRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRelaxedRegisteredPointer<_Ty>>(p); }
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
		TXScopePolyPointer(const mse::TRefCountingPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyPointer(const mse::TRefCountingPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingPointer<_Ty>>(p); }
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
		TXScopePolyPointer(const typename mse::mstd::vector<_Ty>::iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::iterator>(p); }
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
		TXScopePolyPointer(const typename mse::msevector<_Ty>::iterator& p) { m_pointer.template set<typename mse::msevector<_Ty>::iterator>(p); }
		TXScopePolyPointer(const typename mse::msevector<_Ty>::ipointer& p) { m_pointer.template set<typename mse::msevector<_Ty>::ipointer>(p); }
		TXScopePolyPointer(const typename mse::msevector<_Ty>::ss_iterator_type& p) { m_pointer.template set<typename mse::msevector<_Ty>::ss_iterator_type>(p); }
		TXScopePolyPointer(const mse::TAsyncSharedReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedReadWritePointer<_Ty>>(p); }
		TXScopePolyPointer(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>>(p); }
		TXScopePolyPointer(const std::shared_ptr<_Ty>& p) { m_pointer.template set<std::shared_ptr<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyPointer(const std::shared_ptr<_Ty2>& p) { m_pointer.template set<std::shared_ptr<_Ty>>(p); }
		TXScopePolyPointer(const mse::TXScopeAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyPointer<_Ty>>(p); }
		TXScopePolyPointer(const mse::TAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyPointer<_Ty>>(p); }

		TXScopePolyPointer(_Ty* p) { m_pointer.template set<mse::TPointer<_Ty, TPolyPointerID<const _Ty>>>(p); }

		_Ty& operator*() const {
			return *(reinterpret_cast<_Ty*>(m_pointer.arrow_operator()));
		}
		_Ty* operator->() const {
			return reinterpret_cast<_Ty*>(m_pointer.arrow_operator());
		}
		template <typename _Ty2>
		bool operator ==(const _Ty2& _Right_cref) const {
			return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
		}
		template <typename _Ty2>
		bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }

	protected:
		TXScopePolyPointer<_Ty>& operator=(const TXScopePolyPointer<_Ty>& _Right_cref) {
			/* We can't use the "copy and swap idiom" because the "variant" implementation we're using
			doesn't support typesafe swap. */
			m_pointer.~poly_variant();
			new (&m_pointer) poly_variant(_Right_cref.m_pointer);
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopePolyPointer<_Ty>* operator&() { return this; }
		const TXScopePolyPointer<_Ty>* operator&() const { return this; }

		poly_variant m_pointer;
	};

	template<typename _Ty>
	class TPolyPointer : public TXScopePolyPointer<_Ty> {
	public:

		TPolyPointer(const TPolyPointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}

#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
		TPolyPointer(const mse::TRegisteredPointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<
			std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value || std::is_same<const _Ty2, _Ty>::value
			, void>::type>
			TPolyPointer(const mse::TRegisteredPointer<_Ty2>& p) : TXScopePolyPointer<_Ty>(p) {}

		TPolyPointer(const mse::TRelaxedRegisteredPointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointer(const mse::TRelaxedRegisteredPointer<_Ty2>& p) : TXScopePolyPointer<_Ty>(p) {}
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
		TPolyPointer(const mse::TRefCountingPointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointer(const mse::TRefCountingPointer<_Ty2>& p) : TXScopePolyPointer<_Ty>(p) {}
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyPointer(const typename mse::mstd::vector<_Ty>::iterator& p) : TXScopePolyPointer<_Ty>(p) {}
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyPointer(const typename mse::msevector<_Ty>::iterator& p) : TXScopePolyPointer<_Ty>(p) {}
		TPolyPointer(const typename mse::msevector<_Ty>::ipointer& p) : TXScopePolyPointer<_Ty>(p) {}
		TPolyPointer(const typename mse::msevector<_Ty>::ss_iterator_type& p) : TXScopePolyPointer<_Ty>(p) {}
		TPolyPointer(const mse::TAsyncSharedReadWritePointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		TPolyPointer(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		TPolyPointer(const std::shared_ptr<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointer(const std::shared_ptr<_Ty2>& p) : TXScopePolyPointer<_Ty>(p) {}
		//TPolyPointer(const mse::TXScopeAnyPointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}
		TPolyPointer(const mse::TAnyPointer<_Ty>& p) : TXScopePolyPointer<_Ty>(p) {}

		TPolyPointer(_Ty* p) : TXScopePolyPointer<_Ty>(p) {}

		TPolyPointer<_Ty>& operator=(const TPolyPointer<_Ty>& _Right_cref) {
			TXScopePolyPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

	private:
		TPolyPointer<_Ty>* operator&() { return this; }
		const TPolyPointer<_Ty>* operator&() const { return this; }
	};

	template<typename _Ty>
	class TXScopePolyConstPointer {
	public:
		using poly_variant = tdp_pointer_variant<
#if !defined(MSE_SCOPEPOINTER_DISABLED)
			mse::TXScopeFixedConstPointer<_Ty>,
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
			mse::TRegisteredConstPointer<_Ty>,
			mse::TRelaxedRegisteredConstPointer<_Ty>,
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
			mse::TRefCountingConstPointer<_Ty>,
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
			typename mse::mstd::vector<_Ty>::const_iterator,
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
			typename mse::msevector<_Ty>::const_iterator,
			typename mse::msevector<_Ty>::cipointer,
			typename mse::msevector<_Ty>::ss_const_iterator_type,
			mse::TAsyncSharedReadWriteConstPointer<_Ty>,
			mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>,
			std::shared_ptr<const _Ty>,
			mse::TXScopeAnyConstPointer<_Ty>,
			//mse::TAnyConstPointer<_Ty>,

			//mse::TXScopePolyPointer<_Ty>,
			mse::TPointer<const _Ty, TPolyPointerID<const _Ty>>
		>;

		TXScopePolyConstPointer(const TXScopePolyConstPointer<_Ty>& p) : m_pointer(p.m_pointer) {}
		TXScopePolyConstPointer(const mse::TXScopePolyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }

#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TXScopePolyConstPointer(const mse::TXScopeFixedConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TXScopeFixedConstPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }

		TXScopePolyConstPointer(const mse::TXScopeFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TXScopeFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
		TXScopePolyConstPointer(const mse::TRegisteredConstPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TRegisteredConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }

		TXScopePolyConstPointer(const mse::TRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }

		TXScopePolyConstPointer(const mse::TRelaxedRegisteredConstPointer<_Ty>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TRelaxedRegisteredConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }

		TXScopePolyConstPointer(const mse::TRelaxedRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TRelaxedRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
		TXScopePolyConstPointer(const mse::TRefCountingConstPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TRefCountingConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }

		TXScopePolyConstPointer(const mse::TRefCountingPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const mse::TRefCountingPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
		TXScopePolyConstPointer(const typename mse::mstd::vector<_Ty>::const_iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::const_iterator>(p); }
		TXScopePolyConstPointer(const typename mse::mstd::vector<_Ty>::iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::const_iterator>(p); }
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
		TXScopePolyConstPointer(const typename mse::msevector<_Ty>::const_iterator& p) { m_pointer.template set<typename mse::msevector<_Ty>::const_iterator>(p); }
		TXScopePolyConstPointer(const typename mse::msevector<_Ty>::cipointer& p) { m_pointer.template set<typename mse::msevector<_Ty>::cipointer>(p); }
		TXScopePolyConstPointer(const typename mse::msevector<_Ty>::ss_const_iterator_type& p) { m_pointer.template set<typename mse::msevector<_Ty>::ss_const_iterator_type>(p); }
		TXScopePolyConstPointer(const mse::TAsyncSharedReadWriteConstPointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedReadWriteConstPointer<_Ty>>(p); }
		TXScopePolyConstPointer(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>>(p); }
		TXScopePolyConstPointer(const std::shared_ptr<const _Ty>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const std::shared_ptr<const _Ty2>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		TXScopePolyConstPointer(const mse::TXScopeAnyConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(p); }
		TXScopePolyConstPointer(const mse::TAnyConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(p); }

		TXScopePolyConstPointer(const typename mse::msevector<_Ty>::iterator& p) { m_pointer.template set<typename mse::msevector<_Ty>::const_iterator>(p); }
		TXScopePolyConstPointer(const typename mse::msevector<_Ty>::ipointer& p) { m_pointer.template set<typename mse::msevector<_Ty>::cipointer>(p); }
		TXScopePolyConstPointer(const typename mse::msevector<_Ty>::ss_iterator_type& p) { m_pointer.template set<typename mse::msevector<_Ty>::ss_const_iterator_type>(p); }
		TXScopePolyConstPointer(const mse::TAsyncSharedReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedReadWriteConstPointer<_Ty>>(p); }
		TXScopePolyConstPointer(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>>(p); }
		TXScopePolyConstPointer(const std::shared_ptr<_Ty>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePolyConstPointer(const std::shared_ptr<_Ty2>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		TXScopePolyConstPointer(const mse::TXScopeAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }
		TXScopePolyConstPointer(const mse::TAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }

		TXScopePolyConstPointer(const _Ty* p) { m_pointer.template set<mse::TPointer<const _Ty, TPolyPointerID<const _Ty>>>(p); }

		const _Ty& operator*() const {
			return *(reinterpret_cast<const _Ty*>(m_pointer.const_arrow_operator()));
		}
		const _Ty* operator->() const {
			return reinterpret_cast<const _Ty*>(m_pointer.const_arrow_operator());
		}
		template <typename _Ty2>
		bool operator ==(const _Ty2& _Right_cref) const {
			return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
		}
		template <typename _Ty2>
		bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }

	protected:
		TXScopePolyConstPointer<_Ty>& operator=(const TXScopePolyConstPointer<_Ty>& _Right_cref) {
			/* We can't use the "copy and swap idiom" because the "variant" implementation we're using
			doesn't support typesafe swap. */
			m_pointer.~poly_variant();
			new (&m_pointer) poly_variant(_Right_cref.m_pointer);
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopePolyConstPointer<_Ty>* operator&() { return this; }
		const TXScopePolyConstPointer<_Ty>* operator&() const { return this; }

		poly_variant m_pointer;
	};

	template<typename _Ty>
	class TPolyConstPointer : public TXScopePolyConstPointer<_Ty> {
	public:

		TPolyConstPointer(const TPolyConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TPolyPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}

#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
		TPolyConstPointer(const mse::TRegisteredConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value, void>::type>
		TPolyConstPointer(const mse::TRegisteredConstPointer<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer(const mse::TRegisteredPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value, void>::type>
		TPolyConstPointer(const mse::TRegisteredPointer<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer(const mse::TRelaxedRegisteredConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointer(const mse::TRelaxedRegisteredConstPointer<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer(const mse::TRelaxedRegisteredPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointer(const mse::TRelaxedRegisteredPointer<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
		TPolyConstPointer(const mse::TRefCountingConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointer(const mse::TRefCountingConstPointer<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer(const mse::TRefCountingPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointer(const mse::TRefCountingPointer<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyConstPointer(const typename mse::mstd::vector<_Ty>::const_iterator& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const typename mse::mstd::vector<_Ty>::iterator& p) : TXScopePolyConstPointer<_Ty>(p) {}
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyConstPointer(const typename mse::msevector<_Ty>::const_iterator& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const typename mse::msevector<_Ty>::cipointer& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const typename mse::msevector<_Ty>::ss_const_iterator_type& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TAsyncSharedReadWriteConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const std::shared_ptr<const _Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointer(const std::shared_ptr<const _Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TXScopeAnyConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TAnyConstPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer(const typename mse::msevector<_Ty>::iterator& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const typename mse::msevector<_Ty>::ipointer& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const typename mse::msevector<_Ty>::ss_iterator_type& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TAsyncSharedReadWritePointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const std::shared_ptr<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointer(const std::shared_ptr<_Ty2>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TXScopeAnyPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}
		TPolyConstPointer(const mse::TAnyPointer<_Ty>& p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer(const _Ty* p) : TXScopePolyConstPointer<_Ty>(p) {}

		TPolyConstPointer<_Ty>& operator=(const TPolyConstPointer<_Ty>& _Right_cref) {
			TXScopePolyConstPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

	private:
		TPolyConstPointer<_Ty>* operator&() { return this; }
		const TPolyConstPointer<_Ty>* operator&() const { return this; }
	};


	template <typename _Ty> using TRandomAccessIteratorBase = typename mse::msearray<_Ty, 0>::random_access_iterator_base;
	template <typename _Ty> using TRandomAccessConstIteratorBase = typename mse::msearray<_Ty, 0>::random_access_const_iterator_base;

	template <typename _Ty>
	class TCommonRandomAccessIteratorInterface : public TRandomAccessIteratorBase<_Ty> {
	public:
		typedef TRandomAccessIteratorBase<_Ty> base_class;

		virtual ~TCommonRandomAccessIteratorInterface() {}
		virtual _Ty& operator*() const = 0;
		virtual _Ty* operator->() const = 0;
		typedef typename base_class::reference reference_t;
		typedef typename base_class::difference_type difference_t;
		virtual reference_t operator[](difference_t _Off) const = 0;
		virtual void operator +=(difference_t x) = 0;
		virtual void operator -=(difference_t x) { operator +=(-x); }
		virtual void operator ++() { operator +=(1); }
		virtual void operator ++(int) { operator +=(1); }
		virtual void operator --() { operator -=(1); }
		virtual void operator --(int) { operator -=(1); }
		virtual difference_t operator-(const TCommonRandomAccessIteratorInterface& _Right_cref) const = 0;
		virtual bool operator==(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 == operator-(_Right_cref)); }
		virtual bool operator!=(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return !(operator==(_Right_cref)); }
		virtual bool operator<(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		virtual bool operator>(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		virtual bool operator<=(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		virtual bool operator>=(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
	};

	template <typename _Ty, typename _TRandomAccessIterator1>
	class TCommonizedRandomAccessIterator : public TCommonRandomAccessIteratorInterface<_Ty> {
	public:
		TCommonizedRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : m_random_access_iterator(random_access_iterator) {}
		virtual ~TCommonizedRandomAccessIterator() {}

		_Ty& operator*() const {
			return (*m_random_access_iterator);
		}
		_Ty* operator->() const {
			return std::addressof(*m_random_access_iterator);
			//return m_random_access_iterator.operator->();
		}
		typename TCommonRandomAccessIteratorInterface<_Ty>::reference_t operator[](typename TCommonRandomAccessIteratorInterface<_Ty>::difference_t _Off) const {
			return m_random_access_iterator[_Off];
		}
		void operator +=(typename TCommonRandomAccessIteratorInterface<_Ty>::difference_t x) { m_random_access_iterator += x; }
		typename TCommonRandomAccessIteratorInterface<_Ty>::difference_t operator-(const TCommonRandomAccessIteratorInterface<_Ty>& _Right_cref) const {
			const TCommonizedRandomAccessIterator* crai_ptr = static_cast<const TCommonizedRandomAccessIterator*>(&_Right_cref);
			assert(crai_ptr);
			const _TRandomAccessIterator1& _Right_cref_m_random_access_iterator_cref = (*crai_ptr).m_random_access_iterator;
			return m_random_access_iterator - _Right_cref_m_random_access_iterator_cref;
		}

		_TRandomAccessIterator1 m_random_access_iterator;
	};

	template <typename _Ty>
	class TAnyRandomAccessIterator;

	template <typename _Ty>
	class TXScopeAnyRandomAccessIterator : public TRandomAccessIteratorBase<_Ty> {
	public:
		TXScopeAnyRandomAccessIterator(const TXScopeAnyRandomAccessIterator& src) : m_any_random_access_iterator(src.m_any_random_access_iterator) {}
		TXScopeAnyRandomAccessIterator(_Ty arr[]) : m_any_random_access_iterator(TCommonizedRandomAccessIterator<_Ty, _Ty*>((_Ty*)arr)) {}

		template <typename _TRandomAccessIterator1, class = typename std::enable_if<!std::is_convertible<_TRandomAccessIterator1, TXScopeAnyRandomAccessIterator>::value, void>::type>
		TXScopeAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : m_any_random_access_iterator(TCommonizedRandomAccessIterator<_Ty, _TRandomAccessIterator1>(random_access_iterator)) {}

		friend void swap(TXScopeAnyRandomAccessIterator& first, TXScopeAnyRandomAccessIterator& second) {
			std::swap(first.m_any_random_access_iterator, second.m_any_random_access_iterator);
		}

		_Ty& operator*() const {
			return (*(*common_random_access_iterator_interface_ptr()));
		}
		_Ty* operator->() const {
			return common_random_access_iterator_interface_ptr()->operator->();
		}
		typedef typename TCommonRandomAccessIteratorInterface<_Ty>::reference_t reference_t;
		typedef typename TCommonRandomAccessIteratorInterface<_Ty>::difference_t difference_t;
		reference_t operator[](difference_t _Off) const {
			return common_random_access_iterator_interface_ptr()->operator[](_Off);
		}
		void operator +=(difference_t x) { common_random_access_iterator_interface_ptr()->operator+=(x); }
		void operator -=(difference_t x) { operator +=(-x); }
		TXScopeAnyRandomAccessIterator& operator ++() { operator +=(1); return (*this); }
		TXScopeAnyRandomAccessIterator operator ++(int) { auto _Tmp = (*this); operator +=(1); return _Tmp; }
		TXScopeAnyRandomAccessIterator& operator --() { operator -=(1); return (*this); }
		TXScopeAnyRandomAccessIterator operator --(int) { auto _Tmp = (*this); operator -=(1); return _Tmp; }

		TXScopeAnyRandomAccessIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TXScopeAnyRandomAccessIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TXScopeAnyRandomAccessIterator& _Right_cref) const {
			return (*common_random_access_iterator_interface_ptr()) - (*(_Right_cref.common_random_access_iterator_interface_ptr()));
		}
		bool operator==(const TXScopeAnyRandomAccessIterator& _Right_cref) const { return (0 == operator-(_Right_cref)); }
		bool operator!=(const TXScopeAnyRandomAccessIterator& _Right_cref) const { return !(operator==(_Right_cref)); }
		bool operator<(const TXScopeAnyRandomAccessIterator& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TXScopeAnyRandomAccessIterator& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TXScopeAnyRandomAccessIterator& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TXScopeAnyRandomAccessIterator& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TXScopeAnyRandomAccessIterator& operator=(TXScopeAnyRandomAccessIterator _Right) {
			swap(*this, _Right);
			return (*this);
		}

	protected:
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyRandomAccessIterator<_Ty>* operator&() { return this; }
		const TXScopeAnyRandomAccessIterator<_Ty>* operator&() const { return this; }

		TCommonRandomAccessIteratorInterface<_Ty>* common_random_access_iterator_interface_ptr() {
			auto retval = reinterpret_cast<TCommonRandomAccessIteratorInterface<_Ty>*>(m_any_random_access_iterator.storage_address());
			assert(nullptr != retval);
			return retval;
		}
		const TCommonRandomAccessIteratorInterface<_Ty>* common_random_access_iterator_interface_ptr() const {
			auto retval = reinterpret_cast<const TCommonRandomAccessIteratorInterface<_Ty>*>(m_any_random_access_iterator.storage_address());
			assert(nullptr != retval);
			return retval;
		}

		mse::any m_any_random_access_iterator;

		friend class TAnyRandomAccessIterator<_Ty>;
	};

	template <typename _Ty>
	class TCommonRandomAccessConstIteratorInterface : public TRandomAccessConstIteratorBase<_Ty> {
	public:
		typedef TRandomAccessConstIteratorBase<_Ty> base_class;

		virtual ~TCommonRandomAccessConstIteratorInterface() {}
		virtual const _Ty& operator*() const = 0;
		virtual const _Ty* operator->() const = 0;
		typedef typename base_class::reference const_reference_t;
		typedef typename base_class::difference_type difference_t;
		virtual const_reference_t operator[](difference_t _Off) const = 0;
		virtual void operator +=(difference_t x) = 0;
		virtual void operator -=(difference_t x) { operator +=(-x); }
		virtual void operator ++() { operator +=(1); }
		virtual void operator ++(int) { operator +=(1); }
		virtual void operator --() { operator -=(1); }
		virtual void operator --(int) { operator -=(1); }
		virtual difference_t operator-(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const = 0;
		virtual bool operator==(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 == operator-(_Right_cref)); }
		virtual bool operator!=(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return !(operator==(_Right_cref)); }
		virtual bool operator<(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		virtual bool operator>(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		virtual bool operator<=(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		virtual bool operator>=(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
	};

	template <typename _Ty, typename _TRandomAccessConstIterator1>
	class TCommonizedRandomAccessConstIterator : public TCommonRandomAccessConstIteratorInterface<_Ty> {
	public:
		TCommonizedRandomAccessConstIterator(const _TRandomAccessConstIterator1& random_access_const_iterator) : m_random_access_const_iterator(random_access_const_iterator) {}
		virtual ~TCommonizedRandomAccessConstIterator() {}

		const _Ty& operator*() const {
			return (*m_random_access_const_iterator);
		}
		const _Ty* operator->() const {
			return std::addressof(*m_random_access_const_iterator);
			//return m_random_access_const_iterator.operator->();
		}
		typename TCommonRandomAccessConstIteratorInterface<_Ty>::const_reference_t operator[](typename TCommonRandomAccessConstIteratorInterface<_Ty>::difference_t _Off) const {
			return m_random_access_const_iterator[_Off];
		}
		void operator +=(typename TCommonRandomAccessConstIteratorInterface<_Ty>::difference_t x) { m_random_access_const_iterator += x; }
		typename TCommonRandomAccessIteratorInterface<_Ty>::difference_t operator-(const TCommonRandomAccessConstIteratorInterface<_Ty>& _Right_cref) const {
			const TCommonizedRandomAccessConstIterator* crai_ptr = static_cast<const TCommonizedRandomAccessConstIterator*>(&_Right_cref);
			assert(crai_ptr);
			const _TRandomAccessConstIterator1& _Right_cref_m_random_access_const_iterator_cref = (*crai_ptr).m_random_access_const_iterator;
			return m_random_access_const_iterator - _Right_cref_m_random_access_const_iterator_cref;
		}

		_TRandomAccessConstIterator1 m_random_access_const_iterator;
	};

	template <typename _Ty>
	class TAnyRandomAccessConstIterator;

	template <typename _Ty>
	class TXScopeAnyRandomAccessConstIterator : public TRandomAccessConstIteratorBase<_Ty> {
	public:
		TXScopeAnyRandomAccessConstIterator(const TXScopeAnyRandomAccessConstIterator& src) : m_any_random_access_const_iterator(src.m_any_random_access_const_iterator) {}
		TXScopeAnyRandomAccessConstIterator(const _Ty arr[]) : m_any_random_access_const_iterator(TCommonizedRandomAccessConstIterator<const _Ty, const _Ty*>((const _Ty*)arr)) {}

		template <typename _TRandomAccessConstIterator1, class = typename std::enable_if<!std::is_convertible<_TRandomAccessConstIterator1, TXScopeAnyRandomAccessConstIterator>::value, void>::type>
		TXScopeAnyRandomAccessConstIterator(const _TRandomAccessConstIterator1& random_access_const_iterator) : m_any_random_access_const_iterator(TCommonizedRandomAccessConstIterator<const _Ty, _TRandomAccessConstIterator1>(random_access_const_iterator)) {}

		friend void swap(TXScopeAnyRandomAccessConstIterator& first, TXScopeAnyRandomAccessConstIterator& second) {
			std::swap(first.m_any_random_access_const_iterator, second.m_any_random_access_const_iterator);
		}

		const _Ty& operator*() const {
			return (*(*common_random_access_const_iterator_interface_ptr()));
		}
		const _Ty* operator->() const {
			return common_random_access_const_iterator_interface_ptr()->operator->();
		}
		typedef typename TCommonRandomAccessConstIteratorInterface<_Ty>::const_reference_t const_reference_t;
		typedef typename TCommonRandomAccessConstIteratorInterface<_Ty>::difference_t difference_t;
		const_reference_t operator[](difference_t _Off) const {
			return common_random_access_const_iterator_interface_ptr()->operator[](_Off);
		}
		void operator +=(difference_t x) { common_random_access_const_iterator_interface_ptr()->operator+=(x); };
		void operator -=(difference_t x) { operator +=(-x); }
		TXScopeAnyRandomAccessConstIterator& operator ++() { operator +=(1); return (*this); }
		TXScopeAnyRandomAccessConstIterator operator ++(int) { auto _Tmp = (*this); operator +=(1); return _Tmp; }
		TXScopeAnyRandomAccessConstIterator& operator --() { operator -=(1); return (*this); }
		TXScopeAnyRandomAccessConstIterator operator --(int) { auto _Tmp = (*this); operator -=(1); return _Tmp; }

		TXScopeAnyRandomAccessConstIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TXScopeAnyRandomAccessConstIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const {
			return (*common_random_access_const_iterator_interface_ptr()) - (*(_Right_cref.common_random_access_const_iterator_interface_ptr()));
		}
		bool operator==(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const { return (0 == operator-(_Right_cref)); }
		bool operator!=(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const { return !(operator==(_Right_cref)); }
		bool operator<(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TXScopeAnyRandomAccessConstIterator& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TXScopeAnyRandomAccessConstIterator& operator=(TXScopeAnyRandomAccessConstIterator _Right) {
			swap(*this, _Right);
			return (*this);
		}

	protected:
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyRandomAccessConstIterator<_Ty>* operator&() { return this; }
		const TXScopeAnyRandomAccessConstIterator<_Ty>* operator&() const { return this; }

		TCommonRandomAccessConstIteratorInterface<_Ty>* common_random_access_const_iterator_interface_ptr() {
			auto retval = reinterpret_cast<TCommonRandomAccessConstIteratorInterface<_Ty>*>(m_any_random_access_const_iterator.storage_address());
			assert(nullptr != retval);
			return retval;
		}
		const TCommonRandomAccessConstIteratorInterface<_Ty>* common_random_access_const_iterator_interface_ptr() const {
			auto retval = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(m_any_random_access_const_iterator.storage_address());
			assert(nullptr != retval);
			return retval;
		}

		mse::any m_any_random_access_const_iterator;

		friend class TAnyRandomAccessConstIterator<_Ty>;
	};

	template<typename T>
	struct HasXScopeIteratorTagMethod
	{
		template<typename U, void(U::*)() const> struct SFINAE {};
		template<typename U> static char Test(SFINAE<U, &U::xscope_iterator_tag>*);
		template<typename U> static int Test(...);
		static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
	};

	template<typename T>
	struct HasXScopeSSIteratorTypeTagMethod
	{
		template<typename U, void(U::*)() const> struct SFINAE {};
		template<typename U> static char Test(SFINAE<U, &U::xscope_ss_iterator_type_tag>*);
		template<typename U> static int Test(...);
		static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
	};

	template <typename _Ty>
	class TAnyRandomAccessIterator : public TXScopeAnyRandomAccessIterator<_Ty> {
	public:
		typedef TXScopeAnyRandomAccessIterator<_Ty> base_class;
		typedef typename base_class::difference_t difference_t;

		TAnyRandomAccessIterator(const TAnyRandomAccessIterator& src) : TXScopeAnyRandomAccessIterator<_Ty>(src) {}
		TAnyRandomAccessIterator(_Ty arr[]) : TXScopeAnyRandomAccessIterator<_Ty>((_Ty*)arr) {}
		template <typename _TRandomAccessIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessIterator1, TAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_same<_TRandomAccessIterator1, TXScopeAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyRandomAccessIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::is_same<_TRandomAccessIterator1, TXScopeAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyRandomAccessConstIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::integral_constant<bool, HasXScopeIteratorTagMethod<_TRandomAccessIterator1>::Has>())
			&& (!std::integral_constant<bool, HasXScopeSSIteratorTypeTagMethod<_TRandomAccessIterator1>::Has>())
			, void>::type>
			TAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : TXScopeAnyRandomAccessIterator<_Ty>(random_access_iterator) {}

		TAnyRandomAccessIterator& operator ++() { base_class::operator ++(); return (*this); }
		TAnyRandomAccessIterator operator ++(int) { auto _Tmp = (*this); base_class::operator +=(1); return _Tmp; }
		TAnyRandomAccessIterator& operator --() { base_class::operator --(); return (*this); }
		TAnyRandomAccessIterator operator --(int) { auto _Tmp = (*this); base_class::operator -=(1); return _Tmp; }

		TAnyRandomAccessIterator operator+(difference_t n) const { return base_class::operator+(n); }
		TAnyRandomAccessIterator operator-(difference_t n) const { return base_class::operator-(n); }
		difference_t operator-(const TAnyRandomAccessIterator& _Right_cref) const { return base_class::operator-(_Right_cref); }

		TAnyRandomAccessIterator& operator=(TAnyRandomAccessIterator _Right) { base_class::operator=(_Right); return (*this); }

	private:
		TAnyRandomAccessIterator(const TXScopeAnyRandomAccessIterator<_Ty>& src) : TXScopeAnyRandomAccessIterator<_Ty>(src) {}

		TAnyRandomAccessIterator<_Ty>* operator&() { return this; }
		const TAnyRandomAccessIterator<_Ty>* operator&() const { return this; }
	};

	template <typename _Ty>
	class TAnyRandomAccessConstIterator : public TXScopeAnyRandomAccessConstIterator<_Ty> {
	public:
		typedef TXScopeAnyRandomAccessConstIterator<_Ty> base_class;
		typedef typename base_class::difference_t difference_t;

		TAnyRandomAccessConstIterator(const TAnyRandomAccessConstIterator& src) : TXScopeAnyRandomAccessConstIterator<_Ty>(src) {}
		TAnyRandomAccessConstIterator(const TAnyRandomAccessIterator<_Ty>& src) : TXScopeAnyRandomAccessConstIterator<_Ty>(static_cast<TXScopeAnyRandomAccessIterator<_Ty>>(src)) {}
		TAnyRandomAccessConstIterator(const _Ty arr[]) : TXScopeAnyRandomAccessConstIterator<_Ty>((const _Ty*)arr) {}

		template <typename _TRandomAccessConstIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessConstIterator1, TAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_same<_TRandomAccessConstIterator1, TXScopeAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyRandomAccessConstIterator<_Ty>, _TRandomAccessConstIterator1>::value)
			&& (!std::is_same<_TRandomAccessConstIterator1, TXScopeAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeAnyRandomAccessIterator<_Ty>, _TRandomAccessConstIterator1>::value)
			&& (!std::is_same<_TRandomAccessConstIterator1, TAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_base_of<TAnyRandomAccessIterator<_Ty>, _TRandomAccessConstIterator1>::value)
			, void>::type>
		TAnyRandomAccessConstIterator(const _TRandomAccessConstIterator1& random_access_const_iterator) : TXScopeAnyRandomAccessConstIterator<_Ty>(random_access_const_iterator) {}

		TAnyRandomAccessConstIterator& operator ++() { base_class::operator ++(); return (*this); }
		TAnyRandomAccessConstIterator operator ++(int) { auto _Tmp = (*this); base_class::operator +=(1); return _Tmp; }
		TAnyRandomAccessConstIterator& operator --() { base_class::operator --(); return (*this); }
		TAnyRandomAccessConstIterator operator --(int) { auto _Tmp = (*this); base_class::operator -=(1); return _Tmp; }

		TAnyRandomAccessConstIterator operator+(difference_t n) const { return base_class::operator+(n); }
		TAnyRandomAccessConstIterator operator-(difference_t n) const { return base_class::operator-(n); }
		difference_t operator-(const TAnyRandomAccessConstIterator& _Right_cref) const { return base_class::operator-(_Right_cref); }

		TAnyRandomAccessConstIterator& operator=(TAnyRandomAccessConstIterator _Right) { base_class::operator=(_Right); return (*this); }

	private:
		TAnyRandomAccessConstIterator(const TXScopeAnyRandomAccessConstIterator<_Ty>& src) : TXScopeAnyRandomAccessConstIterator<_Ty>(src) {}

		TAnyRandomAccessConstIterator<_Ty>* operator&() { return this; }
		const TAnyRandomAccessConstIterator<_Ty>* operator&() const { return this; }
	};

	//template<typename _Ty> using TAnyArrayIterator = TAnyRandomAccessIterator<_Ty>;
	//template<typename _Ty> using TAnyConstArrayIterator = TAnyRandomAccessConstIterator<_Ty>;

	template <typename _TRAIterator>
	class TRASectionIterator {
	public:
		typedef typename mse::mstd::array<int, 0>::difference_type difference_t;
		typedef typename mse::mstd::array<int, 0>::size_type size_type;

	private:
		const _TRAIterator m_ra_iterator;
		const size_type m_count = 0;
		difference_t m_index = 0;

	public:
		TRASectionIterator(_TRAIterator ra_iterator, size_type count, size_type index = 0)
			: m_ra_iterator(ra_iterator), m_count(count), m_index(difference_t(index)) {}
		TRASectionIterator(const TRASectionIterator& src)
			: m_ra_iterator(src.m_ra_iterator), m_count(src.m_count), m_index(src.m_index) {}

		void dereference_bounds_check() const {
			if ((0 > m_index) || (difference_t(m_count) <= m_index)) {
				MSE_THROW(msearray_range_error("out of bounds index - bool dereference_bounds_check() - TRASectionIterator"));
			}
		}
		auto operator*() -> decltype((m_ra_iterator).operator*()) const {
			dereference_bounds_check();
			auto tmp_ra_iterator(m_ra_iterator);
			tmp_ra_iterator += m_index;
			return (tmp_ra_iterator).operator*();
		}
		auto operator->() -> decltype((m_ra_iterator).operator->()) const {
			dereference_bounds_check();
			auto tmp_ra_iterator(m_ra_iterator);
			tmp_ra_iterator += m_index;
			return (tmp_ra_iterator).operator->();
		}
		TRASectionIterator& operator +=(difference_t x) {
			m_index +=(x);
			return (*this);
		}
		TRASectionIterator& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TRASectionIterator& operator ++() { operator +=(1); return (*this); }
		TRASectionIterator& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TRASectionIterator& operator --() { operator -=(1); return (*this); }
		TRASectionIterator& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TRASectionIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TRASectionIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionIterator& _Right_cref) const {
			if (!(_Right_cref.m_ra_iterator == m_ra_iterator)) { MSE_THROW(msearray_range_error("invalid argument - difference_t operator-() - TRASectionIterator")); }
			return m_index - _Right_cref.m_index;
		}
		bool operator ==(const TRASectionIterator& _Right_cref) const {
			return ((_Right_cref.m_index == m_index) && (_Right_cref.m_count == m_count) && (_Right_cref.m_ra_iterator == m_ra_iterator));
		}
		bool operator !=(const TRASectionIterator& _Right_cref) const { return !((*this) == _Right_cref); }
		bool operator<(const TRASectionIterator& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TRASectionIterator& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TRASectionIterator& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TRASectionIterator& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TRASectionIterator& operator=(const TRASectionIterator& _Right_cref) {
			if (!(_Right_cref.m_ra_iterator == m_ra_iterator)) { MSE_THROW(msearray_range_error("invalid argument - TRASectionIterator& operator=() - TRASectionIterator")); }
			m_index = _Right_cref.m_index;
			return (*this);
		}
	};

	template <typename _Ty>
	class TXScopeRandomAccessConstSection;
	template <typename _Ty>
	class TRandomAccessSection;
	template <typename _Ty>
	class TRandomAccessConstSection;

	template <typename _Ty>
	class TXScopeRandomAccessSection {
	public:
		typedef typename TXScopeAnyRandomAccessIterator<_Ty>::reference_t reference_t;
		typedef typename mse::mstd::array<_Ty, 0>::size_type size_type;
		typedef typename TXScopeAnyRandomAccessIterator<_Ty>::difference_t difference_t;

		TXScopeRandomAccessSection(const TXScopeAnyRandomAccessIterator<_Ty>& start_iter, size_type count) : m_start_iter(start_iter), m_count(count) {}
		TXScopeRandomAccessSection(const TXScopeRandomAccessSection& src) = default;
		TXScopeRandomAccessSection(const TRandomAccessSection<_Ty>& src) : m_start_iter(src.m_start_iter), m_count(src.size()) {}

		reference_t operator[](size_type _P) const {
			if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - reference_t operator[](size_type _P) - TXScopeRandomAccessSection")); }
			return m_start_iter[difference_t(_P)];
		}
		size_type size() const {
			return m_count;
		}

		typedef TRASectionIterator<TXScopeAnyRandomAccessIterator<_Ty>> iterator;
		typedef TRASectionIterator<TXScopeAnyRandomAccessConstIterator<_Ty>> const_iterator;
		iterator begin() const { return iterator(m_start_iter, m_count); }
		const_iterator cbegin() const { return const_iterator(m_start_iter, m_count); }
		iterator end() const {
			auto retval(iterator(m_start_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}
		const_iterator cend() const {
			auto retval(const_iterator(m_start_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}

	private:
		TXScopeRandomAccessSection<_Ty>& operator=(const TXScopeRandomAccessSection<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeRandomAccessSection<_Ty>* operator&() { return this; }
		const TXScopeRandomAccessSection<_Ty>* operator&() const { return this; }

		TXScopeAnyRandomAccessIterator<_Ty> m_start_iter;
		const size_type m_count = 0;

		friend class TXScopeRandomAccessConstSection<_Ty>;
	};

	template <typename _Ty>
	class TXScopeRandomAccessConstSection {
	public:
		typedef typename TXScopeAnyRandomAccessConstIterator<_Ty>::const_reference_t const_reference_t;
		typedef typename mse::mstd::array<_Ty, 0>::size_type size_type;
		typedef typename TXScopeAnyRandomAccessConstIterator<_Ty>::difference_t difference_t;

		TXScopeRandomAccessConstSection(const TXScopeAnyRandomAccessConstIterator<_Ty>& start_const_iter, size_type count) : m_start_const_iter(start_const_iter), m_count(count) {}
		TXScopeRandomAccessConstSection(const TXScopeRandomAccessConstSection& src) = default;
		TXScopeRandomAccessConstSection(const TXScopeRandomAccessSection<_Ty>& src) : m_start_const_iter(src.m_start_iter), m_count(src.size()) {}
		TXScopeRandomAccessConstSection(const TRandomAccessSection<_Ty>& src) : m_start_const_iter(src.m_start_iter), m_count(src.size()) {}
		TXScopeRandomAccessConstSection(const TRandomAccessConstSection<_Ty>& src) : m_start_const_iter(src.m_start_const_iter), m_count(src.size()) {}

		const_reference_t operator[](size_type _P) const {
			if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - const_reference_t operator[](size_type _P) - TXScopeRandomAccessConstSection")); }
			return m_start_const_iter[difference_t(_P)];
		}
		size_type size() const {
			return m_count;
		}

		typedef TRASectionIterator<TXScopeAnyRandomAccessConstIterator<_Ty>> iterator;
		typedef TRASectionIterator<TXScopeAnyRandomAccessConstIterator<_Ty>> const_iterator;
		iterator begin() const { return iterator(m_start_const_iter, m_count); }
		const_iterator cbegin() const { return const_iterator(m_start_const_iter, m_count); }
		iterator end() const {
			auto retval(iterator(m_start_const_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}
		const_iterator cend() const {
			auto retval(const_iterator(m_start_const_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}

	private:
		TXScopeRandomAccessConstSection<_Ty>& operator=(const TXScopeRandomAccessConstSection<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeRandomAccessConstSection<_Ty>* operator&() { return this; }
		const TXScopeRandomAccessConstSection<_Ty>* operator&() const { return this; }

		const TXScopeAnyRandomAccessConstIterator<_Ty> m_start_const_iter;
		const size_type m_count = 0;
	};

	//template<typename _Ty> using TArraySection = TXScopeRandomAccessSection<_Ty>;
	//template<typename _Ty> using TConstArraySection = TXScopeRandomAccessConstSection<_Ty>;

	template <typename _Ty>
	class TRandomAccessSection {
	public:
		typedef typename TAnyRandomAccessIterator<_Ty>::reference_t reference_t;
		typedef typename mse::mstd::array<_Ty, 0>::size_type size_type;
		typedef typename TAnyRandomAccessIterator<_Ty>::difference_t difference_t;

		TRandomAccessSection(const TAnyRandomAccessIterator<_Ty>& start_iter, size_type count) : m_start_iter(start_iter), m_count(count) {}
		TRandomAccessSection(const TRandomAccessSection& src) = default;

		reference_t operator[](size_type _P) const {
			if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - reference_t operator[](size_type _P) - TRandomAccessSection")); }
			return m_start_iter[difference_t(_P)];
		}
		size_type size() const {
			return m_count;
		}
		TAnyRandomAccessIterator<_Ty> begin() const { return m_start_iter; }
		TAnyRandomAccessConstIterator<_Ty> cbegin() const { return m_start_iter; }
		TAnyRandomAccessIterator<_Ty> end() const {
			auto retval(m_start_iter);
			retval += (*this).m_count;
			return retval;
		}
		TAnyRandomAccessConstIterator<_Ty> cend() const { return (*this).end(); }

	private:
		const TAnyRandomAccessIterator<_Ty> m_start_iter;
		const size_type m_count = 0;

		friend class TXScopeRandomAccessSection<_Ty>;
		friend class TXScopeRandomAccessConstSection<_Ty>;
		friend class TRandomAccessConstSection<_Ty>;
	};

	template <typename _Ty>
	class TRandomAccessConstSection {
	public:
		typedef typename TAnyRandomAccessConstIterator<_Ty>::const_reference_t const_reference_t;
		typedef typename mse::mstd::array<_Ty, 0>::size_type size_type;
		typedef typename TAnyRandomAccessConstIterator<_Ty>::difference_t difference_t;

		TRandomAccessConstSection(const TAnyRandomAccessConstIterator<_Ty>& start_const_iter, size_type count) : m_start_const_iter(start_const_iter), m_count(count) {}
		TRandomAccessConstSection(const TRandomAccessConstSection& src) = default;
		TRandomAccessConstSection(const TRandomAccessSection<_Ty>& src) : m_start_const_iter(src.m_start_iter), m_count(src.size()) {}

		const_reference_t operator[](size_type _P) const {
			if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - const_reference_t operator[](size_type _P) - TRandomAccessConstSection")); }
			return m_start_const_iter[difference_t(_P)];
		}
		size_type size() const {
			return m_count;
		}
		TAnyRandomAccessConstIterator<_Ty> begin() const { return m_start_const_iter; }
		TAnyRandomAccessConstIterator<_Ty> cbegin() const { return m_start_const_iter; }
		TAnyRandomAccessConstIterator<_Ty> end() const {
			auto retval(m_start_const_iter);
			retval += (*this).m_count;
			return retval;
		}
		TAnyRandomAccessConstIterator<_Ty> cend() const { return (*this).end(); }

	private:
		const TAnyRandomAccessConstIterator<_Ty> m_start_const_iter;
		const size_type m_count = 0;

		friend class TXScopeRandomAccessConstSection<_Ty>;
	};

	//template<typename _Ty> using TArraySection = TRandomAccessSection<_Ty>;
	//template<typename _Ty> using TConstArraySection = TRandomAccessConstSection<_Ty>;


	template <typename _Ty>
	class TNullableAnyRandomAccessIterator : public TAnyRandomAccessIterator<_Ty> {
	public:
		TNullableAnyRandomAccessIterator() : TAnyRandomAccessIterator<_Ty>(typename mse::mstd::vector<typename std::remove_const<_Ty>::type>::iterator()), m_is_null(true) {}
		TNullableAnyRandomAccessIterator(const std::nullptr_t& src) : TNullableAnyRandomAccessIterator() {}
		TNullableAnyRandomAccessIterator(const TNullableAnyRandomAccessIterator& src) : TAnyRandomAccessIterator<_Ty>(src) {}
		TNullableAnyRandomAccessIterator(const TAnyRandomAccessIterator<_Ty>& src) : TAnyRandomAccessIterator<_Ty>(src) {}
		explicit TNullableAnyRandomAccessIterator(_Ty arr[]) : TAnyRandomAccessIterator<_Ty>((_Ty *)arr) {}

		template <typename _TRandomAccessIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessIterator1, TNullableAnyRandomAccessIterator>::value)
			&& (!std::is_same<_TRandomAccessIterator1, TAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_base_of<TAnyRandomAccessIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, int>::value)
			/*&& (!std::is_same<_TRandomAccessIterator1, TXScopeNullableAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeNullableAnyRandomAccessIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::is_same<_TRandomAccessIterator1, TXScopeNullableAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeNullableAnyRandomAccessConstIterator<_Ty>, _TRandomAccessIterator1>::value)*/
			&& (!std::integral_constant<bool, HasXScopeIteratorTagMethod<_TRandomAccessIterator1>::Has>())
			&& (!std::integral_constant<bool, HasXScopeSSIteratorTypeTagMethod<_TRandomAccessIterator1>::Has>())
			, void>::type>
			TNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : TAnyRandomAccessIterator<_Ty>(random_access_iterator) {}

		friend void swap(TNullableAnyRandomAccessIterator& first, TNullableAnyRandomAccessIterator& second) {
			std::swap(static_cast<TAnyRandomAccessIterator<_Ty>&>(first), static_cast<TAnyRandomAccessIterator<_Ty>&>(second));
			std::swap(first.m_is_null, second.m_is_null);
		}

		bool operator==(const std::nullptr_t& _Right_cref) const { return m_is_null; }
		TNullableAnyRandomAccessIterator& operator=(const std::nullptr_t& _Right_cref) {
			return operator=(TNullableAnyRandomAccessIterator());
		}
		TNullableAnyRandomAccessIterator& operator=(TNullableAnyRandomAccessIterator _Right) {
			swap(*this, _Right);
			return (*this);
		}

		explicit operator bool() const {
			return (!m_is_null);
		}

	private:
		TNullableAnyRandomAccessIterator<_Ty>* operator&() { return this; }
		const TNullableAnyRandomAccessIterator<_Ty>* operator&() const { return this; }
		bool m_is_null = false;
	};

	template <typename _Ty>
	class TNullableAnyPointer : public TAnyPointer<_Ty> {
	public:
		TNullableAnyPointer() : TAnyPointer<_Ty>(mse::TRegisteredPointer<_Ty>()), m_is_null(true) {}
		TNullableAnyPointer(const std::nullptr_t& src) : TNullableAnyPointer() {}
		TNullableAnyPointer(const TNullableAnyPointer& src) : TAnyPointer<_Ty>(src) {}
		TNullableAnyPointer(const TAnyPointer<_Ty>& src) : TAnyPointer<_Ty>(src) {}

		template <typename _TRandomAccessIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessIterator1, TNullableAnyPointer>::value)
			&& (!std::is_same<_TRandomAccessIterator1, TAnyPointer<_Ty>>::value)
			&& (!std::is_base_of<TAnyPointer<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, int>::value)
			/*&& (!std::is_same<_TRandomAccessIterator1, TXScopeNullableAnyRandomAccessIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeNullableAnyRandomAccessIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::is_same<_TRandomAccessIterator1, TXScopeNullableAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_base_of<TXScopeNullableAnyRandomAccessConstIterator<_Ty>, _TRandomAccessIterator1>::value)*/
			, void>::type>
			TNullableAnyPointer(const _TRandomAccessIterator1& random_access_iterator) : TAnyPointer<_Ty>(random_access_iterator) {}

		friend void swap(TNullableAnyPointer& first, TNullableAnyPointer& second) {
			std::swap(static_cast<TAnyPointer<_Ty>&>(first), static_cast<TAnyPointer<_Ty>&>(second));
			std::swap(first.m_is_null, second.m_is_null);
		}

		bool operator==(const std::nullptr_t& _Right_cref) const { return m_is_null; }
		TNullableAnyPointer& operator=(const std::nullptr_t& _Right_cref) {
			return operator=(TNullableAnyPointer());
		}
		TNullableAnyPointer& operator=(TNullableAnyPointer _Right) {
			swap(*this, _Right);
			return (*this);
		}

		operator bool() const {
			return (!m_is_null);
		}

	private:
		TNullableAnyPointer<_Ty>* operator&() { return this; }
		const TNullableAnyPointer<_Ty>* operator&() const { return this; }
		bool m_is_null = false;
	};


	/* shorter aliases */
	template<typename _Ty> using pp = TPolyPointer<_Ty>;
	template<typename _Ty> using pcp = TPolyConstPointer<_Ty>;
	template<typename _Ty> using anyp = TAnyPointer<_Ty>;
	template<typename _Ty> using anycp = TAnyConstPointer<_Ty>;


	/* Deprecated poly pointers. */
	template<typename _Ty> class TRefCountingOrXScopeFixedConstPointer;

	template<typename _Ty>
	class TRefCountingOrXScopeFixedPointer : public TXScopePolyPointer<_Ty> {
	public:
		TRefCountingOrXScopeFixedPointer(const TRefCountingOrXScopeFixedPointer& src_cref) : TXScopePolyPointer<_Ty>(src_cref) {}
		TRefCountingOrXScopeFixedPointer(const TRefCountingPointer<_Ty>& src_cref) : TXScopePolyPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedPointer(const TRefCountingPointer<_Ty2>& src_cref) : TXScopePolyPointer<_Ty>(TRefCountingPointer<_Ty>(src_cref)) {}
		TRefCountingOrXScopeFixedPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopePolyPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopePolyPointer<_Ty>(TXScopeFixedPointer<_Ty>(src_cref)) {}
		virtual ~TRefCountingOrXScopeFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return std::addressof((*this).operator*()); }

#ifndef MSE_SCOPEPOINTER_DISABLED
	protected:
		TRefCountingOrXScopeFixedPointer(_Ty* ptr) : TXScopePolyPointer<_Ty>(ptr) {}
#endif // !MSE_SCOPEPOINTER_DISABLED
	private:
		TRefCountingOrXScopeFixedPointer<_Ty>& operator=(const TRefCountingOrXScopeFixedPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		//TRefCountingOrXScopeFixedPointer<_Ty>* operator&() { return this; }
		//const TRefCountingOrXScopeFixedPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingOrXScopeFixedConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TRefCountingOrXScopeFixedConstPointer : public TXScopePolyConstPointer<_Ty> {
	public:
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingOrXScopeFixedConstPointer& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingOrXScopeFixedPointer<_Ty>& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingConstPointer<_Ty>& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingConstPointer<_Ty2>& src_cref) : TXScopePolyConstPointer<_Ty>(TRefCountingConstPointer<_Ty>(src_cref)) {}
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingPointer<_Ty>& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingPointer<_Ty2>& src_cref) : TXScopePolyConstPointer<_Ty>(TRefCountingPointer<_Ty>(src_cref)) {}
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : TXScopePolyConstPointer<_Ty>(TXScopeFixedConstPointer<_Ty>(src_cref)) {}
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopePolyConstPointer<_Ty>(TXScopeFixedPointer<_Ty>(src_cref)) {}
		virtual ~TRefCountingOrXScopeFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::addressof((*this).operator*()); }

#ifndef MSE_SCOPEPOINTER_DISABLED
	protected:
		TRefCountingOrXScopeFixedConstPointer(_Ty* ptr) : TXScopePolyConstPointer<_Ty>(ptr) {}
#endif // !MSE_SCOPEPOINTER_DISABLED
	private:
		TRefCountingOrXScopeFixedConstPointer<_Ty>& operator=(const TRefCountingOrXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		//TRefCountingOrXScopeFixedConstPointer<_Ty>* operator&() { return this; }
		//const TRefCountingOrXScopeFixedConstPointer<_Ty>* operator&() const { return this; }
	};


	template<typename _Ty>
	class TRefCountingOrXScopeOrRawFixedPointer : public TRefCountingOrXScopeFixedPointer<_Ty> {
	public:
		MSE_SCOPE_USING(TRefCountingOrXScopeOrRawFixedPointer, TRefCountingOrXScopeFixedPointer<_Ty>);
		TRefCountingOrXScopeOrRawFixedPointer(_Ty* ptr) : TRefCountingOrXScopeFixedPointer<_Ty>(ptr) {}
	};

	template<typename _Ty>
	class TRefCountingOrXScopeOrRawFixedConstPointer : public TRefCountingOrXScopeFixedConstPointer<_Ty> {
	public:
		MSE_SCOPE_USING(TRefCountingOrXScopeOrRawFixedConstPointer, TRefCountingOrXScopeFixedConstPointer<_Ty>);
		TRefCountingOrXScopeOrRawFixedConstPointer(_Ty* ptr) : TRefCountingOrXScopeFixedConstPointer<_Ty>(ptr) {}
	};


	template<typename _Ty> class TSharedOrRawFixedConstPointer;

	template<typename _Ty>
	class TSharedOrRawFixedPointer : public TPolyPointer<_Ty> {
	public:
		TSharedOrRawFixedPointer(const TSharedOrRawFixedPointer& src_cref) : TPolyPointer<_Ty>(src_cref) {}
		TSharedOrRawFixedPointer(const std::shared_ptr<_Ty>& src_cref) : TPolyPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TSharedOrRawFixedPointer(const std::shared_ptr<_Ty2>& src_cref) : TPolyPointer<_Ty>(src_cref) {}
		TSharedOrRawFixedPointer(_Ty* ptr) : TPolyPointer<_Ty>(ptr) {}
		virtual ~TSharedOrRawFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return std::addressof((*this).operator*()); }

	private:
		TSharedOrRawFixedPointer<_Ty>& operator=(const TSharedOrRawFixedPointer<_Ty>& _Right_cref) = delete;

		//TSharedOrRawFixedPointer<_Ty>* operator&() { return this; }
		//const TSharedOrRawFixedPointer<_Ty>* operator&() const { return this; }

		friend class TSharedOrRawFixedConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TSharedOrRawFixedConstPointer : public TPolyConstPointer<_Ty> {
	public:
		TSharedOrRawFixedConstPointer(const TSharedOrRawFixedConstPointer& src_cref) : TPolyConstPointer<_Ty>(src_cref) {}
		TSharedOrRawFixedConstPointer(const TSharedOrRawFixedPointer<_Ty>& src_cref) : TPolyConstPointer<_Ty>(src_cref) {}
		TSharedOrRawFixedConstPointer(const std::shared_ptr<const _Ty>& src_cref) : TPolyConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TSharedOrRawFixedConstPointer(const std::shared_ptr<const _Ty2>& src_cref) : TPolyConstPointer<_Ty>(src_cref) {}
		TSharedOrRawFixedConstPointer(const std::shared_ptr<_Ty>& src_cref) : TPolyConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TSharedOrRawFixedConstPointer(const std::shared_ptr<_Ty2>& src_cref) : TStrongFixedConstPointer<_Ty, std::shared_ptr<_Ty>>(src_cref) {}
		TSharedOrRawFixedConstPointer(_Ty* ptr) : TPolyConstPointer<_Ty>(ptr) {}
		virtual ~TSharedOrRawFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::addressof((*this).operator*()); }

	private:
		TSharedOrRawFixedConstPointer<_Ty>& operator=(const TSharedOrRawFixedConstPointer<_Ty>& _Right_cref) = delete;

		//TSharedOrRawFixedConstPointer<_Ty>* operator&() { return this; }
		//const TSharedOrRawFixedConstPointer<_Ty>* operator&() const { return this; }
	};



	static void s_poly_test1() {
#ifdef MSE_SELF_TESTS
		{
			class A {
			public:
				A() {}
				A(int x) : b(x) {}
				virtual ~A() {}

				int b = 3;
			};
			class D : public A {
			public:
				D(int x) : A(x) {}
			};
			class B {
			public:
				static int foo1(mse::TXScopePolyPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
				static int foo2(mse::TXScopePolyConstPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}

				/* Deprecated poly pointers */
				static int foo3(mse::TRefCountingOrXScopeOrRawFixedPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
				static int foo4(mse::TRefCountingOrXScopeOrRawFixedConstPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
				static int foo5(mse::TSharedOrRawFixedPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
				static int foo6(mse::TSharedOrRawFixedConstPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
				static int foo7(mse::TRefCountingOrXScopeFixedPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
				static int foo8(mse::TRefCountingOrXScopeFixedConstPointer<A> ptr) {
					int retval = ptr->b;
					return retval;
				}
			protected:
				~B() {}
			};

			/* To demonstrate, first we'll declare some objects such that we can obtain safe pointers to those
			objects. For better or worse, this library provides a bunch of different safe pointers types. */
			mse::TXScopeObj<A> a_scpobj;
			auto a_refcptr = mse::make_refcounting<A>();
			mse::TRegisteredObj<A> a_regobj;
			mse::TRelaxedRegisteredObj<A> a_rlxregobj;

			/* Safe iterators are a type of safe pointer too. */
			mse::mstd::vector<A> a_mstdvec;
			a_mstdvec.resize(1);
			auto a_mstdvec_iter = a_mstdvec.begin();
			mse::msevector<A> a_msevec;
			a_msevec.resize(1);
			auto a_msevec_ipointer = a_msevec.ibegin();
			auto a_msevec_ssiter = a_msevec.ss_begin();

			/* And don't forget the safe async sharing pointers. */
			auto a_access_requester = mse::make_asyncsharedreadwrite<A>();
			auto a_writelock_ptr = a_access_requester.writelock_ptr();
			auto a_stdshared_const_ptr = mse::make_stdsharedimmutable<A>();

			{
				/* All of these safe pointer types happily convert to an mse::TXScopePolyPointer<>. */
				auto res_using_scpptr = B::foo1(&a_scpobj);
				auto res_using_refcptr = B::foo1(a_refcptr);
				auto res_using_regptr = B::foo1(&a_regobj);
				auto res_using_rlxregptr = B::foo1(&a_rlxregobj);
				auto res_using_mstdvec_iter = B::foo1(a_mstdvec_iter);
				auto res_using_msevec_ipointer = B::foo1(a_msevec_ipointer);
				auto res_using_msevec_ssiter = B::foo1(a_msevec_ssiter);
				auto res_using_writelock_ptr = B::foo1(a_writelock_ptr);

				/* Or an mse::TXScopePolyConstPointer<>. */
				auto res_using_scpptr_via_const_poly = B::foo2(&a_scpobj);
				auto res_using_refcptr_via_const_poly = B::foo2(a_refcptr);
				auto res_using_regptr_via_const_poly = B::foo2(&a_regobj);
				auto res_using_rlxregptr_via_const_poly = B::foo2(&a_rlxregobj);
				auto res_using_mstdvec_iter_via_const_poly = B::foo2(a_mstdvec_iter);
				auto res_using_msevec_ipointer_via_const_poly = B::foo2(a_msevec_ipointer);
				auto res_using_msevec_ssiter_via_const_poly = B::foo2(a_msevec_ssiter);
				auto res_using_writelock_ptr_via_const_poly = B::foo2(a_writelock_ptr);
				auto res_using_stdshared_const_ptr_via_const_poly = B::foo2(a_stdshared_const_ptr);

				mse::TXScopePolyPointer<A> a_polyptr(a_refcptr);
				mse::TXScopePolyPointer<A> a_polyptr2(a_polyptr);
				mse::TXScopePolyConstPointer<A> a_polycptr(a_polyptr);
				mse::TXScopePolyConstPointer<A> a_polycptr2(a_polycptr);
			}

			{
				/* Inheritance polymorphism.  */
				auto D_refcfp = mse::make_refcounting<D>(5);
				mse::TXScopeObj<D> d_xscpobj(7);
				D d_obj(11);
				int res11 = B::foo1(D_refcfp);
				int res12 = B::foo1(&d_xscpobj);
				int res13 = B::foo2(D_refcfp);
				int res14 = B::foo2(&d_xscpobj);
			}

			{
				/* Testing the deprecated poly pointers */
				auto A_refcfp = mse::make_refcounting<A>(5);
				mse::TXScopeObj<A> a_xscpobj(7);
				A a_obj(11);
				int res1 = B::foo7(A_refcfp);
				int res2 = B::foo7(&a_xscpobj);
				int res3 = B::foo8(A_refcfp);
				int res4 = B::foo8(&a_xscpobj);

				auto D_refcfp = mse::make_refcounting<D>(5);
				mse::TXScopeObj<D> d_xscpobj(7);
				D d_obj(11);
				int res11 = B::foo7(D_refcfp);
				int res12 = B::foo7(&d_xscpobj);
				int res13 = B::foo8(D_refcfp);
				int res14 = B::foo8(&d_xscpobj);

				int res21 = B::foo3(A_refcfp);
				int res22 = B::foo3(&a_xscpobj);
				int res23 = B::foo3(&a_obj);
				int res24 = B::foo4(A_refcfp);
				int res25 = B::foo4(&a_xscpobj);
				int res26 = B::foo4(&a_obj);

				int res31 = B::foo3(D_refcfp);
				int res32 = B::foo3(&d_xscpobj);
				int res33 = B::foo3(&d_obj);
				int res34 = B::foo4(D_refcfp);
				int res35 = B::foo4(&d_xscpobj);
				int res36 = B::foo4(&d_obj);

				auto A_shp = std::make_shared<A>(5);
				int res41 = B::foo5(A_shp);
				int res42 = B::foo5(&a_obj);
				int res43 = B::foo6(A_shp);
				int res44 = B::foo6(&a_obj);
			}

			{
				/* Just exercising the tdp_variant type. */
				auto A_refcfp = mse::make_refcounting<A>(5);
				mse::TXScopeObj<A> a_xscpobj(7);

				using my_var = tdp_variant<A*, mse::TScopeFixedPointer<A>, mse::TRefCountingFixedPointer<A>>;

				my_var d;

				d.set<mse::TScopeFixedPointer<A>>(&a_xscpobj);
				//std::cout << d.get<mse::TScopeFixedPointer<A>>()->b << std::endl;

				d.set<mse::TRefCountingFixedPointer<A>>(A_refcfp);
				d.get<mse::TRefCountingFixedPointer<A>>()->b = 42;

				my_var e(std::move(d));
				//std::cout << e.get<mse::TRefCountingFixedPointer<A>>()->b << std::endl;

				e.get<mse::TRefCountingFixedPointer<A>>()->b = 43;

				d = e;

				//std::cout << d.get<mse::TRefCountingFixedPointer<A>>()->b << std::endl;
			}

			{
				/* Poly and "any" pointer assignment operators. */
				mse::TPolyPointer<A> a_poly_pointer1 = a_refcptr;
				mse::TPolyPointer<A> a_poly_pointer2 = &a_regobj;
				auto res21 = a_poly_pointer1->b;
				a_poly_pointer1 = a_poly_pointer2;
				auto res22 = a_poly_pointer1->b;

				mse::TAnyPointer<A> a_any_pointer1 = a_refcptr;
				mse::TAnyPointer<A> a_any_pointer2 = &a_regobj;
				auto res31 = a_any_pointer1->b;
				a_any_pointer1 = a_any_pointer2;
				auto res32 = a_any_pointer1->b;

				mse::mstd::array<int, 4> array1 = { 1, 2, 3, 4 };
				mse::TAnyRandomAccessIterator<int> ara_iter1 = array1.end();
				--ara_iter1;
				mse::TAnyRandomAccessIterator<int> ara_iter2 = array1.begin();
				auto res41 = (*ara_iter1);
				ara_iter1 = ara_iter2;
				auto res42 = (*ara_iter1);
			}
			int q = 3;
		}
#endif // MSE_SELF_TESTS
	}
}

#endif // MSEPOLY_H_
