
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
#include "msecregistered.h"
#include "msenorad.h"
#include "mserefcounting.h"
#include "msescope.h"
#include "msemstdvector.h"
#include "mseasyncshared.h"
#include "mseany.h"
#include "msemstdarray.h"
#include "msemsestring.h"
#include "msemstdstring.h"
#include "msestaticimmutable.h"
#include "mseoptional.h"

#include <memory>
#include <utility>
#include <cassert>

#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <new>

/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_HAS_CXX20
/* for iterator debug values */
#include <span>
#endif // MSE_HAS_CXX20


#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_POLYPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4503 4996 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif /*__GNUC__*/
#endif /*__clang__*/

namespace mse {

#ifdef MSE_POLYPOINTER_DISABLED
#else /*MSE_POLYPOINTER_DISABLED*/
#endif /*MSE_POLYPOINTER_DISABLED*/

	namespace impl {
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
#ifdef MSE_TDP_VARIANT_REQUIRE_MOVABILITY
#define MSE_TDP_VARIANT_STD_MOVE(x) std::move(x)
#define MSE_TDP_VARIANT_STD_FORWARD(X) std::forward<X>
#else // MSE_TDP_VARIANT_REQUIRE_MOVABILITY
#define MSE_TDP_VARIANT_STD_MOVE(x) (x)
#define MSE_TDP_VARIANT_STD_FORWARD(X)
#endif // MSE_TDP_VARIANT_REQUIRE_MOVABILITY

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
					::new (new_v) F(MSE_TDP_VARIANT_STD_MOVE(*reinterpret_cast<F*>(old_v)));
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

			template<typename T>
			struct is_supported_type : public mse::impl::disjunction<
					std::is_same<F, T>
					, typename tdp_variant_helper<Ts...>::template is_supported_type<T>
				>::type {};
		};

