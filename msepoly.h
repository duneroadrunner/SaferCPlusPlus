
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
#include "msemsestring.h"
#include "msemstdstring.h"
#include <memory>
#include <iostream>
#include <utility>
#include <cassert>

#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <new>

/* for the test functions */
#include <map>
#include <string>

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

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

/* The original implementation required that stored types be movable. We may need to relax that requirement.  */
#ifdef TDP_VARIANT_REQUIRE_MOVABILITY
#define TDP_VARIANT_STD_MOVE(x) std::move(x)
#define TDP_VARIANT_STD_FORWARD(X) std::forward<X>
#else // TDP_VARIANT_REQUIRE_MOVABILITY
#define TDP_VARIANT_STD_MOVE(x) (x)
#define TDP_VARIANT_STD_FORWARD(X)
#endif // TDP_VARIANT_REQUIRE_MOVABILITY

	template<typename... Ts>
	struct tdp_variant_helper;

	template<typename F, typename... Ts>
	struct tdp_variant_helper<F, Ts...> {
		inline static void destroy(std::type_index id, void * data)
		{
			if (id == std::type_index(typeid(F)))
				reinterpret_cast<F*>(data)->~F();
			else
				tdp_variant_helper<Ts...>::destroy(id, data);
		}

		inline static void move(std::type_index old_t, void * old_v, void * new_v)
		{
			if (old_t == std::type_index(typeid(F))) {
				::new (new_v) F(TDP_VARIANT_STD_MOVE(*reinterpret_cast<F*>(old_v)));
			}
			else {
				tdp_variant_helper<Ts...>::move(old_t, old_v, new_v);
			}
		}

		inline static void copy(std::type_index old_t, const void * old_v, void * new_v)
		{
			if (old_t == std::type_index(typeid(F)))
				::new (new_v) F(*reinterpret_cast<const F*>(old_v));
			else
				tdp_variant_helper<Ts...>::copy(old_t, old_v, new_v);
		}
	};

	template<> struct tdp_variant_helper<> {
		inline static void destroy(std::type_index id, void * data) { }
		inline static void move(std::type_index old_t, void * old_v, void * new_v) { }
		inline static void copy(std::type_index old_t, const void * old_v, void * new_v) { }
	};

	template<typename... Ts>
	struct tdp_variant {
	protected:
		static const size_t data_size = static_max<sizeof(Ts)...>::value;
		static const size_t data_align = static_max<alignof(Ts)...>::value;

		using data_t = typename std::aligned_storage<data_size, data_align>::type;

		using helper_t = tdp_variant_helper<Ts...>;

		static inline std::type_index invalid_type() {
			return std::type_index(typeid(void));
		}

		std::type_index type_id;
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
			/* The original implementation seemed to assume a bitwise swap was valid, which isn't always
			the case in our application (with registered pointers specifically). This implementation
			doesn't rely on swap functionality, but also doesn't benefit from the inherent exception
			safety of the swap implementation. */
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
			return (type_id == std::type_index(typeid(T)));
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
			::new (&data) T(TDP_VARIANT_STD_FORWARD(Args) (args)...);
			type_id = std::type_index(typeid(T));
		}

		template<typename T>
		const T& get() const
		{
			// It is a dynamic_cast-like behaviour
			if (type_id == std::type_index(typeid(T)))
				return *reinterpret_cast<const T*>(&data);
			else
				MSE_THROW(std::bad_cast());
		}

		template<typename T>
		T& get()
		{
			// It is a dynamic_cast-like behaviour
			if (type_id == std::type_index(typeid(T)))
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

	template <class _TPointer>
	bool operator_bool_helper1(std::true_type, const _TPointer& ptr_cref) {
		return bool(ptr_cref);
	}
	template <class _TPointer>
	bool operator_bool_helper1(std::false_type, const _TPointer& ptr_cref) {
		/* We need to return the result of conversion to bool, but in this case the "pointer" type, _TPointer, is not convertible
		to bool. Presumably because _TPointer is actually an iterator type. Unfortunately there isn't a good way, in general, to
		determine if an iterator points to a valid item. */
		assert(false);
		return false;
	}

	template<typename... Ts>
	struct tdp_pointer_variant_helper;

	template<typename F, typename... Ts>
	struct tdp_pointer_variant_helper<F, Ts...> {
		inline static void* arrow_operator(std::type_index id, const void * data) {
			if (id == std::type_index(typeid(F))) {
				return (reinterpret_cast<const F*>(data))->operator->();
			}
			else {
				return tdp_pointer_variant_helper<Ts...>::arrow_operator(id, data);
			}
		}
		inline static const void* const_arrow_operator(std::type_index id, const void * data) {
			if (id == std::type_index(typeid(F))) {
				return (reinterpret_cast<const F*>(data))->operator->();
			}
			else {
				return tdp_pointer_variant_helper<Ts...>::const_arrow_operator(id, data);
			}
		}
		inline static bool bool_operator(std::type_index id, const void * data) {
			if (id == std::type_index(typeid(F))) {
				//return bool(*(reinterpret_cast<const F*>(data)));
				return operator_bool_helper1<F>(typename std::is_convertible<F, bool>::type(), *(reinterpret_cast<const F*>(data)));
			}
			else {
				return tdp_pointer_variant_helper<Ts...>::bool_operator(id, data);
			}
		}
	};

	template<> struct tdp_pointer_variant_helper<> {
		inline static void* arrow_operator(std::type_index id, const void * data) { return nullptr; }
		inline static const void* const_arrow_operator(std::type_index id, const void * data) { return nullptr; }
		inline static bool bool_operator(std::type_index id, const void * data) { return false; }
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
		bool bool_operator() const {
			return pointer_helper_t::bool_operator((*this).type_id, &((*this).data));
		}
	};

	template<typename T>
	struct HasXScopeTagMethod_poly
	{
		template<typename U, void(U::*)() const> struct SFINAE {};
		template<typename U> static char Test(SFINAE<U, &U::xscope_tag>*);
		template<typename U> static int Test(...);
		static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
	};

	template <typename _Ty> class TAnyPointerBase;
	template <typename _Ty> class TAnyConstPointerBase;
	template <typename _Ty> class TXScopeAnyPointer;
	template <typename _Ty> class TAnyPointer;
	template <typename _Ty> class TXScopeAnyConstPointer;
	template <typename _Ty> class TAnyConstPointer;

	template <typename _Ty>
	class TCommonPointerInterface {
	public:
		virtual ~TCommonPointerInterface() {}
		virtual _Ty& operator*() const = 0;
		virtual _Ty* operator->() const = 0;
		virtual operator bool() const = 0;
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
		operator bool() const {
			//return bool(m_pointer);
			return operator_bool_helper1<_TPointer1>(typename std::is_convertible<_TPointer1, bool>::type(), m_pointer);
		}

		_TPointer1 m_pointer;
	};

	template <typename _Ty>
	class TAnyPointerBase {
	public:
		TAnyPointerBase(const TAnyPointerBase& src) : m_any_pointer(src.m_any_pointer) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyPointerBase>::value)
			&& (!std::is_base_of<TAnyConstPointerBase<_Ty>, _TPointer1>::value)
			, void>::type>
		TAnyPointerBase(const _TPointer1& pointer) : m_any_pointer(TCommonizedPointer<_Ty, _TPointer1>(pointer)) {}

		_Ty& operator*() const {
			return (*(*common_pointer_interface_ptr()));
		}
		_Ty* operator->() const {
			return std::addressof(*(*common_pointer_interface_ptr()));
		}
		template <typename _Ty2>
		bool operator ==(const _Ty2& _Right_cref) const {
			return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
		}
		template <typename _Ty2>
		bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }

	protected:
		operator bool() const {
			return bool(*common_pointer_interface_ptr());
		}

		TAnyPointerBase<_Ty>* operator&() { return this; }
		const TAnyPointerBase<_Ty>* operator&() const { return this; }

		const TCommonPointerInterface<_Ty>* common_pointer_interface_ptr() const {
			auto retval = reinterpret_cast<const TCommonPointerInterface<_Ty>*>(m_any_pointer.storage_address());
			assert(nullptr != retval);
			return retval;
		}

		mse::any m_any_pointer;

		friend class TAnyConstPointerBase<_Ty>;
	};

	template <typename _Ty>
	class TXScopeAnyPointer : public TAnyPointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef TAnyPointerBase<_Ty> base_class;
		TXScopeAnyPointer(const TAnyPointerBase<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyPointerBase<_Ty>>::value)
			&& (!std::is_base_of<TAnyConstPointerBase<_Ty>, _TPointer1>::value)
			, void>::type>
			TXScopeAnyPointer(const _TPointer1& pointer) : base_class(pointer) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TXScopeAnyPointer<_Ty>& operator=(const TXScopeAnyPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyPointer<_Ty>* operator&() { return this; }
		const TXScopeAnyPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TXScopeAnyPointer<_Ty> >;
	};

	/* The intended semantics of "any" pointers is that they always contain a valid pointer (or iterator) to a valid
	object. If you need a "null" state, consider using optional<> (or mse::TNullableAnyPointer<>). */
	template <typename _Ty>
	class TAnyPointer : public TAnyPointerBase<_Ty> {
	public:
		typedef TAnyPointerBase<_Ty> base_class;
		TAnyPointer(const TAnyPointer& src) : base_class(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyPointer>::value)
			&& (!std::is_base_of<TAnyConstPointer<_Ty>, _TPointer1>::value)
			//&& (!std::integral_constant<bool, HasXScopeTagMethod_poly<_TPointer1>::Has>())
			&& (!std::is_base_of<XScopeTagBase, _TPointer1>::value)
			, void>::type>
			TAnyPointer(const _TPointer1& pointer) : base_class(pointer) {}

		TAnyPointer<_Ty>& operator=(const TAnyPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TAnyPointer<_Ty>* operator&() { return this; }
		const TAnyPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TAnyPointer<_Ty> >;
	};

	template <typename _Ty>
	class TCommonConstPointerInterface {
	public:
		virtual ~TCommonConstPointerInterface() {}
		virtual const _Ty& operator*() const = 0;
		virtual const _Ty* operator->() const = 0;
		virtual operator bool() const = 0;
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
		operator bool() const {
			//return bool(m_const_pointer);
			return operator_bool_helper1<_TConstPointer1>(typename std::is_convertible<_TConstPointer1, bool>::type(), m_const_pointer);
		}

		_TConstPointer1 m_const_pointer;
	};

	template <typename _Ty>
	class TAnyConstPointerBase {
	public:
		TAnyConstPointerBase(const TAnyConstPointerBase& src) : m_any_const_pointer(src.m_any_const_pointer) {}
		TAnyConstPointerBase(const TAnyPointerBase<_Ty>& src) : m_any_const_pointer(src.m_any_pointer) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyConstPointerBase>::value)
			&& (!std::is_convertible<TAnyPointerBase<_Ty>, _TPointer1>::value)
			, void>::type>
			TAnyConstPointerBase(const _TPointer1& pointer) : m_any_const_pointer(TCommonizedConstPointer<_Ty, _TPointer1>(pointer)) {}

		const _Ty& operator*() const {
			return (*(*common_pointer_interface_const_ptr()));
		}
		const _Ty* operator->() const {
			return std::addressof(*(*common_pointer_interface_const_ptr()));
		}
		operator bool() const {
			return bool(*common_pointer_interface_const_ptr());
		}
		template <typename _Ty2>
		bool operator ==(const _Ty2& _Right_cref) const {
			return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
		}
		template <typename _Ty2>
		bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }

	protected:
		TAnyConstPointerBase<_Ty>* operator&() { return this; }
		const TAnyConstPointerBase<_Ty>* operator&() const { return this; }

		const TCommonPointerInterface<_Ty>* common_pointer_interface_const_ptr() const {
			auto retval = reinterpret_cast<const TCommonPointerInterface<_Ty>*>(m_any_const_pointer.storage_address());
			assert(nullptr != retval);
			return retval;
		}

		mse::any m_any_const_pointer;
	};

	template <typename _Ty>
	class TXScopeAnyConstPointer : public TAnyConstPointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef TAnyConstPointerBase<_Ty> base_class;
		TXScopeAnyConstPointer(const TAnyConstPointerBase<_Ty>& src) : base_class(src) {}
		TXScopeAnyConstPointer(const TAnyPointerBase<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyConstPointerBase<_Ty>>::value)
			&& (!std::is_convertible<_TPointer1, TAnyPointerBase<_Ty>>::value)
			, void>::type>
		TXScopeAnyConstPointer(const _TPointer1& pointer) : base_class(pointer) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TXScopeAnyConstPointer<_Ty>& operator=(const TXScopeAnyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyConstPointer<_Ty>* operator&() { return this; }
		const TXScopeAnyConstPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TXScopeAnyConstPointer<_Ty> >;
	};

	template <typename _Ty>
	class TAnyConstPointer : public TAnyConstPointerBase<_Ty> {
	public:
		typedef TAnyConstPointerBase<_Ty> base_class;
		TAnyConstPointer(const TAnyConstPointer& src) : base_class(src) {}
		TAnyConstPointer(const TAnyPointer<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TAnyConstPointer>::value)
			&& (!std::is_convertible<_TPointer1, TAnyPointer<_Ty>>::value)
			//&& (!std::integral_constant<bool, HasXScopeTagMethod_poly<_TPointer1>::Has>())
			&& (!std::is_base_of<XScopeTagBase, _TPointer1>::value)
			, void>::type>
			TAnyConstPointer(const _TPointer1& pointer) : base_class(pointer) {}

		TAnyConstPointer<_Ty>& operator=(const TAnyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TAnyConstPointer<_Ty>* operator&() { return this; }
		const TAnyConstPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TAnyConstPointer<_Ty> >;
	};

	namespace us {
		template<typename _Ty>
		class TFParam<mse::TXScopeAnyConstPointer<_Ty> > : public TXScopeAnyConstPointer<_Ty> {
		public:
			typedef TXScopeAnyConstPointer<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				typename mse::is_instantiation_of_msescope<_TRALoneParam, mse::TXScopeCagedItemFixedConstPointerToRValue>::type()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeItemFixedConstPointerFParam<_Ty2>(std::forward<decltype(param)>(param));
			}
			template<typename _TRALoneParam>
			auto constructor_helper1(std::false_type, _TRALoneParam&& param) { return std::forward<_TRALoneParam>(param); }

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TXScopeAnyPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeAnyPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeAnyPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TAnyPointer<_Ty> > {	// hash functor
		typedef mse::TAnyPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TAnyPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TXScopeAnyConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeAnyConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeAnyConstPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TAnyConstPointer<_Ty> > {	// hash functor
		typedef mse::TAnyConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TAnyConstPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	template<typename _Ty>
	class TPolyPointerID {};

	template <typename _Ty> class TPolyPointerBase;
	template <typename _Ty> class TPolyConstPointerBase;
	template <typename _Ty> class TXScopePolyPointer;
	template <typename _Ty> class TPolyPointer;
	template <typename _Ty> class TXScopePolyConstPointer;
	template <typename _Ty> class TPolyConstPointer;

	template<typename _Ty>
	class TPolyPointerBase {
	public:
		using poly_variant = tdp_pointer_variant<
#if !defined(MSE_SCOPEPOINTER_DISABLED)
			mse::TXScopeFixedPointer<_Ty>,
			mse::TXScopeItemFixedPointer<_Ty>,
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
			typename mse::us::msevector<_Ty>::iterator,
			typename mse::us::msevector<_Ty>::ipointer,
			typename mse::us::msevector<_Ty>::ss_iterator_type,
			std::shared_ptr<_Ty>,
			mse::TXScopeAnyPointer<_Ty>,
			//mse::TAnyPointer<_Ty>,

			mse::TPointer<_Ty, TPolyPointerID<const _Ty>>,

			/* deprecated */
			mse::TAsyncSharedReadWritePointer<_Ty>,
			mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>
		>;

		TPolyPointerBase(const TPolyPointerBase<_Ty>& p) : m_pointer(p.m_pointer) {}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TPolyPointerBase(const mse::TXScopeFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointerBase(const mse::TXScopeFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedPointer<_Ty>>(p); }
		TPolyPointerBase(const mse::TXScopeItemFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeItemFixedPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointerBase(const mse::TXScopeItemFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeItemFixedPointer<_Ty>>(p); }
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
		TPolyPointerBase(const mse::TRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<
			std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value || std::is_same<const _Ty2, _Ty>::value
			, void>::type>
			TPolyPointerBase(const mse::TRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredPointer<_Ty>>(p); }

		TPolyPointerBase(const mse::TRelaxedRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRelaxedRegisteredPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointerBase(const mse::TRelaxedRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRelaxedRegisteredPointer<_Ty>>(p); }
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
		TPolyPointerBase(const mse::TRefCountingPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointerBase(const mse::TRefCountingPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingPointer<_Ty>>(p); }
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyPointerBase(const typename mse::mstd::vector<_Ty>::iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::iterator>(p); }
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyPointerBase(const typename mse::us::msevector<_Ty>::iterator& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::iterator>(p); }
		TPolyPointerBase(const typename mse::us::msevector<_Ty>::ipointer& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ipointer>(p); }
		TPolyPointerBase(const typename mse::us::msevector<_Ty>::ss_iterator_type& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ss_iterator_type>(p); }
		TPolyPointerBase(const std::shared_ptr<_Ty>& p) { m_pointer.template set<std::shared_ptr<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyPointerBase(const std::shared_ptr<_Ty2>& p) { m_pointer.template set<std::shared_ptr<_Ty>>(p); }
		TPolyPointerBase(const mse::TXScopeAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyPointer<_Ty>>(p); }
		TPolyPointerBase(const mse::TAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyPointer<_Ty>>(p); }

		TPolyPointerBase(const mse::TPointer<_Ty>& p) { m_pointer.template set<mse::TPointer<_Ty, TPolyPointerID<const _Ty>>>(p); }
		TPolyPointerBase(_Ty* p) { m_pointer.template set<mse::TPointer<_Ty, TPolyPointerID<const _Ty>>>(p); }

		/* deprecated */
		TPolyPointerBase(const mse::TAsyncSharedReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedReadWritePointer<_Ty>>(p); }
		TPolyPointerBase(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>>(p); }

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

		TPolyPointerBase<_Ty>& operator=(const TPolyPointerBase<_Ty>& _Right_cref) {
			/* We can't use the "copy and swap idiom" because the "variant" implementation we're using
			doesn't support typesafe swap. */
			m_pointer.~poly_variant();
			new (&m_pointer) poly_variant(_Right_cref.m_pointer);
			return (*this);
		}

	protected:
		operator bool() const {
			return m_pointer.bool_operator();
		}

		TPolyPointerBase<_Ty>* operator&() { return this; }
		const TPolyPointerBase<_Ty>* operator&() const { return this; }

		poly_variant m_pointer;

		friend class TPolyConstPointerBase<_Ty>;
	};

	template<typename _Ty>
	class TXScopePolyPointer : public TPolyPointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef TPolyPointerBase<_Ty> base_class;

		//MSE_USING(TXScopePolyPointer, TPolyPointerBase<_Ty>);
		//TXScopePolyPointer(const TPolyPointerBase<_Ty>& p) : base_class(p) {}

		template <typename _TPointer1, class = typename std::enable_if<
			/*(!std::is_convertible<_TPointer1, TPolyPointerBase<_Ty>>::value)
			&& */(!std::is_base_of<TPolyConstPointerBase<_Ty>, _TPointer1>::value)
			, void>::type>
			TXScopePolyPointer(const _TPointer1& pointer) : base_class(pointer) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TXScopePolyPointer<_Ty>& operator=(const TXScopePolyPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopePolyPointer<_Ty>* operator&() { return this; }
		const TXScopePolyPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TXScopePolyPointer<_Ty> >;
	};

	/* The intended semantics of poly pointers is that they always contain a valid pointer (or iterator) to a valid
	object. If you need a "null" state, consider using optional<>. */
	template<typename _Ty>
	class TPolyPointer : public TPolyPointerBase<_Ty> {
	public:
		typedef TPolyPointerBase<_Ty> base_class;

		//MSE_USING(TPolyPointer, TPolyPointerBase<_Ty>);
		//TPolyPointer(const TPolyPointerBase<_Ty>& p) : base_class(p) {}

		template <typename _TPointer1, class = typename std::enable_if<
			//(!std::is_convertible<_TPointer1, TPolyPointer>::value)
			//&& (!std::is_base_of<TPolyConstPointer<_Ty>, _TPointer1>::value)
			//&& (!std::integral_constant<bool, HasXScopeTagMethod_poly<_TPointer1>::Has>())
			/*&&*/ (!std::is_base_of<XScopeTagBase, _TPointer1>::value)
			, void>::type>
			TPolyPointer(const _TPointer1& pointer) : base_class(pointer) {}

		TPolyPointer<_Ty>& operator=(const TPolyPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TPolyPointer<_Ty>* operator&() { return this; }
		const TPolyPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TPolyPointer<_Ty> >;
	};

	template<typename _Ty>
	class TPolyConstPointerBase {
	public:
		using poly_variant = tdp_pointer_variant<
#if !defined(MSE_SCOPEPOINTER_DISABLED)
			mse::TXScopeFixedConstPointer<_Ty>,
			mse::TXScopeItemFixedConstPointer<_Ty>,
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
			typename mse::us::msevector<_Ty>::const_iterator,
			typename mse::us::msevector<_Ty>::cipointer,
			typename mse::us::msevector<_Ty>::ss_const_iterator_type,
			std::shared_ptr<const _Ty>,
			mse::TXScopeAnyConstPointer<_Ty>,
			//mse::TAnyConstPointer<_Ty>,

			//mse::TXScopePolyPointer<_Ty>,
			mse::TPointer<const _Ty, TPolyPointerID<const _Ty>>,

			/* deprecated */
			mse::TAsyncSharedReadWriteConstPointer<_Ty>,
			mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>
		>;

		TPolyConstPointerBase(const TPolyConstPointerBase<_Ty>& p) : m_pointer(p.m_pointer) {}
		TPolyConstPointerBase(const mse::TPolyPointerBase<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }

#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TPolyConstPointerBase(const mse::TXScopeFixedConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TXScopeFixedConstPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TXScopeFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TXScopeFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TXScopeItemFixedConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeItemFixedConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TXScopeItemFixedConstPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeItemFixedConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TXScopeItemFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeItemFixedConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TXScopeItemFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeItemFixedConstPointer<_Ty>>(p); }
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
		TPolyConstPointerBase(const mse::TRegisteredConstPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value, void>::type>
		TPolyConstPointerBase(const mse::TRegisteredConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value, void>::type>
		TPolyConstPointerBase(const mse::TRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TRelaxedRegisteredConstPointer<_Ty>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TRelaxedRegisteredConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TRelaxedRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TRelaxedRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRelaxedRegisteredConstPointer<_Ty>>(p); }
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
		TPolyConstPointerBase(const mse::TRefCountingConstPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TRefCountingConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const mse::TRefCountingPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const mse::TRefCountingPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyConstPointerBase(const typename mse::mstd::vector<_Ty>::const_iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::const_iterator>(p); }
		TPolyConstPointerBase(const typename mse::mstd::vector<_Ty>::iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::const_iterator>(p); }
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
		TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::const_iterator& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::const_iterator>(p); }
		TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::cipointer& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::cipointer>(p); }
		TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::ss_const_iterator_type& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ss_const_iterator_type>(p); }
		TPolyConstPointerBase(const std::shared_ptr<const _Ty>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const std::shared_ptr<const _Ty2>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		TPolyConstPointerBase(const mse::TXScopeAnyConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(p); }
		TPolyConstPointerBase(const mse::TAnyConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(p); }

		TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::iterator& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::const_iterator>(p); }
		TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::ipointer& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::cipointer>(p); }
		TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::ss_iterator_type& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ss_const_iterator_type>(p); }
		TPolyConstPointerBase(const std::shared_ptr<_Ty>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TPolyConstPointerBase(const std::shared_ptr<_Ty2>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
		TPolyConstPointerBase(const mse::TXScopeAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }
		TPolyConstPointerBase(const mse::TAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }

		TPolyConstPointerBase(const mse::TPointer<const _Ty>& p) { m_pointer.template set<mse::TPointer<const _Ty, TPolyPointerID<const _Ty>>>(p); }
		TPolyConstPointerBase(const mse::TPointer<_Ty>& p) { m_pointer.template set<mse::TPointer<const _Ty, TPolyPointerID<const _Ty>>>(p); }
		TPolyConstPointerBase(const _Ty* p) { m_pointer.template set<mse::TPointer<const _Ty, TPolyPointerID<const _Ty>>>(p); }

		/* deprecated */
		TPolyConstPointerBase(const mse::TAsyncSharedReadWriteConstPointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedReadWriteConstPointer<_Ty>>(p); }
		TPolyConstPointerBase(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>>(p); }
		TPolyConstPointerBase(const mse::TAsyncSharedReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedReadWriteConstPointer<_Ty>>(p); }
		TPolyConstPointerBase(const mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& p) { m_pointer.template set<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>>(p); }

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

		TPolyConstPointerBase<_Ty>& operator=(const TPolyConstPointerBase<_Ty>& _Right_cref) {
			/* We can't use the "copy and swap idiom" because the "variant" implementation we're using
			doesn't support typesafe swap. */
			m_pointer.~poly_variant();
			new (&m_pointer) poly_variant(_Right_cref.m_pointer);
			return (*this);
		}

	protected:
		operator bool() const {
			return m_pointer.bool_operator();
		}

		TPolyConstPointerBase<_Ty>* operator&() { return this; }
		const TPolyConstPointerBase<_Ty>* operator&() const { return this; }

		poly_variant m_pointer;
	};

	template<typename _Ty>
	class TXScopePolyConstPointer : public TPolyConstPointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef TPolyConstPointerBase<_Ty> base_class;
		//TXScopePolyConstPointer(const TPolyConstPointerBase<_Ty>& src) : base_class(src) {}
		//TXScopePolyConstPointer(const TPolyPointerBase<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1/*, class = typename std::enable_if<
			(!std::is_convertible<_TPointer1, TPolyConstPointerBase<_Ty>>::value)
			&& (!std::is_convertible<_TPointer1, TPolyPointerBase<_Ty>>::value)
			, void>::type*/>
			TXScopePolyConstPointer(const _TPointer1& pointer) : base_class(pointer) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TXScopePolyConstPointer<_Ty>& operator=(const TXScopePolyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopePolyConstPointer<_Ty>* operator&() { return this; }
		const TXScopePolyConstPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TXScopePolyConstPointer<_Ty> >;
	};

	template<typename _Ty>
	class TPolyConstPointer : public TPolyConstPointerBase<_Ty> {
	public:
		typedef TPolyConstPointerBase<_Ty> base_class;
		//TPolyConstPointer(const TPolyConstPointer& src) : base_class(src) {}
		//TPolyConstPointer(const TPolyPointer<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, class = typename std::enable_if<
			//(!std::is_convertible<_TPointer1, TPolyConstPointer>::value)
			//&& (!std::is_convertible<_TPointer1, TPolyPointer<_Ty>>::value)
			//&& (!std::integral_constant<bool, HasXScopeTagMethod_poly<_TPointer1>::Has>())
			/*&&*/ (!std::is_base_of<XScopeTagBase, _TPointer1>::value)
			, void>::type>
			TPolyConstPointer(const _TPointer1& pointer) : base_class(pointer) {}

		TPolyConstPointer<_Ty>& operator=(const TPolyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TPolyConstPointer<_Ty>* operator&() { return this; }
		const TPolyConstPointer<_Ty>* operator&() const { return this; }

		friend struct std::hash<mse::TPolyConstPointer<_Ty> >;
	};

	namespace us {
		template<typename _Ty>
		class TFParam<mse::TXScopePolyConstPointer<_Ty> > : public TXScopePolyConstPointer<_Ty> {
		public:
			typedef TXScopePolyConstPointer<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				typename mse::is_instantiation_of_msescope<_TRALoneParam, mse::TXScopeCagedItemFixedConstPointerToRValue>::type()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeItemFixedConstPointerFParam<_Ty2>(std::forward<decltype(param)>(param));
			}
			template<typename _TRALoneParam>
			auto constructor_helper1(std::false_type, _TRALoneParam&& param) { return std::forward<_TRALoneParam>(param); }

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TXScopePolyPointer<_Ty> > {	// hash functor
		typedef mse::TXScopePolyPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopePolyPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TPolyPointer<_Ty> > {	// hash functor
		typedef mse::TPolyPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TPolyPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TXScopePolyConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopePolyConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopePolyConstPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TPolyConstPointer<_Ty> > {	// hash functor
		typedef mse::TPolyConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TPolyConstPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	template <typename _Ty> using TRandomAccessIteratorStdBase = random_access_iterator_base<_Ty>;
	template <typename _Ty> using TRandomAccessConstIteratorStdBase = random_access_const_iterator_base<_Ty>;

	template <typename _Ty>
	class TCommonRandomAccessIteratorInterface : public TRandomAccessIteratorStdBase<_Ty> {
	public:
		typedef TRandomAccessIteratorStdBase<_Ty> base_class;

		virtual ~TCommonRandomAccessIteratorInterface() {}
		virtual _Ty& operator*() const = 0;
		virtual _Ty* operator->() const = 0;
		typedef typename base_class::reference reference_t;
		typedef typename base_class::difference_type difference_type;
		virtual reference_t operator[](difference_type _Off) const = 0;
		virtual void operator +=(difference_type x) = 0;
		virtual void operator -=(difference_type x) { operator +=(-x); }
		virtual void operator ++() { operator +=(1); }
		virtual void operator ++(int) { operator +=(1); }
		virtual void operator --() { operator -=(1); }
		virtual void operator --(int) { operator -=(1); }
		virtual difference_type operator-(const TCommonRandomAccessIteratorInterface& _Right_cref) const = 0;
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
		typename TCommonRandomAccessIteratorInterface<_Ty>::reference_t operator[](typename TCommonRandomAccessIteratorInterface<_Ty>::difference_type _Off) const {
			return m_random_access_iterator[_Off];
		}
		void operator +=(typename TCommonRandomAccessIteratorInterface<_Ty>::difference_type x) { m_random_access_iterator += x; }
		typename TCommonRandomAccessIteratorInterface<_Ty>::difference_type operator-(const TCommonRandomAccessIteratorInterface<_Ty>& _Right_cref) const {
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
	class TAnyRandomAccessIteratorBase : public TRandomAccessIteratorStdBase<_Ty> {
	public:
		TAnyRandomAccessIteratorBase(const TAnyRandomAccessIteratorBase& src) : m_any_random_access_iterator(src.m_any_random_access_iterator) {}
		TAnyRandomAccessIteratorBase(_Ty arr[]) : m_any_random_access_iterator(TCommonizedRandomAccessIterator<_Ty, _Ty*>(arr)) {}

		template <typename _TRandomAccessIterator1, class = typename std::enable_if<!std::is_convertible<_TRandomAccessIterator1, TAnyRandomAccessIteratorBase>::value, void>::type>
		TAnyRandomAccessIteratorBase(const _TRandomAccessIterator1& random_access_iterator) : m_any_random_access_iterator(TCommonizedRandomAccessIterator<_Ty, _TRandomAccessIterator1>(random_access_iterator)) {}

		friend void swap(TAnyRandomAccessIteratorBase& first, TAnyRandomAccessIteratorBase& second) {
			std::swap(first.m_any_random_access_iterator, second.m_any_random_access_iterator);
		}

		_Ty& operator*() const {
			return (*(*common_random_access_iterator_interface_ptr()));
		}
		_Ty* operator->() const {
			return common_random_access_iterator_interface_ptr()->operator->();
		}
		typedef typename TCommonRandomAccessIteratorInterface<_Ty>::reference_t reference_t;
		typedef typename TCommonRandomAccessIteratorInterface<_Ty>::difference_type difference_type;
		reference_t operator[](difference_type _Off) const {
			return common_random_access_iterator_interface_ptr()->operator[](_Off);
		}
		void operator +=(difference_type x) { common_random_access_iterator_interface_ptr()->operator+=(x); }
		void operator -=(difference_type x) { operator +=(-x); }
		TAnyRandomAccessIteratorBase& operator ++() { operator +=(1); return (*this); }
		TAnyRandomAccessIteratorBase operator ++(int) { auto _Tmp = (*this); operator +=(1); return _Tmp; }
		TAnyRandomAccessIteratorBase& operator --() { operator -=(1); return (*this); }
		TAnyRandomAccessIteratorBase operator --(int) { auto _Tmp = (*this); operator -=(1); return _Tmp; }

		TAnyRandomAccessIteratorBase operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
		TAnyRandomAccessIteratorBase operator-(difference_type n) const { return ((*this) + (-n)); }
		difference_type operator-(const TAnyRandomAccessIteratorBase& _Right_cref) const {
			return (*common_random_access_iterator_interface_ptr()) - (*(_Right_cref.common_random_access_iterator_interface_ptr()));
		}
		bool operator==(const TAnyRandomAccessIteratorBase& _Right_cref) const { return (0 == operator-(_Right_cref)); }
		bool operator!=(const TAnyRandomAccessIteratorBase& _Right_cref) const { return !(operator==(_Right_cref)); }
		bool operator<(const TAnyRandomAccessIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TAnyRandomAccessIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TAnyRandomAccessIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TAnyRandomAccessIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TAnyRandomAccessIteratorBase& operator=(TAnyRandomAccessIteratorBase _Right) {
			swap(*this, _Right);
			return (*this);
		}

	protected:
		TAnyRandomAccessIteratorBase<_Ty>* operator&() { return this; }
		const TAnyRandomAccessIteratorBase<_Ty>* operator&() const { return this; }

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
	};

	template <typename _Ty>
	class TXScopeAnyRandomAccessIterator : public TAnyRandomAccessIteratorBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef TAnyRandomAccessIteratorBase<_Ty> base_class;

		MSE_USING(TXScopeAnyRandomAccessIterator, base_class);

		typedef typename base_class::difference_type difference_type;

		TXScopeAnyRandomAccessIterator& operator ++() { base_class::operator +=(1); return (*this); }
		TXScopeAnyRandomAccessIterator operator ++(int) { auto _Tmp = (*this); base_class::operator +=(1); return _Tmp; }
		TXScopeAnyRandomAccessIterator& operator --() { base_class::operator -=(1); return (*this); }
		TXScopeAnyRandomAccessIterator operator --(int) { auto _Tmp = (*this); base_class::operator -=(1); return _Tmp; }

		TXScopeAnyRandomAccessIterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
		TXScopeAnyRandomAccessIterator operator-(difference_type n) const { return ((*this) + (-n)); }
		difference_type operator-(const base_class& _Right_cref) const {
			return base_class::operator-(_Right_cref);
		}

		TXScopeAnyRandomAccessIterator& operator=(const base_class& _Right) {
			base_class::operator=(_Right);
			return (*this);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyRandomAccessIterator<_Ty>* operator&() { return this; }
		const TXScopeAnyRandomAccessIterator<_Ty>* operator&() const { return this; }

		friend class TAnyRandomAccessIterator<_Ty>;
	};

	template <typename _Ty>
	class TCommonRandomAccessConstIteratorInterface : public TRandomAccessConstIteratorStdBase<_Ty> {
	public:
		typedef TRandomAccessConstIteratorStdBase<_Ty> base_class;

		virtual ~TCommonRandomAccessConstIteratorInterface() {}
		virtual const _Ty& operator*() const = 0;
		virtual const _Ty* operator->() const = 0;
		typedef typename base_class::reference const_reference_t;
		typedef typename base_class::difference_type difference_type;
		virtual const_reference_t operator[](difference_type _Off) const = 0;
		virtual void operator +=(difference_type x) = 0;
		virtual void operator -=(difference_type x) { operator +=(-x); }
		virtual void operator ++() { operator +=(1); }
		virtual void operator ++(int) { operator +=(1); }
		virtual void operator --() { operator -=(1); }
		virtual void operator --(int) { operator -=(1); }
		virtual difference_type operator-(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const = 0;
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
		typename TCommonRandomAccessConstIteratorInterface<_Ty>::const_reference_t operator[](typename TCommonRandomAccessConstIteratorInterface<_Ty>::difference_type _Off) const {
			return m_random_access_const_iterator[_Off];
		}
		void operator +=(typename TCommonRandomAccessConstIteratorInterface<_Ty>::difference_type x) { m_random_access_const_iterator += x; }
		typename TCommonRandomAccessIteratorInterface<_Ty>::difference_type operator-(const TCommonRandomAccessConstIteratorInterface<_Ty>& _Right_cref) const {
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
	class TAnyRandomAccessConstIteratorBase : public TRandomAccessConstIteratorStdBase<_Ty> {
	public:
		TAnyRandomAccessConstIteratorBase(const TAnyRandomAccessConstIteratorBase& src) : m_any_random_access_const_iterator(src.m_any_random_access_const_iterator) {}
		TAnyRandomAccessConstIteratorBase(const _Ty arr[]) : m_any_random_access_const_iterator(TCommonizedRandomAccessConstIterator<const _Ty, const _Ty*>(arr)) {}

		template <typename _TRandomAccessConstIterator1, class = typename std::enable_if<!std::is_convertible<_TRandomAccessConstIterator1, TAnyRandomAccessConstIteratorBase>::value, void>::type>
		TAnyRandomAccessConstIteratorBase(const _TRandomAccessConstIterator1& random_access_const_iterator) : m_any_random_access_const_iterator(TCommonizedRandomAccessConstIterator<const _Ty, _TRandomAccessConstIterator1>(random_access_const_iterator)) {}

		friend void swap(TAnyRandomAccessConstIteratorBase& first, TAnyRandomAccessConstIteratorBase& second) {
			std::swap(first.m_any_random_access_const_iterator, second.m_any_random_access_const_iterator);
		}

		const _Ty& operator*() const {
			return (*(*common_random_access_const_iterator_interface_ptr()));
		}
		const _Ty* operator->() const {
			return common_random_access_const_iterator_interface_ptr()->operator->();
		}
		typedef typename TCommonRandomAccessConstIteratorInterface<_Ty>::const_reference_t const_reference_t;
		typedef typename TCommonRandomAccessConstIteratorInterface<_Ty>::difference_type difference_type;
		const_reference_t operator[](difference_type _Off) const {
			return common_random_access_const_iterator_interface_ptr()->operator[](_Off);
		}
		void operator +=(difference_type x) { common_random_access_const_iterator_interface_ptr()->operator+=(x); };
		void operator -=(difference_type x) { operator +=(-x); }
		TAnyRandomAccessConstIteratorBase& operator ++() { operator +=(1); return (*this); }
		TAnyRandomAccessConstIteratorBase operator ++(int) { auto _Tmp = (*this); operator +=(1); return _Tmp; }
		TAnyRandomAccessConstIteratorBase& operator --() { operator -=(1); return (*this); }
		TAnyRandomAccessConstIteratorBase operator --(int) { auto _Tmp = (*this); operator -=(1); return _Tmp; }

		TAnyRandomAccessConstIteratorBase operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
		TAnyRandomAccessConstIteratorBase operator-(difference_type n) const { return ((*this) + (-n)); }
		difference_type operator-(const TAnyRandomAccessConstIteratorBase& _Right_cref) const {
			return (*common_random_access_const_iterator_interface_ptr()) - (*(_Right_cref.common_random_access_const_iterator_interface_ptr()));
		}
		bool operator==(const TAnyRandomAccessConstIteratorBase& _Right_cref) const { return (0 == operator-(_Right_cref)); }
		bool operator!=(const TAnyRandomAccessConstIteratorBase& _Right_cref) const { return !(operator==(_Right_cref)); }
		bool operator<(const TAnyRandomAccessConstIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TAnyRandomAccessConstIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TAnyRandomAccessConstIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TAnyRandomAccessConstIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TAnyRandomAccessConstIteratorBase& operator=(TAnyRandomAccessConstIteratorBase _Right) {
			swap(*this, _Right);
			return (*this);
		}

	protected:
		TAnyRandomAccessConstIteratorBase<_Ty>* operator&() { return this; }
		const TAnyRandomAccessConstIteratorBase<_Ty>* operator&() const { return this; }

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
	};

	template <typename _Ty>
	class TXScopeAnyRandomAccessConstIterator : public TAnyRandomAccessConstIteratorBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef TAnyRandomAccessConstIteratorBase<_Ty> base_class;

		MSE_USING(TXScopeAnyRandomAccessConstIterator, base_class);

		typedef typename base_class::difference_type difference_type;

		TXScopeAnyRandomAccessConstIterator& operator ++() { base_class::operator +=(1); return (*this); }
		TXScopeAnyRandomAccessConstIterator operator ++(int) { auto _Tmp = (*this); base_class::operator +=(1); return _Tmp; }
		TXScopeAnyRandomAccessConstIterator& operator --() { base_class::operator -=(1); return (*this); }
		TXScopeAnyRandomAccessConstIterator operator --(int) { auto _Tmp = (*this); base_class::operator -=(1); return _Tmp; }

		TXScopeAnyRandomAccessConstIterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
		TXScopeAnyRandomAccessConstIterator operator-(difference_type n) const { return ((*this) + (-n)); }
		difference_type operator-(const base_class& _Right_cref) const {
			return base_class::operator-(_Right_cref);
		}

		TXScopeAnyRandomAccessConstIterator& operator=(const base_class& _Right) {
			base_class::operator=(_Right);
			return (*this);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeAnyRandomAccessConstIterator<_Ty>* operator&() { return this; }
		const TXScopeAnyRandomAccessConstIterator<_Ty>* operator&() const { return this; }

		friend class TAnyRandomAccessConstIterator<_Ty>;
	};

	template<typename T>
	struct HasXScopeIteratorTagMethod_poly
	{
		template<typename U, void(U::*)() const> struct SFINAE {};
		template<typename U> static char Test(SFINAE<U, &U::xscope_iterator_tag>*);
		template<typename U> static int Test(...);
		static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
	};

	template<typename T>
	struct HasXScopeSSIteratorTypeTagMethod_poly
	{
		template<typename U, void(U::*)() const> struct SFINAE {};
		template<typename U> static char Test(SFINAE<U, &U::xscope_ss_iterator_type_tag>*);
		template<typename U> static int Test(...);
		static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
	};

	template <typename _Ty>
	class TAnyRandomAccessIterator : public TAnyRandomAccessIteratorBase<_Ty> {
	public:
		typedef TAnyRandomAccessIteratorBase<_Ty> base_class;
		typedef typename base_class::difference_type difference_type;

		TAnyRandomAccessIterator(const TAnyRandomAccessIterator& src) : base_class(src) {}
		TAnyRandomAccessIterator(_Ty arr[]) : base_class(arr) {}
		template <typename _TRandomAccessIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessIterator1, TAnyRandomAccessIterator>::value)
			&& (!std::is_base_of<TAnyRandomAccessConstIterator<_Ty>, _TRandomAccessIterator1>::value)
			//&& (!std::integral_constant<bool, HasXScopeIteratorTagMethod_poly<_TRandomAccessIterator1>::Has>())
			//&& (!std::integral_constant<bool, HasXScopeSSIteratorTypeTagMethod_poly<_TRandomAccessIterator1>::Has>())
			&& (!std::is_base_of<XScopeTagBase, _TRandomAccessIterator1>::value)
			, void>::type>
			TAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : base_class(random_access_iterator) {}

		TAnyRandomAccessIterator& operator ++() { base_class::operator ++(); return (*this); }
		TAnyRandomAccessIterator operator ++(int) { auto _Tmp = (*this); base_class::operator +=(1); return _Tmp; }
		TAnyRandomAccessIterator& operator --() { base_class::operator --(); return (*this); }
		TAnyRandomAccessIterator operator --(int) { auto _Tmp = (*this); base_class::operator -=(1); return _Tmp; }

		TAnyRandomAccessIterator operator+(difference_type n) const { return base_class::operator+(n); }
		TAnyRandomAccessIterator operator-(difference_type n) const { return base_class::operator-(n); }
		difference_type operator-(const base_class& _Right_cref) const { return base_class::operator-(_Right_cref); }

		TAnyRandomAccessIterator& operator=(TAnyRandomAccessIterator _Right) { base_class::operator=(_Right); return (*this); }

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TAnyRandomAccessIterator<_Ty>* operator&() { return this; }
		const TAnyRandomAccessIterator<_Ty>* operator&() const { return this; }
	};

	template <typename _Ty>
	class TAnyRandomAccessConstIterator : public TAnyRandomAccessConstIteratorBase<_Ty> {
	public:
		typedef TAnyRandomAccessConstIteratorBase<_Ty> base_class;
		typedef typename base_class::difference_type difference_type;

		TAnyRandomAccessConstIterator(const TAnyRandomAccessConstIterator& src) : base_class(src) {}
		TAnyRandomAccessConstIterator(const TAnyRandomAccessIterator<_Ty>& src) : base_class(src) {}
		TAnyRandomAccessConstIterator(const _Ty arr[]) : base_class(arr) {}

		template <typename _TRandomAccessConstIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessConstIterator1, TAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_base_of<TAnyRandomAccessIterator<_Ty>, _TRandomAccessConstIterator1>::value)
			//&& (!std::integral_constant<bool, HasXScopeIteratorTagMethod_poly<_TRandomAccessConstIterator1>::Has>())
			//&& (!std::integral_constant<bool, HasXScopeSSIteratorTypeTagMethod_poly<_TRandomAccessConstIterator1>::Has>())
			&& (!std::is_base_of<XScopeTagBase, _TRandomAccessConstIterator1>::value)
			, void>::type>
		TAnyRandomAccessConstIterator(const _TRandomAccessConstIterator1& random_access_const_iterator) : base_class(random_access_const_iterator) {}

		TAnyRandomAccessConstIterator& operator ++() { base_class::operator ++(); return (*this); }
		TAnyRandomAccessConstIterator operator ++(int) { auto _Tmp = (*this); base_class::operator +=(1); return _Tmp; }
		TAnyRandomAccessConstIterator& operator --() { base_class::operator --(); return (*this); }
		TAnyRandomAccessConstIterator operator --(int) { auto _Tmp = (*this); base_class::operator -=(1); return _Tmp; }

		TAnyRandomAccessConstIterator operator+(difference_type n) const { return base_class::operator+(n); }
		TAnyRandomAccessConstIterator operator-(difference_type n) const { return base_class::operator-(n); }
		difference_type operator-(const base_class& _Right_cref) const { return base_class::operator-(_Right_cref); }

		TAnyRandomAccessConstIterator& operator=(const TAnyRandomAccessConstIterator& _Right) { base_class::operator=(_Right); return (*this); }

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TAnyRandomAccessConstIterator<_Ty>* operator&() { return this; }
		const TAnyRandomAccessConstIterator<_Ty>* operator&() const { return this; }
	};


	template <typename _Ty>
	class TXScopeAnyRandomAccessSection : public TXScopeRandomAccessSection<TXScopeAnyRandomAccessIterator<_Ty>> {
	public:
		typedef TXScopeRandomAccessSection<TXScopeAnyRandomAccessIterator<_Ty>> base_class;
		MSE_USING(TXScopeAnyRandomAccessSection, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	template <typename _Ty>
	class TAnyRandomAccessSection : public TRandomAccessSection<TAnyRandomAccessIterator<_Ty>> {
	public:
		typedef TRandomAccessSection<TAnyRandomAccessIterator<_Ty>> base_class;
		MSE_USING(TAnyRandomAccessSection, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	template <typename _Ty>
	class TXScopeAnyRandomAccessConstSection : public TXScopeRandomAccessConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>> {
	public:
		typedef TXScopeRandomAccessConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>> base_class;
		MSE_USING(TXScopeAnyRandomAccessConstSection, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	template <typename _Ty>
	class TAnyRandomAccessConstSection : public TRandomAccessConstSection<TAnyRandomAccessConstIterator<_Ty>> {
	public:
		typedef TRandomAccessConstSection<TAnyRandomAccessConstIterator<_Ty>> base_class;
		MSE_USING(TAnyRandomAccessConstSection, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	namespace us {
		template<typename _Ty>
		class TFParam<mse::TXScopeAnyRandomAccessConstSection<_Ty> > : public TXScopeAnyRandomAccessConstSection<_Ty> {
		public:
			typedef TXScopeAnyRandomAccessConstSection<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				typename std::conditional<
				mse::is_instantiation_of_msescope<_TRALoneParam, mse::TXScopeCagedItemFixedConstPointerToRValue>::value
				|| mse::is_instantiation_of_msescope<_TRALoneParam, mse::TXScopeCagedRandomAccessConstSectionToRValue>::value
				, std::true_type, std::false_type>::type()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeItemFixedConstPointerFParam<_Ty2>(std::forward<decltype(param)>(param));
			}
			template<typename _TRAIterator>
			auto constructor_helper1(std::true_type, TXScopeCagedRandomAccessConstSectionToRValue<_TRAIterator>&& param) {
				return TXScopeRandomAccessConstSectionFParam<_TRAIterator>(std::forward<decltype(param)>(param));
			}
			template<typename _TRALoneParam>
			auto constructor_helper1(std::false_type, _TRALoneParam&& param) { return std::forward<_TRALoneParam>(param); }

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}


	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	using TXScopeAnyStringSection = TXScopeStringSection<TXScopeAnyRandomAccessIterator<_Ty>, _Traits>;

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	using TAnyStringSection = TStringSection<TAnyRandomAccessIterator<_Ty>, _Traits>;

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TXScopeAnyStringConstSection : public TXScopeStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TXScopeStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::const_reference const_reference;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		static const size_t npos = size_t(-1);
		typedef typename std::remove_const<value_type>::type nonconst_value_type;

		MSE_USING(TXScopeAnyStringConstSection, base_class);
		TXScopeAnyStringConstSection() : base_class(&s_default_string_ref()) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		static auto& s_default_string_ref() { static /*const*/ mse::nii_basic_string<nonconst_value_type, _Traits> s_default_string; return s_default_string; }
	};

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TAnyStringConstSection : public TStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::const_reference const_reference;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		static const size_t npos = size_t(-1);
		typedef typename std::remove_const<value_type>::type nonconst_value_type;

		MSE_USING(TAnyStringConstSection, base_class);
		TAnyStringConstSection() : base_class(&s_default_string_ref()) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		static auto& s_default_string_ref() { static /*const*/ mse::nii_basic_string<nonconst_value_type, _Traits> s_default_string; return s_default_string; }
	};

	namespace us {
		template<typename _Ty>
		class TFParam<mse::TXScopeAnyStringConstSection<_Ty> > : public TXScopeAnyStringConstSection<_Ty> {
		public:
			typedef TXScopeAnyStringConstSection<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				typename std::conditional<
				mse::is_instantiation_of_msescope<_TRALoneParam, mse::TXScopeCagedItemFixedConstPointerToRValue>::value
				|| std::is_base_of<CagedStringSectionTagBase, _TRALoneParam>::value
				//|| mse::is_instantiation_of_msescope<_TRALoneParam, mse::TXScopeCagedStringConstSectionToRValue>::value
				, std::true_type, std::false_type>::type()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeItemFixedConstPointerFParam<_Ty2>(std::forward<decltype(param)>(param));
			}
			template<typename _TRAIterator>
			auto constructor_helper1(std::true_type, TXScopeCagedStringConstSectionToRValue<_TRAIterator>&& param) {
				return TXScopeStringConstSectionFParam<_TRAIterator>(std::forward<decltype(param)>(param));
			}
			template<typename _TRALoneParam>
			auto constructor_helper1(std::false_type, _TRALoneParam&& param) { return std::forward<_TRALoneParam>(param); }

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}


	template<typename _Ty, typename _TRALoneParam, class = typename std::enable_if<
		(!std::is_same<std::basic_string<_Ty>, typename std::remove_const<_TRALoneParam>::type>::value), void>::type>
	void T_valid_if_not_an_std_basic_string_msepoly() {}

	template<typename _Ty, typename _TPtr>
	void T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly_helper(std::true_type) {
		T_valid_if_not_an_std_basic_string_msepoly<_Ty, typename std::remove_reference<decltype(*std::declval<_TPtr>())>::type>();
	}
	template<typename _Ty, typename _TRALoneParam>
	void T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly_helper(std::false_type) {}

	template<typename _Ty, typename _TRALoneParam>
	void T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly() {
		T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly_helper<_Ty, _TRALoneParam>(typename IsDereferenceable_msemsearray<_TRALoneParam>::type());
	}

	template<typename _Ty, typename _TRALoneParam>
	void T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly() {
#if !defined(MSE_SCOPEPOINTER_DISABLED) && !defined(MSE_REGISTEREDPOINTER_DISABLED)
		T_valid_if_not_a_native_pointer_msemsestring<_TRALoneParam>();
#endif /*!defined(MSE_SCOPEPOINTER_DISABLED) && !defined(MSE_REGISTEREDPOINTER_DISABLED)*/
#ifndef MSE_MSTDSTRING_DISABLED
		T_valid_if_not_an_std_basic_string_msepoly<_Ty, _TRALoneParam>();
		//T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly<_Ty, _TRALoneParam>();
#endif /*!MSE_MSTDSTRING_DISABLED*/
	}

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TXScopeAnyNRPStringSection : public TXScopeNRPStringSection<TXScopeAnyRandomAccessIterator<_Ty>, _Traits> {
	public:
		typedef TXScopeNRPStringSection<TXScopeAnyRandomAccessIterator<_Ty>, _Traits> base_class;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::reference reference;
		typedef typename base_class::const_reference const_reference;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		static const size_t npos = size_t(-1);
		typedef typename std::remove_const<value_type>::type nonconst_value_type;

		//MSE_USING(TXScopeAnyNRPStringSection, base_class);
		TXScopeAnyNRPStringSection(const TXScopeAnyNRPStringSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TXScopeAnyNRPStringSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TXScopeAnyNRPStringSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
			/* Note: Use TXScopeAnyNRPStringConstSection instead if referencing a string literal. */
			T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
		}
		template <typename _TRALoneParam>
		TXScopeAnyNRPStringSection(const _TRALoneParam& param) : base_class(param) {
			T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TAnyNRPStringSection : public TNRPStringSection<TAnyRandomAccessIterator<_Ty>, _Traits> {
	public:
		typedef TNRPStringSection<TAnyRandomAccessIterator<_Ty>, _Traits> base_class;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::reference reference;
		typedef typename base_class::const_reference const_reference;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		static const size_t npos = size_t(-1);
		typedef typename std::remove_const<value_type>::type nonconst_value_type;

		//MSE_USING(TAnyNRPStringSection, base_class);
		TAnyNRPStringSection(const TAnyNRPStringSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TAnyNRPStringSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TAnyNRPStringSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
			/* Note: Use TXScopeAnyNRPStringConstSection instead if referencing a string literal. */
			T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
		}
		template <typename _TRALoneParam>
		TAnyNRPStringSection(const _TRALoneParam& param) : base_class(param) {
			T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TXScopeAnyNRPStringConstSection : public TXScopeNRPStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TXScopeNRPStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::const_reference const_reference;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		static const size_t npos = size_t(-1);
		typedef typename std::remove_const<value_type>::type nonconst_value_type;

		//MSE_USING(TXScopeAnyNRPStringConstSection, base_class);
		TXScopeAnyNRPStringConstSection(const TXScopeAnyNRPStringConstSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TXScopeAnyNRPStringConstSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TXScopeAnyNRPStringConstSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
			T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
		}
		template <typename _TRALoneParam>
		TXScopeAnyNRPStringConstSection(const _TRALoneParam& param) : base_class(param) {
			T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}
		TXScopeAnyNRPStringConstSection() : base_class(&s_default_string_ref()) {}

		template<size_t Tn, typename = typename std::enable_if<1 <= Tn>::type>
		explicit TXScopeAnyNRPStringConstSection(const value_type(&presumed_string_literal)[Tn]) : base_class(presumed_string_literal) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		/* Construction from a const native array is publicly supported (only) because string literals are const
		native arrays. We do not want construction from a non-const native array to be publicly supported. */
		template<size_t Tn>
		explicit TXScopeAnyNRPStringConstSection(typename std::remove_const<value_type>::type(&native_array)[Tn]) : base_class(native_array) {}

		static auto& s_default_string_ref() { static /*const*/ mse::nii_basic_string<nonconst_value_type, _Traits> s_default_string; return s_default_string; }
	};

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TAnyNRPStringConstSection : public TNRPStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TNRPStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::const_reference const_reference;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		static const size_t npos = size_t(-1);
		typedef typename std::remove_const<value_type>::type nonconst_value_type;

		//MSE_USING(TAnyNRPStringConstSection, base_class);
		TAnyNRPStringConstSection(const TAnyNRPStringConstSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TAnyNRPStringConstSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TAnyNRPStringConstSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
			T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
		}
		template <typename _TRALoneParam>
		TAnyNRPStringConstSection(const _TRALoneParam& param) : base_class(param) {
			T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}
		TAnyNRPStringConstSection() : base_class(&s_default_string_ref()) {}

		template<size_t Tn, typename = typename std::enable_if<1 <= Tn>::type>
		explicit TAnyNRPStringConstSection(const value_type(&presumed_string_literal)[Tn]) : base_class(presumed_string_literal) {}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		/* Construction from a const native array is publicly supported (only) because string literals are const
		native arrays. We do not want construction from a non-const native array to be publicly supported. */
		template<size_t Tn>
		explicit TAnyNRPStringConstSection(typename std::remove_const<value_type>::type(&native_array)[Tn]) : base_class(native_array) {}

		static auto& s_default_string_ref() { static /*const*/ mse::nii_basic_string<nonconst_value_type, _Traits> s_default_string; return s_default_string; }
	};
}

namespace std {

	template <typename _Ty, class _Traits>
	struct hash<mse::TXScopeAnyNRPStringSection<_Ty, _Traits> > : public hash<mse::TXScopeNRPStringSection<mse::TXScopeAnyRandomAccessIterator<_Ty>, _Traits> > {
	typedef hash<mse::TXScopeNRPStringSection<mse::TXScopeAnyRandomAccessIterator<_Ty>, _Traits> > base_class;
	//typedef typename base_class::argument_type argument_type;
	//typedef typename base_class::result_type result_type;
	};

	template <typename _Ty, class _Traits>
	struct hash<mse::TAnyNRPStringSection<_Ty, _Traits> > : public hash<mse::TNRPStringSection<mse::TAnyRandomAccessIterator<_Ty>, _Traits> > {
	typedef hash<mse::TAnyNRPStringSection<mse::TAnyRandomAccessIterator<_Ty>, _Traits> > base_class;
	//typedef typename base_class::argument_type argument_type;
	//typedef typename base_class::result_type result_type;
	};

	template <typename _Ty, class _Traits>
	struct hash<mse::TXScopeAnyNRPStringConstSection<_Ty, _Traits> > : public hash<mse::TXScopeNRPStringConstSection<mse::TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> > {
	typedef hash<mse::TXScopeNRPStringConstSection<mse::TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> > base_class;
	//typedef typename base_class::argument_type argument_type;
	//typedef typename base_class::result_type result_type;
	};

	template <typename _Ty, class _Traits>
	struct hash<mse::TAnyNRPStringConstSection<_Ty, _Traits> > : public hash<mse::TNRPStringConstSection<mse::TAnyRandomAccessConstIterator<_Ty>, _Traits> > {
	typedef hash<mse::TNRPStringConstSection<mse::TAnyRandomAccessConstIterator<_Ty>, _Traits> > base_class;
	//typedef typename base_class::argument_type argument_type;
	//typedef typename base_class::result_type result_type;
	};
}

namespace mse {

	namespace mstd {
		template <typename _Ty, class _Traits = std::char_traits<_Ty> >
		class basic_string_view : public TAnyStringConstSection<_Ty, _Traits> {
		public:
			typedef TAnyStringConstSection<_Ty, _Traits> base_class;
			typedef typename base_class::value_type value_type;
			typedef typename base_class::const_reference const_reference;
			typedef typename base_class::size_type size_type;
			typedef typename base_class::difference_type difference_type;
			static const size_t npos = size_t(-1);

			MSE_USING(basic_string_view, base_class);
			explicit basic_string_view(const char* sz) : base_class(sz, _Traits::length(sz)) {}
		};

		template <typename _Ty, class _Traits = std::char_traits<_Ty> >
		class xscope_basic_string_view : public TXScopeAnyStringConstSection<_Ty, _Traits> {
		public:
			typedef TXScopeAnyStringConstSection<_Ty, _Traits> base_class;
			typedef typename base_class::value_type value_type;
			typedef typename base_class::const_reference const_reference;
			typedef typename base_class::size_type size_type;
			typedef typename base_class::difference_type difference_type;
			static const size_t npos = size_t(-1);

			MSE_USING(xscope_basic_string_view, base_class);
			explicit xscope_basic_string_view(const char* sz) : base_class(sz, _Traits::length(sz)) {}
		};

		typedef basic_string_view<char>     string_view;
		typedef basic_string_view<char16_t> u16string_view;
		typedef basic_string_view<char32_t> u32string_view;
		typedef basic_string_view<wchar_t>  wstring_view;
		typedef xscope_basic_string_view<char>     xscope_string_view;
		typedef xscope_basic_string_view<char16_t> xscope_u16string_view;
		typedef xscope_basic_string_view<char32_t> xscope_u32string_view;
		typedef xscope_basic_string_view<wchar_t>  xscope_wstring_view;
	}

	template <typename _Ty, class _Traits = std::char_traits<_Ty> >
	using nrp_basic_string_view = TAnyNRPStringConstSection<_Ty, _Traits>;

	template <typename _Ty, class _Traits = std::char_traits<_Ty> >
	using xscope_nrp_basic_string_view = TXScopeAnyNRPStringConstSection<_Ty, _Traits>;

	typedef nrp_basic_string_view<char>     nrp_string_view;
	typedef nrp_basic_string_view<char16_t> nrp_u16string_view;
	typedef nrp_basic_string_view<char32_t> nrp_u32string_view;
	typedef nrp_basic_string_view<wchar_t>  nrp_wstring_view;
	typedef xscope_nrp_basic_string_view<char>     xscope_nrp_string_view;
	typedef xscope_nrp_basic_string_view<char16_t> xscope_nrp_u16string_view;
	typedef xscope_nrp_basic_string_view<char32_t> xscope_nrp_u32string_view;
	typedef xscope_nrp_basic_string_view<wchar_t>  xscope_nrp_wstring_view;


	template <typename _Ty>
	class TNullableAnyRandomAccessIterator : public TAnyRandomAccessIterator<_Ty> {
	public:
		TNullableAnyRandomAccessIterator() : TAnyRandomAccessIterator<_Ty>(typename mse::mstd::vector<typename std::remove_const<_Ty>::type>::iterator()), m_is_null(true) {}
		TNullableAnyRandomAccessIterator(const std::nullptr_t& src) : TNullableAnyRandomAccessIterator() {}
		TNullableAnyRandomAccessIterator(const TNullableAnyRandomAccessIterator& src) : TAnyRandomAccessIterator<_Ty>(src) {}
		TNullableAnyRandomAccessIterator(const TAnyRandomAccessIterator<_Ty>& src) : TAnyRandomAccessIterator<_Ty>(src) {}
		explicit TNullableAnyRandomAccessIterator(_Ty arr[]) : TAnyRandomAccessIterator<_Ty>(arr) {}

		template <typename _TRandomAccessIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessIterator1, TNullableAnyRandomAccessIterator>::value)
			&& (!std::is_base_of<TAnyRandomAccessIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, int>::value)
			//&& (!std::integral_constant<bool, HasXScopeIteratorTagMethod_poly<_TRandomAccessIterator1>::Has>())
			//&& (!std::integral_constant<bool, HasXScopeSSIteratorTypeTagMethod_poly<_TRandomAccessIterator1>::Has>())
			&& (!std::is_base_of<XScopeTagBase, _TRandomAccessIterator1>::value)
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

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TNullableAnyRandomAccessIterator<_Ty>* operator&() { return this; }
		const TNullableAnyRandomAccessIterator<_Ty>* operator&() const { return this; }
		bool m_is_null = false;
	};

	/* The intended semantics of TNullableAnyPointer<> is that it always contains either an std::nullptr_t or a
	valid pointer (or iterator) to a valid object. TNullableAnyPointer<> is primarily designed for compatibility
	with legacy code. For other use cases you might prefer optional<TAnyPointer<> > instead. */
	template <typename _Ty>
	class TNullableAnyPointer : public TAnyPointer<_Ty> {
	public:
		typedef TAnyPointer<_Ty> base_class;
		TNullableAnyPointer() : base_class(mse::TRegisteredPointer<_Ty>()), m_is_null(true) {}
		TNullableAnyPointer(const std::nullptr_t& src) : TNullableAnyPointer() {}
		TNullableAnyPointer(const TNullableAnyPointer& src) : base_class(src) {}
		TNullableAnyPointer(const base_class& src) : base_class(src) {}

		template <typename _TRandomAccessIterator1, class = typename std::enable_if<
			(!std::is_convertible<_TRandomAccessIterator1, TNullableAnyPointer>::value)
			&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, int>::value)
			//&& (!std::integral_constant<bool, HasXScopeIteratorTagMethod_poly<_TRandomAccessIterator1>::Has>())
			//&& (!std::integral_constant<bool, HasXScopeSSIteratorTypeTagMethod_poly<_TRandomAccessIterator1>::Has>())
			&& (!std::is_base_of<XScopeTagBase, _TRandomAccessIterator1>::value)
			, void>::type>
			TNullableAnyPointer(const _TRandomAccessIterator1& random_access_iterator) : base_class(random_access_iterator) {}

		friend void swap(TNullableAnyPointer& first, TNullableAnyPointer& second) {
			std::swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
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

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TNullableAnyPointer<_Ty>* operator&() { return this; }
		const TNullableAnyPointer<_Ty>* operator&() const { return this; }
		bool m_is_null = false;
	};
}

namespace std {
	template<class _Ty>
	struct hash<mse::TNullableAnyPointer<_Ty> > {	// hash functor
		typedef mse::TNullableAnyPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNullableAnyPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

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
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

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
#ifndef MSE_SCOPEPOINTER_DISABLED
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopePolyConstPointer<_Ty>(src_cref) {}
#endif // !MSE_SCOPEPOINTER_DISABLED
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopePolyConstPointer<_Ty>(TXScopeFixedPointer<_Ty>(src_cref)) {}
		virtual ~TRefCountingOrXScopeFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::addressof((*this).operator*()); }
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

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
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	template<typename _Ty>
	class TRefCountingOrXScopeOrRawFixedConstPointer : public TRefCountingOrXScopeFixedConstPointer<_Ty> {
	public:
		MSE_SCOPE_USING(TRefCountingOrXScopeOrRawFixedConstPointer, TRefCountingOrXScopeFixedConstPointer<_Ty>);
		TRefCountingOrXScopeOrRawFixedConstPointer(_Ty* ptr) : TRefCountingOrXScopeFixedConstPointer<_Ty>(ptr) {}
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
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
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

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
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	private:
		TSharedOrRawFixedConstPointer<_Ty>& operator=(const TSharedOrRawFixedConstPointer<_Ty>& _Right_cref) = delete;

		//TSharedOrRawFixedConstPointer<_Ty>* operator&() { return this; }
		//const TSharedOrRawFixedConstPointer<_Ty>* operator&() const { return this; }
	};




#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-braces"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class CPolyPtrTest1 {
	public:
		static void s_test1() {
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
				mse::us::msevector<A> a_msevec;
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

#endif // MSEPOLY_H_