		template<> struct tdp_variant_helper<> {
			inline static void destroy(std::type_index id, void * data) { }
			inline static void move(std::type_index old_t, void * old_v, void * new_v) { }
			inline static void copy(std::type_index old_t, const void * old_v, void * new_v) { }

			template<typename T> struct is_supported_type : public std::is_same<void, T> {};
		};
	}

	template<typename... Ts>
	struct tdp_variant {
	protected:
		static const size_t data_size = impl::static_max<sizeof(Ts)...>::value;
		static const size_t data_align = impl::static_max<alignof(Ts)...>::value;

		using data_t = typename std::aligned_storage<data_size, data_align>::type;

		using helper_t = impl::tdp_variant_helper<Ts...>;

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

#ifdef MSE_MSE_TDP_VARIANT_ASSIGNMENT_OPERATOR_USE_NON_TYPESAFE_SWAP
		// Serves as both the move and the copy asignment operator.
		tdp_variant<Ts...>& operator= (tdp_variant<Ts...> old)
		{
			std::swap(type_id, old.type_id);
			std::swap(data, old.data);

			return *this;
		}
#else // MSE_MSE_TDP_VARIANT_ASSIGNMENT_OPERATOR_USE_NON_TYPESAFE_SWAP
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
#endif // MSE_MSE_TDP_VARIANT_ASSIGNMENT_OPERATOR_USE_NON_TYPESAFE_SWAP

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
			static_assert(helper_t::template is_supported_type<T>::value, "type is not a member type of the tdp_variant");
			// First we destroy the current contents    
			auto held_type_id = type_id;
			type_id = invalid_type();
			helper_t::destroy(held_type_id, &data);
			::new (&data) T(MSE_TDP_VARIANT_STD_FORWARD(Args) (args)...);
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

	namespace impl {
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
					return mse::impl::operator_bool_helper1<F>(typename std::is_convertible<F, bool>::type(), *(reinterpret_cast<const F*>(data)));
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
	}

	template<typename... Ts>
	struct tdp_pointer_variant : public tdp_variant<Ts...> {
	protected:
		using pointer_helper_t = impl::tdp_pointer_variant_helper<Ts...>;
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

	template <typename _Ty> class TXScopeAnyPointer;
	template <typename _Ty> class TAnyPointer;
	template <typename _Ty> class TXScopeAnyConstPointer;
	template <typename _Ty> class TAnyConstPointer;

	template <typename _Ty>
	class TXScopeAnyPointer : public us::impl::TAnyPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef us::impl::TAnyPointerBase<_Ty> base_class;
		TXScopeAnyPointer(const us::impl::TAnyPointerBase<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, us::impl::TAnyPointerBase<_Ty>>::value)
			&& (!std::is_base_of<us::impl::TAnyConstPointerBase<_Ty>, _TPointer1>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
			> MSE_IMPL_EIS >
			TXScopeAnyPointer(const _TPointer1& pointer) : base_class(pointer) {}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TXScopeAnyPointer);

		void async_not_shareable_and_not_passable_tag() const {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		TXScopeAnyPointer<_Ty>& operator=(const TXScopeAnyPointer<_Ty>& _Right_cref) {
			base_class::operator=(static_cast<const base_class&>(_Right_cref));
			return (*this);
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TXScopeAnyPointer<_Ty> >;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TXScopeAnyPointer);

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_any_pointer(const _Ty& x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_pointer(_Ty&& x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyPointer<_Tx2>(MSE_FWD(x));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_pointer)

	/* The intended semantics of "any" pointers is that they always contain a valid pointer (or iterator) to a valid
	object. If you need a "null" state, consider using optional<> (or mse::TNullableAnyPointer<>). */
	template <typename _Ty>
	class TAnyPointer : public us::impl::TAnyPointerBase<_Ty> {
	public:
		typedef us::impl::TAnyPointerBase<_Ty> base_class;
		TAnyPointer(const TAnyPointer& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, TAnyPointer>::value)
			&& (!std::is_base_of<TAnyConstPointer<_Ty>, _TPointer1>::value)
			&& (!std::is_base_of<TAnyConstPointer<_Ty>, _TPointer1>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
			&& mse::impl::is_potentially_not_xscope<_TPointer1>::value
		> MSE_IMPL_EIS >
		TAnyPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)") : base_class(pointer) {
			mse::impl::T_valid_if_not_an_xscope_type<_TPointer1>();
		}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TAnyPointer);

		TAnyPointer<_Ty>& operator=(const TAnyPointer<_Ty>& _Right_cref) {
			base_class::operator=(static_cast<const base_class&>(_Right_cref));
			return (*this);
		}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TAnyPointer<_Ty> >;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TAnyPointer);

	template <typename _Tx = void, typename _Ty = void>
	auto make_any_pointer(const _Ty & x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_any_pointer(_Ty && x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyPointer<_Tx2>(MSE_FWD(x));
	}

	template <typename _Ty>
	class TXScopeAnyConstPointer : public us::impl::TAnyConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef us::impl::TAnyConstPointerBase<_Ty> base_class;
		TXScopeAnyConstPointer(const us::impl::TAnyConstPointerBase<_Ty>& src) : base_class(src) {}
		/*
		TXScopeAnyConstPointer(const us::impl::TAnyPointerBase<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, us::impl::TAnyConstPointerBase<_Ty>>::value)
			&& (!std::is_convertible<_TPointer1, us::impl::TAnyPointerBase<_Ty>>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
			> MSE_IMPL_EIS >
		TXScopeAnyConstPointer(const _TPointer1& pointer) : base_class(pointer) {}
		*/

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TXScopeAnyConstPointer);

		void async_not_shareable_and_not_passable_tag() const {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		TXScopeAnyConstPointer<_Ty>& operator=(const TXScopeAnyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(static_cast<const base_class&>(_Right_cref));
			return (*this);
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TXScopeAnyConstPointer<_Ty> >;
	};


	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_any_const_pointer(const _Ty & x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyConstPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_const_pointer(_Ty && x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyConstPointer<_Tx2>(MSE_FWD(x));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_const_pointer)

	template <typename _Ty>
	class TAnyConstPointer : public us::impl::TAnyConstPointerBase<_Ty> {
	public:
		typedef us::impl::TAnyConstPointerBase<_Ty> base_class;
		TAnyConstPointer(const TAnyConstPointer& src) : base_class(src) {}
		TAnyConstPointer(const TAnyPointer<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, TAnyConstPointer>::value)
			&& (!std::is_convertible<_TPointer1, TAnyPointer<_Ty>>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
			&& (mse::impl::is_potentially_not_xscope<_TPointer1>::value)
			> MSE_IMPL_EIS >
		TAnyConstPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)") : base_class(pointer) {
			mse::impl::T_valid_if_not_an_xscope_type<_TPointer1>();
		}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TAnyConstPointer);

		TAnyConstPointer<_Ty>& operator=(const TAnyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(static_cast<const base_class&>(_Right_cref));
			return (*this);
		}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TAnyConstPointer<_Ty> >;
	};

	template <typename _Tx = void, typename _Ty = void>
	auto make_any_const_pointer(const _Ty & x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyConstPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_any_const_pointer(_Ty && x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyConstPointer<_Tx2>(MSE_FWD(x));
	}

	namespace us {
		template<typename _Ty>
		TAnyPointer<_Ty> unsafe_make_any_pointer_to(_Ty& ref) {
			return make_any_pointer(mse::us::TSaferPtrForLegacy<_Ty>(std::addressof(ref)));
		}
		template<typename _Ty>
		TAnyConstPointer<_Ty> unsafe_make_any_const_pointer_to(const _Ty& cref) {
			return make_any_const_pointer(mse::us::TSaferPtrForLegacy<_Ty>(std::addressof(cref)));
		}
	}

	namespace rsv {
		template<typename _Ty>
		class TFParam<mse::TXScopeAnyConstPointer<_Ty> > : public TXScopeAnyConstPointer<_Ty> {
		public:
			typedef TXScopeAnyConstPointer<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				typename mse::rsv::impl::is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue<_TRALoneParam>::type()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TFParam);

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeFixedConstPointerFParam<_Ty2>(MSE_FWD(param));
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

	namespace impl {
		template<typename _Ty> class TPolyPointerID {};
	}

	template <typename _Ty> class TXScopePolyPointer;
	template <typename _Ty> class TPolyPointer;
	template <typename _Ty> class TXScopePolyConstPointer;
	template <typename _Ty> class TPolyConstPointer;

	namespace us {
		namespace impl {

			template <typename _Ty> class TPolyConstPointerBase;

			template<typename _Ty>
			class TPolyPointerBase {
			public:
				typedef TPolyPointerBase _Myt;
				template<typename _Ty2> using writelock_ptr_t = decltype(std::declval<mse::TAsyncSharedV2ReadWriteAccessRequester<_Ty2> >().writelock_ptr());

				using poly_variant = tdp_pointer_variant <
#if !defined(MSE_SCOPEPOINTER_DISABLED)
					mse::TXScopeObjFixedPointer<_Ty>,
					mse::TXScopeFixedPointer<_Ty>,
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
					mse::TRegisteredPointer<_Ty>,
					mse::TCRegisteredPointer<_Ty>,
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_NORADPOINTER_DISABLED)
					mse::TNoradPointer<_Ty>,
#endif // !defined(MSE_NORADPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
					mse::TRefCountingPointer<_Ty>,
					mse::TRefCountingNotNullPointer<_Ty>,
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

					mse::us::impl::TPointer<_Ty, mse::impl::TPolyPointerID<const _Ty>>,

					writelock_ptr_t<_Ty>
				> ;

				TPolyPointerBase(const us::impl::TPolyPointerBase<_Ty>& p) : m_pointer(p.m_pointer) {}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
				TPolyPointerBase(const mse::TXScopeObjFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeObjFixedPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const mse::TXScopeObjFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeObjFixedPointer<_Ty>>(p); }
				TPolyPointerBase(const mse::TXScopeFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const mse::TXScopeFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedPointer<_Ty>>(p); }
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
				TPolyPointerBase(const mse::TRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<
					std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value || std::is_same<const _Ty2, _Ty>::value
					> MSE_IMPL_EIS >
					TPolyPointerBase(const mse::TRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredPointer<_Ty>>(p); }

				TPolyPointerBase(const mse::TCRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TCRegisteredPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const mse::TCRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TCRegisteredPointer<_Ty>>(p); }
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_NORADPOINTER_DISABLED)
				TPolyPointerBase(const mse::TNoradPointer<_Ty>& p) { m_pointer.template set<mse::TNoradPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const mse::TNoradPointer<_Ty2>& p) { m_pointer.template set<mse::TNoradPointer<_Ty>>(p); }
#endif // !defined(MSE_NORADPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
				TPolyPointerBase(const mse::TRefCountingPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const mse::TRefCountingPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingPointer<_Ty>>(p); }
				TPolyPointerBase(const mse::TRefCountingNotNullPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingNotNullPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const mse::TRefCountingNotNullPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingNotNullPointer<_Ty>>(p); }
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
				TPolyPointerBase(const typename mse::mstd::vector<_Ty>::iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::iterator>(p); }
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
				TPolyPointerBase(const typename mse::us::msevector<_Ty>::iterator& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::iterator>(p); }
				TPolyPointerBase(const typename mse::us::msevector<_Ty>::ipointer& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ipointer>(p); }
				TPolyPointerBase(const typename mse::us::msevector<_Ty>::ss_iterator_type& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ss_iterator_type>(p); }
				TPolyPointerBase(const std::shared_ptr<_Ty>& p) { m_pointer.template set<std::shared_ptr<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyPointerBase(const std::shared_ptr<_Ty2>& p) { m_pointer.template set<std::shared_ptr<_Ty>>(p); }
				TPolyPointerBase(const mse::TXScopeAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyPointer<_Ty>>(p); }
				TPolyPointerBase(const mse::TAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyPointer<_Ty>>(p); }

				TPolyPointerBase(const mse::us::impl::TPointer<_Ty>& p) { m_pointer.template set<mse::us::impl::TPointer<_Ty, mse::impl::TPolyPointerID<const _Ty>>>(p); }
				TPolyPointerBase(_Ty* p) { m_pointer.template set<mse::us::impl::TPointer<_Ty, mse::impl::TPolyPointerID<const _Ty>>>(p); }

				TPolyPointerBase(const writelock_ptr_t<_Ty>& p) { m_pointer.template set<writelock_ptr_t<_Ty>>(p); }

				_Ty& operator*() const {
					return *(static_cast<_Ty*>(m_pointer.arrow_operator()));
				}
				_Ty* operator->() const {
					return static_cast<_Ty*>(m_pointer.arrow_operator());
				}

#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				template <typename _Ty2>
				bool operator ==(const _Ty2& _Right_cref) const {
					return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
				}
				template <typename _Ty2>
				bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }
#else // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
				friend bool operator==(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					if (!bool(_Left_cref)) {
						if (!bool(_Right_cref)) {
							return true;
						}
						else {
							return false;
						}
					}
					else if (!bool(_Right_cref)) {
						return false;
					}
					return (std::addressof(*_Right_cref) == std::addressof(*_Left_cref));
				}
#endif // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)

				TPolyPointerBase<_Ty>& operator=(const us::impl::TPolyPointerBase<_Ty>& _Right_cref) {
					/* We can't use the "copy and swap idiom" because the "variant" implementation we're using
					doesn't support typesafe swap. */
					m_pointer.~poly_variant();
					new (&m_pointer) poly_variant(_Right_cref.m_pointer);
					return (*this);
				}

			protected:
				explicit operator bool() const {
					return m_pointer.bool_operator();
				}

				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				poly_variant m_pointer;

				friend class us::impl::TPolyConstPointerBase<_Ty>;
			};
		}
	}

	template<typename _Ty>
	class TXScopePolyPointer : public us::impl::TPolyPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef us::impl::TPolyPointerBase<_Ty> base_class;

		//MSE_USING(TXScopePolyPointer, us::impl::TPolyPointerBase<_Ty>);
		//TXScopePolyPointer(const us::impl::TPolyPointerBase<_Ty>& p) : base_class(p) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			/*(!std::is_convertible<_TPointer1, us::impl::TPolyPointerBase<_Ty>>::value)
			&& */(!std::is_base_of<us::impl::TPolyConstPointerBase<_Ty>, _TPointer1>::value)
			> MSE_IMPL_EIS >
			TXScopePolyPointer(const _TPointer1& pointer) : base_class(pointer) {}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TXScopePolyPointer);

		void async_not_shareable_and_not_passable_tag() const {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		TXScopePolyPointer<_Ty>& operator=(const TXScopePolyPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TXScopePolyPointer<_Ty> >;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TXScopePolyPointer);

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_poly_pointer(const _Ty & x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopePolyPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_poly_pointer(_Ty && x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopePolyPointer<_Tx2>(MSE_FWD(x));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_poly_pointer)

	/* The intended semantics of poly pointers is that they always contain a valid pointer (or iterator) to a valid
	object. If you need a "null" state, consider using optional<>. */
	template<typename _Ty>
	class TPolyPointer : public us::impl::TPolyPointerBase<_Ty> {
	public:
		typedef us::impl::TPolyPointerBase<_Ty> base_class;

		//MSE_USING(TPolyPointer, us::impl::TPolyPointerBase<_Ty>);
		//TPolyPointer(const us::impl::TPolyPointerBase<_Ty>& p) : base_class(p) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TPointer1>::value)> MSE_IMPL_EIS >
		TPolyPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)") : base_class(pointer) {
			mse::impl::T_valid_if_not_an_xscope_type<_TPointer1>();
		}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TPolyPointer);

		TPolyPointer<_Ty>& operator=(const TPolyPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TPolyPointer<_Ty> >;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TPolyPointer);

	template <typename _Tx = void, typename _Ty = void>
	auto make_poly_pointer(const _Ty & x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TPolyPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_poly_pointer(_Ty && x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TPolyPointer<_Tx2>(MSE_FWD(x));
	}

	namespace us {
		namespace impl {
			template<typename _Ty>
			class TPolyConstPointerBase {
			public:
				typedef TPolyConstPointerBase _Myt;
				template<typename _Ty2> using writelock_ptr_t = decltype(std::declval<mse::TAsyncSharedV2ReadWriteAccessRequester<_Ty2> >().writelock_ptr());
				template<typename _Ty2> using readlock_ptr_t = decltype(std::declval<mse::TAsyncSharedV2ReadWriteAccessRequester<_Ty2> >().readlock_ptr());

				using poly_variant = tdp_pointer_variant <
#if !defined(MSE_SCOPEPOINTER_DISABLED)
					mse::TXScopeObjFixedConstPointer<_Ty>,
					mse::TXScopeFixedConstPointer<_Ty>,
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
					mse::TRegisteredConstPointer<_Ty>,
					mse::TCRegisteredConstPointer<_Ty>,
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_NORADPOINTER_DISABLED)
					mse::TNoradConstPointer<_Ty>,
#endif // !defined(MSE_NORADPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
					mse::TRefCountingConstPointer<_Ty>,
					mse::TRefCountingNotNullConstPointer<_Ty>,
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
					mse::us::impl::TPointer<const _Ty, mse::impl::TPolyPointerID<const _Ty>>,

					readlock_ptr_t<_Ty>,
					writelock_ptr_t<_Ty>
				> ;

				TPolyConstPointerBase(const us::impl::TPolyConstPointerBase<_Ty>& p) : m_pointer(p.m_pointer) {}
				TPolyConstPointerBase(const mse::us::impl::TPolyPointerBase<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }

#if !defined(MSE_SCOPEPOINTER_DISABLED)
				TPolyConstPointerBase(const mse::TXScopeObjFixedConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeObjFixedConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TXScopeObjFixedConstPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeObjFixedConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TXScopeObjFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeObjFixedConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TXScopeObjFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeObjFixedConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TXScopeFixedConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TXScopeFixedConstPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TXScopeFixedPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TXScopeFixedPointer<_Ty2>& p) { m_pointer.template set<mse::TXScopeFixedConstPointer<_Ty>>(p); }
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
#if !defined(MSE_REGISTEREDPOINTER_DISABLED)
				TPolyConstPointerBase(const mse::TRegisteredConstPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TRegisteredConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TRegisteredObj<_Ty2> *, TRegisteredObj<_Ty> *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TRegisteredConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TCRegisteredConstPointer<_Ty>& p) { m_pointer.template set<mse::TCRegisteredConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TCRegisteredConstPointer<_Ty2>& p) { m_pointer.template set<mse::TCRegisteredConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TCRegisteredPointer<_Ty>& p) { m_pointer.template set<mse::TCRegisteredConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TCRegisteredPointer<_Ty2>& p) { m_pointer.template set<mse::TCRegisteredConstPointer<_Ty>>(p); }
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_NORADPOINTER_DISABLED)
				TPolyConstPointerBase(const mse::TNoradConstPointer<_Ty>& p) { m_pointer.template set<mse::TNoradConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TNoradConstPointer<_Ty2>& p) { m_pointer.template set<mse::TNoradConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TNoradPointer<_Ty>& p) { m_pointer.template set<mse::TNoradConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TNoradPointer<_Ty2>& p) { m_pointer.template set<mse::TNoradConstPointer<_Ty>>(p); }
#endif // !defined(MSE_NORADPOINTER_DISABLED)
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
				TPolyConstPointerBase(const mse::TRefCountingConstPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TRefCountingConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TRefCountingPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TRefCountingPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TRefCountingNotNullConstPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingNotNullConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TRefCountingNotNullConstPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingNotNullConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const mse::TRefCountingNotNullPointer<_Ty>& p) { m_pointer.template set<mse::TRefCountingNotNullConstPointer<_Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const mse::TRefCountingNotNullPointer<_Ty2>& p) { m_pointer.template set<mse::TRefCountingNotNullConstPointer<_Ty>>(p); }
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
#if !defined(MSE_MSTDVECTOR_DISABLED)
				TPolyConstPointerBase(const typename mse::mstd::vector<_Ty>::const_iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::const_iterator>(p); }
				TPolyConstPointerBase(const typename mse::mstd::vector<_Ty>::iterator& p) { m_pointer.template set<typename mse::mstd::vector<_Ty>::const_iterator>(p); }
#endif // !defined(MSE_MSTDVECTOR_DISABLED)
				TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::const_iterator& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::const_iterator>(p); }
				TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::cipointer& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::cipointer>(p); }
				TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::ss_const_iterator_type& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ss_const_iterator_type>(p); }
				TPolyConstPointerBase(const std::shared_ptr<const _Ty>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const std::shared_ptr<const _Ty2>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
				TPolyConstPointerBase(const mse::TXScopeAnyConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(p); }
				TPolyConstPointerBase(const mse::TAnyConstPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(p); }

				TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::iterator& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::const_iterator>(p); }
				TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::ipointer& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::cipointer>(p); }
				TPolyConstPointerBase(const typename mse::us::msevector<_Ty>::ss_iterator_type& p) { m_pointer.template set<typename mse::us::msevector<_Ty>::ss_const_iterator_type>(p); }
				TPolyConstPointerBase(const std::shared_ptr<_Ty>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
				TPolyConstPointerBase(const std::shared_ptr<_Ty2>& p) { m_pointer.template set<std::shared_ptr<const _Ty>>(p); }
				TPolyConstPointerBase(const mse::TXScopeAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }
				TPolyConstPointerBase(const mse::TAnyPointer<_Ty>& p) { m_pointer.template set<mse::TXScopeAnyConstPointer<_Ty>>(mse::TXScopeAnyConstPointer<_Ty>(p)); }

				TPolyConstPointerBase(const mse::us::impl::TPointer<const _Ty>& p) { m_pointer.template set<mse::us::impl::TPointer<const _Ty, mse::impl::TPolyPointerID<const _Ty>>>(p); }
				TPolyConstPointerBase(const mse::us::impl::TPointer<_Ty>& p) { m_pointer.template set<mse::us::impl::TPointer<const _Ty, mse::impl::TPolyPointerID<const _Ty>>>(p); }
				TPolyConstPointerBase(const _Ty* p) { m_pointer.template set<mse::us::impl::TPointer<const _Ty, mse::impl::TPolyPointerID<const _Ty>>>(p); }

				TPolyConstPointerBase(const readlock_ptr_t<_Ty>& p) { m_pointer.template set<readlock_ptr_t<_Ty>>(p); }
				TPolyConstPointerBase(const writelock_ptr_t<_Ty>& p) { m_pointer.template set<writelock_ptr_t<_Ty>>(p); }

				const _Ty& operator*() const {
					return *(static_cast<const _Ty*>(m_pointer.const_arrow_operator()));
				}
				const _Ty* operator->() const {
					return static_cast<const _Ty*>(m_pointer.const_arrow_operator());
				}

#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				template <typename _Ty2>
				bool operator ==(const _Ty2& _Right_cref) const {
					return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
				}
				template <typename _Ty2>
				bool operator !=(const _Ty2& _Right_cref) const { return !((*this) == _Right_cref); }
#else // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (std::is_base_of<TPolyPointerBase<_Ty>, TLHSPointer_ecwapt>::value) || ((!std::is_base_of<TPolyPointerBase<_Ty>, TLHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<TPolyPointerBase<_Ty>, TRHSPointer_ecwapt>::value) || ((!std::is_base_of<TPolyPointerBase<_Ty>, TRHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (std::is_base_of<TPolyPointerBase<_Ty>, TLHSPointer_ecwapt>::value) || ((!std::is_base_of<TPolyPointerBase<_Ty>, TLHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<TPolyPointerBase<_Ty>, TRHSPointer_ecwapt>::value) || ((!std::is_base_of<TPolyPointerBase<_Ty>, TRHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<TPolyPointerBase<_Ty>, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyConstPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
				friend bool operator==(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					if (!bool(_Left_cref)) {
						if (!bool(_Right_cref)) {
							return true;
						}
						else {
							return false;
						}
					}
					else if (!bool(_Right_cref)) {
						return false;
					}
					return (std::addressof(*_Right_cref) == std::addressof(*_Left_cref));
				}
#endif // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)

				us::impl::TPolyConstPointerBase<_Ty>& operator=(const us::impl::TPolyConstPointerBase<_Ty>& _Right_cref) {
					/* We can't use the "copy and swap idiom" because the "variant" implementation we're using
					doesn't support typesafe swap. */
					m_pointer.~poly_variant();
					new (&m_pointer) poly_variant(_Right_cref.m_pointer);
					return (*this);
				}

			protected:
				explicit operator bool() const {
					return m_pointer.bool_operator();
				}

				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				poly_variant m_pointer;
			};
		}
	}

	template<typename _Ty>
	class TXScopePolyConstPointer : public us::impl::TPolyConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef us::impl::TPolyConstPointerBase<_Ty> base_class;
		//TXScopePolyConstPointer(const us::impl::TPolyConstPointerBase<_Ty>& src) : base_class(src) {}
		//TXScopePolyConstPointer(const us::impl::TPolyPointerBase<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1/*, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, us::impl::TPolyConstPointerBase<_Ty>>::value)
			&& (!std::is_convertible<_TPointer1, us::impl::TPolyPointerBase<_Ty>>::value)
			> MSE_IMPL_EIS*/>
			TXScopePolyConstPointer(const _TPointer1& pointer) : base_class(pointer) {}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TXScopePolyConstPointer);

		void async_not_shareable_and_not_passable_tag() const {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		TXScopePolyConstPointer<_Ty>& operator=(const TXScopePolyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TXScopePolyConstPointer<_Ty> >;
	};

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_poly_const_pointer(const _Ty & x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopePolyConstPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_poly_const_pointer(_Ty && x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopePolyConstPointer<_Tx2>(MSE_FWD(x));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_poly_const_pointer)

	template<typename _Ty>
	class TPolyConstPointer : public us::impl::TPolyConstPointerBase<_Ty> {
	public:
		typedef us::impl::TPolyConstPointerBase<_Ty> base_class;
		//TPolyConstPointer(const TPolyConstPointer& src) : base_class(src) {}
		//TPolyConstPointer(const TPolyPointer<_Ty>& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TPointer1>::value)> MSE_IMPL_EIS >
		TPolyConstPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)") : base_class(pointer) {
			mse::impl::T_valid_if_not_an_xscope_type<_TPointer1>();
		}

		//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TPolyConstPointer);

		TPolyConstPointer<_Ty>& operator=(const TPolyConstPointer<_Ty>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend struct std::hash<mse::TPolyConstPointer<_Ty> >;
	};

	template <typename _Tx = void, typename _Ty = void>
	auto make_poly_const_pointer(const _Ty & x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TPolyConstPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_poly_const_pointer(_Ty && x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TPolyConstPointer<_Tx2>(MSE_FWD(x));
	}

	namespace rsv {
		template<typename _Ty>
		class TFParam<mse::TXScopePolyConstPointer<_Ty> > : public TXScopePolyConstPointer<_Ty> {
		public:
			typedef TXScopePolyConstPointer<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				typename mse::rsv::impl::is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue<_TRALoneParam>::type()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			//MSE_IMPL_EQUALITY_COMPARISON_OPERATOR_DECLARATIONS_FOR_ANY_POINTERS(TFParam);

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeFixedConstPointerFParam<_Ty2>(MSE_FWD(param));
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

	template <typename _Ty> class TAnyRandomAccessIterator;
	template <typename _Ty> class TAnyRandomAccessConstIterator;

	namespace impl {

		template <typename T, typename = void>
		struct SupportsSubtraction_poly : std::false_type {};
		template <typename T>
		struct SupportsSubtraction_poly<T, mse::impl::void_t<decltype(std::declval<T>() - std::declval<T>())> > : std::true_type {};

		template <typename TToBeSubtracted, typename TToBeSubtractedFrom, typename = void>
		struct SupportsSubtractionFrom_poly : std::false_type {};
		template <typename TToBeSubtracted, typename TToBeSubtractedFrom>
		struct SupportsSubtractionFrom_poly<TToBeSubtracted, TToBeSubtractedFrom,
			mse::impl::void_t<decltype(std::declval<TToBeSubtractedFrom>() - std::declval<TToBeSubtracted>())>>
			: std::true_type {};


		template<class _Ty, class TID = void>
		struct test_iterator : public mse::impl::random_access_iterator_base<_Ty> {
			typedef mse::impl::random_access_iterator_base<_Ty> base_class;
			MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);
			test_iterator() : m_ptr(nullptr) {}
			explicit test_iterator(_Ty* ptr) : m_ptr(ptr) {}
			test_iterator(const test_iterator& src) = default;
			_Ty& operator*() const { return (*m_ptr); }
			_Ty* operator->() const { return m_ptr; }
			_Ty& operator[](difference_type _Off) const { return m_ptr[_Off]; }
			bool operator==(const test_iterator& _Right_cref) const { return (m_ptr == _Right_cref.m_ptr); }
#ifndef MSE_HAS_CXX20
			bool operator!=(const test_iterator& _Right_cref) const { return (!((*this) == _Right_cref.m_ptr)); }
#endif // !MSE_HAS_CXX20
			difference_type operator-(const test_iterator& src) const { return m_ptr - src.m_ptr; }
			test_iterator operator-(difference_type n) const { auto retval = *this; retval.m_ptr -= n; return retval; }
			void operator +=(difference_type x) { *this = (*this) - (-x); }
			explicit operator bool() const { return bool(m_ptr); }
			_Ty* m_ptr = nullptr;
		};

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
	}
	namespace us {
		namespace impl {
			template <typename _Ty> class TAnyRandomAccessIteratorBase;
			template <typename _Ty> class TAnyRandomAccessConstIteratorBase;
			template <typename _Ty> class TNullableAnyRandomAccessIteratorBase;
		}
	}
	namespace lh {
		namespace us {
			namespace impl {
#if !defined(MSE_SAFER_SUBSTITUTES_DISABLED)
				template <typename _Ty> class TLHNullableAnyRandomAccessIteratorBase;
#endif // !defined(MSE_SAFER_SUBSTITUTES_DISABLED)
			}
		}
	}
	namespace impl {
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))

#if 1
		template<class T, class TID, class EqualTo>
		struct SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype((std::declval<U>() == std::declval<test_iterator<mse::impl::remove_reference_t<decltype(*std::declval<U>())>, TID> >())
				, (std::declval<V>() == std::declval<test_iterator<mse::impl::remove_reference_t<decltype(*std::declval<V>())>, TID> >()), bool(true));
			template<typename, typename>
			static auto test(...) -> std::false_type;

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
			/* Apr 2025: When compiling in C++17 mode, msvc2022 complained: "mseany.h(3268,45): error C1202: recursive type or function dependency context too complex".
			For now we're just going to simplify it by just comparing to the set of matches we know about. */
			static const bool value = mse::impl::first_is_or_is_subclass_of_any<T
				, mse::us::impl::TAnyRandomAccessIteratorBase<mse::impl::target_or_given_default_type<T, std::nullptr_t> >
				, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::target_or_given_default_type<T, std::nullptr_t> >
				, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<mse::impl::target_or_given_default_type<T, std::nullptr_t> > >
				, mse::us::impl::TNullableAnyRandomAccessIteratorBase<mse::impl::target_or_given_default_type<T, std::nullptr_t> >
#if !defined(MSE_SAFER_SUBSTITUTES_DISABLED)
				, mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<mse::impl::target_or_given_default_type<T, std::nullptr_t> >
#endif // !defined(MSE_SAFER_SUBSTITUTES_DISABLED)
			>::value;
			using type = mse::impl::conditional_t<value, std::true_type, std::false_type>;
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
		};
		template<class TID>
		struct SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly_impl<void*, TID, void*> : std::false_type {};
		template<class T, class TID = void, class EqualTo = T>
		struct SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly : SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly_impl<
			mse::impl::remove_reference_t<T>, TID, mse::impl::remove_reference_t<EqualTo> >::type {};
#else // 1

		template <typename T, class TID = void, typename = void>
		struct SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly : std::false_type {};
		template <typename T, class TID>
		struct SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<T, TID, mse::impl::void_t<decltype((std::declval<T>() == std::declval<test_iterator<mse::impl::remove_reference_t<decltype(*std::declval<T>())>, TID> >()))> > : std::true_type {};
		template <typename T>
		struct SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<T, void, mse::impl::void_t<decltype((std::declval<T>() == std::declval<test_iterator<mse::impl::remove_reference_t<decltype(*std::declval<T>())>, void> >()))> > : std::true_type {};
#endif // 1
	}

	namespace us {
		namespace impl {
			template <typename _Ty> using TRandomAccessIteratorStdBase = mse::impl::random_access_iterator_base<_Ty>;
			template <typename _Ty> using TRandomAccessConstIteratorStdBase = mse::impl::random_access_const_iterator_base<_Ty>;

			/* Note: This class needs to be maintained as structurally identical to its const counterpart (below) as there may
			be some `reinterpret_cast<>`s between the two. */
			template <typename _Ty>
			class TCommonRandomAccessIteratorInterface : public TRandomAccessIteratorStdBase<_Ty> {
			public:
				typedef TRandomAccessIteratorStdBase<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

				virtual ~TCommonRandomAccessIteratorInterface() {}
				virtual _Ty& operator*() const = 0;
				virtual _Ty* operator->() const = 0;
				virtual reference operator[](difference_type _Off) const = 0;
				virtual void operator +=(difference_type x) = 0;
				virtual void operator -=(difference_type x) { operator +=(-x); }
				virtual void operator ++() { operator +=(1); }
				virtual void operator ++(int) { operator +=(1); }
				virtual void operator --() { operator -=(1); }
				virtual void operator --(int) { operator -=(1); }
				virtual difference_type operator-(const TCommonRandomAccessIteratorInterface& _Right_cref) const = 0;
				virtual bool operator==(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 == operator-(_Right_cref)); }
#ifndef MSE_HAS_CXX20
				virtual bool operator!=(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return !(operator==(_Right_cref)); }
				virtual bool operator<(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
				virtual bool operator>(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
				virtual bool operator<=(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
				virtual bool operator>=(const TCommonRandomAccessIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
#else // !MSE_HAS_CXX20
				virtual std::strong_ordering operator<=>(const TCommonRandomAccessIteratorInterface& _Right_cref) const {
					auto diff = operator-(_Right_cref);
					return (diff <=> 0); /* that's the right order, right? */
				}
#endif // !MSE_HAS_CXX20
				virtual optional1<std::type_info const*> typeid_if_available() const { return {}; }

				virtual optional1<_Ty const*> debug_start_of_sequence_cptr_if_available() const { return {}; }
				virtual optional1<size_t> debug_iterator_index_if_available() const { return {}; }
				virtual optional1<_Ty const*> debug_item_cptr_if_available() const { return {}; }
#ifdef MSE_HAS_CXX20
				typedef mse::impl::conditional_t<mse::impl::is_complete_type<_Ty>::value, std::span<_Ty const>, _Ty const*> span1_t;
				virtual optional1<span1_t> debug_sequence_span_if_available() const { return {}; }
#endif // MSE_HAS_CXX20
				virtual optional1<mse::us::impl::ns_any::any> as_a_wrapped_TCommonizedRandomAccessConstIterator_if_available() const { return {}; }
			};

			/* Note: This class needs to be maintained as structurally identical to its const counterpart (below) as there may
			be some `reinterpret_cast<>`s between the two. */
			template <typename _Ty, typename _TRandomAccessIterator1>
			class TCommonizedRandomAccessIterator : public TCommonRandomAccessIteratorInterface<_Ty> {
			public:
				typedef TCommonRandomAccessIteratorInterface<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

				TCommonizedRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : m_random_access_iterator(random_access_iterator) {}
				virtual ~TCommonizedRandomAccessIterator() {}

				_Ty& operator*() const override {
					/* Using the mse::us::impl::raw_reference_to<>() function allows us to, for example, obtain an 'int&' to
					an mse::Tint<int>. This allows an iterator to an mse::TInt<int> to be used as an iterator to an int. */
					return mse::us::impl::raw_reference_to<_Ty>(*m_random_access_iterator);
				}
				_Ty* operator->() const override {
					return std::addressof(mse::us::impl::raw_reference_to<_Ty>(*m_random_access_iterator));
					//return m_random_access_iterator.operator->();
				}
				reference operator[](difference_type _Off) const override {
					return mse::us::impl::raw_reference_to<_Ty>(m_random_access_iterator[_Off]);
				}
				void operator +=(difference_type x) override { m_random_access_iterator += x; }
				difference_type operator-(const TCommonRandomAccessIteratorInterface<_Ty>& _Right_cref) const override {
					std::string lhs_type_name;
					std::string rhs_type_name;
					if (false) {
						/* Just some type info for debugging. */
						auto lhs_maybe_type_info_cptr = (*this).typeid_if_available();
						if (lhs_maybe_type_info_cptr.has_value()) {
							auto type_info_cptr = lhs_maybe_type_info_cptr.value();
							assert(type_info_cptr);
							lhs_type_name = type_info_cptr->name();
						}
						auto rhs_maybe_type_info_cptr = _Right_cref.typeid_if_available();
						if (rhs_maybe_type_info_cptr.has_value()) {
							auto type_info_cptr = rhs_maybe_type_info_cptr.value();
							assert(type_info_cptr);
							rhs_type_name = type_info_cptr->name();
						}
					}

					const TCommonizedRandomAccessIterator* crai_ptr = dynamic_cast<const TCommonizedRandomAccessIterator*>(&_Right_cref);

					if (!crai_ptr) {
						/* The right argument cannot be dynamically cast to match the left argument. The discrepancy might only be a `const`
						qualification of the target object type, in which case it should be safe to just reinterpret_cast<> the right argument
						to match the `const`ness of the left argument. */

#define MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(interface_target_type, wrapped_iterator_type) \
						if (!crai_ptr) { \
							/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessIteratorInterface<const T> being "structurally identical". */ \
							auto& _Right_rc_cref = reinterpret_cast<const TCommonRandomAccessIteratorInterface<interface_target_type>&>(_Right_cref); \
							auto* crai_rc_ptr = dynamic_cast<const TCommonizedRandomAccessIterator<interface_target_type, wrapped_iterator_type>*>(&_Right_rc_cref); \
							if (crai_rc_ptr) { \
								/* We're relying on TCommonizedRandomAccessIterator<T, _TRandomAccessIterator1> and TCommonizedRandomAccessIterator<const T, _TRandomAccessIterator1> being "structurally identical". */ \
								crai_ptr = reinterpret_cast<const TCommonizedRandomAccessIterator*>(crai_rc_ptr); \
							} \
						}

						MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_const_target<_TRandomAccessIterator1>::type);
						MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessIterator1>::type);
#if !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
						MSE_IF_CONSTEXPR(!std::is_const<mse::impl::target_type<typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessIterator1>::type> >::value) {
							/* We needed to verify that the type given by mse::impl::corresponding_type_with_nonconst_target<> does in fact have a non-const 
							target type (as mse::impl::corresponding_type_with_nonconst_target<> does not recognize all iterator types). */

							MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessIterator1>::type);
						}
#endif // !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
					}

					if (!crai_ptr) {
						MSE_THROW(std::logic_error("attempt to subtract or compare iterators of different (underlying) types - operator-() - TCommonizedRandomAccessIterator"));
					}
					const _TRandomAccessIterator1& _Right_cref_m_random_access_iterator_cref = (*crai_ptr).m_random_access_iterator;
					return m_random_access_iterator - _Right_cref_m_random_access_iterator_cref;
				}
				bool operator==(const TCommonRandomAccessIteratorInterface<_Ty>& _Right_cref) const override {
					const TCommonizedRandomAccessIterator* crai_ptr = dynamic_cast<const TCommonizedRandomAccessIterator*>(&_Right_cref);

					if (!crai_ptr) {
						/* The right argument cannot be dynamically cast to match the left argument. The discrepancy might only be a `const`
						qualification of the target object type, in which case it should be safe to just reinterpret_cast<> the right argument
						to match the `const`ness of the left argument. */

						MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_const_target<_TRandomAccessIterator1>::type);
						MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessIterator1>::type);
#if !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
						MSE_IF_CONSTEXPR(!std::is_const<mse::impl::target_type<typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessIterator1>::type> >::value) {
							/* We needed to verify that the type given by mse::impl::corresponding_type_with_nonconst_target<> does in fact have a non-const
							target type (as mse::impl::corresponding_type_with_nonconst_target<> does not recognize all iterator types). */

							MSE_IMPL_TCRAI_TRY_DYNAMIC_CAST(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessIterator1>::type);
						}
#endif // !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
					}

					if (!crai_ptr) {
						//MSE_THROW(std::logic_error("attempt to compare iterators of different (underlying) types - operator==() - TCommonizedRandomAccessIterator"));
						return false;
					}
					const _TRandomAccessIterator1& _Right_cref_m_random_access_iterator_cref = (*crai_ptr).m_random_access_iterator;
					return (m_random_access_iterator == _Right_cref_m_random_access_iterator_cref);
				}
				virtual optional1<std::type_info const*> typeid_if_available() const override { return &typeid(*this); }

				auto debug_begin_iter_if_available_helper2(std::true_type) const {
					typedef decltype(mse::make_begin_iterator(m_random_access_iterator.target_container_ptr())) iter_t;
					optional1<iter_t> retval;
					auto ptr1 = m_random_access_iterator.target_container_ptr();
					if (ptr1) {
						auto size1 = mse::container_size(*ptr1);
						if (1 <= size1) {
							auto iter1 = mse::make_begin_iterator(ptr1);
							retval = iter1;
						}
					}
					return retval;
				}
				optional1<_Ty const*> debug_begin_iter_if_available_helper2(std::false_type) const { return {}; }
				auto debug_begin_iter_if_available_helper1(std::true_type) const {
					return debug_begin_iter_if_available_helper2(typename mse::impl::IsDereferenceable_pb<decltype(mse::make_begin_iterator(m_random_access_iterator.target_container_ptr()))>::type());
				}
				optional1<_Ty const*> debug_begin_iter_if_available_helper1(std::false_type) const { return {}; }
				auto debug_begin_iter_if_available() const {
					return debug_begin_iter_if_available_helper1(typename mse::impl::HasOrInheritsTargetContainerPtrMethod_msemsearray<_TRandomAccessIterator1>::type());
				}

				optional1<_Ty const*> debug_start_of_sequence_cptr_if_available() const override {
					auto maybe_begin_iter = debug_begin_iter_if_available();
					if (maybe_begin_iter.has_value()) {
						auto& begin_iter_ref = maybe_begin_iter.value();
						return std::addressof(*begin_iter_ref);
					}
					return {};
				}

				optional1<size_t> debug_iterator_index_if_available_helper1(std::true_type) const {
					auto maybe_begin_iter = debug_begin_iter_if_available();
					if (maybe_begin_iter.has_value()) {
						auto& begin_iter_ref = maybe_begin_iter.value();
						return m_random_access_iterator - begin_iter_ref;
					}
					return {};
				}
				optional1<size_t> debug_iterator_index_if_available_helper1(std::false_type) const { return {}; }

				optional1<size_t> debug_iterator_index_if_available() const override {
					typedef mse::impl::remove_reference_t<decltype(debug_begin_iter_if_available().value())> iter_t;
					return debug_iterator_index_if_available_helper1(typename mse::impl::SupportsSubtractionFrom_poly<iter_t, _TRandomAccessIterator1>::type());
				}

				optional1<_Ty const*> debug_item_cptr_if_available_helper1(std::true_type) const {
					const auto ptr1 = m_random_access_iterator.target_container_ptr();
					if (ptr1) {
						const auto size1 = mse::container_size(*ptr1);
						const auto maybe_index = debug_iterator_index_if_available();
						if (maybe_index.has_value()) {
							const auto& index_ref = maybe_index.value();
							if (size1 > index_ref) {
								return std::addressof(*m_random_access_iterator);
							}
						}
					}
					return {};
				}
				optional1<_Ty const*> debug_item_cptr_if_available_helper1(std::false_type) const { return {}; }

				optional1<_Ty const*> debug_item_cptr_if_available() const override {
					return debug_item_cptr_if_available_helper1(typename mse::impl::HasOrInheritsTargetContainerPtrMethod_msemsearray<_TRandomAccessIterator1>::type());
				}

#ifdef MSE_HAS_CXX20
				typedef mse::impl::conditional_t<mse::impl::is_complete_type<_Ty>::value, std::span<_Ty const>, _Ty const*> span1_t;
				optional1<span1_t> debug_sequence_span_if_available_helper2(std::true_type) const {
					const auto ptr1 = m_random_access_iterator.target_container_ptr();
					if (ptr1) {
						auto maybe_start_of_sequence_cptr = debug_start_of_sequence_cptr_if_available();
						if (maybe_start_of_sequence_cptr.has_value()) {
							auto start_of_sequence_cptr = maybe_start_of_sequence_cptr.value();
							const auto size1 = mse::container_size(*ptr1);
							return std::span<_Ty const>(start_of_sequence_cptr, size1);
						}
					}
					return {};
				}
				optional1<span1_t> debug_sequence_span_if_available_helper2(std::false_type) const { return {}; }
				optional1<span1_t> debug_sequence_span_if_available_helper1(std::true_type) const {
					return debug_sequence_span_if_available_helper2(typename mse::impl::HasOrInheritsTargetContainerPtrMethod_msemsearray<_TRandomAccessIterator1>::type());
				}
				optional1<span1_t> debug_sequence_span_if_available_helper1(std::false_type) const {
					return debug_start_of_sequence_cptr_if_available();
				}

				optional1<span1_t> debug_sequence_span_if_available() const override {
					return debug_sequence_span_if_available_helper1(typename mse::impl::is_complete_type<_Ty>::type());
				}
#endif // MSE_HAS_CXX20
				optional1<mse::us::impl::ns_any::any> as_a_wrapped_TCommonizedRandomAccessConstIterator_if_available() const override;

				_TRandomAccessIterator1 m_random_access_iterator;

				template <typename T, typename = void>
				struct IsDynamicCastable : std::false_type {};
				template <typename T>
				struct IsDynamicCastable<T, mse::impl::void_t<decltype(dynamic_cast<const TCommonizedRandomAccessIterator<mse::impl::remove_const_t<_Ty>, T>*>(&std::declval<const TCommonRandomAccessIteratorInterface<_Ty>&>()))> > : std::true_type {};
			};

			/* Note: This class needs to be maintained as structurally identical to its non-const counterpart (above) as there may 
			be some `reinterpret_cast<>`s between the two. */
			template <typename _Ty>
			class TCommonRandomAccessConstIteratorInterface : public TRandomAccessConstIteratorStdBase<_Ty> {
			public:
				typedef TRandomAccessConstIteratorStdBase<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

				virtual ~TCommonRandomAccessConstIteratorInterface() {}
				virtual const _Ty& operator*() const = 0;
				virtual const _Ty* operator->() const = 0;
				virtual const_reference operator[](difference_type _Off) const = 0;
				virtual void operator +=(difference_type x) = 0;
				virtual void operator -=(difference_type x) { operator +=(-x); }
				virtual void operator ++() { operator +=(1); }
				virtual void operator ++(int) { operator +=(1); }
				virtual void operator --() { operator -=(1); }
				virtual void operator --(int) { operator -=(1); }
				virtual difference_type operator-(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const = 0;
				virtual bool operator==(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 == operator-(_Right_cref)); }
#ifndef MSE_HAS_CXX20
				virtual bool operator!=(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return !(operator==(_Right_cref)); }
				virtual bool operator<(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
				virtual bool operator>(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 > operator-(_Right_cref)); }
				virtual bool operator<=(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
				virtual bool operator>=(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
#else // !MSE_HAS_CXX20
				virtual std::strong_ordering operator<=>(const TCommonRandomAccessConstIteratorInterface& _Right_cref) const {
					auto diff = operator-(_Right_cref);
					return (diff <=> 0); /* that's the right order, right? */
				}
#endif // !MSE_HAS_CXX20
				virtual optional1<std::type_info const*> typeid_if_available() const { return {}; }

				virtual optional1<_Ty const*> debug_start_of_sequence_cptr_if_available() const { return {}; }
				virtual optional1<size_t> debug_iterator_index_if_available() const { return {}; }
				virtual optional1<_Ty const*> debug_item_cptr_if_available() const { return {}; }
#ifdef MSE_HAS_CXX20
				typedef mse::impl::conditional_t<mse::impl::is_complete_type<_Ty>::value, std::span<_Ty const>, _Ty const*> span1_t;
				virtual optional1<span1_t> debug_sequence_span_if_available() const { return {}; }
#endif // MSE_HAS_CXX20
				/* This is here just because TCommonizedRandomAccessIterator<> has it and we need to maintain this class as "structurally equivalent" 
				as there may be some `reinterpret_cast<>`s between the two. */
				virtual optional1<mse::us::impl::ns_any::any> as_a_wrapped_TCommonizedRandomAccessConstIterator_if_available() const { return {}; }
			};

			/* Note: This class needs to be maintained as structurally identical to its non-const counterpart (above) as there may
			be some `reinterpret_cast<>`s between the two. */
			template <typename _Ty, typename _TRandomAccessConstIterator1>
			class TCommonizedRandomAccessConstIterator : public TCommonRandomAccessConstIteratorInterface<_Ty> {
			public:
				typedef TCommonRandomAccessConstIteratorInterface<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

				TCommonizedRandomAccessConstIterator(const _TRandomAccessConstIterator1& random_access_const_iterator) : m_random_access_const_iterator(random_access_const_iterator) {}
				virtual ~TCommonizedRandomAccessConstIterator() {}

				const _Ty& operator*() const override {
					/* Using the mse::us::impl::raw_reference_to<>() function allows us to, for example, obtain a 'const int&' to
					an mse::Tint<int>. This allows an iterator to an mse::TInt<int> to be used as an iterator to a const int. */
					return mse::us::impl::raw_reference_to<const _Ty>(*m_random_access_const_iterator);
				}
				const _Ty* operator->() const override {
					return std::addressof(mse::us::impl::raw_reference_to<const _Ty>(*m_random_access_const_iterator));
					//return m_random_access_const_iterator.operator->();
				}
				const_reference operator[](difference_type _Off) const override {
					return mse::us::impl::raw_reference_to<const _Ty>(m_random_access_const_iterator[_Off]);
				}
				void operator +=(difference_type x) override { m_random_access_const_iterator += x; }
				difference_type operator-(const TCommonRandomAccessConstIteratorInterface<_Ty>& _Right_cref) const override {
					std::string lhs_type_name;
					std::string rhs_type_name;
					if (false) {
						/* Just some type info for debugging. */
						auto lhs_maybe_type_info_cptr = (*this).typeid_if_available();
						if (lhs_maybe_type_info_cptr.has_value()) {
							auto type_info_cptr = lhs_maybe_type_info_cptr.value();
							assert(type_info_cptr);
							lhs_type_name = type_info_cptr->name();
						}
						auto rhs_maybe_type_info_cptr = _Right_cref.typeid_if_available();
						if (rhs_maybe_type_info_cptr.has_value()) {
							auto type_info_cptr = rhs_maybe_type_info_cptr.value();
							assert(type_info_cptr);
							rhs_type_name = type_info_cptr->name();
						}
					}

					const TCommonizedRandomAccessConstIterator* craci_ptr = dynamic_cast<const TCommonizedRandomAccessConstIterator*>(&_Right_cref);

					if (!craci_ptr) {
						/* The right argument cannot be dynamically cast to match the left argument. The discrepancy might only be a `const`
						qualification of the target object type, in which case it should be safe to just reinterpret_cast<> the right argument
						to match the `const`ness of the left argument. */

#define MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(interface_target_type, wrapped_iterator_type) \
						if (!craci_ptr) { \
							/* We're relying on TCommonRandomAccessConstIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<const T> being "structurally identical". */ \
							auto& _Right_rc_cref = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<interface_target_type>&>(_Right_cref); \
							auto* craci_rc_ptr = dynamic_cast<const TCommonizedRandomAccessConstIterator<interface_target_type, wrapped_iterator_type>*>(&_Right_rc_cref); \
							if (craci_rc_ptr) { \
								/* We're relying on TCommonizedRandomAccessConstIterator<T, _TRandomAccessConstIterator1> and TCommonizedRandomAccessConstIterator<const T, _TRandomAccessConstIterator1> being "structurally identical". */ \
								craci_ptr = reinterpret_cast<const TCommonizedRandomAccessConstIterator*>(craci_rc_ptr); \
							} \
						}

						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_const_target<_TRandomAccessConstIterator1>::type);
						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessConstIterator1>::type);
						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_const_target<_TRandomAccessConstIterator1>::type);
						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessConstIterator1>::type);
					}

					if (!craci_ptr) {
						MSE_THROW(std::logic_error("attempt to subtract or compare iterators of different (underlying) types - operator-() - TCommonizedRandomAccessConstIterator"));
					}
					const _TRandomAccessConstIterator1& _Right_cref_m_random_access_const_iterator_cref = (*craci_ptr).m_random_access_const_iterator;
					return m_random_access_const_iterator - _Right_cref_m_random_access_const_iterator_cref;
				}
				bool operator==(const TCommonRandomAccessConstIteratorInterface<_Ty>& _Right_cref) const override {
					const TCommonizedRandomAccessConstIterator* craci_ptr = dynamic_cast<const TCommonizedRandomAccessConstIterator*>(&_Right_cref);

					if (!craci_ptr) {
						/* The right argument cannot be dynamically cast to match the left argument. The discrepancy might only be a `const`
						qualification of the target object type, in which case it should be safe to just reinterpret_cast<> the right argument
						to match the `const`ness of the left argument. */

						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_const_target<_TRandomAccessConstIterator1>::type);
						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessConstIterator1>::type);
						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_const_target<_TRandomAccessConstIterator1>::type);
						MSE_IMPL_TCRACI_TRY_DYNAMIC_CAST(const _Ty, typename mse::impl::corresponding_type_with_nonconst_target<_TRandomAccessConstIterator1>::type);
					}

					if (!craci_ptr) {
						//MSE_THROW(std::logic_error("attempt to compare iterators of different (underlying) types - operator==() - TCommonizedRandomAccessConstIterator"));
						return false;
					}
					const _TRandomAccessConstIterator1& _Right_cref_m_random_access_const_iterator_cref = (*craci_ptr).m_random_access_const_iterator;
					return (m_random_access_const_iterator == _Right_cref_m_random_access_const_iterator_cref);
				}
				virtual optional1<std::type_info const*> typeid_if_available() const override { return &typeid(*this); }

				auto debug_begin_iter_if_available_helper2(std::true_type) const {
					typedef decltype(mse::make_begin_iterator(m_random_access_const_iterator.target_container_ptr())) iter_t;
					optional1<iter_t> retval;
					auto ptr1 = m_random_access_const_iterator.target_container_ptr();
					if (ptr1) {
						auto size1 = mse::container_size(*ptr1);
						if (1 <= size1) {
							auto iter1 = mse::make_begin_iterator(ptr1);
							retval = iter1;
						}
					}
					return retval;
				}
				optional1<_Ty const*> debug_begin_iter_if_available_helper2(std::false_type) const { return {}; }
				auto debug_begin_iter_if_available_helper1(std::true_type) const {
					return debug_begin_iter_if_available_helper2(typename mse::impl::IsDereferenceable_pb<decltype(mse::make_begin_iterator(m_random_access_const_iterator.target_container_ptr()))>::type());
				}
				optional1<_Ty const*> debug_begin_iter_if_available_helper1(std::false_type) const { return {}; }
				auto debug_begin_iter_if_available() const {
					return debug_begin_iter_if_available_helper1(typename mse::impl::HasOrInheritsTargetContainerPtrMethod_msemsearray<_TRandomAccessConstIterator1>::type());
				}

				optional1<_Ty const*> debug_start_of_sequence_cptr_if_available() const override {
					auto maybe_begin_iter = debug_begin_iter_if_available();
					if (maybe_begin_iter.has_value()) {
						auto& begin_iter_ref = maybe_begin_iter.value();
						return std::addressof(*begin_iter_ref);
					}
					return {};
				}

				optional1<size_t> debug_iterator_index_if_available_helper1(std::true_type) const {
					auto maybe_begin_iter = debug_begin_iter_if_available();
					if (maybe_begin_iter.has_value()) {
						auto& begin_iter_ref = maybe_begin_iter.value();
						return m_random_access_const_iterator - begin_iter_ref;
					}
					return {};
				}
				optional1<size_t> debug_iterator_index_if_available_helper1(std::false_type) const { return {}; }

				optional1<size_t> debug_iterator_index_if_available() const override {
					typedef mse::impl::remove_reference_t<decltype(debug_begin_iter_if_available().value())> iter_t;
					return debug_iterator_index_if_available_helper1(typename mse::impl::SupportsSubtractionFrom_poly<iter_t, _TRandomAccessConstIterator1>::type());
				}

				optional1<_Ty const*> debug_item_cptr_if_available_helper1(std::true_type) const {
					const auto ptr1 = m_random_access_const_iterator.target_container_ptr();
					if (ptr1) {
						const auto size1 = mse::container_size(*ptr1);
						const auto maybe_index = debug_iterator_index_if_available();
						if (maybe_index.has_value()) {
							const auto& index_ref = maybe_index.value();
							if (size1 > index_ref) {
								return std::addressof(*m_random_access_const_iterator);
							}
						}
					}
					return {};
				}
				optional1<_Ty const*> debug_item_cptr_if_available_helper1(std::false_type) const { return {}; }

				optional1<_Ty const*> debug_item_cptr_if_available() const override {
					return debug_item_cptr_if_available_helper1(typename mse::impl::HasOrInheritsTargetContainerPtrMethod_msemsearray<_TRandomAccessConstIterator1>::type());
				}

#ifdef MSE_HAS_CXX20
				typedef mse::impl::conditional_t<mse::impl::is_complete_type<_Ty>::value, std::span<_Ty const>, _Ty const*> span1_t;
				optional1<span1_t> debug_sequence_span_if_available_helper2(std::true_type) const {
					const auto ptr1 = m_random_access_const_iterator.target_container_ptr();
					if (ptr1) {
						auto maybe_start_of_sequence_cptr = debug_start_of_sequence_cptr_if_available();
						if (maybe_start_of_sequence_cptr.has_value()) {
							auto start_of_sequence_cptr = maybe_start_of_sequence_cptr.value();
							const auto size1 = mse::container_size(*ptr1);
							return std::span<_Ty const>(start_of_sequence_cptr, size1);
						}
					}
					return {};
				}
				optional1<span1_t> debug_sequence_span_if_available_helper2(std::false_type) const { return {}; }
				optional1<span1_t> debug_sequence_span_if_available_helper1(std::true_type) const {
					return debug_sequence_span_if_available_helper2(typename mse::impl::HasOrInheritsTargetContainerPtrMethod_msemsearray<_TRandomAccessConstIterator1>::type());
				}
				optional1<span1_t> debug_sequence_span_if_available_helper1(std::false_type) const {
					return debug_start_of_sequence_cptr_if_available();
				}

				optional1<span1_t> debug_sequence_span_if_available() const override {
					return debug_sequence_span_if_available_helper1(typename mse::impl::is_complete_type<_Ty>::type());
				}
#endif // MSE_HAS_CXX20
				/* This is here just because TCommonizedRandomAccessIterator<> has it and we need to maintain this class as "structurally equivalent"
				as there may be some `reinterpret_cast<>`s between the two. */
				optional1<mse::us::impl::ns_any::any> as_a_wrapped_TCommonizedRandomAccessConstIterator_if_available() const override { return mse::us::impl::ns_any::any(*this); }

				_TRandomAccessConstIterator1 m_random_access_const_iterator;

				template <typename T, typename = void>
				struct IsDynamicCastable : std::false_type {};
				template <typename T>
				struct IsDynamicCastable<T, mse::impl::void_t<decltype(dynamic_cast<const TCommonizedRandomAccessConstIterator<mse::impl::remove_const_t<_Ty>, T>*>(&std::declval<const TCommonRandomAccessConstIteratorInterface<_Ty>&>()))> > : std::true_type {};
			};

			template <typename _Ty, typename _TRandomAccessIterator1>
			optional1<mse::us::impl::ns_any::any> TCommonizedRandomAccessIterator<_Ty, _TRandomAccessIterator1>::as_a_wrapped_TCommonizedRandomAccessConstIterator_if_available() const {
				return mse::us::impl::ns_any::any(TCommonizedRandomAccessConstIterator<_Ty, _TRandomAccessIterator1>(m_random_access_iterator));
			}

			template <typename _Ty>
			class TAnyRandomAccessConstIteratorBase;

			template <typename _Ty>
			class TAnyRandomAccessIteratorBase : public TRandomAccessIteratorStdBase<_Ty> {
			public:
				typedef TRandomAccessIteratorStdBase<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);
				typedef TAnyRandomAccessIteratorBase _Myt;

				TAnyRandomAccessIteratorBase(const TAnyRandomAccessIteratorBase& src) : m_any_random_access_iterator(src.m_any_random_access_iterator) { update_debug_values(); }
				TAnyRandomAccessIteratorBase(_Ty arr[]) : m_any_random_access_iterator(TCommonizedRandomAccessIterator<_Ty, _Ty*>(arr)) { update_debug_values(); }

				template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
					(!std::is_convertible<_TRandomAccessIterator1 const *, TAnyRandomAccessIteratorBase<_Ty> const *>::value)
					&& (!std::is_convertible<_TRandomAccessIterator1 const*, TAnyRandomAccessConstIteratorBase<_Ty> const*>::value)
					&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
					&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
				> MSE_IMPL_EIS >
				TAnyRandomAccessIteratorBase(const _TRandomAccessIterator1& random_access_iterator) : m_any_random_access_iterator(constructor_helper1(typename std::is_base_of< TAnyRandomAccessIteratorBase<_Ty>, _TRandomAccessIterator1>::type(), random_access_iterator)) { update_debug_values(); }

				friend void swap(TAnyRandomAccessIteratorBase& first, TAnyRandomAccessIteratorBase& second) {
					std::swap(first.m_any_random_access_iterator, second.m_any_random_access_iterator);
					MSE_IF_DEBUG(first.update_debug_values(); second.update_debug_values();)
				}

				_Ty& operator*() const {
					return (*(*common_random_access_iterator_interface_ptr()));
				}
				_Ty* operator->() const {
					return common_random_access_iterator_interface_ptr()->operator->();
				}
				reference operator[](difference_type _Off) const {
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
				difference_type operator-(const TAnyRandomAccessConstIteratorBase<_Ty>& _Right_cref) const {
					return (TAnyRandomAccessConstIteratorBase<_Ty>(*this) - _Right_cref);
				}

#if !defined(MSE_HAS_CXX17)
				MSE_IMPL_ORDERED_TYPE_OPERATOR_DELEGATING_DECLARATIONS(TAnyRandomAccessIteratorBase, TAnyRandomAccessConstIteratorBase<_Ty>);
#else // !defined(MSE_HAS_CXX17)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				MSE_IMPL_ORDERING_COMPARISON_OPERATOR_DELEGATING_DECLARATIONS(TAnyRandomAccessIteratorBase, TAnyRandomAccessConstIteratorBase<_Ty>);

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TRHSIterator_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value))> MSE_IMPL_EIS >
				friend std::strong_ordering operator<=>(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) { return (((_Left_cref)-(_Right_cref)) <=> 0); } /* this is the proper order, right? */

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
				friend bool operator==(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					const TCommonRandomAccessIteratorInterface<_Ty>* LHS_crai_ptr = nullptr;
					const TCommonRandomAccessIteratorInterface<_Ty>* RHS_crai_ptr = nullptr;

					MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value) {
						LHS_crai_ptr = _Left_cref.common_random_access_iterator_interface_ptr();
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TLHSIterator_ecwapt>::value) {
						const TCommonRandomAccessIteratorInterface<mse::impl::remove_const_t<_Ty> >* LHS_crai_rc_ptr = s_common_random_access_iterator_interface_ptr(_Left_cref);
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessIteratorInterface<const T> being "structurally identical". */
						LHS_crai_ptr = reinterpret_cast<const TCommonRandomAccessIteratorInterface<_Ty>*>(LHS_crai_rc_ptr);
					}
					MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TRHSIterator_ecwapt>::value) {
						RHS_crai_ptr = _Right_cref.common_random_access_iterator_interface_ptr();
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TRHSIterator_ecwapt>::value) {
						const TCommonRandomAccessIteratorInterface<mse::impl::remove_const_t<_Ty> >* RHS_crai_rc_ptr = s_common_random_access_iterator_interface_ptr(_Right_cref);
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessIteratorInterface<const T> being "structurally identical". */
						RHS_crai_ptr = reinterpret_cast<const TCommonRandomAccessIteratorInterface<_Ty>*>(RHS_crai_rc_ptr);
					}
					mse::optional<_Myt> maybe_adj_LHS;
					mse::optional<_Myt> maybe_adj_RHS;
					if (!LHS_crai_ptr) {
						maybe_adj_LHS = _Myt(_Right_cref);
						LHS_crai_ptr = maybe_adj_LHS.value().common_random_access_iterator_interface_ptr();
						assert(LHS_crai_ptr);
					}
					if (!RHS_crai_ptr) {
						maybe_adj_RHS = _Myt(_Right_cref);
						RHS_crai_ptr = maybe_adj_RHS.value().common_random_access_iterator_interface_ptr();
						assert(RHS_crai_ptr);
					}
					return (*LHS_crai_ptr) == (*RHS_crai_ptr);
				}
#endif // !defined(MSE_HAS_CXX17)

				TAnyRandomAccessIteratorBase& operator=(TAnyRandomAccessIteratorBase _Right) {
					swap(*this, _Right);
					update_debug_values();
					return (*this);
				}

			protected:
				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				template <typename _TRandomAccessIterator1>
				static auto construction_helper3(std::false_type, const _TRandomAccessIterator1& src) {
					return TCommonizedRandomAccessIterator<_Ty, _TRandomAccessIterator1>(src);
				}
				template <typename _Ty2>
				static auto construction_helper3(std::true_type, const _Ty2& src) {
					/* argument seems to be a TLHNullableAnyRandomAccessIteratorBase<std::remove_const_t<_Ty> > */
					//return reinterpret_cast<const TLHNullableAnyRandomAccessIteratorBase&>(src).m_iter;
					return src.m_any_random_access_iterator;
				}

				template <typename _TRandomAccessIterator1>
				static auto& constructor_helper1(std::true_type, const _TRandomAccessIterator1& random_access_iterator) {
					return reinterpret_cast<const TAnyRandomAccessIteratorBase&>(random_access_iterator).m_any_random_access_iterator;
				}
				template <typename _TRandomAccessIterator1>
				static auto constructor_helper1(std::false_type, const _TRandomAccessIterator1& random_access_iterator) {
					return construction_helper3(typename std::integral_constant<bool, std::is_const<_Ty>::value && std::is_base_of<TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, _TRandomAccessIterator1>::value>::type(), random_access_iterator);
				}

				TCommonRandomAccessIteratorInterface<_Ty>* common_random_access_iterator_interface_ptr() {
					auto retval = static_cast<TCommonRandomAccessIteratorInterface<_Ty>*>(m_any_random_access_iterator.storage_address());
					assert(nullptr != retval);
					return retval;
				}
				const TCommonRandomAccessIteratorInterface<_Ty>* common_random_access_iterator_interface_ptr() const {
					auto retval = static_cast<const TCommonRandomAccessIteratorInterface<_Ty>*>(m_any_random_access_iterator.storage_address());
					assert(nullptr != retval);
					return retval;
				}
				template <typename _Ty2>
				static auto* s_common_random_access_iterator_interface_ptr(TAnyRandomAccessIteratorBase<_Ty2> const& iter) { return iter.common_random_access_iterator_interface_ptr(); }

				struct CDebugValues {
					optional1<_Ty const*> maybe_debug_item_cptr;
					optional1<size_t> maybe_debug_iterator_index;
#ifdef MSE_HAS_CXX20
					typedef mse::impl::conditional_t<mse::impl::is_complete_type<_Ty>::value, std::span<_Ty const>, _Ty const*> span1_t;
					optional1<span1_t> maybe_debug_sequence_span;
#else // MSE_HAS_CXX20
					optional1<_Ty const*> maybe_debug_start_of_sequence_cptr;
#endif // MSE_HAS_CXX20
				};
				CDebugValues updated_debug_values() const {
					return CDebugValues{
						common_random_access_iterator_interface_ptr()->debug_item_cptr_if_available()
						, common_random_access_iterator_interface_ptr()->debug_iterator_index_if_available()
#ifdef MSE_HAS_CXX20
						, common_random_access_iterator_interface_ptr()->debug_sequence_span_if_available()
#else // MSE_HAS_CXX20
						, common_random_access_iterator_interface_ptr()->debug_start_of_sequence_cptr_if_available()
#endif // MSE_HAS_CXX20
					};
				}

				mse::us::impl::ns_any::any m_any_random_access_iterator = mse::TRAIterator<mse::TRefCountingPointer<std::array<_Ty, 0> > >(mse::TRefCountingPointer<std::array<_Ty, 0> >(), 0);

				MSE_IF_DEBUG(mutable CDebugValues m_stale_debug_values;)

				void update_debug_values() const {
					MSE_IF_DEBUG(m_stale_debug_values = updated_debug_values();)
				}

				MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_any_random_access_iterator, contained_any);

				MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1;

				template <typename _Ty2> friend class TAnyRandomAccessIteratorBase;
				template <typename _Ty2> friend class TAnyRandomAccessConstIteratorBase;
				template <typename _Ty2> friend class TNullableAnyRandomAccessIteratorBase;
			};

			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(const TAnyRandomAccessIteratorBase<_Ty>& operand) {
				return mse::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_any_random_access_iterator;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyRandomAccessIteratorBase<_Ty>& operand) {
				return mse::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_any_random_access_iterator;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyRandomAccessIteratorBase<_Ty>&& operand) {
				return mse::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_any_random_access_iterator;
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(const TAnyRandomAccessIteratorBase<_Ty>& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType> >(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_random_access_iterator };
				}
				return retval_t{};
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(TAnyRandomAccessIteratorBase<_Ty>&& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType> >(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessIterator<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_random_access_iterator };
				}
				return retval_t{};
			}


			template <typename _Ty>
			class TAnyRandomAccessConstIteratorBase : public TRandomAccessConstIteratorStdBase<_Ty> {
			public:
				typedef TRandomAccessConstIteratorStdBase<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);
				typedef TAnyRandomAccessConstIteratorBase _Myt;

				TAnyRandomAccessConstIteratorBase(const TAnyRandomAccessConstIteratorBase& src) : m_any_random_access_const_iterator(src.m_any_random_access_const_iterator) {}
				TAnyRandomAccessConstIteratorBase(const TAnyRandomAccessIteratorBase< _Ty>& src) : m_any_random_access_const_iterator([&]() {
						/* The value stored in src.m_any_random_access_iterator should be a TCommonizedRandomAccessIterator<>, but the values we store in 
						m_any_random_access_const_iterator need ot be TCommonizedRandomAccessConstIterator<>s, so we'll request 
						src.m_any_random_access_iterator to provide the converted value we need. */
						auto common_ra_iter_interface_cptr = static_cast<const TCommonRandomAccessIteratorInterface<_Ty>*>(src.m_any_random_access_iterator.storage_address());
						if (common_ra_iter_interface_cptr) {
							auto maybe_any1 = common_ra_iter_interface_cptr->as_a_wrapped_TCommonizedRandomAccessConstIterator_if_available();
							if (maybe_any1.has_value()) {
								return maybe_any1.value();
							}
						}
						assert(false);
						return src.m_any_random_access_iterator; 
					}()) {}
				TAnyRandomAccessConstIteratorBase(const _Ty arr[]) : m_any_random_access_const_iterator(TCommonizedRandomAccessConstIterator<const _Ty, const _Ty*>(arr)) {}

				template <typename _TRandomAccessConstIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
					(!std::is_convertible<_TRandomAccessConstIterator1 const*, TAnyRandomAccessIteratorBase<_Ty> const*>::value)
					&& (!std::is_convertible<_TRandomAccessConstIterator1 const*, TAnyRandomAccessConstIteratorBase<_Ty> const*>::value)
					&& (MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessConstIterator1, const _Ty) || MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessConstIterator1, _Ty))
					&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessConstIterator1>::value)
				> MSE_IMPL_EIS >
				TAnyRandomAccessConstIteratorBase(const _TRandomAccessConstIterator1& random_access_const_iterator) : m_any_random_access_const_iterator(constructor_helper1(typename std::is_base_of< TAnyRandomAccessConstIteratorBase<_Ty>, _TRandomAccessConstIterator1>::type(), random_access_const_iterator)) {}

				friend void swap(TAnyRandomAccessConstIteratorBase& first, TAnyRandomAccessConstIteratorBase& second) {
					std::swap(first.m_any_random_access_const_iterator, second.m_any_random_access_const_iterator);
				}

				const _Ty& operator*() const {
					return (*(*common_random_access_const_iterator_interface_ptr()));
				}
				const _Ty* operator->() const {
					return common_random_access_const_iterator_interface_ptr()->operator->();
				}
				const_reference operator[](difference_type _Off) const {
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

#if !defined(MSE_HAS_CXX17)
				MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_GIVEN_SUBTRACTION(_Myt);
#else // !defined(MSE_HAS_CXX17)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				MSE_IMPL_LESS_THAN_OPERATOR_DECLARATION(TAnyRandomAccessConstIteratorBase);
				MSE_IMPL_GREATER_THAN_OPERATOR_DECLARATION(TAnyRandomAccessConstIteratorBase);
				MSE_IMPL_GREATER_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TAnyRandomAccessConstIteratorBase);
				MSE_IMPL_LESS_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TAnyRandomAccessConstIteratorBase);

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
				friend std::strong_ordering operator<=>(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) { return (((_Left_cref)-(_Right_cref)) <=> 0); } /* this is the proper order, right? */

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
				friend bool operator==(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					const TCommonRandomAccessConstIteratorInterface<_Ty>* LHS_craci_ptr = nullptr;
					const TCommonRandomAccessConstIteratorInterface<_Ty>* RHS_craci_ptr = nullptr;

					MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value) {
						LHS_craci_ptr = _Left_cref.common_random_access_const_iterator_interface_ptr();
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TLHSIterator_ecwapt>::value) {
						const TCommonRandomAccessConstIteratorInterface<mse::impl::remove_const_t<_Ty> >* LHS_crai_rc_ptr = s_common_random_access_const_iterator_interface_ptr(_Left_cref);
						/* We're relying on TCommonRandomAccessConstIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<const T> being "structurally identical". */
						LHS_craci_ptr = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(LHS_crai_rc_ptr);
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessIteratorBase<_Ty>, TLHSIterator_ecwapt>::value) {
						const TCommonRandomAccessIteratorInterface<_Ty>* LHS_crai_ptr = _Myt::s_common_random_access_iterator_interface_ptr(_Left_cref);
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<T> being "structurally identical". */
						LHS_craci_ptr = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(LHS_crai_ptr);
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TLHSIterator_ecwapt>::value) {
						const TCommonRandomAccessIteratorInterface<mse::impl::remove_const_t<_Ty> >* LHS_crai_rc_ptr = s_common_random_access_iterator_interface_ptr(_Left_cref);
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessIteratorInterface<const T> being "structurally identical". */
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<T> being "structurally identical". */
						LHS_craci_ptr = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(LHS_crai_rc_ptr);
					}

					MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TRHSIterator_ecwapt>::value) {
						RHS_craci_ptr = _Right_cref.common_random_access_const_iterator_interface_ptr();
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> >, TRHSIterator_ecwapt>::value) {
						const TCommonRandomAccessConstIteratorInterface<mse::impl::remove_const_t<_Ty> >* RHS_crai_rc_ptr = s_common_random_access_const_iterator_interface_ptr(_Right_cref);
						/* We're relying on TCommonRandomAccessConstIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<const T> being "structurally identical". */
						RHS_craci_ptr = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(RHS_crai_rc_ptr);
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessIteratorBase<_Ty>, TRHSIterator_ecwapt>::value) {
						const TCommonRandomAccessIteratorInterface<_Ty>* RHS_crai_ptr = _Myt::s_common_random_access_iterator_interface_ptr(_Right_cref);
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<T> being "structurally identical". */
						RHS_craci_ptr = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(RHS_crai_ptr);
					} else MSE_IF_CONSTEXPR(std::is_base_of<TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TRHSIterator_ecwapt>::value) {
						const TCommonRandomAccessIteratorInterface<mse::impl::remove_const_t<_Ty> >* RHS_crai_rc_ptr = s_common_random_access_iterator_interface_ptr(_Right_cref);
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessIteratorInterface<const T> being "structurally identical". */
						/* We're relying on TCommonRandomAccessIteratorInterface<T> and TCommonRandomAccessConstIteratorInterface<T> being "structurally identical". */
						RHS_craci_ptr = reinterpret_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(RHS_crai_rc_ptr);
					}

					mse::optional<_Myt> maybe_adj_LHS;
					mse::optional<_Myt> maybe_adj_RHS;
					if (!LHS_craci_ptr) {
						maybe_adj_LHS = _Myt(_Right_cref);
						LHS_craci_ptr = maybe_adj_LHS.value().common_random_access_const_iterator_interface_ptr();
						assert(LHS_craci_ptr);
					}
					if (!RHS_craci_ptr) {
						maybe_adj_RHS = _Myt(_Right_cref);
						RHS_craci_ptr = maybe_adj_RHS.value().common_random_access_const_iterator_interface_ptr();
						assert(RHS_craci_ptr);
					}
					return (*LHS_craci_ptr) == (*RHS_craci_ptr);
				}
#endif // !defined(MSE_HAS_CXX17)

				TAnyRandomAccessConstIteratorBase& operator=(TAnyRandomAccessConstIteratorBase _Right) {
					swap(*this, _Right);
					return (*this);
				}

			protected:
				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				template <typename _TRandomAccessConstIterator1>
				static auto construction_helper3(std::false_type, const _TRandomAccessConstIterator1& src) {
					return TCommonizedRandomAccessConstIterator<_Ty, _TRandomAccessConstIterator1>(src);
				}
				template <typename _Ty2>
				static auto construction_helper3(std::true_type, const _Ty2& src) {
					/* argument seems to be a TAnyRandomAccessConstIteratorBase<std::remove_const_t<_Ty> > */
					//return reinterpret_cast<const TAnyRandomAccessConstIteratorBase&>(src).m_iter;
					return src.m_any_random_access_const_iterator;
				}

				template <typename _TRandomAccessConstIterator1>
				auto& constructor_helper1(std::true_type, const _TRandomAccessConstIterator1& random_access_const_iterator) {
					return reinterpret_cast<const TAnyRandomAccessConstIteratorBase&>(random_access_const_iterator).m_any_random_access_const_iterator;
				}
				template <typename _TRandomAccessConstIterator1>
				auto constructor_helper1(std::false_type, const _TRandomAccessConstIterator1& random_access_const_iterator) {
					return construction_helper3(typename std::integral_constant<bool, std::is_const<_Ty>::value && std::is_base_of<TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, _TRandomAccessConstIterator1>::value>::type(), random_access_const_iterator);
				}

				TCommonRandomAccessConstIteratorInterface<_Ty>* common_random_access_const_iterator_interface_ptr() {
					auto retval = static_cast<TCommonRandomAccessConstIteratorInterface<_Ty>*>(m_any_random_access_const_iterator.storage_address());
					assert(nullptr != retval);
					return retval;
				}
				const TCommonRandomAccessConstIteratorInterface<_Ty>* common_random_access_const_iterator_interface_ptr() const {
					auto retval = static_cast<const TCommonRandomAccessConstIteratorInterface<_Ty>*>(m_any_random_access_const_iterator.storage_address());
					assert(nullptr != retval);
					return retval;
				}
				template <typename _Ty2>
				static auto* s_common_random_access_iterator_interface_ptr(TAnyRandomAccessIteratorBase<_Ty2> const& iter) { return iter.common_random_access_iterator_interface_ptr(); }
				template <typename _Ty2>
				static auto* s_common_random_access_const_iterator_interface_ptr(TAnyRandomAccessConstIteratorBase<_Ty2> const& iter) { return iter.common_random_access_const_iterator_interface_ptr(); }

				struct CDebugValues {
					optional1<_Ty const*> maybe_debug_item_cptr;
					optional1<size_t> maybe_debug_iterator_index;
#ifdef MSE_HAS_CXX20
					typedef mse::impl::conditional_t<mse::impl::is_complete_type<_Ty>::value, std::span<_Ty const>, _Ty const*> span1_t;
					optional1<span1_t> maybe_debug_sequence_span;
#else // MSE_HAS_CXX20
					optional1<_Ty const*> maybe_debug_start_of_sequence_cptr;
#endif // MSE_HAS_CXX20
				};
				CDebugValues updated_debug_values() const {
					return CDebugValues{
						common_random_access_const_iterator_interface_ptr()->debug_item_cptr_if_available()
						, common_random_access_const_iterator_interface_ptr()->debug_iterator_index_if_available()
#ifdef MSE_HAS_CXX20
						, common_random_access_const_iterator_interface_ptr()->debug_sequence_span_if_available()
#else // MSE_HAS_CXX20
						, common_random_access_const_iterator_interface_ptr()->debug_start_of_sequence_cptr_if_available()
#endif // MSE_HAS_CXX20
					};
				}

				MSE_IF_DEBUG(mutable CDebugValues m_stale_debug_values;)

					void update_debug_values() const {
					MSE_IF_DEBUG(m_stale_debug_values = updated_debug_values();)
				}

				mse::us::impl::ns_any::any m_any_random_access_const_iterator = []() { 
						assert(false);
						auto default_iter_value = mse::TRAIterator<mse::TRefCountingPointer<std::array<const _Ty, 0> > >(mse::TRefCountingPointer<std::array<const _Ty, 0> >(), 0);
						return TCommonizedRandomAccessConstIterator<const _Ty, decltype(default_iter_value)>(default_iter_value);
					}();

				MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_any_random_access_const_iterator, contained_any);

				MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1;

				template <typename _Ty2> friend class TAnyRandomAccessConstIteratorBase;
			};

			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(const TAnyRandomAccessConstIteratorBase<_Ty>& operand) {
				return mse::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_any_random_access_const_iterator;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyRandomAccessConstIteratorBase<_Ty>& operand) {
				return mse::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_any_random_access_const_iterator;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyRandomAccessConstIteratorBase<_Ty>&& operand) {
				return mse::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_any_random_access_const_iterator;
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(const TAnyRandomAccessConstIteratorBase<_Ty>& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType> >(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(operand)).m_random_access_const_iterator };
				}
				return retval_t{};
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(TAnyRandomAccessConstIteratorBase<_Ty>&& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType> >(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedRandomAccessConstIterator<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_random_access_const_iterator };
				}
				return retval_t{};
			}
		}
	}

	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(const mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>&& operand) {
		return mse::us::impl::any_cast<ValueType>(MSE_FWD(operand));
	}
	template<typename ValueType, typename _Ty/* = mse::impl::target_type<ValueType> */>
	inline auto maybe_any_cast(const mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline auto maybe_any_cast(mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>&& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(MSE_FWD(operand));
	}

	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(const mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>&& operand) {
		return mse::us::impl::any_cast<ValueType>(MSE_FWD(operand));
	}
	template<typename ValueType, typename _Ty/* = mse::impl::target_type<ValueType> */>
	inline auto maybe_any_cast(const mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline auto maybe_any_cast(mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>&& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(MSE_FWD(operand));
	}


	template <typename _Ty>
	class TXScopeAnyRandomAccessIterator : public us::impl::TAnyRandomAccessIteratorBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef us::impl::TAnyRandomAccessIteratorBase<_Ty> base_class;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		MSE_USING(TXScopeAnyRandomAccessIterator, base_class);

		MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeAnyRandomAccessIterator);

		void async_not_shareable_and_not_passable_tag() const {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		TXScopeAnyRandomAccessIterator& operator=(const base_class& _Right) {
			base_class::operator=(_Right);
			return (*this);
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TAnyRandomAccessIterator<_Ty>;
	};

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_any_random_access_iterator(const _Ty & x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyRandomAccessIterator<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_random_access_iterator(_Ty && x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyRandomAccessIterator<_Tx2>(MSE_FWD(x));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_random_access_iterator)

	template <typename _Ty>
	class TXScopeAnyRandomAccessConstIterator : public us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		typedef us::impl::TAnyRandomAccessConstIteratorBase<_Ty> base_class;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		MSE_USING(TXScopeAnyRandomAccessConstIterator, base_class);

		MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeAnyRandomAccessConstIterator);

		void async_not_shareable_and_not_passable_tag() const {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		TXScopeAnyRandomAccessConstIterator& operator=(const base_class& _Right) {
			base_class::operator=(_Right);
			return (*this);
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TAnyRandomAccessConstIterator<_Ty>;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_ITERATOR_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TXScopeAnyRandomAccessIterator, mse::TXScopeAnyRandomAccessConstIterator);

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_any_random_access_const_iterator(const _Ty & x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyRandomAccessConstIterator<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_random_access_const_iterator(_Ty && x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeAnyRandomAccessConstIterator<_Tx2>(MSE_FWD(x));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_random_access_const_iterator)

	template <typename _Ty>
	class TAnyRandomAccessIterator : public us::impl::TAnyRandomAccessIteratorBase<_Ty> {
	public:
		typedef us::impl::TAnyRandomAccessIteratorBase<_Ty> base_class;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		TAnyRandomAccessIterator(const TAnyRandomAccessIterator& src) : base_class(src) {}
		TAnyRandomAccessIterator(_Ty arr[]) : base_class(arr) {}
		template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TRandomAccessIterator1, TAnyRandomAccessIterator>::value)
			&& (!std::is_base_of<TAnyRandomAccessConstIterator<_Ty>, _TRandomAccessIterator1>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
			&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
			&& (mse::impl::is_potentially_not_xscope<_TRandomAccessIterator1>::value)
			> MSE_IMPL_EIS >
		TAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRandomAccessIterator1)") : base_class(random_access_iterator) {
			mse::impl::T_valid_if_not_an_xscope_type<_TRandomAccessIterator1>();
		}

		MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TAnyRandomAccessIterator);

		TAnyRandomAccessIterator& operator=(TAnyRandomAccessIterator _Right) { base_class::operator=(_Right); return (*this); }

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
	};

	template <typename _Tx = void, typename _Ty = void>
	auto make_any_random_access_iterator(const _Ty & x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyRandomAccessIterator<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_any_random_access_iterator(_Ty && x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyRandomAccessIterator<_Tx2>(MSE_FWD(x));
	}

	template <typename _Ty>
	class TAnyRandomAccessConstIterator : public us::impl::TAnyRandomAccessConstIteratorBase<_Ty> {
	public:
		typedef us::impl::TAnyRandomAccessConstIteratorBase<_Ty> base_class;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		TAnyRandomAccessConstIterator(const TAnyRandomAccessConstIterator& src) : base_class(src) {}
		TAnyRandomAccessConstIterator(const TAnyRandomAccessIterator<_Ty>& src) : base_class(src) {}
		TAnyRandomAccessConstIterator(const _Ty arr[]) : base_class(arr) {}

		template <typename _TRandomAccessConstIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TRandomAccessConstIterator1, TAnyRandomAccessConstIterator<_Ty>>::value)
			&& (!std::is_base_of<TAnyRandomAccessIterator<_Ty>, _TRandomAccessConstIterator1>::value)
			&& (MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessConstIterator1, const _Ty) || MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessConstIterator1, _Ty))
			&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessConstIterator1>::value)
			&& (mse::impl::is_potentially_not_xscope<_TRandomAccessConstIterator1>::value)
			> MSE_IMPL_EIS >
		TAnyRandomAccessConstIterator(const _TRandomAccessConstIterator1& random_access_const_iterator) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRandomAccessConstIterator1)") : base_class(random_access_const_iterator) {
			mse::impl::T_valid_if_not_an_xscope_type<_TRandomAccessConstIterator1>();
#if (!defined(MSE_SOME_NON_XSCOPE_POINTER_TYPE_IS_DISABLED)) && (!defined(MSE_SAFER_SUBSTITUTES_DISABLED))
			//mse::impl::T_valid_if_not_an_xscope_type<_TRandomAccessConstIterator1>();
#endif // (!defined(MSE_SOME_NON_XSCOPE_POINTER_TYPE_IS_DISABLED)) && (!defined(MSE_SAFER_SUBSTITUTES_DISABLED))
		}

		MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TAnyRandomAccessConstIterator);

		TAnyRandomAccessConstIterator& operator=(const TAnyRandomAccessConstIterator& _Right) { base_class::operator=(_Right); return (*this); }

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_ITERATOR_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TAnyRandomAccessIterator, mse::TAnyRandomAccessConstIterator);

	template <typename _Tx = void, typename _Ty = void>
	auto make_any_random_access_const_iterator(const _Ty & x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyRandomAccessConstIterator<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_any_random_access_const_iterator(_Ty && x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TAnyRandomAccessConstIterator<_Tx2>(MSE_FWD(x));
	}


	template <typename _Ty>
	class TXScopeAnyRandomAccessSection : public TXScopeRandomAccessSection<TXScopeAnyRandomAccessIterator<_Ty>> {
	public:
		typedef TXScopeRandomAccessSection<TXScopeAnyRandomAccessIterator<_Ty>> base_class;
		typedef TXScopeAnyRandomAccessIterator<_Ty> iterator_type;
		MSE_INHERITED_RANDOM_ACCESS_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		MSE_USING(TXScopeAnyRandomAccessSection, base_class);

		void async_not_shareable_and_not_passable_tag() const {}
	};

	template <typename _Ty, class... _Args>
	auto make_xscope_any_random_access_section(const _Ty& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_random_access_section(arg1, std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessSection<typename ra_section_t::value_type>(make_xscope_random_access_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_random_access_section(_Ty&& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_random_access_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessSection<typename ra_section_t::value_type>(make_xscope_random_access_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_random_access_section)

	template <typename _Ty>
	class TAnyRandomAccessSection : public TRandomAccessSection<TAnyRandomAccessIterator<_Ty>> {
	public:
		typedef TRandomAccessSection<TAnyRandomAccessIterator<_Ty>> base_class;
		MSE_USING(TAnyRandomAccessSection, base_class);

		void async_not_shareable_and_not_passable_tag() const {}
	};

	template <class... _Args, typename _TRASection = decltype(make_random_access_section(std::forward<_Args>(std::declval<_Args>())...))>
	auto make_any_random_access_section(_Args&& ... _Ax) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRASection)") {
		return TAnyRandomAccessSection<typename _TRASection::value_type>(make_random_access_section(std::forward<_Args>(_Ax)...));
	}

	template <typename _Ty>
	class TXScopeAnyRandomAccessConstSection : public TXScopeRandomAccessConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>> {
	public:
		typedef TXScopeRandomAccessConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>> base_class;
		MSE_USING(TXScopeAnyRandomAccessConstSection, base_class);

		void async_not_shareable_and_not_passable_tag() const {}
	};

	template <typename _Ty, class... _Args>
	auto make_xscope_any_random_access_const_section(const _Ty& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_random_access_const_section(arg1, std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessConstSection<typename ra_section_t::value_type>(make_xscope_random_access_const_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_random_access_const_section(_Ty&& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_random_access_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessConstSection<typename ra_section_t::value_type>(make_xscope_random_access_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_random_access_const_section)

	template <typename _Ty>
	class TAnyRandomAccessConstSection : public TRandomAccessConstSection<TAnyRandomAccessConstIterator<_Ty>> {
	public:
		typedef TRandomAccessConstSection<TAnyRandomAccessConstIterator<_Ty>> base_class;
		MSE_USING(TAnyRandomAccessConstSection, base_class);

		void async_not_shareable_and_not_passable_tag() const {}
	};

	template <class... _Args, typename _TRASection = decltype(make_random_access_const_section(std::forward<_Args>(std::declval<_Args>())...))>
	auto make_any_random_access_const_section(_Args&& ... _Ax) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRASection)") {
		return TAnyRandomAccessConstSection<typename _TRASection::value_type>(make_random_access_const_section(std::forward<_Args>(_Ax)...));
	}

	namespace rsv {
		template<typename _Ty>
		class TFParam<mse::TXScopeAnyRandomAccessConstSection<_Ty> > : public TXScopeAnyRandomAccessConstSection<_Ty> {
		public:
			typedef TXScopeAnyRandomAccessConstSection<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				mse::impl::conditional_t<
				mse::rsv::impl::is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue<_TRALoneParam>::value
				|| mse::impl::is_instantiation_of<_TRALoneParam, TXScopeCagedRandomAccessConstSectionToRValue>::value
				, std::true_type, std::false_type>()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeFixedConstPointerFParam<_Ty2>(MSE_FWD(param));
			}
			template<typename _TRAIterator>
			auto constructor_helper1(std::true_type, TXScopeCagedRandomAccessConstSectionToRValue<_TRAIterator>&& param) {
				return TXScopeRandomAccessConstSectionFParam<_TRAIterator>(MSE_FWD(param));
			}
			template<typename _TRALoneParam>
			auto constructor_helper1(std::false_type, _TRALoneParam&& param) { return std::forward<_TRALoneParam>(param); }

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}


	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	using TXScopeAnyStringSection = TXScopeStringSection<TXScopeAnyRandomAccessIterator<_Ty>, _Traits>;

	template <typename _Ty, class... _Args>
	auto make_xscope_any_string_section(const _Ty& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_string_section(arg1, std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessSection<typename ra_section_t::value_type>(make_xscope_string_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_string_section(_Ty&& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_string_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessSection<typename ra_section_t::value_type>(make_xscope_string_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_string_section)

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	using TAnyStringSection = TStringSection<TAnyRandomAccessIterator<_Ty>, _Traits>;

	template <class... _Args, typename _TStrSection = decltype(make_string_section(std::forward<_Args>(std::declval<_Args>())...))>
	auto make_any_string_section(_Args&& ... _Ax) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TStrSection)") {
		return TAnyStringSection<typename _TStrSection::value_type, typename _TStrSection::traits_type>(make_string_section(std::forward<_Args>(_Ax)...));
	}

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TXScopeAnyStringConstSection : public TXScopeStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TXScopeStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		MSE_USING(TXScopeAnyStringConstSection, base_class);
		TXScopeAnyStringConstSection() : base_class(s_default_string_siptr()) {}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		static auto s_default_string_siptr() { typedef mse::nii_basic_string<nonconst_value_type, _Traits> str_t; MSE_DECLARE_STATIC_IMMUTABLE(str_t) s_default_string; return &s_default_string; }
	};

	template <typename _Ty, class... _Args>
	auto make_xscope_any_string_const_section(const _Ty& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_string_const_section(arg1, std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessConstSection<typename ra_section_t::value_type>(make_xscope_string_const_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_string_const_section(_Ty&& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_string_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessConstSection<typename ra_section_t::value_type>(make_xscope_string_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_string_const_section)

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TAnyStringConstSection : public TStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		MSE_INHERITED_RANDOM_ACCESS_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		MSE_USING(TAnyStringConstSection, base_class);
		TAnyStringConstSection() : base_class(s_default_string_siptr()) {}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		static auto s_default_string_siptr() { typedef mse::nii_basic_string<nonconst_value_type, _Traits> str_t; MSE_DECLARE_STATIC_IMMUTABLE(str_t) s_default_string; return &s_default_string; }
	};

	template <class... _Args, typename _TStrSection = decltype(make_string_const_section(std::forward<_Args>(std::declval<_Args>())...))>
	auto make_any_string_const_section(_Args&& ... _Ax) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TStrSection)") {
		return TAnyStringConstSection<typename _TStrSection::value_type, typename _TStrSection::traits_type>(make_string_const_section(std::forward<_Args>(_Ax)...));
	}

	namespace rsv {
		template<typename _Ty>
		class TFParam<mse::TXScopeAnyStringConstSection<_Ty> > : public TXScopeAnyStringConstSection<_Ty> {
		public:
			typedef TXScopeAnyStringConstSection<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);

			template<typename _TRALoneParam>
			TFParam(_TRALoneParam&& src) : base_class(constructor_helper1(
#ifndef MSE_SCOPEPOINTER_DISABLED
				mse::impl::conditional_t<
				mse::rsv::impl::is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue<_TRALoneParam>::value
				|| std::is_base_of<mse::us::impl::CagedStringSectionTagBase, _TRALoneParam>::value
				//|| mse::impl::is_instantiation_of<_TRALoneParam, mse::TXScopeCagedStringConstSectionToRValue>::value
				, std::true_type, std::false_type>()
#else //!MSE_SCOPEPOINTER_DISABLED
				std::false_type()
#endif //!MSE_SCOPEPOINTER_DISABLED
				, std::forward<_TRALoneParam>(src))) {}

			template<typename _TRALoneParam>
			TFParam(const _TRALoneParam& src) : base_class(src) {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			template<typename _Ty2>
			auto constructor_helper1(std::true_type, TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& param) {
				return TXScopeFixedConstPointerFParam<_Ty2>(MSE_FWD(param));
			}
			template<typename _TRAIterator>
			auto constructor_helper1(std::true_type, TXScopeCagedStringConstSectionToRValue<_TRAIterator>&& param) {
				return TXScopeStringConstSectionFParam<_TRAIterator>(MSE_FWD(param));
			}
			template<typename _TRALoneParam>
			auto constructor_helper1(std::false_type, _TRALoneParam&& param) { return std::forward<_TRALoneParam>(param); }

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}


	namespace impl {
		template<typename _Ty, typename _TRALoneParam, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_same<std::basic_string<_Ty>, mse::impl::remove_const_t<_TRALoneParam> >::value)
			&& (!std::is_same<mse::TXScopeObj<std::basic_string<_Ty> >, mse::impl::remove_const_t<_TRALoneParam> >::value)
			> MSE_IMPL_EIS >
		void T_valid_if_not_an_std_basic_string_msepoly() {}

		template<typename _Ty, typename _TPtr>
		void T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly_helper(std::true_type) {
			T_valid_if_not_an_std_basic_string_msepoly<_Ty, mse::impl::remove_reference_t<decltype(*std::declval<_TPtr>())> >();
		}
		template<typename _Ty, typename _TRALoneParam>
		void T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly_helper(std::false_type) {}

		template<typename _Ty, typename _TRALoneParam>
		void T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly() {
			T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly_helper<_Ty, _TRALoneParam>(typename IsDereferenceable_pb<_TRALoneParam>::type());
		}

		template<typename _Ty, typename _TRALoneParam>
		void T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly() {
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			mse::impl::T_valid_if_not_a_native_pointer_msemsestring<_TRALoneParam>();
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
#ifndef MSE_MSTDSTRING_DISABLED
			T_valid_if_not_an_std_basic_string_msepoly<_Ty, _TRALoneParam>();
			T_valid_if_not_a_pointer_to_an_std_basic_string_msepoly<_Ty, _TRALoneParam>();
#endif /*!MSE_MSTDSTRING_DISABLED*/
		}
	}

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TXScopeAnyNRPStringSection : public TXScopeNRPStringSection<TXScopeAnyRandomAccessIterator<_Ty>, _Traits> {
	public:
		typedef TXScopeNRPStringSection<TXScopeAnyRandomAccessIterator<_Ty>, _Traits> base_class;
		MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		//MSE_USING(TXScopeAnyNRPStringSection, base_class);
		TXScopeAnyNRPStringSection(const TXScopeAnyNRPStringSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TXScopeAnyNRPStringSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TXScopeAnyNRPStringSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			/* Note: Use TXScopeAnyNRPStringConstSection instead if referencing a string literal. */
			mse::impl::T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
#ifndef MSE_MSTDSTRING_DISABLED
			mse::impl::T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
			mse::impl::T_valid_if_not_an_std_basic_string_xscope_iterator_msemsestring<_TRAIterator>();
#endif /*!MSE_MSTDSTRING_DISABLED*/
		}
		template <typename _TRALoneParam>
		TXScopeAnyNRPStringSection(const _TRALoneParam& param) : base_class(param) {
			impl::T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}

		void async_not_shareable_and_not_passable_tag() const {}
	};

	template <typename _Ty, class... _Args>
	auto make_xscope_any_nrp_string_section(const _Ty& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_nrp_string_section(arg1, std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessSection<typename ra_section_t::value_type>(make_xscope_nrp_string_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_nrp_string_section(_Ty&& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_nrp_string_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessSection<typename ra_section_t::value_type>(make_xscope_nrp_string_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_nrp_string_section)

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TAnyNRPStringSection : public TNRPStringSection<TAnyRandomAccessIterator<_Ty>, _Traits> {
	public:
		typedef TNRPStringSection<TAnyRandomAccessIterator<_Ty>, _Traits> base_class;
		MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		//MSE_USING(TAnyNRPStringSection, base_class);
		TAnyNRPStringSection(const TAnyNRPStringSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TAnyNRPStringSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TAnyNRPStringSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
			/* Note: Use TXScopeAnyNRPStringConstSection instead if referencing a string literal. */
			mse::impl::T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			mse::impl::T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
		}
		template <typename _TRALoneParam>
		TAnyNRPStringSection(const _TRALoneParam& param) : base_class(param) {
			impl::T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}

		void async_not_shareable_and_not_passable_tag() const {}
	};

	template <class... _Args, typename _TStrSection = decltype(make_nrp_string_section(std::forward<_Args>(std::declval<_Args>())...))>
	auto make_any_nrp_string_section(_Args&& ... _Ax) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TStrSection)") {
		return TAnyNRPStringSection<typename _TStrSection::value_type, typename _TStrSection::traits_type>(make_nrp_string_section(std::forward<_Args>(_Ax)...));
	}

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TXScopeAnyNRPStringConstSection : public TXScopeNRPStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TXScopeNRPStringConstSection<TXScopeAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		//MSE_USING(TXScopeAnyNRPStringConstSection, base_class);
		TXScopeAnyNRPStringConstSection(const TXScopeAnyNRPStringConstSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TXScopeAnyNRPStringConstSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TXScopeAnyNRPStringConstSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			/* Note: Use TXScopeAnyNRPStringConstSection instead if referencing a string literal. */
			mse::impl::T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			mse::impl::T_valid_if_not_an_std_basic_string_xscope_iterator_msemsestring<_TRAIterator>();
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
#ifndef MSE_MSTDSTRING_DISABLED
			mse::impl::T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
#endif /*!MSE_MSTDSTRING_DISABLED*/
		}
		template <typename _TRALoneParam>
		TXScopeAnyNRPStringConstSection(const _TRALoneParam& param) : base_class(param) {
			impl::T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}
		TXScopeAnyNRPStringConstSection() : base_class(s_default_string_siptr()) {}

		template<size_t Tn, typename = typename std::enable_if<1 <= Tn>::type>
		explicit TXScopeAnyNRPStringConstSection(const value_type(&presumed_string_literal)[Tn]) : base_class(presumed_string_literal) {}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		/* Construction from a const native array is publicly supported (only) because string literals are const
		native arrays. We do not want construction from a non-const native array to be publicly supported. */
		template<size_t Tn>
		explicit TXScopeAnyNRPStringConstSection(mse::impl::remove_const_t<value_type>(&native_array)[Tn]) : base_class(native_array) {}

		static auto s_default_string_siptr() { typedef mse::nii_basic_string<nonconst_value_type, _Traits> str_t; MSE_DECLARE_STATIC_IMMUTABLE(str_t) s_default_string; return &s_default_string; }
	};

	template <typename _Ty, class... _Args>
	auto make_xscope_any_nrp_string_const_section(const _Ty& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_nrp_string_const_section(arg1, std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessConstSection<typename ra_section_t::value_type>(make_xscope_nrp_string_const_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_any_nrp_string_const_section(_Ty&& arg1, _Args&& ... _Ax) {
		typedef decltype(make_xscope_nrp_string_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...)) ra_section_t;
		return TXScopeAnyRandomAccessConstSection<typename ra_section_t::value_type>(make_xscope_nrp_string_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_nrp_string_const_section)

	template <typename _Ty = char, class _Traits = std::char_traits<_Ty> >
	class TAnyNRPStringConstSection : public TNRPStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> {
	public:
		typedef TNRPStringConstSection<TAnyRandomAccessConstIterator<_Ty>, _Traits> base_class;
		MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

		//MSE_USING(TAnyNRPStringConstSection, base_class);
		TAnyNRPStringConstSection(const TAnyNRPStringConstSection& src) : base_class(static_cast<const base_class&>(src)) {}
		TAnyNRPStringConstSection(const base_class& src) : base_class(src) {}
		template <typename _TRAIterator>
		TAnyNRPStringConstSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {
			mse::impl::T_valid_if_not_a_native_pointer_msemsestring<_TRAIterator>();
			mse::impl::T_valid_if_not_an_std_basic_string_iterator_msemsestring<_TRAIterator>();
		}
		template <typename _TRALoneParam>
		TAnyNRPStringConstSection(const _TRALoneParam& param) : base_class(param) {
			impl::T_valid_if_not_an_unsupported_NRPStringSection_lone_parameter_msepoly<_Ty, _TRALoneParam>();
		}
		TAnyNRPStringConstSection() : base_class(s_default_string_siptr()) {}

		template<size_t Tn, typename = typename std::enable_if<1 <= Tn>::type>
		explicit TAnyNRPStringConstSection(const value_type(&presumed_string_literal)[Tn]) : base_class(presumed_string_literal) {}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		/* Construction from a const native array is publicly supported (only) because string literals are const
		native arrays. We do not want construction from a non-const native array to be publicly supported. */
		template<size_t Tn>
		explicit TAnyNRPStringConstSection(mse::impl::remove_const_t<value_type>(&native_array)[Tn]) : base_class(native_array) {}

		static auto s_default_string_siptr() { typedef mse::nii_basic_string<nonconst_value_type, _Traits> str_t; MSE_DECLARE_STATIC_IMMUTABLE(str_t) s_default_string; return &s_default_string; }
	};

	template <class... _Args, typename _TStrSection = decltype(make_nrp_string_const_section(std::forward<_Args>(std::declval<_Args>())...))>
	auto make_any_nrp_string_const_section(_Args&& ... _Ax) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TStrSection)") {
		return TAnyNRPStringConstSection<typename _TStrSection::value_type, typename _TStrSection::traits_type>(make_nrp_string_const_section(std::forward<_Args>(_Ax)...));
	}
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
			MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

			MSE_USING(basic_string_view, base_class);
			explicit basic_string_view(const char* sz) : base_class(sz, _Traits::length(sz)) {}
		};

		template <typename _Ty, class _Traits = std::char_traits<_Ty> >
		class xscope_basic_string_view : public TXScopeAnyStringConstSection<_Ty, _Traits> {
		public:
			typedef TXScopeAnyStringConstSection<_Ty, _Traits> base_class;
			MSE_INHERITED_STRING_SECTION_MEMBER_TYPE_AND_NPOS_DECLARATIONS(base_class);

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

		template <typename _Ty, class... _Args>
		auto make_xscope_string_view(const _Ty& arg1, _Args&& ... _Ax) {
			return xscope_string_view(make_xscope_string_const_section(arg1, std::forward<_Args>(_Ax)...));
		}
		template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
		auto make_xscope_string_view(_Ty&& arg1, _Args&& ... _Ax) {
			return xscope_string_view(make_xscope_string_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
		}
		/* Overloads for rsv::TReturnableFParam<>. */
		MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_string_view)
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

	template <typename _Ty, class... _Args>
	auto make_xscope_nrp_string_view(const _Ty& arg1, _Args&& ... _Ax) {
		return xscope_nrp_string_view(make_xscope_nrp_string_const_section(arg1, std::forward<_Args>(_Ax)...));
	}
	template <typename _Ty, class... _Args, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_Ty)>
	auto make_xscope_nrp_string_view(_Ty&& arg1, _Args&& ... _Ax) {
		return xscope_nrp_string_view(make_xscope_nrp_string_const_section(MSE_FWD(arg1), std::forward<_Args>(_Ax)...));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_nrp_string_view);

	namespace impl {
		namespace ns_evaluates_to_false {
			template <typename _Ty>
			bool evaluates_to_false_helper1(std::false_type, _Ty const& x) {
				return false;
			}
			template <typename _Ty>
			bool evaluates_to_false_helper1(std::true_type, _Ty const& x) {
				return !bool(x);
			}
		}
		template <typename _Ty>
		bool evaluates_to_false(_Ty const& x) {
			return ns_evaluates_to_false::evaluates_to_false_helper1(typename std::is_constructible<bool, _Ty const&>::type(), x);
		}
	}

	template <typename _Ty> class TNullableAnyRandomAccessIterator;
	template <typename _Ty> class TXScopeNullableAnyRandomAccessIterator;
	template<typename _Ty2>
	TAnyRandomAccessIterator<_Ty2> not_null_from_nullable(const TNullableAnyRandomAccessIterator<_Ty2>& src);
	template<typename _Ty2>
	TXScopeAnyRandomAccessIterator<_Ty2> not_null_from_nullable(const TXScopeNullableAnyRandomAccessIterator<_Ty2>& src);

	namespace us {
		namespace impl {
			template <typename _Ty>
			class TNullableAnyRandomAccessIteratorBase {
			public:
				typedef mse::us::impl::TAnyRandomAccessIteratorBase<_Ty> base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);
				typedef TNullableAnyRandomAccessIteratorBase _Myt;
				TNullableAnyRandomAccessIteratorBase() : m_iter(mse::TRAIterator<mse::TRefCountingPointer<std::array<_Ty, 0> > >(mse::TRefCountingPointer<std::array<_Ty, 0> >(), 0)), m_is_null(true) {}
				TNullableAnyRandomAccessIteratorBase(const std::nullptr_t& src) : TNullableAnyRandomAccessIteratorBase() {}
				TNullableAnyRandomAccessIteratorBase(const TNullableAnyRandomAccessIteratorBase& src) = default;
				TNullableAnyRandomAccessIteratorBase(const base_class& src) : m_iter(src) {}
				explicit TNullableAnyRandomAccessIteratorBase(_Ty arr[]) : m_iter(arr), m_is_null(!bool(arr)) {}

				template <typename _Ty2>
				static auto s_construction_helper1(std::false_type, const _Ty2& src) {
					return src;
				}
				template <typename _Ty2>
				static auto s_construction_helper1(std::true_type, const _Ty2& src) {
					/* argument seems to be a TNullableAnyRandomAccessIteratorBase<std::remove_const_t<_Ty> > */
					//return reinterpret_cast<const TNullableAnyRandomAccessIteratorBase&>(src).m_iter;
					return src.m_iter;
				}

				template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
					(!std::is_convertible<_TRandomAccessIterator1, TNullableAnyRandomAccessIteratorBase>::value)
					&& (!std::is_base_of<_Myt, _TRandomAccessIterator1>::value)
					&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
					&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
					&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
				> MSE_IMPL_EIS >
				TNullableAnyRandomAccessIteratorBase(const _TRandomAccessIterator1& random_access_iterator)
					: m_iter(s_construction_helper1(typename std::integral_constant<bool, std::is_const<_Ty>::value && std::is_base_of<TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, _TRandomAccessIterator1>::value>::type(), random_access_iterator))
					, m_is_null(mse::impl::evaluates_to_false(random_access_iterator)) {}

				friend void swap(TNullableAnyRandomAccessIteratorBase& first, TNullableAnyRandomAccessIteratorBase& second) {
					std::swap(first.m_iter, second.m_iter);
					std::swap(first.m_is_null, second.m_is_null);
				}

				_Ty& operator*() const {
					if (m_is_null) { MSE_THROW(std::logic_error("attempting to derference a null value  - TNullableAnyRandomAccessIteratorBase")); }
					return *m_iter;
				}
				_Ty* operator->() const {
					if (m_is_null) { MSE_THROW(std::logic_error("attempting to derference a null value  - TNullableAnyRandomAccessIteratorBase")); }
					return m_iter.operator->();
				}
				reference operator[](difference_type _Off) const {
					if (m_is_null) { MSE_THROW(std::logic_error("attempting to derference a null value  - TNullableAnyRandomAccessIteratorBase")); }
					return m_iter.operator[](_Off);
				}
				void operator +=(difference_type x) {
					if (m_is_null) { MSE_THROW(std::logic_error("attempting to apply an arthmetic operation on a null value  - TNullableAnyRandomAccessIteratorBase")); }
					m_iter.operator+=(x);
				}
				void operator -=(difference_type x) { operator +=(-x); }
				TNullableAnyRandomAccessIteratorBase& operator ++() { operator +=(1); return (*this); }
				TNullableAnyRandomAccessIteratorBase operator ++(int) { auto _Tmp = (*this); operator +=(1); return _Tmp; }
				TNullableAnyRandomAccessIteratorBase& operator --() { operator -=(1); return (*this); }
				TNullableAnyRandomAccessIteratorBase operator --(int) { auto _Tmp = (*this); operator -=(1); return _Tmp; }

				TNullableAnyRandomAccessIteratorBase operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				TNullableAnyRandomAccessIteratorBase operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const TNullableAnyRandomAccessIteratorBase<_Ty>& _Right_cref) const {
					if (m_is_null != _Right_cref.m_is_null) {
						MSE_THROW(std::logic_error("attempting to apply an arthmetic operation on a null value  - TNullableAnyRandomAccessIteratorBase"));
					}
					else if (m_is_null) {
						return 0; /* This is just to correspond with the comparison operators that consider two null values as being equal. */
					}
					return ((*this).m_iter - _Right_cref.m_iter);
				}

#if !defined(MSE_HAS_CXX17)
				MSE_IMPL_LESS_THAN_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);
				MSE_IMPL_GREATER_THAN_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);
				MSE_IMPL_GREATER_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);
				MSE_IMPL_LESS_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);

				friend bool operator==(const TNullableAnyRandomAccessIteratorBase& lhs, const TNullableAnyRandomAccessIteratorBase& rhs) {
					if (rhs.m_is_null != lhs.m_is_null) {
						return false;
					}
					else if (lhs.m_is_null) {
						return true;
					}
					return lhs.m_iter == rhs.m_iter;
				}
				friend bool operator!=(const TNullableAnyRandomAccessIteratorBase& lhs, const TNullableAnyRandomAccessIteratorBase& rhs) { return !(lhs == rhs); }
#else // !defined(MSE_HAS_CXX17)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				MSE_IMPL_LESS_THAN_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);
				MSE_IMPL_GREATER_THAN_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);
				MSE_IMPL_GREATER_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);
				MSE_IMPL_LESS_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TNullableAnyRandomAccessIteratorBase);

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
				friend std::strong_ordering operator<=>(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					bool null_and_not_null_flag = false;
					MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TLHSIterator_ecwapt>::value) {
						MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TRHSIterator_ecwapt>::value) {
							if (bool(_Right_cref) != bool(_Left_cref)) {
								null_and_not_null_flag = true;
							}
							else if (!bool(_Left_cref)) {
								/* If both sides are null, we'll treat the values as equivalent? */
								return (0 <=> 0);
							}
						} else if (!bool(_Left_cref)) {
							null_and_not_null_flag = true;
						}
					} else MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TRHSIterator_ecwapt>::value) {
						if (!bool(_Right_cref)) {
							null_and_not_null_flag = true;
						}
					}
					if (null_and_not_null_flag) {
						MSE_THROW(std::logic_error("attempting to compare a null value with a non-null value - std::strong_ordering operator<=>() - TNullableAnyRandomAccessIteratorBase"));
					}
					MSE_IF_CONSTEXPR((std::is_base_of<_Myt, TLHSIterator_ecwapt>::value) && (std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)) {
						return ((_Myt::s_contained_iterator_cref(_Left_cref) - _Myt::s_contained_iterator_cref(_Right_cref)) <=> 0); /* this is the proper order, right? */
					} else MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value) {
						return ((_Myt::s_contained_iterator_cref(_Left_cref) - _Right_cref) <=> 0); /* this is the proper order, right? */
					} else MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TRHSIterator_ecwapt>::value) {
						return ((-(_Myt::s_contained_iterator_cref(_Right_cref) - _Left_cref)) <=> 0); /* this is the proper order, right? */
					} else {
						assert(false);
						return (0 <=> 0);
					}
				}

				template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
					&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
						|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
				> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
				friend bool operator==(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
					bool null_and_not_null_flag = false;
					MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TLHSIterator_ecwapt>::value) {
						MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TRHSIterator_ecwapt>::value) {
							if (bool(_Right_cref) != bool(_Left_cref)) {
								null_and_not_null_flag = true;
							}
							else if (!bool(_Left_cref)) {
								/* If both sides are null, we'll treat the values as equivalent? */
								return true;
							}
						} else if (!bool(_Left_cref)) {
							null_and_not_null_flag = true;
						}
					} else MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TRHSIterator_ecwapt>::value) {
						if (!bool(_Right_cref)) {
							null_and_not_null_flag = true;
						}
					}
					if (null_and_not_null_flag) {
						return false;
					}
					MSE_IF_CONSTEXPR((std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TLHSIterator_ecwapt>::value)
						&& (std::is_base_of<_Myt, TRHSIterator_ecwapt>::value || std::is_base_of<TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, TRHSIterator_ecwapt>::value)) {
						return (_Myt::s_contained_iterator_cref(_Left_cref) == _Myt::s_contained_iterator_cref(_Right_cref));
					} else MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value) {
						return (_Myt::s_contained_iterator_cref(_Left_cref) == _Right_cref);
					} else MSE_IF_CONSTEXPR(std::is_base_of<_Myt, TRHSIterator_ecwapt>::value) {
						return (_Left_cref == _Myt::s_contained_iterator_cref(_Right_cref));
					} else {
						assert(false);
						return false;
					}
				}
#endif // !defined(MSE_HAS_CXX17)

#if (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
				friend bool operator!=(const std::nullptr_t& _Left_cref, const TNullableAnyRandomAccessIteratorBase& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}
				friend bool operator!=(const TNullableAnyRandomAccessIteratorBase& _Left_cref, const std::nullptr_t& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				friend bool operator==(const std::nullptr_t&, const TNullableAnyRandomAccessIteratorBase& _Right_cref) {
					return !bool(_Right_cref);
				}
				friend bool operator==(const TNullableAnyRandomAccessIteratorBase& _Left_cref, const std::nullptr_t&) {
					return !bool(_Left_cref);
				}
#else // (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
#ifndef MSE_HAS_CXX20
				template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyRandomAccessIteratorBase, TRHS>::value)> MSE_IMPL_EIS >
				friend bool operator!=(const std::nullptr_t& _Left_cref, const TRHS& _Right_cref) {
					return bool(_Right_cref);
				}
				template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyRandomAccessIteratorBase, TLHS>::value)> MSE_IMPL_EIS >
				friend bool operator!=(const TLHS& _Left_cref, const std::nullptr_t& _Right_cref) {
					return bool(_Left_cref);
				}

				template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyRandomAccessIteratorBase, TRHS>::value)> MSE_IMPL_EIS >
				friend bool operator==(const std::nullptr_t&, const TRHS& _Right_cref) {
					return !bool(_Right_cref);
				}
#endif // !MSE_HAS_CXX20
				template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyRandomAccessIteratorBase, TLHS>::value)> MSE_IMPL_EIS >
				friend bool operator==(const TLHS& _Left_cref, const std::nullptr_t&) {
					return !bool(_Left_cref);
				}
#endif // (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))

				TNullableAnyRandomAccessIteratorBase& operator=(const std::nullptr_t& _Right_cref) {
					return operator=(TNullableAnyRandomAccessIteratorBase());
				}
				TNullableAnyRandomAccessIteratorBase& operator=(TNullableAnyRandomAccessIteratorBase _Right) {
					swap(*this, _Right);
					return (*this);
				}

				explicit operator bool() const {
					return (!m_is_null);
				}

				//MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TNullableAnyRandomAccessIteratorBase);

				void async_not_shareable_and_not_passable_tag() const {}

			private:
				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				template <typename _Ty2>
				static auto& s_contained_iterator_cref(TNullableAnyRandomAccessIteratorBase<_Ty2> const& iter) { return iter.m_iter; }

				base_class m_iter;
				bool m_is_null = false;

				template <typename _Ty2> friend class TNullableAnyRandomAccessIteratorBase;
				template <typename _Ty2> friend class TXScopeNullableAnyRandomAccessIterator;
				template<typename _Ty2>
				friend mse::TAnyRandomAccessIterator<_Ty2> mse::not_null_from_nullable(const mse::TNullableAnyRandomAccessIterator<_Ty2>& src);
				template<typename _Ty2>
				friend mse::TXScopeAnyRandomAccessIterator<_Ty2> mse::not_null_from_nullable(const mse::TXScopeNullableAnyRandomAccessIterator<_Ty2>& src);
			};
		}
	}

	template <typename _Ty>
	class TNullableAnyRandomAccessIterator : public mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty> {
	public:
		typedef mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty> base_class;
		MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);
		TNullableAnyRandomAccessIterator() : base_class() {}
		TNullableAnyRandomAccessIterator(const std::nullptr_t& src) : TNullableAnyRandomAccessIterator() {}
		TNullableAnyRandomAccessIterator(const TNullableAnyRandomAccessIterator& src) = default;
		TNullableAnyRandomAccessIterator(const base_class& src) : base_class(src) {}
		explicit TNullableAnyRandomAccessIterator(_Ty arr[]) : base_class(arr) {}

		template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TRandomAccessIterator1, TNullableAnyRandomAccessIterator>::value)
			&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
			&& (mse::impl::is_potentially_not_xscope<_TRandomAccessIterator1>::value)
			&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
		> MSE_IMPL_EIS >
		TNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRandomAccessIterator1)") 
			: base_class(random_access_iterator)
		{
			mse::impl::T_valid_if_not_an_xscope_type<_TRandomAccessIterator1>();
		}

		friend void swap(TNullableAnyRandomAccessIterator& first, TNullableAnyRandomAccessIterator& second) {
			std::swap(first.contained_iter(), second.contained_iter());
		}

#if (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
		friend bool operator!=(const std::nullptr_t& _Left_cref, const TNullableAnyRandomAccessIterator& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}
		friend bool operator!=(const TNullableAnyRandomAccessIterator& _Left_cref, const std::nullptr_t& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}

		friend bool operator==(const std::nullptr_t&, const TNullableAnyRandomAccessIterator& _Right_cref) {
			return !bool(_Right_cref);
		}
		friend bool operator==(const TNullableAnyRandomAccessIterator& _Left_cref, const std::nullptr_t&) {
			return !bool(_Left_cref);
		}
#endif // (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))

		TNullableAnyRandomAccessIterator& operator=(const std::nullptr_t& _Right_cref) {
			return operator=(TNullableAnyRandomAccessIterator());
		}
		TNullableAnyRandomAccessIterator& operator=(TNullableAnyRandomAccessIterator _Right) {
			(*this).contained_iter() = _Right.contained_iter();
			return (*this);
		}

		MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TNullableAnyRandomAccessIterator);

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		const base_class& contained_iter() const& { return (*this); }
		//const base_class& contained_iter() const&& { return (*this); }
		base_class& contained_iter()& { return (*this); }
		base_class&& contained_iter()&& { return std::move(*this); }

		template<typename _Ty2>
		friend TAnyRandomAccessIterator<_Ty2> not_null_from_nullable(const TNullableAnyRandomAccessIterator<_Ty2>& src);
	};
	/* Since there is no mse::TNullableAnyRandomAccessConstIterator<> at the moment, we'll designate mse::TNullableAnyRandomAccessIterator<const T> 
	to be the "const" counterpart to mse::TNullableAnyRandomAccessIterator<T>. */
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TNullableAnyRandomAccessIterator);

	template <typename _Tx = void, typename _Ty = void>
	auto make_nullable_any_random_access_iterator(const _Ty& x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TNullableAnyRandomAccessIterator<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_nullable_any_random_access_iterator(_Ty&& x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TNullableAnyRandomAccessIterator<_Tx2>(MSE_FWD(x));
	}

	template<typename _Ty>
	TAnyRandomAccessIterator<_Ty> not_null_from_nullable(const TNullableAnyRandomAccessIterator<_Ty>& src) {
		return src.s_contained_iterator_cref(src);
	}

	template <typename _Ty>
	class TXScopeNullableAnyRandomAccessIterator : public mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty> {
	public:
		typedef mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty> base_class;
		MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);
		TXScopeNullableAnyRandomAccessIterator() : base_class() {}
		TXScopeNullableAnyRandomAccessIterator(const std::nullptr_t& src) : TXScopeNullableAnyRandomAccessIterator() {}
		TXScopeNullableAnyRandomAccessIterator(const TXScopeNullableAnyRandomAccessIterator& src) = default;
		TXScopeNullableAnyRandomAccessIterator(const base_class& src) : base_class(src) {}
		explicit TXScopeNullableAnyRandomAccessIterator(_Ty arr[]) : base_class(arr) {}

		template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TRandomAccessIterator1, TXScopeNullableAnyRandomAccessIterator>::value)
			&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
			&& (!std::is_convertible<_TRandomAccessIterator1, std::nullptr_t>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
			&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
		> MSE_IMPL_EIS >
		TXScopeNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) 
			: base_class(random_access_iterator) {}

		friend void swap(TXScopeNullableAnyRandomAccessIterator& first, TXScopeNullableAnyRandomAccessIterator& second) {
			std::swap(first.contained_iter(), second.contained_iter());
		}

#if (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
		friend bool operator!=(const std::nullptr_t& _Left_cref, const TXScopeNullableAnyRandomAccessIterator& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}
		friend bool operator!=(const TXScopeNullableAnyRandomAccessIterator& _Left_cref, const std::nullptr_t& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}

		friend bool operator==(const std::nullptr_t&, const TXScopeNullableAnyRandomAccessIterator& _Right_cref) {
			return !bool(_Right_cref);
		}
		friend bool operator==(const TXScopeNullableAnyRandomAccessIterator& _Left_cref, const std::nullptr_t&) {
			return !bool(_Left_cref);
		}
#endif // (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))

		MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeNullableAnyRandomAccessIterator);

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		const base_class& contained_iter() const& { return (*this); }
		//const base_class& contained_iter() const&& { return (*this); }
		base_class& contained_iter()& { return (*this); }
		base_class&& contained_iter()&& { return std::move(*this); }

		template<typename _Ty2>
		friend TXScopeAnyRandomAccessIterator<_Ty2> not_null_from_nullable(const TXScopeNullableAnyRandomAccessIterator<_Ty2>& src);
	};
	/* Since there is no mse::TXScopeNullableAnyRandomAccessConstIterator<> at the moment, we'll designate mse::TXScopeNullableAnyRandomAccessIterator<const T>
	to be the "const" counterpart to mse::TXScopeNullableAnyRandomAccessIterator<T>. */
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TXScopeNullableAnyRandomAccessIterator);

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_nullable_any_random_access_iterator(const _Ty& x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeNullableAnyRandomAccessIterator<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_nullable_any_random_access_iterator(_Ty&& x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeNullableAnyRandomAccessIterator<_Tx2>(MSE_FWD(x));
	}

	template<typename _Ty>
	TXScopeAnyRandomAccessIterator<_Ty> not_null_from_nullable(const TXScopeNullableAnyRandomAccessIterator<_Ty>& src) {
		return src.s_contained_iterator_cref(src);
	}

	/* The intended semantics of TNullableAnyPointer<> is that it always contains either an std::nullptr_t or a
	valid pointer (or iterator) to a valid object. TNullableAnyPointer<> is primarily designed for compatibility
	with legacy code. For other use cases you might prefer optional<TAnyPointer<> > instead. */
	template <typename _Ty>
	class TNullableAnyPointer : public TAnyPointer<_Ty> {
	public:
		typedef TAnyPointer<_Ty> base_class;
		TNullableAnyPointer() : base_class(mse::TRefCountingPointer<_Ty>()), m_is_null(true) {}
		TNullableAnyPointer(const std::nullptr_t& src) : TNullableAnyPointer() {}
		TNullableAnyPointer(const TNullableAnyPointer& src) = default;
		TNullableAnyPointer(const base_class& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, TNullableAnyPointer>::value)
			&& (!std::is_base_of<base_class, _TPointer1>::value)
			&& (!std::is_convertible<_TPointer1, std::nullptr_t>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
			&& mse::impl::is_potentially_not_xscope<_TPointer1>::value
			> MSE_IMPL_EIS >
		TNullableAnyPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)") 
			: base_class(pointer), m_is_null(mse::impl::evaluates_to_false(pointer)) {
			mse::impl::T_valid_if_not_an_xscope_type<_TPointer1>();
		}

		friend void swap(TNullableAnyPointer& first, TNullableAnyPointer& second) {
			std::swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
			std::swap(first.m_is_null, second.m_is_null);
		}

		TNullableAnyPointer& operator=(TNullableAnyPointer _Right) {
			swap(*this, _Right);
			return (*this);
		}

		/* There is already a generic equality operator for all classes descended from TAnyPointerBaseV1<> . */

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
		friend bool operator!=(const std::nullptr_t& _Left_cref, const TNullableAnyPointer& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}
		friend bool operator!=(const TNullableAnyPointer& _Left_cref, const std::nullptr_t& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}

		friend bool operator==(const std::nullptr_t&, const TNullableAnyPointer& _Right_cref) {
			return !bool(_Right_cref);
		}
		friend bool operator==(const TNullableAnyPointer& _Left_cref, const std::nullptr_t&) {
			return !bool(_Left_cref);
		}
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
		/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
		template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyPointer, TRHS>::value)> MSE_IMPL_EIS >
		friend bool operator!=(const std::nullptr_t& _Left_cref, const TRHS& _Right_cref) {
			return bool(_Right_cref);
		}
		template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyPointer, TLHS>::value)> MSE_IMPL_EIS >
		friend bool operator!=(const TLHS& _Left_cref, const std::nullptr_t& _Right_cref) {
			return bool(_Left_cref);
		}

		template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyPointer, TRHS>::value)> MSE_IMPL_EIS >
		friend bool operator==(const std::nullptr_t&, const TRHS& _Right_cref) {
			return !bool(_Right_cref);
		}
#endif // !MSE_HAS_CXX20
		template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TNullableAnyPointer, TLHS>::value)> MSE_IMPL_EIS >
		friend bool operator==(const TLHS& _Left_cref, const std::nullptr_t&) {
			return !bool(_Left_cref);
		}

#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

		explicit operator bool() const {
			return (!m_is_null);
		}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		bool m_is_null = false;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TNullableAnyPointer);

	template <typename _Tx = void, typename _Ty = void>
	auto make_nullable_any_pointer(const _Ty& x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TNullableAnyPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_nullable_any_pointer(_Ty&& x) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)") {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TNullableAnyPointer<_Tx2>(MSE_FWD(x));
	}

	template <typename _Ty>
	class TXScopeNullableAnyPointer : public TXScopeAnyPointer<_Ty> {
	public:
		typedef TXScopeAnyPointer<_Ty> base_class;
		TXScopeNullableAnyPointer() : base_class(mse::us::impl::TPointer<_Ty>()), m_is_null(true) {}
		TXScopeNullableAnyPointer(const std::nullptr_t& src) : TXScopeNullableAnyPointer() {}
		TXScopeNullableAnyPointer(const TXScopeNullableAnyPointer& src) = default;
		TXScopeNullableAnyPointer(const base_class& src) : base_class(src) {}

		template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_convertible<_TPointer1, TXScopeNullableAnyPointer>::value)
			&& (!std::is_base_of<base_class, _TPointer1>::value)
			&& (!std::is_convertible<_TPointer1, std::nullptr_t>::value)
			&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
		> MSE_IMPL_EIS >
		TXScopeNullableAnyPointer(const _TPointer1& pointer) 
			: base_class(pointer), m_is_null(mse::impl::evaluates_to_false(pointer)) {}

		friend void swap(TXScopeNullableAnyPointer& first, TXScopeNullableAnyPointer& second) {
			std::swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
			std::swap(first.m_is_null, second.m_is_null);
		}

		/* There is already a generic equality operator for all classes descended from TAnyPointerBaseV1<> . */

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
		friend bool operator!=(const std::nullptr_t& _Left_cref, const TXScopeNullableAnyPointer& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}
		friend bool operator!=(const TXScopeNullableAnyPointer& _Left_cref, const std::nullptr_t& _Right_cref) {
			return !(_Left_cref == _Right_cref);
		}

		friend bool operator==(const std::nullptr_t&, const TXScopeNullableAnyPointer& _Right_cref) {
			return !bool(_Right_cref);
		}
		friend bool operator==(const TXScopeNullableAnyPointer& _Left_cref, const std::nullptr_t&) {
			return !bool(_Left_cref);
		}
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
		/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
		template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TXScopeNullableAnyPointer, TRHS>::value)> MSE_IMPL_EIS >
		friend bool operator!=(const std::nullptr_t& _Left_cref, const TRHS& _Right_cref) {
			return bool(_Right_cref);
		}
		template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TXScopeNullableAnyPointer, TLHS>::value)> MSE_IMPL_EIS >
		friend bool operator!=(const TLHS& _Left_cref, const std::nullptr_t& _Right_cref) {
			return bool(_Left_cref);
		}

		template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TXScopeNullableAnyPointer, TRHS>::value)> MSE_IMPL_EIS >
		friend bool operator==(const std::nullptr_t&, const TRHS& _Right_cref) {
			return !bool(_Right_cref);
		}
#endif // !MSE_HAS_CXX20
		template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TXScopeNullableAnyPointer, TLHS>::value)> MSE_IMPL_EIS >
		friend bool operator==(const TLHS& _Left_cref, const std::nullptr_t&) {
			return !bool(_Left_cref);
		}

#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

		explicit operator bool() const {
			return (!m_is_null);
		}

		void async_not_shareable_and_not_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		bool m_is_null = false;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TXScopeNullableAnyPointer);

	template <typename _Ty>
	bool operator==(const std::nullptr_t& lhs, const TXScopeNullableAnyPointer<_Ty>& rhs) { return rhs == lhs; }
#ifndef MSE_HAS_CXX20
	template <typename _Ty>
	bool operator!=(const std::nullptr_t& lhs, const TXScopeNullableAnyPointer<_Ty>& rhs) { return rhs != lhs; }
#endif // !MSE_HAS_CXX20

	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_nullable_any_pointer(const _Ty& x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeNullableAnyPointer<_Tx2>(x);
	}
	template <typename _Tx = void, typename _Ty = void>
	auto make_xscope_nullable_any_pointer(_Ty&& x) {
		typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*x)>, _Tx> _Tx2;
		return TXScopeNullableAnyPointer<_Tx2>(MSE_FWD(x));
	}
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
	template<class _Ty>
	struct hash<mse::TXScopeNullableAnyPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeNullableAnyPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeNullableAnyPointer<_Ty>& _Keyval) const {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
}

namespace mse {

	/* shorter aliases */
	template<typename _Ty> using pp = TPolyPointer<_Ty>;
	template<typename _Ty> using pcp = TPolyConstPointer<_Ty>;
	template<typename _Ty> using anyp = TAnyPointer<_Ty>;
	template<typename _Ty> using anycp = TAnyConstPointer<_Ty>;

	template<typename _Ty> using poly_ptr = TPolyPointer<_Ty>;
	template<typename _Ty> using poly_cptr = TPolyConstPointer<_Ty>;
	template<typename _Ty> using any_ptr = TAnyPointer<_Ty>;
	template<typename _Ty> using any_cptr = TAnyConstPointer<_Ty>;


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

	namespace self_test {
		class CPolyPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				{
					class A1 {
					public:
						A1() {}
						A1(int x) : b(x) {}
						virtual ~A1() {}

						int b = 3;
					};
					typedef mse::rsv::TAsyncShareableAndPassableObj<A1> A;
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

#ifdef MSE_POLY_SELF_TEST_DEPRECATED_POLY_POINTERS
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
#endif // MSE_POLY_SELF_TEST_DEPRECATED_POLY_POINTERS
					};

					/* To demonstrate, first we'll declare some objects such that we can obtain safe pointers to those
				objects. For better or worse, this library provides a bunch of different safe pointers types. */
					mse::TXScopeObj<A> a_scpobj;
					auto a_refcptr = mse::make_refcounting<A>();
					mse::TRegisteredObj<A> a_regobj;
					mse::TCRegisteredObj<A> a_rlxregobj;

					/* Safe iterators are a type of safe pointer too. */
					mse::mstd::vector<A> a_mstdvec;
					a_mstdvec.resize(1);
					auto a_mstdvec_iter = a_mstdvec.begin();
					mse::us::msevector<A> a_msevec;
					a_msevec.resize(1);
					auto a_msevec_ipointer = a_msevec.ibegin();
					auto a_msevec_ssiter = a_msevec.ss_begin();

					/* And don't forget the safe async sharing pointers. */
					auto a_access_requester = mse::make_asyncsharedv2readwrite<A>();
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
						int res12 = B::foo1(mse::TXScopeFixedPointer<D>(&d_xscpobj));
						int res13 = B::foo2(D_refcfp);
						int res14 = B::foo2(mse::TXScopeFixedPointer<D>(&d_xscpobj));
					}

#ifdef MSE_POLY_SELF_TEST_DEPRECATED_POLY_POINTERS
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
						int res13 = B::foo8(D_refcfp);

						int res21 = B::foo3(A_refcfp);
						int res22 = B::foo3(&a_xscpobj);
						int res23 = B::foo3(&a_obj);
						int res24 = B::foo4(A_refcfp);
						int res25 = B::foo4(&a_xscpobj);
						int res26 = B::foo4(&a_obj);

						int res31 = B::foo3(D_refcfp);
						int res33 = B::foo3(&d_obj);
						int res34 = B::foo4(D_refcfp);
						int res36 = B::foo4(&d_obj);

						auto A_shp = std::make_shared<A>(5);
						int res41 = B::foo5(A_shp);
						int res42 = B::foo5(&a_obj);
						int res43 = B::foo6(A_shp);
						int res44 = B::foo6(&a_obj);
					}
#endif // MSE_POLY_SELF_TEST_DEPRECATED_POLY_POINTERS

					{
						/* Just exercising the tdp_variant type. */
						auto A_refcfp = mse::make_refcounting<A>(5);
						mse::TXScopeObj<A> a_xscpobj(7);

						using my_var = tdp_variant<A*, mse::TXScopeFixedPointer<A>, mse::TRefCountingFixedPointer<A>>;

						my_var d;

						d.set<mse::TXScopeFixedPointer<A>>(&a_xscpobj);
						//std::cout << d.get<mse::TXScopeFixedPointer<A>>()->b << std::endl;

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
	}

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

}

#ifdef __clang__
#pragma clang diagnostic pop // -Wdeprecated-declarations
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop // -Wdeprecated-declarations
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#endif // MSEPOLY_H_
