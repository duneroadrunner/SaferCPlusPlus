
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESCOPE_H_
#define MSESCOPE_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#include "msenorad.h"
#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

#ifdef MSE_SELF_TESTS
#include <string>
#endif // MSE_SELF_TESTS

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-value"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
//#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif /*__GNUC__*/
#endif /*__clang__*/

/* Note that by default, MSE_SCOPEPOINTER_DISABLED is defined in non-debug builds. This is enacted in "msepointerbasics.h". */

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("_NOEXCEPT")
#pragma push_macro("MSE_THROW")
#pragma push_macro("_NODISCARD")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NODISCARD
#define _NODISCARD
#endif /*_NODISCARD*/


/* This macro roughly simulates constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base) MSE_USING(Derived, Base)

#ifdef MSE_SCOPEPOINTER_DISABLED
#define MSE_IF_NOT_SCOPEPOINTER_DISABLED(x)
#else // MSE_SCOPEPOINTER_DISABLED
#define MSE_IF_NOT_SCOPEPOINTER_DISABLED(x) x
#endif // MSE_SCOPEPOINTER_DISABLED


#ifdef MSE_HAS_CXX17
#else // MSE_HAS_CXX17
	/* The xscope_borrowing_fixed_* types "should" be unmoveable (as well as uncopyable). But since C++14 doesn't
	have guaranteed copy elision, moveability is required for the make_xscope_borrowing_fixed_*() functions to work. . */
#define MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
#endif // MSE_HAS_CXX17


namespace mse {

	namespace us {
		namespace impl {
			class StructureLockingObjectTagBase {};
		}
	}

	namespace impl {
		class TScopeID {};

		template <typename _Ty> struct is_potentially_not_structure_locking_reference : std::integral_constant<bool,
			(!std::is_base_of<mse::us::impl::StructureLockingObjectTagBase, mse::impl::remove_reference_t<_Ty> >::value)> {};

		template <typename _Ty> struct is_potentially_structure_locking_reference : std::integral_constant<bool, 
			(!is_potentially_not_structure_locking_reference<_Ty>::value)> {};

		template<typename T>
		struct HasXScopeReturnableTagMethod
		{
			template<typename U, void(U::*)() const> struct SFINAE {};
			template<typename U> static char Test(SFINAE<U, &U::xscope_returnable_tag>*);
			template<typename U> static int Test(...);
			static const bool value = (sizeof(Test<T>(0)) == sizeof(char));
		};

		/*
		template<typename T>
		struct HasXScopeNotReturnableTagMethod
		{
			template<typename U, void(U::*)() const> struct SFINAE {};
			template<typename U> static char Test(SFINAE<U, &U::xscope_not_returnable_tag>*);
			template<typename U> static int Test(...);
			static const bool value = (sizeof(Test<T>(0)) == sizeof(char));
		};
		*/

		template<typename T>
		struct is_nonowning_scope_pointer : std::integral_constant<bool, ((std::is_base_of<mse::us::impl::XScopeTagBase, T>::value
			&& std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, T>::value && std::is_base_of<mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, T>::value)
#ifdef MSE_SCOPEPOINTER_DISABLED
			|| (std::is_pointer<T>::value && (!mse::impl::is_potentially_not_xscope<T>::value))
#endif /*MSE_SCOPEPOINTER_DISABLED*/
			)> {};

		template <class _Ty, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<
			(!std::is_same<_Ty&&, _Ty2>::value) || (!std::is_rvalue_reference<_Ty2>::value)
			> MSE_IMPL_EIS >
		static void valid_if_not_rvalue_reference_of_given_type(_Ty2 src) {}
	}

	typedef size_t lifetime_info1_t;
	static const lifetime_info1_t no_lifetime_info1 = 0/*std::numeric_limits<lifetime_info1_t>::max()*/;

#ifdef MSE_SCOPEPOINTER_DISABLED

	namespace rsv {
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObjPointer = _Ty*;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObjConstPointer = const _Ty*;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObjNotNullPointer = _Ty*;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObjNotNullConstPointer = const _Ty*;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObjFixedPointer = _Ty* /*const*/; /* Can't be const qualified because standard
																		   library containers don't support const elements. */
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObjFixedConstPointer = const _Ty* /*const*/;

		template<typename _TROy, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeObj = _TROy;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeFixedPointer = _Ty* /*const*/;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeFixedConstPointer = const _Ty* /*const*/;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeCagedItemFixedPointerToRValue = _Ty* /*const*/;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> using TXScopeCagedItemFixedConstPointerToRValue = const _Ty* /*const*/;
	}

	template<typename _Ty> using TXScopeObjPointer = _Ty * ;
	template<typename _Ty> using TXScopeObjConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeObjNotNullPointer = _Ty * ;
	template<typename _Ty> using TXScopeObjNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeObjFixedPointer = _Ty * /*const*/; /* Can't be const qualified because standard
																	   library containers don't support const elements. */
	template<typename _Ty> using TXScopeObjFixedConstPointer = const _Ty* /*const*/;

	namespace us {
		namespace impl {
			template<typename _TROy> using TXScopeObjBase = _TROy;
		}
	}
	template<typename _TROy> using TXScopeObj = _TROy;
	template<typename _Ty> using TXScopeFixedPointer = _Ty * /*const*/;
	template<typename _Ty> using TXScopeFixedConstPointer = const _Ty* /*const*/;
	template<typename _Ty> using TXScopeCagedItemFixedPointerToRValue = _Ty * /*const*/;
	template<typename _Ty> using TXScopeCagedItemFixedConstPointerToRValue = const _Ty* /*const*/;
	//template<typename _TROy> using TXScopeReturnValue = _TROy;

	template<typename _Ty> auto xscope_fptr_to(_Ty&& _X) { return std::addressof(_X); }
	template<typename _Ty> auto xscope_fptr_to(const _Ty& _X) { return std::addressof(_X); }

	namespace us {
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
	}

	//template<typename _Ty> const _Ty& return_value(const _Ty& _X) { return _X; }
	//template<typename _Ty> _Ty&& return_value(_Ty&& _X) { return MSE_FWD(_X); }
	template<typename _TROy> using TNonXScopeObj = _TROy;

#else /*MSE_SCOPEPOINTER_DISABLED*/

	namespace rsv {
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/> class TXScopeObj;
	}

	namespace us {
		namespace impl {

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
			namespace xsrtc {
				template<typename _Ty> class TAnyPointerBaseV1;
				template<typename _Ty> class TAnyConstPointerBaseV1;

				template<std::size_t Len, std::size_t Align>
				struct any_aligned_storage {
					struct type {
						alignas(Align) unsigned char data[Len];
					};
				};

				class bad_any_cast : public std::bad_cast
				{
				public:
					const char* what() const noexcept override
					{
						return "bad any cast";
					}
				};

				class any /*final*/
				{
				public:
					/// Constructs an object of type any with an empty state.
					any() :
						vtable(nullptr)
					{
					}

					/// Constructs an object of type any with an equivalent state as other.
					any(const any& rhs) :
						vtable(rhs.vtable)
					{
						if (!rhs.empty())
						{
							rhs.vtable->copy(rhs.storage, this->storage);
						}
					}

					/// Constructs an object of type any with a state equivalent to the original state of other.
					/// rhs is left in a valid but otherwise unspecified state.
					any(any&& rhs) noexcept :
						vtable(rhs.vtable)
					{
						if (!rhs.empty())
						{
							rhs.vtable->move(rhs.storage, this->storage);
							rhs.vtable = nullptr;
						}
					}

					/// Same effect as this->reset().
					~any()
					{
						this->reset();
					}

					/// Constructs an object of type any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
					///
					/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
					/// This is because an `any` may be copy constructed into another `any` at any time, so a copy should always be allowed.
					template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, any>::value)
						&& (!std::is_convertible<typename std::decay<ValueType>::type*, any const*>::value)> MSE_IMPL_EIS >
						any(ValueType&& value)
					{
						static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
							"T shall satisfy the CopyConstructible requirements.");
						this->construct(std::forward<ValueType>(value));
					}

					/// Has the same effect as any(rhs).swap(*this). No effects if an exception is thrown.
					any& operator=(const any& rhs)
					{
						any(rhs).swap(*this);
						return *this;
					}

					/// Has the same effect as any(std::move(rhs)).swap(*this).
					///
					/// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
					/// but otherwise unspecified state.
					any& operator=(any&& rhs) noexcept
					{
						any(std::move(rhs)).swap(*this);
						return *this;
					}

					/// Has the same effect as any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
					///
					/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
					/// This is because an `any` may be copy constructed into another `any` at any time, so a copy should always be allowed.
					template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, any>::value)
						&& (!std::is_convertible<typename std::decay<ValueType>::type*, any const*>::value)> MSE_IMPL_EIS >
						any& operator=(ValueType&& value)
					{
						static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
							"T shall satisfy the CopyConstructible requirements.");
						any(std::forward<ValueType>(value)).swap(*this);
						return *this;
					}

					/// If not empty, destroys the contained object.
					void reset() noexcept
					{
						if (!empty())
						{
							this->vtable->destroy(storage);
							this->vtable = nullptr;
						}
					}

					/// Returns true if *this has no contained object, otherwise false.
					_NODISCARD constexpr bool empty() const noexcept
					{
						return this->vtable == nullptr;
					}

					_NODISCARD /*constexpr */bool has_value() const noexcept {
						return !empty();
					}

					/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
					const std::type_info& type() const noexcept
					{
						return empty() ? typeid(void) : this->vtable->type();
					}

					/// Exchange the states of *this and rhs.
					void swap(any& rhs) noexcept
					{
						if (this->vtable != rhs.vtable)
						{
							any tmp(std::move(rhs));

							// move from *this to rhs.
							rhs.vtable = this->vtable;
							if (this->vtable != nullptr)
							{
								this->vtable->move(this->storage, rhs.storage);
								//this->vtable = nullptr; -- uneeded, see below
							}

							// move from tmp (previously rhs) to *this.
							this->vtable = tmp.vtable;
							if (tmp.vtable != nullptr)
							{
								tmp.vtable->move(tmp.storage, this->storage);
								tmp.vtable = nullptr;
							}
						}
						else // same types
						{
							if (this->vtable != nullptr)
								this->vtable->swap(this->storage, rhs.storage);
						}
					}

				private: // Storage and Virtual Method Table

					void* storage_address() noexcept
					{
						return empty() ? nullptr : this->vtable->storage_address(storage);
					}
					const void* storage_address() const noexcept
					{
						return empty() ? nullptr : this->vtable->const_storage_address(storage);
					}

					union storage_union
					{
#ifdef MSE_DISABLE_SOO_EXTENSIONS1
#define MSE_IMPL_ANY_SOO_SIZE_FACTOR	1
#else // MSE_DISABLE_SOO_EXTENSIONS1
#define MSE_IMPL_ANY_SOO_SIZE_FACTOR	4
#endif // MSE_DISABLE_SOO_EXTENSIONS1

						using stack_storage_t = typename any_aligned_storage<MSE_IMPL_ANY_SOO_SIZE_FACTOR * 2 * sizeof(void*), std::alignment_of<void*>::value>::type;

						void* dynamic;
						stack_storage_t     stack;      // 2 words for e.g. shared_ptr
					};

					/// Base VTable specification.
					struct vtable_type
					{
						// Note: The caller is responssible for doing .vtable = nullptr after destructful operations
						// such as destroy() and/or move().

						/// The type of the object this vtable is for.
						const std::type_info& (*type)() noexcept;

						/// Destroys the object in the union.
						/// The state of the union after this call is unspecified, caller must ensure not to use src anymore.
						void(*destroy)(storage_union&) noexcept;

						/// Copies the **inner** content of the src union into the yet unitialized dest union.
						/// As such, both inner objects will have the same state, but on separate memory locations.
						void(*copy)(const storage_union& src, storage_union& dest);

						/// Moves the storage from src to the yet unitialized dest union.
						/// The state of src after this call is unspecified, caller must ensure not to use src anymore.
						void(*move)(storage_union& src, storage_union& dest) noexcept;

						/// Exchanges the storage between lhs and rhs.
						void(*swap)(storage_union& lhs, storage_union& rhs) noexcept;

						void* (*storage_address)(storage_union&) noexcept;
						const void* (*const_storage_address)(const storage_union&) noexcept;
					};

					/// VTable for dynamically allocated storage.
					template<typename T>
					struct vtable_dynamic
					{
						static const std::type_info& type() noexcept
						{
							return typeid(T);
						}

						static void destroy(storage_union& storage) noexcept
						{
							//assert(reinterpret_cast<T*>(storage.dynamic));
							delete reinterpret_cast<T*>(storage.dynamic);
						}

						static void copy(const storage_union& src, storage_union& dest)
						{
							dest.dynamic = new T(*reinterpret_cast<const T*>(src.dynamic));
						}

						static void move(storage_union& src, storage_union& dest) noexcept
						{
							dest.dynamic = src.dynamic;
							src.dynamic = nullptr;
						}

						static void swap(storage_union& lhs, storage_union& rhs) noexcept
						{
							// just exchage the storage pointers.
							std::swap(lhs.dynamic, rhs.dynamic);
						}

						static void* storage_address(storage_union& storage) noexcept
						{
							return static_cast<void*>(storage.dynamic);
						}

						static const void* const_storage_address(const storage_union& storage) noexcept
						{
							return static_cast<const void*>(storage.dynamic);
						}
					};

					/// VTable for stack allocated storage.
					template<typename T>
					struct vtable_stack
					{
						static const std::type_info& type() noexcept
						{
							return typeid(T);
						}

						static void destroy(storage_union& storage) noexcept
						{
							reinterpret_cast<T*>(&storage.stack)->~T();
						}

						static void copy(const storage_union& src, storage_union& dest)
						{
							new (&dest.stack) T(reinterpret_cast<const T&>(src.stack));
						}

						static void move(storage_union& src, storage_union& dest) noexcept
						{
							// one of the conditions for using vtable_stack is a nothrow move constructor,
							// so this move constructor will never throw a exception.
							new (&dest.stack) T(std::move(reinterpret_cast<T&>(src.stack)));
							destroy(src);
						}

						static void swap(storage_union& lhs, storage_union& rhs) noexcept
						{
							std::swap(reinterpret_cast<T&>(lhs.stack), reinterpret_cast<T&>(rhs.stack));
						}

						static void* storage_address(storage_union& storage) noexcept
						{
							return static_cast<void*>(&storage.stack);
						}

						static const void* const_storage_address(const storage_union& storage) noexcept
						{
							return static_cast<const void*>(&storage.stack);
						}
					};

					template<class T, class EqualTo>
					struct SupportsStdSwap_impl
					{
						template<class U, class V>
						static auto test(U* u) -> decltype(std::swap(*u, *u), std::declval<V>(), bool(true));
						template<typename, typename>
						static auto test(...)->std::false_type;

						using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
						static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
					};
					template<class T, class EqualTo = T>
					struct SupportsStdSwap : SupportsStdSwap_impl<
						mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

					/// Whether the type T must be dynamically allocated or can be stored on the stack.
					template<typename T>
					struct requires_allocation :
						std::integral_constant<bool,
						!(
#ifdef MSE_DISABLE_SOO_EXTENSIONS1
							std::is_nothrow_move_constructible<T>::value      // N4562 ?6.3/3 [any.class]
#else // MSE_DISABLE_SOO_EXTENSIONS1
							SupportsStdSwap<T>::value
							&& std::is_move_constructible<T>::value
							&& (std::is_move_assignable<T>::value || std::is_copy_assignable<T>::value)
#endif // MSE_DISABLE_SOO_EXTENSIONS1
							&& sizeof(T) <= sizeof(storage_union::stack)
							&& std::alignment_of<T>::value <= std::alignment_of<storage_union::stack_storage_t>::value)>
					{};

					/// Returns the pointer to the vtable of the type T.
					template<typename T>
					static vtable_type* vtable_for_type()
					{
						using VTableType = mse::impl::conditional_t<requires_allocation<T>::value, vtable_dynamic<T>, vtable_stack<T>>;
						static vtable_type table = {
							VTableType::type, VTableType::destroy,
							VTableType::copy, VTableType::move,
							VTableType::swap, VTableType::storage_address,
							VTableType::const_storage_address,
						};
						return &table;
					}

				private:
					template<typename T>
					friend const T* any_cast(const any* operand) noexcept;
					template<typename T>
					friend T* any_cast(any* operand) noexcept;

					/// Same effect as is_same(this->type(), t);
					bool is_typed(const std::type_info& t) const
					{
						return is_same(this->type(), t);
					}

					/// Checks if two type infos are the same.
					///
					/// If ANY_IMPL_FAST_TYPE_INFO_COMPARE is defined, checks only the address of the
					/// type infos, otherwise does an actual comparision. Checking addresses is
					/// only a valid approach when there's no interaction with outside sources
					/// (other shared libraries and such).
					static bool is_same(const std::type_info& a, const std::type_info& b)
					{
#ifdef ANY_IMPL_FAST_TYPE_INFO_COMPARE
						return &a == &b;
#else
						return a == b;
#endif
					}

					/// Casts (with no type_info checks) the storage pointer as const T*.
					template<typename T>
					const T* cast() const noexcept
					{
						return requires_allocation<typename std::decay<T>::type>::value ?
							reinterpret_cast<const T*>(storage.dynamic) :
							reinterpret_cast<const T*>(&storage.stack);
					}

					/// Casts (with no type_info checks) the storage pointer as T*.
					template<typename T>
					T* cast() noexcept
					{
						return requires_allocation<typename std::decay<T>::type>::value ?
							reinterpret_cast<T*>(storage.dynamic) :
							reinterpret_cast<T*>(&storage.stack);
					}

				private:
					storage_union storage; // on offset(0) so no padding for align
					vtable_type* vtable;

					template<typename ValueType, typename T>
					mse::impl::enable_if_t<requires_allocation<T>::value>
						do_construct(ValueType&& value)
					{
						storage.dynamic = new T(std::forward<ValueType>(value));
					}

					template<typename ValueType, typename T>
					mse::impl::enable_if_t<!requires_allocation<T>::value>
						do_construct(ValueType&& value)
					{
						new (&storage.stack) T(std::forward<ValueType>(value));
					}

					/// Chooses between stack and dynamic allocation for the type decay_t<ValueType>,
					/// assigns the correct vtable, and constructs the object on our storage.
					template<typename ValueType>
					void construct(ValueType&& value)
					{
						using T = typename std::decay<ValueType>::type;

						this->vtable = vtable_for_type<T>();

						do_construct<ValueType, T>(std::forward<ValueType>(value));
					}

					template<typename _Ty2> friend class mse::us::impl::xsrtc::TAnyPointerBaseV1;
					template<typename _Ty2> friend class mse::us::impl::xsrtc::TAnyConstPointerBaseV1;
				};


				template <class _TPointer>
				bool operator_bool_helper1(std::true_type, const _TPointer& ptr_cref) {
					return !(!ptr_cref);
				}
				template <class _TPointer>
				bool operator_bool_helper1(std::false_type, const _TPointer&) {
					/* We need to return the result of conversion to bool, but in this case the "pointer" type, _TPointer, is not convertible
					to bool. Presumably because _TPointer is actually an iterator type. Unfortunately there isn't a good way, in general, to
					determine if an iterator points to a valid item. */
					assert(false);
					return false;
				}

				template <typename _Ty>
				class TCommonPointerInterface {
				public:
					virtual ~TCommonPointerInterface() {}
					virtual _Ty& operator*() const = 0;
					virtual _Ty* operator->() const = 0;
					virtual explicit operator bool() const = 0;
				};

				template <typename _Ty, typename _TPointer1>
				class TCommonizedPointer : public TCommonPointerInterface<_Ty> {
				public:
					TCommonizedPointer(const _TPointer1& pointer) : m_pointer(pointer) {}
					virtual ~TCommonizedPointer() {}

					_Ty& operator*() const {
						/* Using the mse::us::impl::raw_reference_to<>() function allows us to, for example, obtain an 'int&' to
						an mse::Tint<int>. This allows a pointer to an mse::TInt<int> to be used as a pointer to an int. */
						return mse::us::impl::raw_reference_to<_Ty>(*m_pointer);
					}
					_Ty* operator->() const {
						return std::addressof(mse::us::impl::raw_reference_to<_Ty>(*m_pointer));
					}
					explicit operator bool() const {
						//return bool(m_pointer);
						return mse::us::impl::xsrtc::operator_bool_helper1<_TPointer1>(typename std::is_convertible<_TPointer1, bool>::type(), m_pointer);
					}

					_TPointer1 m_pointer;
				};

				template <typename _Ty> class TAnyConstPointerBaseV1;

				template <typename _Ty>
				class TAnyPointerBaseV1 {
				public:
					TAnyPointerBaseV1(const TAnyPointerBaseV1& src) : m_any_pointer(src.m_any_pointer) {}

					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TPointer1, TAnyPointerBaseV1>::value)
						&& (!std::is_base_of<TAnyConstPointerBaseV1<_Ty>, _TPointer1>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)
						> MSE_IMPL_EIS >
					TAnyPointerBaseV1(const _TPointer1& pointer) : m_any_pointer(TCommonizedPointer<_Ty, _TPointer1>(pointer)) {}

					_Ty& operator*() const {
						return (*(*common_pointer_interface_ptr()));
					}
					_Ty* operator->() const {
						return std::addressof(*(*common_pointer_interface_ptr()));
					}
					explicit operator bool() const {
						return bool(*common_pointer_interface_ptr());
					}

					MSE_IMPL_POINTER_EQUALITY_COMPARISON_OPERATOR_DECLARATION(TAnyPointerBaseV1);

					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_convertible<_TPointer1, TAnyPointerBaseV1>::value) && MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)> MSE_IMPL_EIS >
					bool operator==(const _TPointer1& _Right_cref) const {
						if (!bool(*this)) {
							if (!bool(_Right_cref)) {
								return true;
							}
							else {
								return false;
							}
						}
						/* Note that both underlying stored pointers are dereferenced here and we may be relying on the intrinsic
						safety of those pointers to ensure the safety of the dereference operations. */
						return ((void*)(std::addressof(*(*this))) == (void*)(std::addressof(*_Right_cref)));
					}
#ifndef MSE_HAS_CXX20
					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)> MSE_IMPL_EIS >
					bool operator!=(const _TPointer1& _Right_cref) const { return !((*this) == _Right_cref); }
#ifndef MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY
					template <typename _TPointer1, typename _TPointer2, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TPointer1, TAnyPointerBaseV1>::value)
						&& (!std::is_base_of<TAnyConstPointerBaseV1<_Ty>, _TPointer1>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)
						&& std::is_convertible<_TPointer2*, TAnyPointerBaseV1 const *>::value
					> MSE_IMPL_EIS >
						friend bool operator==(const _TPointer1& _Left_cref, const _TPointer2& _Right_cref) {
						return _Right_cref.operator==(_Left_cref);
					}
					template <typename _TPointer1, typename _TPointer2, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TPointer1, TAnyPointerBaseV1>::value)
						&& (!std::is_base_of<TAnyConstPointerBaseV1<_Ty>, _TPointer1>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)
						&& std::is_convertible<_TPointer2*, TAnyPointerBaseV1 const*>::value
					> MSE_IMPL_EIS >
						friend bool operator!=(const _TPointer1& _Left_cref, const _TPointer2& _Right_cref) {
						return !(_Right_cref.operator==(_Left_cref));
					}
#endif // !MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY
#endif // !MSE_HAS_CXX20

				protected:
					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					const TCommonPointerInterface<_Ty>* common_pointer_interface_ptr() const {
						auto retval = static_cast<const TCommonPointerInterface<_Ty>*>(m_any_pointer.storage_address());
						assert(nullptr != retval);
						return retval;
					}

					mse::us::impl::xsrtc::any m_any_pointer;

					friend class TAnyConstPointerBaseV1<_Ty>;
				};

				template <typename _Ty>
				class TCommonConstPointerInterface {
				public:
					virtual ~TCommonConstPointerInterface() {}
					virtual const _Ty& operator*() const = 0;
					virtual const _Ty* operator->() const = 0;
					virtual explicit operator bool() const = 0;
				};

				template <typename _Ty, typename _TConstPointer1>
				class TCommonizedConstPointer : public TCommonConstPointerInterface<_Ty> {
				public:
					TCommonizedConstPointer(const _TConstPointer1& const_pointer) : m_const_pointer(const_pointer) {}
					virtual ~TCommonizedConstPointer() {}

					const _Ty& operator*() const {
						/* Using the mse::us::impl::raw_reference_to<>() function allows us to, for example, obtain a 'const int&' to
						an mse::Tint<int>. This allows a pointer to an mse::TInt<int> to be used as a pointer to a const int. */
						return mse::us::impl::raw_reference_to<const _Ty>(*m_const_pointer);
					}
					const _Ty* operator->() const {
						return std::addressof(mse::us::impl::raw_reference_to<const _Ty>(*m_const_pointer));
					}
					explicit operator bool() const {
						//return bool(m_const_pointer);
						return mse::us::impl::xsrtc::operator_bool_helper1<_TConstPointer1>(typename std::is_convertible<_TConstPointer1, bool>::type(), m_const_pointer);
					}

					_TConstPointer1 m_const_pointer;
				};

				template <typename _Ty>
				class TAnyConstPointerBaseV1 {
				public:
					TAnyConstPointerBaseV1(const TAnyConstPointerBaseV1& src) : m_any_const_pointer(src.m_any_const_pointer) {}
					TAnyConstPointerBaseV1(const TAnyPointerBaseV1<_Ty>& src) : m_any_const_pointer(src.m_any_pointer) {}

					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TPointer1, TAnyConstPointerBaseV1>::value)
						&& (!std::is_convertible<TAnyPointerBaseV1<_Ty>, _TPointer1>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)
					> MSE_IMPL_EIS >
						TAnyConstPointerBaseV1(const _TPointer1& pointer) : m_any_const_pointer(TCommonizedConstPointer<_Ty, _TPointer1>(pointer)) {}

					const _Ty& operator*() const {
						return (*(*common_pointer_interface_const_ptr()));
					}
					const _Ty* operator->() const {
						return std::addressof(*(*common_pointer_interface_const_ptr()));
					}
					explicit operator bool() const {
						return bool(*common_pointer_interface_const_ptr());
					}

					MSE_IMPL_POINTER_EQUALITY_COMPARISON_OPERATOR_DECLARATION(TAnyConstPointerBaseV1);

					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_convertible<_TPointer1, TAnyConstPointerBaseV1>::value) && MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)> MSE_IMPL_EIS >
					bool operator==(const _TPointer1& _Right_cref) const {
						if (!bool(*this)) {
							if (!bool(_Right_cref)) {
								return true;
							}
							else {
								return false;
							}
						}
						/* Note that both underlying stored pointers are dereferenced here and we may be relying on the intrinsic
						safety of those pointers to ensure the safety of the dereference operations. */
						return (std::addressof(*(*this)) == std::addressof(*_Right_cref));
					}
					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)> MSE_IMPL_EIS >
					bool operator !=(const _TPointer1& _Right_cref) const { return !((*this) == _Right_cref); }
#ifndef MSE_HAS_CXX20
#ifndef MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY
					template <typename _TPointer1, typename _TPointer2, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TPointer1, TAnyConstPointerBaseV1>::value)
						&& (!std::is_base_of<TAnyConstPointerBaseV1<_Ty>, _TPointer1>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)
						&& std::is_convertible<_TPointer2*, TAnyConstPointerBaseV1 const*>::value
					> MSE_IMPL_EIS >
						friend bool operator==(const _TPointer1& _Left_cref, const TAnyConstPointerBaseV1& _Right_cref) {
						return _Right_cref.operator==(_Left_cref);
					}
					template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TPointer1, TAnyConstPointerBaseV1>::value)
						&& (!std::is_base_of<TAnyConstPointerBaseV1<_Ty>, _TPointer1>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(_TPointer1)
					> MSE_IMPL_EIS >
						friend bool operator!=(const _TPointer1& _Left_cref, const TAnyConstPointerBaseV1& _Right_cref) {
						return !(_Right_cref.operator==(_Left_cref));
					}
#endif // !MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY
#endif // !MSE_HAS_CXX20

				protected:
					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					const TCommonPointerInterface<_Ty>* common_pointer_interface_const_ptr() const {
						/* This use of mse::us::impl::xsrtc::any::storage_address() brings to mind the fact that the (pre-C++17) implementation
						of mse::us::impl::xsrtc::any that we're using does not support over-aligned types. (And therefore neither does this
						template.) Though it's hard to imagine a reason why a pointer would be declared an over-aligned type. */
						auto retval = static_cast<const TCommonPointerInterface<_Ty>*>(m_any_const_pointer.storage_address());
						assert(nullptr != retval);
						return retval;
					}

					mse::us::impl::xsrtc::any m_any_const_pointer;
				};
			}

			template<typename _TROz> using TXScopeObjBase = mse::TNDNoradObj<_TROz>;
			template<typename _Ty> using TXScopeItemPointerBase = mse::us::impl::xsrtc::TAnyPointerBaseV1<_Ty>;
			template<typename _Ty> using TXScopeItemConstPointerBase = mse::us::impl::xsrtc::TAnyConstPointerBaseV1<_Ty>;

			template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1>
			class TXScopeObjPointerBase : public mse::TNDNoradPointer<_Ty> {
			public:
				typedef mse::TNDNoradPointer<_Ty> base_class;
				TXScopeObjPointerBase(const TXScopeObjPointerBase&) = default;
				TXScopeObjPointerBase(TXScopeObjPointerBase&&) = default;
				TXScopeObjPointerBase(rsv::TXScopeObj<_Ty, lt_info1>& scpobj_ref) : base_class(&(static_cast<TXScopeObjBase<_Ty>&>(scpobj_ref))) {}

				rsv::TXScopeObj<_Ty, lt_info1>& operator*() const {
					return static_cast<rsv::TXScopeObj<_Ty, lt_info1>& >(*(static_cast<const base_class&>(*this)));
				}
				rsv::TXScopeObj<_Ty, lt_info1>* operator->() const {
					return std::addressof(static_cast<rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1>
			class TXScopeObjConstPointerBase : public mse::TNDNoradConstPointer<_Ty> {
			public:
				typedef mse::TNDNoradConstPointer<_Ty> base_class;
				TXScopeObjConstPointerBase(const TXScopeObjConstPointerBase&) = default;
				TXScopeObjConstPointerBase(TXScopeObjConstPointerBase&&) = default;
				TXScopeObjConstPointerBase(const TXScopeObjPointerBase<_Ty, lt_info1>& src_cref) : base_class(src_cref) {}
				TXScopeObjConstPointerBase(const rsv::TXScopeObj<_Ty, lt_info1>& scpobj_cref) : base_class(&(static_cast<const TXScopeObjBase<_Ty>&>(scpobj_cref))) {}

				const rsv::TXScopeObj<_Ty, lt_info1>& operator*() const {
					return static_cast<const rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this)));
				}
				const rsv::TXScopeObj<_Ty, lt_info1>* operator->() const {
					return std::addressof(static_cast<const rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this))));
				}
			};

#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

			template<typename _TROz>
			class TXScopeObjBase : public mse::impl::conditional_t<std::is_enum<_TROz>::value, mse::us::impl::TOpaqueImplicitlyConvertingWrapper1<_TROz>, _TROz> {
			public:
				typedef mse::impl::conditional_t<std::is_enum<_TROz>::value, mse::us::impl::TOpaqueImplicitlyConvertingWrapper1<_TROz>, _TROz> base_class;
				MSE_SCOPE_USING(TXScopeObjBase, _TROz);
				TXScopeObjBase(const TXScopeObjBase& _X) = default;
				TXScopeObjBase(TXScopeObjBase&& _X) = default;
				//TXScopeObjBase(const TXScopeObjBase& _X) : base_class(_X) {}
				//TXScopeObjBase(TXScopeObjBase&& _X) : base_class(MSE_FWD(_X)) {}

				TXScopeObjBase& operator=(TXScopeObjBase&& _X) { _TROz::operator=(MSE_FWD(_X)); return (*this); }
				TXScopeObjBase& operator=(const TXScopeObjBase& _X) { _TROz::operator=(_X); return (*this); }
				template<class _Ty2>
				TXScopeObjBase& operator=(_Ty2&& _X) { _TROz::operator=(MSE_FWD(_X)); return (*this); }
				template<class _Ty2>
				TXScopeObjBase& operator=(const _Ty2& _X) { _TROz::operator=(_X); return (*this); }

				auto operator&() {
					return this;
				}
				auto operator&() const {
					return this;
				}

				/* provisional */
				typedef mse::us::impl::base_type_t<base_class> base_type;
				const base_type& mse_base_type_ref() const& { return mse::us::impl::raw_reference_to<const base_type>(mse::us::impl::as_ref<const base_class>(*this)); }
				const base_type& mse_base_type_ref() const&& = delete;
				base_type& mse_base_type_ref()& { return mse::us::impl::raw_reference_to<base_type>(mse::us::impl::as_ref<base_class>(*this)); }
				base_type& mse_base_type_ref() && = delete;
			};

			template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1>
			class TXScopeObjPointerBase : public mse::us::impl::TPointer<TXScopeObjBase<_Ty>, mse::impl::TScopeID> {
			public:
				typedef mse::us::impl::TPointer<TXScopeObjBase<_Ty>, mse::impl::TScopeID> base_class;
				TXScopeObjPointerBase(const TXScopeObjPointerBase&) = default;
				TXScopeObjPointerBase(TXScopeObjPointerBase&&) = default;
				TXScopeObjPointerBase(rsv::TXScopeObj<_Ty, lt_info1>& scpobj_ref) : base_class(&(static_cast<TXScopeObjBase<_Ty>&>(scpobj_ref))) {}

				rsv::TXScopeObj<_Ty, lt_info1>& operator*() const {
					return static_cast<rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this)));
				}
				rsv::TXScopeObj<_Ty, lt_info1>* operator->() const {
					return std::addressof(static_cast<rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1>
			class TXScopeObjConstPointerBase : public mse::us::impl::TPointer<const TXScopeObjBase<_Ty>, mse::impl::TScopeID> {
			public:
				typedef mse::us::impl::TPointer<const TXScopeObjBase<_Ty>, mse::impl::TScopeID> base_class;
				TXScopeObjConstPointerBase(const TXScopeObjConstPointerBase&) = default;
				TXScopeObjConstPointerBase(TXScopeObjConstPointerBase&&) = default;
				TXScopeObjConstPointerBase(const TXScopeObjPointerBase<_Ty, lt_info1>& src_cref) : base_class(src_cref) {}
				TXScopeObjConstPointerBase(const rsv::TXScopeObj<_Ty, lt_info1>& scpobj_cref) : base_class(&(static_cast<const TXScopeObjBase<_Ty>&>(scpobj_cref))) {}

				const rsv::TXScopeObj<_Ty, lt_info1>& operator*() const {
					return static_cast<const rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this)));
				}
				const rsv::TXScopeObj<_Ty, lt_info1>* operator->() const {
					return std::addressof(static_cast<const rsv::TXScopeObj<_Ty, lt_info1>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty> using TXScopeItemPointerBase = mse::us::impl::TPointer<_Ty, mse::impl::TScopeID>;
			template<typename _Ty> using TXScopeItemConstPointerBase = mse::us::impl::TPointer<const _Ty, mse::impl::TScopeID>;

#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

		}
	}

	namespace rsv {
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeObj;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeObjNotNullPointer;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeObjNotNullConstPointer;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeObjFixedPointer;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeObjFixedConstPointer;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeFixedPointer;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeFixedConstPointer;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeCagedItemFixedPointerToRValue;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeCagedItemFixedConstPointerToRValue;
	}

	template<typename _Ty> using TXScopeObj = rsv::TXScopeObj<_Ty>;
	template<typename _Ty> using TXScopeObjNotNullPointer = rsv::TXScopeObjNotNullPointer<_Ty>;
	template<typename _Ty> using TXScopeObjNotNullConstPointer = rsv::TXScopeObjNotNullConstPointer<_Ty>;
	template<typename _Ty> using TXScopeObjFixedPointer = rsv::TXScopeObjFixedPointer<_Ty>;
	template<typename _Ty> using TXScopeObjFixedConstPointer = rsv::TXScopeObjFixedConstPointer<_Ty>;

	template<typename _Ty> using TXScopeFixedPointer = rsv::TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using TXScopeFixedConstPointer = rsv::TXScopeFixedConstPointer<_Ty>;
	template<typename _Ty> using TXScopeCagedItemFixedPointerToRValue = rsv::TXScopeCagedItemFixedPointerToRValue<_Ty>;
	template<typename _Ty> using TXScopeCagedItemFixedConstPointerToRValue = rsv::TXScopeCagedItemFixedConstPointerToRValue<_Ty>;

#endif // MSE_SCOPEPOINTER_DISABLED

	namespace rsv {
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeOwnerPointer;
		template <class X, lifetime_info1_t lt_info1, class... Args>
		TXScopeOwnerPointer<X, lt_info1> make_xscope_owner(Args&&... args);

		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeFixedPointerFParam;
		template<typename _Ty, lifetime_info1_t lt_info1 = no_lifetime_info1> class TXScopeFixedConstPointerFParam;
	}
	template<typename _Ty> using TXScopeOwnerPointer = rsv::TXScopeOwnerPointer<_Ty>;
	template <class X, class... Args>
	TXScopeOwnerPointer<X> make_xscope_owner(Args&&... args) {
		return rsv::make_xscope_owner<X, no_lifetime_info1>(std::forward<Args>(args)...);
	}

	template<typename _Ty> using TXScopeItemFixedPointer = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using TXScopeItemFixedConstPointer = TXScopeFixedConstPointer<_Ty>;

	namespace impl {
		template<typename TPtr, typename TTarget>
		struct is_convertible_to_nonowning_scope_pointer_helper1 : std::integral_constant<bool,
			std::is_convertible<TPtr, mse::TXScopeFixedPointer<TTarget>>::value || std::is_convertible<TPtr, mse::TXScopeFixedConstPointer<TTarget>>::value> {};
		template<typename TPtr>
		struct is_convertible_to_nonowning_scope_pointer : is_convertible_to_nonowning_scope_pointer_helper1<TPtr
			, mse::impl::remove_reference_t<decltype(*std::declval<TPtr>())> > {};

		template<typename TPtr, typename TTarget>
		struct is_convertible_to_nonowning_scope_or_indeterminate_pointer_helper1 : std::integral_constant<bool,
			is_convertible_to_nonowning_scope_pointer<TPtr>::value
#ifdef MSE_SCOPEPOINTER_DISABLED
			|| std::is_convertible<TPtr, TTarget*>::value
#endif // MSE_SCOPEPOINTER_DISABLED
		> {};
		template<typename TPtr>
		struct is_convertible_to_nonowning_scope_or_indeterminate_pointer : is_convertible_to_nonowning_scope_or_indeterminate_pointer_helper1
			<TPtr, mse::impl::remove_reference_t<decltype(*std::declval<TPtr>())> > {};
	}
	namespace rsv {
		namespace impl {
			template<typename Type>
			struct is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue : mse::impl::is_instantiation_of_template_with_type_and_size_t_params<Type, TXScopeCagedItemFixedConstPointerToRValue> { };
			template <class _Ty> using is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue_t = typename is_instantiation_of_TXScopeCagedItemFixedConstPointerToRValue<_Ty>::type;
		}
	}

#ifdef MSE_SCOPEPOINTER_DISABLED
#else // MSE_SCOPEPOINTER_DISABLED

	namespace rsv {
		/* Use TXScopeObjFixedPointer instead. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObjPointer : public mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
		public:
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjPointer() {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			TXScopeObjPointer() : mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>() {}
			TXScopeObjPointer(TXScopeObj<_Ty>& scpobj_ref) : mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>(scpobj_ref) {}
			TXScopeObjPointer(const TXScopeObjPointer& src_cref) : mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>(
				static_cast<const mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>&>(src_cref)) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjPointer(const TXScopeObjPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>(mse::us::impl::TXScopeObjPointerBase<_Ty2>(src_cref)) {}
			TXScopeObjPointer<_Ty, lt_info1>& operator=(TXScopeObj<_Ty, lt_info1>* ptr) {
				return mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>::operator=(ptr);
			}
			TXScopeObjPointer<_Ty, lt_info1>& operator=(const TXScopeObjPointer<_Ty, lt_info1>& _Right_cref) {
				return mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>::operator=(_Right_cref);
			}
			explicit operator bool() const {
				bool retval = (bool(*static_cast<const mse::us::impl::TXScopeObjPointerBase<_Ty, lt_info1>*>(this)));
				return retval;
			}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeObjNotNullPointer<_Ty>;
		};

		/* Use TXScopeObjFixedConstPointer instead. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObjConstPointer : public mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
		public:
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjConstPointer() {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			TXScopeObjConstPointer() : mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>() {}
			TXScopeObjConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>(scpobj_cref) {}
			TXScopeObjConstPointer(const TXScopeObjConstPointer& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>(
				static_cast<const mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>&>(src_cref)) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjConstPointer(const TXScopeObjConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>(src_cref) {}
			TXScopeObjConstPointer(const TXScopeObjPointer<_Ty, lt_info1>& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>(src_cref) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjConstPointer(const TXScopeObjPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>(mse::us::impl::TXScopeItemConstPointerBase<_Ty2>(src_cref)) {}
			TXScopeObjConstPointer<_Ty, lt_info1>& operator=(const TXScopeObj<_Ty>* ptr) {
				return mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator=(ptr);
			}
			TXScopeObjConstPointer<_Ty, lt_info1>& operator=(const TXScopeObjConstPointer<_Ty, lt_info1>& _Right_cref) {
				return mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator=(_Right_cref);
			}
			explicit operator bool() const {
				bool retval = (bool(*static_cast<const mse::us::impl::TXScopeObjConstPointerBase<_Ty, lt_info1>*>(this)));
				return retval;
			}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeObjNotNullConstPointer<_Ty>;
		};

		/* Use TXScopeObjFixedPointer instead. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObjNotNullPointer : public TXScopeObjPointer<_Ty, lt_info1>, public mse::us::impl::NeverNullTagBase {
		public:
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjNotNullPointer() {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			TXScopeObjNotNullPointer(TXScopeObj<_Ty>& scpobj_ref) : TXScopeObjPointer<_Ty, lt_info1>(scpobj_ref) {}
			TXScopeObjNotNullPointer(TXScopeObj<_Ty>* ptr) : TXScopeObjPointer<_Ty, lt_info1>(ptr) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjNotNullPointer(const TXScopeObjNotNullPointer<_Ty2>& src_cref) : TXScopeObjPointer<_Ty, lt_info1>(src_cref) {}
			TXScopeObjNotNullPointer<_Ty>& operator=(const TXScopeObjPointer<_Ty, lt_info1>& _Right_cref) {
				TXScopeObjPointer<_Ty, lt_info1>::operator=(_Right_cref);
				return (*this);
			}
			explicit operator bool() const { return bool(*static_cast<const TXScopeObjPointer<_Ty, lt_info1>*>(this)); }

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeObjFixedPointer<_Ty>;
		};

		/* Use TXScopeObjFixedConstPointer instead. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObjNotNullConstPointer : public TXScopeObjConstPointer<_Ty, lt_info1>, public mse::us::impl::NeverNullTagBase {
		public:
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjNotNullConstPointer() {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			TXScopeObjNotNullConstPointer(const TXScopeObjNotNullConstPointer<_Ty>& src_cref) : TXScopeObjConstPointer<_Ty, lt_info1>(src_cref) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjNotNullConstPointer(const TXScopeObjNotNullConstPointer<_Ty2>& src_cref) : TXScopeObjConstPointer<_Ty, lt_info1>(src_cref) {}
			TXScopeObjNotNullConstPointer(const TXScopeObjNotNullPointer<_Ty>& src_cref) : TXScopeObjConstPointer<_Ty, lt_info1>(src_cref) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjNotNullConstPointer(const TXScopeObjNotNullPointer<_Ty2>& src_cref) : TXScopeObjConstPointer<_Ty, lt_info1>(src_cref) {}
			TXScopeObjNotNullConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : TXScopeObjConstPointer<_Ty, lt_info1>(scpobj_cref) {}
			explicit operator bool() const { return bool(*static_cast<const TXScopeObjConstPointer<_Ty, lt_info1>*>(this)); }

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeObjFixedConstPointer<_Ty>;
		};

		/* A TXScopeObjFixedPointer points to a TXScopeObj. Its intended for very limited use. Basically just to pass a TXScopeObj
		by reference as a function parameter. TXScopeObjFixedPointers can be obtained from TXScopeObj's "&" (address of) operator. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObjFixedPointer : public TXScopeObjNotNullPointer<_Ty> {
		public:
			TXScopeObjFixedPointer(const TXScopeObjFixedPointer& src_cref) : TXScopeObjNotNullPointer<_Ty>(src_cref) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjFixedPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : TXScopeObjNotNullPointer<_Ty>(src_cref) {}
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjFixedPointer() {}
			explicit operator bool() const { return bool(*static_cast<const TXScopeObjNotNullPointer<_Ty>*>(this)); }
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			TXScopeObjFixedPointer(TXScopeObj<_Ty>& scpobj_ref) : TXScopeObjNotNullPointer<_Ty>(scpobj_ref) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
			/* Disabling public move construction prevents some unsafe operations, like some, but not all,
			attempts to use a TXScopeObjFixedPointer<> as a return value. But it also prevents some safe
			operations too, like initialization via '=' (assignment operator). And the set of prevented
			operations might not be consistent across compilers. We're deciding here that the minor safety
			benefits outweigh the minor inconveniences. Note that we can't disable public move construction
			in the other scope pointers as it would interfere with implicit conversions. */
			TXScopeObjFixedPointer(TXScopeObjFixedPointer&& src_ref) : TXScopeObjNotNullPointer<_Ty>(src_ref) {
				int q = 5;
			}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
			TXScopeObjFixedPointer<_Ty>& operator=(const TXScopeObjFixedPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeObj<_Ty>;
		};

		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObjFixedConstPointer : public TXScopeObjNotNullConstPointer<_Ty> {
		public:
			TXScopeObjFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty2>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
			TXScopeObjFixedConstPointer(const TXScopeObjFixedPointer<_Ty>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
			//template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2 *, _Ty *>::value> MSE_IMPL_EIS >
			//TXScopeObjFixedConstPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjFixedConstPointer() {}
			explicit operator bool() const { return bool(*static_cast<const TXScopeObjNotNullConstPointer<_Ty>*>(this)); }
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			TXScopeObjFixedConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : TXScopeObjNotNullConstPointer<_Ty>(scpobj_cref) {}
			TXScopeObjFixedConstPointer<_Ty>& operator=(const TXScopeObjFixedConstPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeObj<_Ty>;
		};

		/* TXScopeObj is intended as a transparent wrapper for other classes/objects with "scope lifespans". That is, objects
		that are either allocated on the stack, or whose "owning" pointer is allocated on the stack. Unfortunately it's not
		really possible to completely prevent misuse. For example, std::list<TXScopeObj<mse::CInt>> is an improper, and
		dangerous, use of TXScopeObj<>. So we provide the option of using an mse::us::TFLRegisteredObj as TXScopeObj's base
		class to enforce safety and to help catch misuse. Defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause
		mse::us::TFLRegisteredObj to be used in non-debug modes as well. */
		template<typename _TROy, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeObj : public mse::us::impl::TXScopeObjBase<_TROy>
			, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::XScopeTagBase, mse::us::impl::TXScopeObjBase<_TROy>, TXScopeObj<_TROy> >
			, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ReferenceableByScopePointerTagBase, mse::us::impl::TXScopeObjBase<_TROy>, TXScopeObj<_TROy> >
			, public std::conditional<mse::impl::is_shared_ptr<_TROy>::value || mse::impl::is_unique_ptr<_TROy>::value, mse::us::impl::StrongPointerTagBase, mse::impl::TPlaceHolder<mse::us::impl::StrongPointerTagBase, TXScopeObj<_TROy> > >::type
		{
		public:
			typedef mse::us::impl::TXScopeObjBase<_TROy> base_class;
			typedef _TROy element_t;

			TXScopeObj(const TXScopeObj& _X) = default;
			TXScopeObj(TXScopeObj&& _X) = default;

			MSE_SCOPE_USING(TXScopeObj, base_class);
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObj() {}

			TXScopeObj& operator=(TXScopeObj&& _X) {
				//mse::impl::valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
				base_class::operator=(MSE_FWD(_X));
				return (*this);
			}
			TXScopeObj& operator=(const TXScopeObj& _X) { base_class::operator=(_X); return (*this); }
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_assignable<base_class, _Ty2&&>::value> MSE_IMPL_EIS >
			TXScopeObj& operator=(_Ty2&& _X) {
				base_class::operator=(MSE_FWD(_X));
				return (*this);
			}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_assignable<base_class, const _Ty2&>::value> MSE_IMPL_EIS >
			TXScopeObj& operator=(const _Ty2& _X) { base_class::operator=(_X); return (*this); }

			const TXScopeFixedPointer<_TROy, lt_info1> operator&()& {
				return TXScopeObjFixedPointer<_TROy, lt_info1>(*this);
			}
			const TXScopeFixedConstPointer<_TROy, lt_info1> operator&() const& {
				return TXScopeObjFixedConstPointer<_TROy, lt_info1>(*this);
			}
			const TXScopeFixedPointer<_TROy, lt_info1> mse_xscope_fptr()& { return &(*this); }
			const TXScopeFixedConstPointer<_TROy, lt_info1> mse_xscope_fptr() const& { return &(*this); }

			TXScopeCagedItemFixedConstPointerToRValue<_TROy, lt_info1> operator&()&& {
				//return TXScopeFixedConstPointer<_TROy, lt_info1>(TXScopeObjFixedPointer<_TROy, lt_info1>(&(*static_cast<base_class*>(this))));
				return TXScopeFixedConstPointer<_TROy, lt_info1>(TXScopeObjFixedPointer<_TROy, lt_info1>(*this));
			}
			TXScopeCagedItemFixedConstPointerToRValue<_TROy, lt_info1> operator&() const&& {
				return TXScopeObjFixedConstPointer<_TROy, lt_info1>(TXScopeObjConstPointer<_TROy, lt_info1>(&(*static_cast<const base_class*>(this))));
			}
			const TXScopeCagedItemFixedConstPointerToRValue<_TROy, lt_info1> mse_xscope_fptr()&& { return &(*this); }
			const TXScopeCagedItemFixedConstPointerToRValue<_TROy, lt_info1> mse_xscope_fptr() const&& { return &(*this); }

			void xscope_tag() const {}
			//void xscope_contains_accessible_scope_addressof_operator_tag() const {}
			/* This type can be safely used as a function return value if _Ty is also safely returnable. */
			template<class _Ty2 = _TROy, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TROy>::value) && (
				(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::value>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
				)> MSE_IMPL_EIS >
				void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			MSE_DEFAULT_OPERATOR_NEW_DECLARATION

			template<typename _TROy2, lifetime_info1_t lt_info1_2>
			friend class TXScopeOwnerPointer;
			//friend class TXScopeOwnerPointer<_TROy, lt_info1>;
		};

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<typename _TROy>
		TXScopeObj(_TROy)->TXScopeObj<_TROy>;
#endif /* MSE_HAS_CXX17 */
	}

	template<typename _Ty>
	auto xscope_fptr_to(_Ty&& _X) {
		return _X.mse_xscope_fptr();
	}
	template<typename _Ty>
	auto xscope_fptr_to(const _Ty& _X) {
		return _X.mse_xscope_fptr();
	}


	namespace us {
		/* A couple of unsafe functions for internal use. */
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
	}


	namespace rsv {
		namespace impl {
			/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
			template<class _Ty2, class _TMemberObjectPointer>
			using make_xscope_pointer_to_member_v2_return_type1 = TXScopeFixedPointer<mse::impl::remove_reference_t<decltype(std::declval<_Ty2>().*std::declval<_TMemberObjectPointer>())> >;
		}
	}
	namespace impl {
		/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
		template<class _Ty2, class _TMemberObjectPointer>
		using make_xscope_pointer_to_member_v2_return_type1 = rsv::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty2, _TMemberObjectPointer>;
	}
	template<class _Ty2, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		->rsv::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty2, _TMemberObjectPointer>;
	template<class _Ty2, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		->rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >;
	template<class _Ty2, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		->rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >;
	template<class _Ty2, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		->rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >;


	namespace rsv {

		/* While TXScopeObjFixedPointer<> points to a TXScopeObj<>, TXScopeFixedPointer<> is intended to be able to point to a
		TXScopeObj<>, any member of a TXScopeObj<>, or various other items with scope lifetime that, for various reasons, aren't
		declared as TXScopeObj<>. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeFixedPointer : public mse::us::impl::TXScopeItemPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
		public:
			typedef mse::us::impl::TXScopeItemPointerBase<_Ty> base_class;
			TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) = default;
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

			TXScopeFixedPointer(const TXScopeObjFixedPointer<_Ty>& src_cref) : base_class(static_cast<const base_class&>(src_cref)) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

			//TXScopeFixedPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeFixedPointer(&(*src_cref)) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeFixedPointer(&(*src_cref)) {}

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {}

			explicit operator bool() const { return true; }

			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			TXScopeFixedPointer(_Ty* ptr) : base_class(ptr) {}
			TXScopeFixedPointer(const base_class& ptr) : base_class(ptr) {}
			TXScopeFixedPointer<_Ty>& operator=(const TXScopeFixedPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			template<class _Ty2, class _TMemberObjectPointer>
			friend auto mse::make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
				->impl::make_xscope_pointer_to_member_v2_return_type1<_Ty2, _TMemberObjectPointer>;
			/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeObjFixedPointer<_Ty2>& lease_pointer);
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty2>& lease_pointer);

			template<class _Ty2> friend TXScopeFixedPointer<_Ty2> mse::us::unsafe_make_xscope_pointer_to(_Ty2& ref);
		};

		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeFixedConstPointer : public mse::us::impl::TXScopeItemConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
		public:
			typedef mse::us::impl::TXScopeItemConstPointerBase<_Ty> base_class;
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer& src_cref) = default;
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty2>&>(src_cref)) {}

			TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty>& src_cref) : base_class(static_cast<const base_class&>(src_cref)) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty2>&>(src_cref)) {}

			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<_Ty>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

			//TXScopeFixedConstPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {}

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {}

			explicit operator bool() const { return true; }

			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			TXScopeFixedConstPointer(const _Ty* ptr) : base_class(ptr) {}
			TXScopeFixedConstPointer(const base_class& ptr) : base_class(ptr) {}
			TXScopeFixedConstPointer<_Ty>& operator=(const TXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			template<class _Ty2, class _TMemberObjectPointer>
			friend auto mse::make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >;
			template<class _Ty2, class _TMemberObjectPointer>
			friend auto mse::make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >;
			template<class _Ty2, class _TMemberObjectPointer>
			friend auto mse::make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty2>& lease_pointer, const _TMemberObjectPointer& member_object_ptr)
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >;
			/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty2>& lease_pointer);
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedPointer<_Ty2>& lease_pointer);
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty2>& lease_pointer);

			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty2>& lease_pointer);
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty2>& lease_pointer);
			template<class _TTargetType, class _Ty2>
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty2>& lease_pointer);
			template<class _Ty2> friend TXScopeFixedConstPointer<_Ty2> mse::us::unsafe_make_xscope_const_pointer_to(const _Ty2& cref);
		};

		/* TXScopeCagedItemFixedPointerToRValue<> holds a TXScopeFixedPointer<> that points to an r-value which can only be
		accessed when converted to a rsv::TXScopeFixedPointerFParam<>. */
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeCagedItemFixedPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
		public:
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			TXScopeCagedItemFixedPointerToRValue(const TXScopeCagedItemFixedPointerToRValue&) = delete;
			TXScopeCagedItemFixedPointerToRValue(TXScopeCagedItemFixedPointerToRValue&&) = default;
			TXScopeCagedItemFixedPointerToRValue(const TXScopeFixedPointer<_Ty, lt_info1>& ptr) : m_xscope_ptr(ptr) {}

			auto uncaged_pointer() const {
				return m_xscope_ptr;
			}

			TXScopeCagedItemFixedPointerToRValue& operator=(const TXScopeCagedItemFixedPointerToRValue& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			TXScopeFixedPointer<_Ty, lt_info1> m_xscope_ptr;

			template<class _Ty2, lifetime_info1_t lt_info1_2>
			friend class TXScopeObj;
			template<class _Ty2, lifetime_info1_t lt_info1_2> friend class mse::rsv::TXScopeFixedPointerFParam;
			template<class _Ty2, lifetime_info1_t lt_info1_2> friend class mse::rsv::TXScopeFixedConstPointerFParam;
			template<class _Ty2, lifetime_info1_t lt_info1_2> friend class TXScopeStrongPointerStore;
		};

		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeCagedItemFixedConstPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
		public:
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			TXScopeCagedItemFixedConstPointerToRValue(const TXScopeCagedItemFixedConstPointerToRValue& src_cref) = delete;
			TXScopeCagedItemFixedConstPointerToRValue(TXScopeCagedItemFixedConstPointerToRValue&& src_cref) = default;
			TXScopeCagedItemFixedConstPointerToRValue(const TXScopeFixedConstPointer<_Ty, lt_info1>& ptr) : m_xscope_ptr(ptr) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
			TXScopeCagedItemFixedConstPointerToRValue(TXScopeCagedItemFixedConstPointerToRValue&& src_ref) : m_xscope_ptr(src_ref) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

			auto uncaged_pointer() const { return m_xscope_ptr; }

			TXScopeCagedItemFixedConstPointerToRValue& operator=(const TXScopeCagedItemFixedConstPointerToRValue& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			TXScopeFixedConstPointer<_Ty, lt_info1> m_xscope_ptr;

			template<class _Ty2, lifetime_info1_t lt_info1_2>
			friend class TXScopeObj;
			template<class _Ty2, lifetime_info1_t lt_info1_2> friend class mse::rsv::TXScopeFixedConstPointerFParam;
			template<typename _Ty2> friend auto mse::pointer_to(_Ty2&& _X) -> decltype(&std::forward<_Ty2>(_X));
			template<class _Ty2, lifetime_info1_t lt_info1_2> friend class TXScopeStrongConstPointerStore;
		};
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TXScopeFixedPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TXScopeObjFixedPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeObjFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeObjFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::rsv::TXScopeFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TXScopeObjFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeObjFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeObjFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	template<typename _TROy>
	class TNonXScopeObj : public _TROy {
	public:
		MSE_USING(TNonXScopeObj, _TROy);
		TNonXScopeObj(const TNonXScopeObj& _X) : _TROy(_X) {}
		TNonXScopeObj(TNonXScopeObj&& _X) : _TROy(MSE_FWD(_X)) {}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TNonXScopeObj() {
			mse::impl::T_valid_if_not_an_xscope_type<_TROy>();
		}

		TNonXScopeObj& operator=(TNonXScopeObj&& _X) { _TROy::operator=(MSE_FWD(_X)); return (*this); }
		TNonXScopeObj& operator=(const TNonXScopeObj& _X) { _TROy::operator=(_X); return (*this); }
		template<class _Ty2>
		TNonXScopeObj& operator=(_Ty2&& _X) { _TROy::operator=(MSE_FWD(_X)); return (*this); }
		//TNonXScopeObj& operator=(_Ty2&& _X) { static_cast<_TROy&>(*this) = (MSE_FWD(_X)); return (*this); }
		template<class _Ty2>
		TNonXScopeObj& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TROy)");


	namespace rsv {
		/* template specializations */

#define MSE_SCOPE_IMPL_OBJ_INHERIT_ASSIGNMENT_OPERATOR(class_name) \
		auto& operator=(class_name&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); } \
		auto& operator=(const class_name& _X) { base_class::operator=(_X); return (*this); } \
		template<class _Ty2> auto& operator=(_Ty2&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); } \
		template<class _Ty2> auto& operator=(const _Ty2& _X) { base_class::operator=(_X); return (*this); }

#define MSE_SCOPE_IMPL_OBJ_SPECIALIZATION_DEFINITIONS1(class_name) \
		class_name(const class_name&) = default; \
		class_name(class_name&&) = default; \
		MSE_SCOPE_IMPL_OBJ_INHERIT_ASSIGNMENT_OPERATOR(class_name);

#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
#define MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_PRIVATE_CONSTRUCTORS1(class_name) \
			class_name(std::nullptr_t) {} \
			class_name() {}
#else // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
#define MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_PRIVATE_CONSTRUCTORS1(class_name)
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)

	/* Note that because we explicitly define some (private) constructors, default copy and move constructors
	and assignment operators won't be generated, so we have to define those as well. */
#define MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type) \
		template<typename _Ty> \
		class TXScopeObj<specified_type> : public TXScopeObj<mapped_type> { \
		public: \
			typedef TXScopeObj<mapped_type> base_class; \
			MSE_USING(TXScopeObj, base_class); \
			MSE_SCOPE_IMPL_OBJ_SPECIALIZATION_DEFINITIONS1(TXScopeObj); \
		private: \
			MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_PRIVATE_CONSTRUCTORS1(TXScopeObj); \
		};

	/* To achieve compatibility with the mse::us::unsafe_make_xscope_pointer() functions, these specializations make use of
	reinterpret_cast<>s in certain situations. The safety of these reinterpret_cast<>s rely on the 'mapped_type'
	being safely "reinterpretable" as a 'specified_type'. */
#define MSE_SCOPE_IMPL_PTR_SPECIALIZATION(specified_type, mapped_type) \
		template<typename _Ty> \
		class TXScopeFixedPointer<specified_type> : public mse::us::impl::TXScopeItemPointerBase<specified_type>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase { \
		public: \
			typedef mse::us::impl::TXScopeItemPointerBase<specified_type> base_class; \
			TXScopeFixedPointer(const TXScopeFixedPointer<mapped_type>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) = default; \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedPointer(const TXScopeFixedPointer<specified_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<specified_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<specified_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedPointer(const TXScopeOwnerPointer<specified_type2> & src_cref) : TXScopeFixedPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {} \
			explicit operator bool() const { return true; } \
			void xscope_tag() const {} \
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION \
		private: \
			TXScopeFixedPointer(specified_type * ptr) : base_class(ptr) {} \
			TXScopeFixedPointer(mapped_type * ptr) : base_class(reinterpret_cast<specified_type *>(ptr)) {} \
			TXScopeFixedPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedPointer<specified_type>& operator=(const TXScopeFixedPointer<specified_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->impl::make_xscope_pointer_to_member_v2_return_type1<specified_type2, _TMemberObjectPointer>; \
			template<class _Ty2> friend TXScopeFixedPointer<_Ty2> mse::us::unsafe_make_xscope_pointer_to(_Ty2 & ref); \
		}; \
		template<typename _Ty> \
		class TXScopeFixedConstPointer<specified_type> : public TXScopeFixedConstPointer<mapped_type> { \
		public: \
			typedef mse::us::impl::TXScopeItemConstPointerBase<specified_type> base_class; \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<mapped_type>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<specified_type>& src_cref) = default; \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<specified_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<specified_type> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type>&>(src_cref)) {} \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<specified_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<specified_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<specified_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemConstPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<specified_type> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type>&>(src_cref)) {} \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<specified_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			template<class specified_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<specified_type2*, specified_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeOwnerPointer<specified_type2> & src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {} \
			explicit operator bool() const { return true; } \
			void xscope_tag() const {} \
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION \
		private: \
			TXScopeFixedConstPointer(typename std::add_const<specified_type>::type * ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer(typename std::add_const<mapped_type>::type * ptr) : base_class(reinterpret_cast<const specified_type *>(ptr)) {} \
			TXScopeFixedConstPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer<specified_type>& operator=(const TXScopeFixedConstPointer<specified_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >; \
			/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */ \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<specified_type2> & lease_pointer); \
			template<class specified_type2> friend TXScopeFixedConstPointer<specified_type2> mse::us::unsafe_make_xscope_const_pointer_to(const specified_type2 & cref); \
		};

#define MSE_SCOPE_IMPL_PTR_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type) \
			MSE_SCOPE_IMPL_PTR_SPECIALIZATION(specified_type, mapped_type)

#define MSE_SCOPE_IMPL_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type) \
		MSE_SCOPE_IMPL_PTR_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type); \
		MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type);

		MSE_SCOPE_IMPL_NATIVE_POINTER_SPECIALIZATION(_Ty*, mse::us::impl::TPointerForLegacy<_Ty>);
		MSE_SCOPE_IMPL_NATIVE_POINTER_SPECIALIZATION(_Ty* const, const mse::us::impl::TPointerForLegacy<_Ty>);

#ifdef MSEPRIMITIVES_H

#define MSE_SCOPE_IMPL_OBJ_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper) \
		template<> \
		class TXScopeObj<arithmetic_type> : public TXScopeObj<template_wrapper<arithmetic_type>> { \
		public: \
			typedef TXScopeObj<template_wrapper<arithmetic_type>> base_class; \
			MSE_USING(TXScopeObj, base_class); \
		};

		/* To achieve compatibility with the mse::us::unsafe_make_xscope_pointer() functions, these specializations make use of
		reinterpret_cast<>s in certain situations. The safety of these reinterpret_cast<>s rely on 'template_wrapper<arithmetic_type>'
		being safely "reinterpretable" as an 'arithmetic_type'. */
#define MSE_SCOPE_IMPL_PTR_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper) \
		template<> \
		class TXScopeFixedPointer<arithmetic_type> : public mse::us::impl::TXScopeItemPointerBase<arithmetic_type>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase { \
		public: \
			typedef mse::us::impl::TXScopeItemPointerBase<arithmetic_type> base_class; \
			TXScopeFixedPointer(const TXScopeFixedPointer<template_wrapper<arithmetic_type>>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) = default; \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedPointer(const TXScopeFixedPointer<arithmetic_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<arithmetic_type2>&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<arithmetic_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<arithmetic_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<arithmetic_type2>&>(src_cref)) {} \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedPointer(const TXScopeOwnerPointer<arithmetic_type2> & src_cref) : TXScopeFixedPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {} \
			explicit operator bool() const { return true; } \
			void xscope_tag() const {} \
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION \
		private: \
			TXScopeFixedPointer(arithmetic_type * ptr) : base_class(ptr) {} \
			TXScopeFixedPointer(template_wrapper<arithmetic_type> * ptr) : base_class(reinterpret_cast<arithmetic_type *>(ptr)) {} \
			TXScopeFixedPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedPointer<arithmetic_type>& operator=(const TXScopeFixedPointer<arithmetic_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class arithmetic_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<arithmetic_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->impl::make_xscope_pointer_to_member_v2_return_type1<arithmetic_type2, _TMemberObjectPointer>; \
			template<class arithmetic_type2> friend TXScopeFixedPointer<arithmetic_type2> mse::us::unsafe_make_xscope_pointer_to(arithmetic_type2 & ref); \
		}; \
		template<> \
		class TXScopeFixedConstPointer<arithmetic_type> : public mse::us::impl::TXScopeItemConstPointerBase<arithmetic_type>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase { \
		public: \
			typedef mse::us::impl::TXScopeItemConstPointerBase<arithmetic_type> base_class; \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<template_wrapper<arithmetic_type>>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<arithmetic_type>& src_cref) = default; \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<arithmetic_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<arithmetic_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<arithmetic_type> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<arithmetic_type>&>(src_cref)) {} \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<arithmetic_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<arithmetic_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<arithmetic_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<arithmetic_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemConstPointerBase<arithmetic_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<arithmetic_type> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<arithmetic_type>&>(src_cref)) {} \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<arithmetic_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<arithmetic_type2>&>(src_cref)) {} \
			template<class arithmetic_type2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<arithmetic_type2*, arithmetic_type*>::value> MSE_IMPL_EIS > \
			TXScopeFixedConstPointer(const TXScopeOwnerPointer<arithmetic_type2> & src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {} \
			explicit operator bool() const { return true; } \
			void xscope_tag() const {} \
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION \
		private: \
			TXScopeFixedConstPointer(typename std::add_const<arithmetic_type>::type * ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer(typename std::add_const<template_wrapper<arithmetic_type>>::type * ptr) : base_class(reinterpret_cast<const arithmetic_type *>(ptr)) {} \
			TXScopeFixedConstPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer<arithmetic_type>& operator=(const TXScopeFixedConstPointer<arithmetic_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class arithmetic_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<arithmetic_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >; \
			template<class arithmetic_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<arithmetic_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >; \
			template<class arithmetic_type2, class _TMemberObjectPointer> \
			friend auto mse::make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<arithmetic_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >; \
			/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */ \
			template<class _TTargetType, class arithmetic_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<arithmetic_type2> & lease_pointer); \
			template<class _TTargetType, class arithmetic_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedPointer<arithmetic_type2> & lease_pointer); \
			template<class _TTargetType, class arithmetic_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<arithmetic_type2> & lease_pointer); \
			template<class _TTargetType, class arithmetic_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<arithmetic_type2> & lease_pointer); \
			template<class _TTargetType, class arithmetic_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedPointer<arithmetic_type2> & lease_pointer); \
			template<class _TTargetType, class arithmetic_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<arithmetic_type2> & lease_pointer); \
			template<class arithmetic_type2> friend TXScopeFixedConstPointer<arithmetic_type2> mse::us::unsafe_make_xscope_const_pointer_to(const arithmetic_type2 & cref); \
		};

#define MSE_SCOPE_IMPL_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper) \
		MSE_SCOPE_IMPL_PTR_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper); \
		MSE_SCOPE_IMPL_OBJ_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper); \
		MSE_SCOPE_IMPL_PTR_ARITHMETIC_SPECIALIZATION(typename std::add_const<arithmetic_type>::type, template_wrapper); \
		MSE_SCOPE_IMPL_OBJ_ARITHMETIC_SPECIALIZATION(typename std::add_const<arithmetic_type>::type, template_wrapper);

		MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_SCOPE_IMPL_ARITHMETIC_SPECIALIZATION)

#endif /*MSEPRIMITIVES_H*/

		/* end of template specializations */
	}

#endif /*MSE_SCOPEPOINTER_DISABLED*/

#ifdef MSE_MSTDARRAY_DISABLED

	/* When mstd::array is "disabled" it is just aliased to std::array. But since std::array (and std::vector, etc.)
	iterators are dependent types, they do not participate in overload resolution. So the xscope_pointer() overload for
	those iterators actually needs to be a "universal" (template) overload that accepts any type. The reason it needs
	to be here (rather than in the msemstdarray.h file) is that if scope pointers are disabled, then it's possible that
	both scope pointers and std::array iterators could manifest as raw pointers and so would need to be handled (and
	(heuristically) disambiguated) by the same overload implementation. */

	namespace impl {

		template<class T, class EqualTo>
		struct HasOrInheritsIteratorCategoryMemberType_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype(std::declval<typename U::iterator_category>(), std::declval<typename V::iterator_category>(), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct HasOrInheritsIteratorCategoryMemberType : HasOrInheritsIteratorCategoryMemberType_impl<
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};
		template<class _Ty>
		struct is_non_pointer_iterator : std::integral_constant<bool, HasOrInheritsIteratorCategoryMemberType<_Ty>::value> {};

	}
	template<class _Ty>
	auto xscope_const_pointer(const _Ty& param) {
		return mse::us::unsafe_make_xscope_const_pointer_to(*param);
	}
	template<class _Ty>
	auto xscope_pointer(const _Ty& param) {
		return mse::us::unsafe_make_xscope_pointer_to(*param);
	}
#endif // MSE_MSTDARRAY_DISABLED

#if defined(MSE_SCOPEPOINTER_DISABLED) && defined(MSE_MSTDARRAY_DISABLED)
#define MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS
#endif // defined(MSE_SCOPEPOINTER_DISABLED) && defined(MSE_MSTDARRAY_DISABLED)

#ifndef MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS
	/* When mstd::arrays, etc. are disabled, a "universal" overload of xscope_pointer() is provided for their iterators.
	That overload already handles raw pointers (which may be potentially ambiguous in that situation), so we shouldn't
	provide another one. */
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
#ifndef MSE_SCOPEPOINTER_DISABLED
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeObjFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeObjFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeObjFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeObjFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
#endif //!MSE_SCOPEPOINTER_DISABLED
#else // !MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS

#endif // !MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS

	namespace us {
		template <class _TTargetType, class _TLeaseType> class TXScopeSyncWeakFixedConstPointer;

		template <class _TTargetType, class _TLeaseType>
		class TXScopeSyncWeakFixedPointer : public TSyncWeakFixedPointer<_TTargetType, _TLeaseType>, public mse::us::impl::XScopeTagBase {
		public:
			typedef TSyncWeakFixedPointer<_TTargetType, _TLeaseType> base_class;

			TXScopeSyncWeakFixedPointer(const TXScopeSyncWeakFixedPointer&) = default;
			template<class _TLeaseType2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TLeaseType2, _TLeaseType>::value> MSE_IMPL_EIS >
			TXScopeSyncWeakFixedPointer(const TXScopeSyncWeakFixedPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TSyncWeakFixedPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}

			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeSyncWeakFixedPointer make(_TTargetType2& target, const _TLeaseType2& lease) {
				return base_class::make(target, lease);
			}
			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeSyncWeakFixedPointer make(_TTargetType2& target, _TLeaseType2&& lease) {
				return base_class::make(target, MSE_FWD(lease));
			}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		protected:
			TXScopeSyncWeakFixedPointer(_TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
				: base_class(target, lease) {}
			TXScopeSyncWeakFixedPointer(_TTargetType& target/* often a struct member */, _TLeaseType&& lease)
				: base_class(target, MSE_FWD(lease)) {}
		private:
			TXScopeSyncWeakFixedPointer(const base_class& src_cref) : base_class(src_cref) {}

			TXScopeSyncWeakFixedPointer& operator=(const TXScopeSyncWeakFixedPointer& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeSyncWeakFixedConstPointer<_TTargetType, _TLeaseType>;
		};

		template <class _TTargetType, class _TLeaseType>
		TXScopeSyncWeakFixedPointer<_TTargetType, _TLeaseType> make_xscope_syncweak(_TTargetType& target, const _TLeaseType& lease) {
			return TXScopeSyncWeakFixedPointer<_TTargetType, _TLeaseType>::make(target, lease);
		}
		template <class _TTargetType, class _TLeaseType>
		auto make_xscope_syncweak(_TTargetType& target, _TLeaseType&& lease) -> TXScopeSyncWeakFixedPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> > {
			return TXScopeSyncWeakFixedPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> >::make(target, MSE_FWD(lease));
		}

		template <class _TTargetType, class _TLeaseType>
		class TXScopeSyncWeakFixedConstPointer : public TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType>, public mse::us::impl::XScopeTagBase {
		public:
			typedef TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType> base_class;

			TXScopeSyncWeakFixedConstPointer(const TXScopeSyncWeakFixedConstPointer&) = default;
			template<class _TLeaseType2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TLeaseType2, _TLeaseType>::value> MSE_IMPL_EIS >
			TXScopeSyncWeakFixedConstPointer(const TXScopeSyncWeakFixedConstPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}
			TXScopeSyncWeakFixedConstPointer(const TXScopeSyncWeakFixedPointer<_TTargetType, _TLeaseType>& src_cref) : base_class(static_cast<const TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType>&>(src_cref)) {}
			template<class _TLeaseType2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TLeaseType2, _TLeaseType>::value> MSE_IMPL_EIS >
			TXScopeSyncWeakFixedConstPointer(const TXScopeSyncWeakFixedPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}

			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeSyncWeakFixedConstPointer make(const _TTargetType2& target, const _TLeaseType2& lease) {
				return base_class::make(target, lease);
			}
			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeSyncWeakFixedConstPointer make(const _TTargetType2& target, _TLeaseType2&& lease) {
				return base_class::make(target, MSE_FWD(lease));
			}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		protected:
			TXScopeSyncWeakFixedConstPointer(const _TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
				: base_class(target, lease) {}
			TXScopeSyncWeakFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeaseType&& lease)
				: base_class(target, MSE_FWD(lease)) {}
		private:
			TXScopeSyncWeakFixedConstPointer(const base_class& src_cref) : base_class(src_cref) {}

			TXScopeSyncWeakFixedConstPointer& operator=(const TXScopeSyncWeakFixedConstPointer& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};

		template <class _TTargetType, class _TLeaseType>
		TXScopeSyncWeakFixedConstPointer<_TTargetType, _TLeaseType> make_xscope_const_syncweak(const _TTargetType& target, const _TLeaseType& lease) {
			return TXScopeSyncWeakFixedConstPointer<_TTargetType, _TLeaseType>::make(target, lease);
		}
		template <class _TTargetType, class _TLeaseType>
		auto make_xscope_const_syncweak(const _TTargetType& target, _TLeaseType&& lease) -> TXScopeSyncWeakFixedConstPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> > {
			return TXScopeSyncWeakFixedConstPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> >::make(target, MSE_FWD(lease));
		}
	}

	namespace us {
		template <class _TTargetType, class _TLeaseType> class TXScopeStrongFixedConstPointer;

		template <class _TTargetType, class _TLeaseType>
		class TXScopeStrongFixedPointer : public TStrongFixedPointer<_TTargetType, _TLeaseType>, public mse::us::impl::XScopeTagBase {
		public:
			typedef TStrongFixedPointer<_TTargetType, _TLeaseType> base_class;

			TXScopeStrongFixedPointer(const TXScopeStrongFixedPointer&) = default;
			template<class _TLeaseType2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TLeaseType2, _TLeaseType>::value> MSE_IMPL_EIS >
			TXScopeStrongFixedPointer(const TXScopeStrongFixedPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TStrongFixedPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}

			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeStrongFixedPointer make(_TTargetType2& target, const _TLeaseType2& lease) {
				return base_class::make(target, lease);
			}
			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeStrongFixedPointer make(_TTargetType2& target, _TLeaseType2&& lease) {
				return base_class::make(target, MSE_FWD(lease));
			}

			auto xscope_ptr() const& {
				return mse::us::unsafe_make_xscope_pointer_to(*(*this));
			}
			auto xscope_ptr() const&& = delete;
			operator mse::TXScopeFixedPointer<_TTargetType>() const& {
				return xscope_ptr();
			}
			operator mse::TXScopeFixedPointer<_TTargetType>() const&& = delete;
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		protected:
			TXScopeStrongFixedPointer(_TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
				: base_class(target, lease) {}
			TXScopeStrongFixedPointer(_TTargetType& target/* often a struct member */, _TLeaseType&& lease)
				: base_class(target, MSE_FWD(lease)) {}
		private:
			TXScopeStrongFixedPointer(const base_class& src_cref) : base_class(src_cref) {}

			TXScopeStrongFixedPointer& operator=(const TXScopeStrongFixedPointer& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

			friend class TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType>;
		};

		template <class _TTargetType, class _TLeaseType>
		TXScopeStrongFixedPointer<_TTargetType, _TLeaseType> make_xscope_strong(_TTargetType& target, const _TLeaseType& lease) {
			return TXScopeStrongFixedPointer<_TTargetType, _TLeaseType>::make(target, lease);
		}
		template <class _TTargetType, class _TLeaseType>
		auto make_xscope_strong(_TTargetType& target, _TLeaseType&& lease) -> TXScopeStrongFixedPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> > {
			return TXScopeStrongFixedPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> >::make(target, MSE_FWD(lease));
		}

		template <class _TTargetType, class _TLeaseType>
		class TXScopeStrongFixedConstPointer : public TStrongFixedConstPointer<_TTargetType, _TLeaseType>, public mse::us::impl::XScopeTagBase {
		public:
			typedef TStrongFixedConstPointer<_TTargetType, _TLeaseType> base_class;

			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedConstPointer&) = default;
			template<class _TLeaseType2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TLeaseType2, _TLeaseType>::value> MSE_IMPL_EIS >
			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TStrongFixedConstPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}
			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedPointer<_TTargetType, _TLeaseType>& src_cref) : base_class(static_cast<const TStrongFixedConstPointer<_TTargetType, _TLeaseType>&>(src_cref)) {}
			template<class _TLeaseType2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TLeaseType2, _TLeaseType>::value> MSE_IMPL_EIS >
			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TStrongFixedConstPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}

			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeStrongFixedConstPointer make(const _TTargetType2& target, const _TLeaseType2& lease) {
				return base_class::make(target, lease);
			}
			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeStrongFixedConstPointer make(const _TTargetType2& target, _TLeaseType2&& lease) {
				return base_class::make(target, MSE_FWD(lease));
			}

			auto xscope_ptr() const& {
				return mse::us::unsafe_make_xscope_const_pointer_to(*(*this));
			}
			auto xscope_ptr() const&& = delete;
			operator mse::TXScopeFixedConstPointer<_TTargetType>() const& {
				return xscope_ptr();
			}
			operator mse::TXScopeFixedConstPointer<_TTargetType>() const&& = delete;
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		protected:
			TXScopeStrongFixedConstPointer(const _TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
				: base_class(target, lease) {}
			TXScopeStrongFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeaseType&& lease)
				: base_class(target, MSE_FWD(lease)) {}
		private:
			TXScopeStrongFixedConstPointer(const base_class& src_cref) : base_class(src_cref) {}

			TXScopeStrongFixedConstPointer& operator=(const TXScopeStrongFixedConstPointer& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};

		template <class _TTargetType, class _TLeaseType>
		TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType> make_xscope_const_strong(const _TTargetType& target, const _TLeaseType& lease) {
			return TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType>::make(target, lease);
		}
		template <class _TTargetType, class _TLeaseType>
		auto make_xscope_const_strong(const _TTargetType& target, _TLeaseType&& lease) -> TXScopeStrongFixedConstPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> > {
			return TXScopeStrongFixedConstPointer<_TTargetType, mse::impl::remove_reference_t<_TLeaseType> >::make(target, MSE_FWD(lease));
		}
	}


	namespace impl {
		template<typename _Ty, class... Args>
		auto make_xscope_helper(std::true_type, Args&&... args) {
			return _Ty(std::forward<Args>(args)...);
		}
		template<typename _Ty, class... Args>
		auto make_xscope_helper(std::false_type, Args&&... args) {
			return TXScopeObj<_Ty>(std::forward<Args>(args)...);
		}
	}
	template <class X, class... Args>
	auto make_xscope(Args&&... args) {
		typedef mse::impl::remove_reference_t<X> nrX;
		return impl::make_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TXScopeObj>::type(), std::forward<Args>(args)...);
	}
	template <class X>
	auto make_xscope(const X& arg) {
		typedef mse::impl::remove_reference_t<X> nrX;
		return impl::make_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TXScopeObj>::type(), arg);
	}
	template <class X>
	auto make_xscope(X&& arg) {
		typedef mse::impl::remove_reference_t<X> nrX;
		return impl::make_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TXScopeObj>::type(), MSE_FWD(arg));
	}

	namespace us {
		namespace impl {
#ifdef MSE_SCOPEPOINTER_DISABLED
			template<typename _Ty> using TXScopeFixedPointerFParamBase = mse::us::impl::TPointer<_Ty>;
			template<typename _Ty> using TXScopeFixedConstPointerFParamBase = mse::us::impl::TPointer<const _Ty>;
#else /*MSE_SCOPEPOINTER_DISABLED*/
			template<typename _Ty> using TXScopeFixedPointerFParamBase = TXScopeFixedPointer<_Ty>;
			template<typename _Ty> using TXScopeFixedConstPointerFParamBase = TXScopeFixedConstPointer<_Ty>;
#endif /*MSE_SCOPEPOINTER_DISABLED*/
		}
	}

	namespace rsv {

		/* TXScopeFixedPointerFParam<> is just a version of TXScopeFixedPointer<> which may only be used for
		function parameter declations. It has the extra ability to accept (caged) scope pointers to r-value scope objects
		(i.e. supports temporaries by scope reference). */

		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeFixedPointerFParam : public mse::us::impl::TXScopeFixedPointerFParamBase<_Ty> {
		public:
			typedef mse::us::impl::TXScopeFixedPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeFixedPointerFParam, base_class);

			TXScopeFixedPointerFParam(const TXScopeFixedPointerFParam& src_cref) = default;

#ifndef MSE_SCOPEPOINTER_DISABLED
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedPointerFParam(TXScopeCagedItemFixedPointerToRValue<_Ty2>&& src_cref) : base_class(src_cref.uncaged_pointer()) {}

#ifndef MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) { intentional_compile_error<_Ty2>(); }
#else // !MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
#endif // !MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1

#endif //!MSE_SCOPEPOINTER_DISABLED

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointerFParam() {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			template<class _Ty2>
			void intentional_compile_error() const {
				/*
				You are attempting to use an lvalue "scope pointer to a temporary". (Currently) only rvalue
				"scope pointer to a temporary"s are supported.
				*/
				mse::impl::T_valid_if_same_pb<_Ty2, void>();
			}
			TXScopeFixedPointerFParam& operator=(const TXScopeFixedPointerFParam& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};

		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeFixedConstPointerFParam : public mse::us::impl::TXScopeFixedConstPointerFParamBase<_Ty> {
		public:
			typedef mse::us::impl::TXScopeFixedConstPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeFixedConstPointerFParam, base_class);

			TXScopeFixedConstPointerFParam(const TXScopeFixedConstPointerFParam& src_cref) = default;
			TXScopeFixedConstPointerFParam(const TXScopeFixedPointerFParam<_Ty, lt_info1>& src_cref) : base_class(src_cref) {}

#ifndef MSE_SCOPEPOINTER_DISABLED
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointerFParam(TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& src_cref) : base_class(src_cref.uncaged_pointer()) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointerFParam(TXScopeCagedItemFixedPointerToRValue<_Ty2>&& src_cref) : base_class(src_cref.uncaged_pointer()) {}

#ifndef MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedConstPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) { intentional_compile_error<_Ty2>(); }
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) { intentional_compile_error<_Ty2>(); }
#else // !MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedConstPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value> MSE_IMPL_EIS >
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
#endif // !MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1

#endif //!MSE_SCOPEPOINTER_DISABLED

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointerFParam() {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			template<class _Ty2>
			void intentional_compile_error() const {
				/*
				You are attempting to use an lvalue "scope pointer to a temporary". (Currently) only rvalue
				"scope pointer to a temporary"s are supported.
				*/
				mse::impl::T_valid_if_same_pb<_Ty2, void>();
			}
			TXScopeFixedConstPointerFParam& operator=(const TXScopeFixedConstPointerFParam& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
	}

	namespace rsv {
		namespace ltn {
			template<typename... Args>
			class lifetime_notes {
			public:
				lifetime_notes() {}
			};
#define MSE_LIFETIME_NOTES_TYPE(...) mse::rsv::ltn::lifetime_notes<__VA_ARGS__ >
#define MSE_LIFETIME_NOTES_PARAM(...) MSE_LIFETIME_NOTES_TYPE(__VA_ARGS__) = {}

			typedef size_t parameter_ordinal_t;
#define MSE_IMPLICIT_THIS_PARAM_ORDINAL 0
			typedef size_t parameter_lifetime_label_t;
#define MSE_LIFETIME_LABEL(x) x

			template<parameter_ordinal_t param_ordinal, parameter_lifetime_label_t lifetime_label>
			class pll {};
#define MSE_PLL(x, y) mse::rsv::ltn::pll<x, y>

			template<typename... Args>
			class parameter_lifetime_labels {
			public:
				parameter_lifetime_labels() {}
			};
#define MSE_PARAMETER_LIFETIME_LABELS(...) mse::rsv::ltn::parameter_lifetime_labels<__VA_ARGS__ >

			template<parameter_lifetime_label_t... Args>
			class return_value_lifetime {
			public:
				return_value_lifetime() {}
			};
#define MSE_RETURN_VALUE_LIFETIME(...) mse::rsv::ltn::return_value_lifetime<__VA_ARGS__ >

			template<parameter_lifetime_label_t lifetime_label1, parameter_lifetime_label_t lifetime_label2>
			class encompasses {};
#define MSE_ENCOMPASSES(x, y) mse::rsv::ltn::encompasses<x, y>
		}

#if defined(__clang__)
/* We need to support the augmentation of C++ elements with lifetime annotations used by the scpptool. Ideally, we
would just want an generic attribute that places a specified string in the (clang) ast (so that it's accessible to
the scpptool). Unfortunately, such an attribute is apparently not available at the moment. As a substitute we coul
use an already available (relatively benign) attribute that takes a string parameter that clang will place in the
ast. At the time of writing, "gsl::suppress" seems to be one of the few (if not the only one) that works for our
purposes. */
/* MSE_ATTR_STR(x) is used to add a string attribute to an element. */
//define MSE_ATTR_STR(x) [[gsl::suppress(x)]]
#define MSE_ATTR_STR(x) __attribute__((annotate(x)))
#else // defined(__clang__)
#define MSE_ATTR_STR(x)
#endif // defined(__clang__)
/* MSE_ATTR_FUNC_STR(x) is used to add a string attribute to a function declaration. */
#define MSE_ATTR_FUNC_STR(x) MSE_ATTR_STR(x)
/* MSE_ATTR_PARAM_STR(x) is used to add a string attribute to a function parameter declaration. */
#define MSE_ATTR_PARAM_STR(x) MSE_ATTR_STR(x)


		/* rsv::TFParam<> is just a transparent template wrapper for function parameter declarations. In most cases
		use of this wrapper is not necessary, but in some cases it enables functionality only available to variables
		that are function parameters. Specifically, it allows functions to support scope pointer/references to
		temporary objects. For safety reasons, by default, scope pointer/references to temporaries are actually
		"functionally disabled" types distinct from regular scope pointer/reference types. Because it's safe to do so
		in the case of function parameters, the rsv::TFParam<> wrapper enables certain scope pointer/reference types
		(like TXScopeFixedPointer<>, and "random access section" types) to be constructed from their
		"functionally disabled" counterparts.
		*/
		template<typename _Ty>
		class TFParam : public _Ty, public std::conditional<std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::value, mse::impl::TPlaceHolder<TFParam<_Ty> >, mse::us::impl::XScopeTagBase>::type {
		public:
			typedef _Ty base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Ty>
		TFParam(_Ty)->TFParam<_Ty>;
		template<class _Ty>
		TFParam(TFParam<_Ty>)->TFParam<_Ty>;
#endif /* MSE_HAS_CXX17 */

		template<typename _Ty> using TXScopeFParam = TFParam<_Ty>;

		namespace impl {
			namespace fparam {
				template<typename _Ty>
				const auto& as_an_fparam_helper1(std::false_type, const _Ty& param) {
					return param;
				}
				template<typename _Ty>
				auto as_an_fparam_helper1(std::true_type, const _Ty& param) -> typename TFParam<mse::impl::remove_reference_t<_Ty> >::base_class {
					return TFParam<mse::impl::remove_reference_t<_Ty> >(param);
				}

				template<typename _Ty>
				auto as_an_fparam_helper1(std::false_type, _Ty&& param) {
					return std::forward<_Ty>(param);
				}
				template<typename _Ty>
				auto as_an_fparam_helper1(std::true_type, _Ty&& param) -> typename TFParam<mse::impl::remove_reference_t<_Ty> >::base_class {
					return TFParam<mse::impl::remove_reference_t<_Ty> >(std::forward<_Ty>(param));
				}
			}
		}

		template<typename _Ty>
		auto as_an_fparam(_Ty& param) -> decltype(impl::fparam::as_an_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), param)) {
			//static_assert(std::is_const<_Ty>::value, "fparams are currently required to be of const type (or reference to const type).");
			return impl::fparam::as_an_fparam_helper1(typename mse::impl::is_potentially_xscope<mse::impl::remove_reference_t<_Ty> >::type(), param);
		}
		template<typename _Ty>
		auto as_an_fparam(_Ty&& param) {
			return impl::fparam::as_an_fparam_helper1(typename mse::impl::is_potentially_xscope<mse::impl::remove_reference_t<_Ty> >::type(), std::forward<_Ty>(param));
		}

		template<typename _Ty>
		auto xscope_as_an_fparam(_Ty& param) -> decltype(as_an_fparam(param)) {
			//static_assert(std::is_const<_Ty>::value, "fparams are currently required to be of const type (or reference to const type).");
			return as_an_fparam(param);
		}
		template<typename _Ty>
		auto xscope_as_an_fparam(_Ty&& param) {
			return as_an_fparam(std::forward<_Ty>(param));
		}

		/* Template specializations of TFParam<>. There are a number of them. */

#ifndef MSE_SCOPEPOINTER_DISABLED
		template<typename _Ty>
		class TFParam<mse::rsv::TXScopeFixedConstPointer<_Ty> > : public TXScopeFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<const mse::rsv::TXScopeFixedConstPointer<_Ty> > : public TXScopeFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<mse::rsv::TXScopeCagedItemFixedConstPointerToRValue<_Ty> > : public TXScopeFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
#endif //!MSE_SCOPEPOINTER_DISABLED

		template<typename _Ty>
		class TFParam<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TFParam<const _Ty*> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TFParam<const _Ty* const> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		/* These specializations for native arrays aren't actually meant to be used. They're just needed because when you call
		as_an_fparam() on a native array, msvc2017 will try to instantiate a TFParam<> with the native array even though it is
		determined at compile that it will never be used. clang6, for example, doesn't have the same issue. */
		template<typename _Ty, size_t _Size>
		class TFParam<const _Ty[_Size]> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);
			TFParam(const _Ty(&param)[_Size]) : base_class(param) {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
		template<typename _Ty, size_t _Size>
		class TFParam<_Ty[_Size]> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);
			TFParam(_Ty(&param)[_Size]) : base_class(param) {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
		private:
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};


		/* Some forward declarations needed to support "cyclic friending". */
		template<typename _Ty>
		class TReturnableFParam;

		namespace impl {
			namespace returnable_fparam {
				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, const _Ty& param)->TReturnableFParam<mse::impl::remove_reference_t<_Ty> >;

				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, _Ty&& param)->TReturnableFParam<mse::impl::remove_reference_t<_Ty> >;
			}
		}

		/* rsv::TReturnableFParam<> is just a transparent template wrapper for function parameter declarations. Like
		us::FParam<>, in most cases use of this wrapper is not necessary, but in some cases it enables functionality
		only available to variables that are function parameters. Specifically, rsv::TReturnableFParam<> "marks"
		scope pointer/reference parameters as safe to use as the return value of the function, whereas by default,
		scope pointer/references are not considered safe to use as a return value. Note that unlike us::FParam<>,
		rsv::TReturnableFParam<> does not enable the function to accept scope pointer/reference temporaries.
		*/
		template<typename _Ty>
		class TReturnableFParam : public _Ty, public std::conditional<std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::value, mse::impl::TPlaceHolder<TReturnableFParam<_Ty> >, mse::us::impl::XScopeTagBase>::type {
		public:
			typedef _Ty base_class;
			typedef _Ty returnable_fparam_contained_type;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam);

			MSE_USING_AMPERSAND_OPERATOR(base_class);

			void returnable_once_tag() const {}
			void xscope_returnable_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			MSE_USING(TReturnableFParam, base_class);

			//MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
			MSE_USING_ASSIGNMENT_OPERATOR(base_class);
			MSE_DEFAULT_OPERATOR_NEW_DECLARATION;

			template<typename _Ty2>
			friend auto impl::returnable_fparam::as_a_returnable_fparam_helper1(std::true_type, const _Ty2& param)->TReturnableFParam<mse::impl::remove_reference_t<_Ty2> >;
			template<typename _Ty2>
			friend auto impl::returnable_fparam::as_a_returnable_fparam_helper1(std::true_type, _Ty2&& param)->TReturnableFParam<mse::impl::remove_reference_t<_Ty2> >;
		};

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Ty>
		TReturnableFParam(_Ty)->TReturnableFParam<_Ty>;
		template<class _Ty>
		TReturnableFParam(TReturnableFParam<_Ty>)->TReturnableFParam<_Ty>;
#endif /* MSE_HAS_CXX17 */

		template<typename _Ty> using TXScopeReturnableFParam = TReturnableFParam<_Ty>;


		template<typename _Ty>
		auto returnable_fparam_as_base_type(TReturnableFParam<_Ty>&& _X) {
			return std::forward<_Ty>(_X);
		}
		template<typename _Ty>
		auto returnable_fparam_as_base_type(const TReturnableFParam<_Ty>& _X) -> const typename TReturnableFParam<_Ty>::base_class& {
			return static_cast<const typename TReturnableFParam<_Ty>::base_class&>(_X);
		}


		namespace impl {
			namespace returnable_fparam {
				template<typename _Ty>
				const auto& as_a_returnable_fparam_helper1(std::false_type, const _Ty& param) {
					return param;
				}
				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, const _Ty& param) -> TReturnableFParam<mse::impl::remove_reference_t<_Ty> > {
					return TReturnableFParam<mse::impl::remove_reference_t<_Ty> >(param);
				}

				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::false_type, _Ty&& param) {
					return std::forward<_Ty>(param);
				}
				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, _Ty&& param) -> TReturnableFParam<mse::impl::remove_reference_t<_Ty> > {
					return TReturnableFParam<mse::impl::remove_reference_t<_Ty> >(std::forward<_Ty>(param));
				}
			}
		}

		template<typename _Ty>
		auto as_a_returnable_fparam(_Ty& param)
			-> decltype(impl::returnable_fparam::as_a_returnable_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), param)) {
			static_assert(std::is_const<_Ty>::value, "fparams are currently required to be of const type (or reference to const type).");
			return impl::returnable_fparam::as_a_returnable_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), param);
		}
		template<typename _Ty>
		auto as_a_returnable_fparam(_Ty&& param) {
			return impl::returnable_fparam::as_a_returnable_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), std::forward<_Ty>(param));
		}

		template<typename _Ty>
		auto xscope_as_a_returnable_fparam(_Ty& param) -> decltype(as_a_returnable_fparam(param)) {
			static_assert(std::is_const<_Ty>::value, "fparams are currently required to be of const type (or reference to const type).");
			return as_a_returnable_fparam(param);
		}
		template<typename _Ty>
		auto xscope_as_a_returnable_fparam(_Ty&& param) {
			return as_a_returnable_fparam(std::forward<_Ty>(param));
		}

		/* This macro adds an overload of the given (template) function that bequeaths the (first) input parameter's "returnable
		function parameter" status to the function's return value. */
#define MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_function) \
		template <typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS > \
		auto make_xscope_function(const mse::rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax) \
			-> decltype(mse::rsv::as_a_returnable_fparam(make_xscope_function(mse::us::impl::raw_reference_to<_Ty>(param), std::forward<_Args>(_Ax)...))) { \
			const auto& param_base_ref = mse::us::impl::raw_reference_to<_Ty>(param); \
			return mse::rsv::as_a_returnable_fparam(make_xscope_function(param_base_ref, std::forward<_Args>(_Ax)...)); \
		} \
		template <typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS > \
		auto make_xscope_function(mse::rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax) \
			-> decltype(mse::rsv::as_a_returnable_fparam(make_xscope_function(mse::us::impl::raw_reference_to<_Ty>(param), std::forward<_Args>(_Ax)...))) { \
			auto& param_base_ref = mse::us::impl::raw_reference_to<_Ty>(param); \
			return mse::rsv::as_a_returnable_fparam(make_xscope_function(param_base_ref, std::forward<_Args>(_Ax)...)); \
		} \
		template <typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS > \
		auto make_xscope_function(mse::rsv::TReturnableFParam<_Ty>&& param, _Args&&... _Ax) { \
			return mse::rsv::as_a_returnable_fparam(make_xscope_function(std::forward<_Ty>(param), std::forward<_Args>(_Ax)...)); \
		}

#define MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(x) 		mse::impl::enable_if_t<!mse::impl::is_instantiation_of<mse::impl::remove_reference_t<x>, mse::rsv::TReturnableFParam>::value>

		/* Template specializations of TReturnableFParam<>. */

		template<typename _Ty>
		class TReturnableFParam<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TReturnableFParam<const _Ty*> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TReturnableFParam<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TReturnableFParam<const _Ty* const> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}

	/* If a rsv::TReturnableFParam<> wrapped reference is used to make a pointer to a member of its target object, then the
	created pointer to member can inherit the "returnability" of the original wrapped reference. */
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_pointer_to_member_v2)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_const_pointer_to_member_v2)

	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(xscope_pointer)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(xscope_const_pointer)
}

namespace std {
	/* Overloads for rsv::TReturnableFParam<>. */
	template <size_t _Index, typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS >
	auto get(const mse::rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax)
		-> decltype(mse::rsv::as_a_returnable_fparam(get<_Index>(mse::us::impl::raw_reference_to<_Ty>(param), std::forward<_Args>(_Ax)...))) {
		const auto& param_base_ref = mse::us::impl::raw_reference_to<_Ty>(param);
		return mse::rsv::as_a_returnable_fparam(get<_Index>(param_base_ref, std::forward<_Args>(_Ax)...));
	}
	template <size_t _Index, typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS >
	auto get(mse::rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax)
		-> decltype(mse::rsv::as_a_returnable_fparam(get<_Index>(mse::us::impl::raw_reference_to<_Ty>(param), std::forward<_Args>(_Ax)...))) {
		auto& param_base_ref = mse::us::impl::raw_reference_to<_Ty>(param);
		return mse::rsv::as_a_returnable_fparam(get<_Index>(param_base_ref, std::forward<_Args>(_Ax)...));
	}
	template <size_t _Index, typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS >
	auto get(mse::rsv::TReturnableFParam<_Ty>&& param, _Args&&... _Ax) {
		return mse::rsv::as_a_returnable_fparam(get<_Index>(std::forward<_Ty>(param), std::forward<_Args>(_Ax)...));
	}

	template <typename _Tx, typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS >
	auto get(const mse::rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax)
		-> decltype(mse::rsv::as_a_returnable_fparam(get<_Tx>(std::declval<const _Ty&>(), std::forward<_Args>(_Ax)...))) {
		const auto& param_base_ref = mse::us::impl::raw_reference_to<_Ty>(param);
		return mse::rsv::as_a_returnable_fparam(get<_Tx>(param_base_ref, std::forward<_Args>(_Ax)...));
	}
	template <typename _Tx, typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS >
	auto get(mse::rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax)
		-> decltype(mse::rsv::as_a_returnable_fparam(get<_Tx>(std::declval<_Ty&>(), std::forward<_Args>(_Ax)...))) {
		auto& param_base_ref = mse::us::impl::raw_reference_to<_Ty>(param);
		return mse::rsv::as_a_returnable_fparam(get<_Tx>(param_base_ref, std::forward<_Args>(_Ax)...));
	}
	template <typename _Tx, typename _Ty, class... _Args, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_Ty>::value> MSE_IMPL_EIS >
	auto get(mse::rsv::TReturnableFParam<_Ty>&& param, _Args&&... _Ax) {
		return mse::rsv::as_a_returnable_fparam(get<_Tx>(std::forward<_Ty>(param), std::forward<_Args>(_Ax)...));
	}
}

namespace mse {

	template<typename _TROy>
	class TReturnValue : public _TROy {
	public:
		typedef _TROy base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, _TROy);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
			/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
			valid_if_TROy_is_marked_as_returnable_or_not_xscope_type();
		}

		template<class _Ty2 = _TROy, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TROy>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value)> MSE_IMPL_EIS >
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		/* If _TROy is not recognized as safe to use as a function return value, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TROy>::value)
			|| (mse::impl::potentially_does_not_contain_non_owning_scope_reference<_TROy>::value)
			|| (true
				&& (std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_TROy>::value>())
				/*&& (!std::integral_constant<bool, mse::impl::HasXScopeNotReturnableTagMethod<_TROy>::value>())*/
				)> MSE_IMPL_EIS >
		void valid_if_TROy_is_marked_as_returnable_or_not_xscope_type() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _TROy>
	class TXScopeReturnValue : public TReturnValue<_TROy>
		, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::XScopeTagBase, TReturnValue<_TROy>, TXScopeReturnValue<_TROy> >
	{
	public:
		typedef TReturnValue<_TROy> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TXScopeReturnValue, base_class);

		template<class _Ty2 = _TROy, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TROy>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value)> MSE_IMPL_EIS >
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	namespace impl {
		template<typename _Ty>
		static void z__returnable_noop(const _Ty&) {}

		template<typename _Ty>
		const auto& return_value_helper12(const _Ty& _X) {
			return rsv::returnable_fparam_as_base_type(_X);
		}
		template<typename _Ty>
		auto return_value_helper12(_Ty&& _X) {
			return rsv::returnable_fparam_as_base_type(MSE_FWD(_X));
		}

		template<typename _Ty>
		const auto& return_value_helper11(std::false_type, const _Ty& _X) {
			z__returnable_noop<mse::TReturnValue<_Ty> >(_X);
			return _X;
		}
		template<typename _Ty>
		const auto& return_value_helper11(std::true_type, const _Ty& _X) {
			return return_value_helper12(_X);
		}
	}

	template<typename _Ty>
	const auto& return_value(const _Ty& _X) {
		typedef mse::impl::remove_reference_t<_Ty> _Ty_noref;
		return impl::return_value_helper11(mse::impl::conditional_t<
			impl::is_instantiation_of<_Ty_noref, rsv::TReturnableFParam>::value
			|| impl::is_instantiation_of<_Ty_noref, rsv::TXScopeReturnableFParam>::value
			, std::true_type, std::false_type>(), _X);
	}

	namespace impl {
		template<typename _Ty>
		auto return_value_helper11(std::false_type, _Ty&& _X) {
			z__returnable_noop<mse::TReturnValue<mse::impl::remove_reference_t<_Ty> > >(_X);
			return MSE_FWD(_X);
		}
		template<typename _Ty>
		auto return_value_helper11(std::true_type, _Ty&& _X) {
			return return_value_helper12(MSE_FWD(_X));
		}
	}
	template<typename _Ty>
	auto return_value(_Ty&& _X) {
		typedef mse::impl::remove_reference_t<_Ty> _Ty_noref;
		return impl::return_value_helper11(mse::impl::conditional_t<
			impl::is_instantiation_of<_Ty_noref, rsv::TReturnableFParam>::value
			|| impl::is_instantiation_of<_Ty_noref, rsv::TXScopeReturnableFParam>::value
			, std::true_type, std::false_type>(), MSE_FWD(_X));
	}

	/* Template specializations of TReturnValue<>. */

	template<typename _Ty>
	class TReturnValue<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<!(std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};
	template<typename _Ty>
	class TReturnValue<const _Ty*> : public mse::us::impl::TPointerForLegacy<const _Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<!(std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _Ty>
	class TReturnValue<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<!(std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};
	template<typename _Ty>
	class TReturnValue<const _Ty* const> : public mse::us::impl::TPointerForLegacy<const _Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<!(std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};




#define MSE_SCOPE_IMPL_RETURNVALUE_INHERIT_ASSIGNMENT_OPERATOR(class_name) \
		auto& operator=(class_name&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); } \
		auto& operator=(const class_name& _X) { base_class::operator=(_X); return (*this); } \
		template<class _Ty2> auto& operator=(_Ty2&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); } \
		template<class _Ty2> auto& operator=(const _Ty2& _X) { base_class::operator=(_X); return (*this); }

#define MSE_SCOPE_IMPL_RETURNVALUE_SPECIALIZATION_DEFINITIONS1(class_name) \
		class_name(const class_name&) = default; \
		class_name(class_name&&) = default; \
		MSE_SCOPE_IMPL_RETURNVALUE_INHERIT_ASSIGNMENT_OPERATOR(class_name);


#ifdef MSEPRIMITIVES_H

#define MSE_SCOPE_IMPL_RETURNVALUE_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper) \
		template<> \
		class TReturnValue<arithmetic_type> : public TReturnValue<template_wrapper<arithmetic_type>> { \
		public: \
			typedef TReturnValue<template_wrapper<arithmetic_type>> base_class; \
			MSE_USING(TReturnValue, base_class); \
		};

#define MSE_SCOPE_IMPL_RETURNVALUE_ARITHMETIC_SPECIALIZATIONS(arithmetic_type, template_wrapper) \
		MSE_SCOPE_IMPL_RETURNVALUE_ARITHMETIC_SPECIALIZATION(arithmetic_type, template_wrapper); \
		MSE_SCOPE_IMPL_RETURNVALUE_ARITHMETIC_SPECIALIZATION(typename std::add_const<arithmetic_type>::type, template_wrapper);

	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_SCOPE_IMPL_RETURNVALUE_ARITHMETIC_SPECIALIZATIONS)

#endif /*MSEPRIMITIVES_H*/




	/* TMemberObj is a transparent wrapper that can be used to wrap class/struct members to ensure that they are not scope
	types. This might be particularly relevant when the member type is, or is derived from, a template parameter. */
	template<typename _TROy> using TMemberObj = TNonXScopeObj<_TROy>;

	/* TBaseClass is a transparent wrapper that can be used to wrap base classes to ensure that they are not scope
	types. This might be particularly relevant when the base class is, or is derived from, a template parameter. */
	template<typename _TROy> using TBaseClass = TNonXScopeObj<_TROy>;

	namespace rsv {
		/* TXScopeOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
		TXScopeOwnerPointer just forwards it's constructor arguments to the constructor of the TXScopeObj<_Ty>.
		TXScopeOwnerPointers are meant to be allocated on the stack only. Unfortunately there's really no way to
		enforce this, which makes this data type less intrinsically safe than say, "reference counting" pointers.
		*/
		template<typename _Ty, lifetime_info1_t lt_info1/* = no_lifetime_info1*/>
		class TXScopeOwnerPointer : public mse::us::impl::XScopeTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase
			, mse::us::impl::ReferenceableByScopePointerTagBase
			, mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty, TXScopeOwnerPointer<_Ty> >
		{
		public:
			TXScopeOwnerPointer(TXScopeOwnerPointer&& src_ref) = default;

			template <class... Args>
			TXScopeOwnerPointer(Args&&... args) {
				/* In the case where there is exactly one argument and its type is derived from this type, we want to
				act like a move constructor here. We use a helper function to check for this case and act accordingly. */
				constructor_helper1(std::forward<Args>(args)...);
			}

			TXScopeObj<_Ty, lt_info1>& operator*() const& {
				return (*m_ptr);
			}
			TXScopeObj<_Ty, lt_info1>&& operator*() const&& {
				return std::move(*m_ptr);
			}
			TXScopeObj<_Ty, lt_info1>* operator->() const& {
				return std::addressof(*m_ptr);
			}
			void operator->() const&& = delete;

#ifdef MSE_SCOPEPOINTER_DISABLED
			operator _Ty* () const {
				return std::addressof(*(*this));
			}
			explicit operator const _Ty* () const {
				return std::addressof(*(*this));
			}
#endif /*MSE_SCOPEPOINTER_DISABLED*/

			template <class... Args>
			static TXScopeOwnerPointer make(Args&&... args) {
				return TXScopeOwnerPointer(std::forward<Args>(args)...);
			}

			void xscope_tag() const {}
			/* This type can be safely used as a function return value if _TROy is also safely returnable. */
			template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (
				(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::value>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
				)> MSE_IMPL_EIS >
				void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

		private:
			/* construction helper functions */
			template <class... Args>
			void initialize(Args&&... args) {
				/* We can't use std::make_unique<> because TXScopeObj<>'s "operator new()" is private and inaccessible to
				std::make_unique<> (which is not a friend of TXScopeObj<> like we are). */
				auto new_ptr = new TXScopeObj<_Ty, lt_info1>(std::forward<Args>(args)...);
				m_ptr.reset(new_ptr);
			}
			template <class _TSoleArg>
			void constructor_helper2(std::true_type, _TSoleArg&& sole_arg) {
				/* The sole parameter is derived from, or of this type, so we're going to consider the constructor
				a move constructor. */
#ifdef MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY
				/* You would probably only consider defining MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY for extra safety if for
				some reason you couldn't rely on the availability of a tool (like scpptool) to statically enforce the safety of
				these moves. */
#ifdef MSE_HAS_CXX17
				static_assert(false, "The MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY preprocessor symbol is defined, which prohibits the use of TXScopeOwnerPointer<>'s move constructor. ");
#endif // MSE_HAS_CXX17
#endif // MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY
				m_ptr = MSE_FWD(sole_arg.m_ptr);
			}
			template <class _TSoleArg>
			void constructor_helper2(std::false_type, _TSoleArg&& sole_arg) {
				/* The sole parameter is not derived from, or of this type, so the constructor is not a move
				constructor. */
				initialize(MSE_FWD(sole_arg));
			}
			template <class... Args>
			void constructor_helper1(Args&&... args) {
				initialize(std::forward<Args>(args)...);
			}
			template <class _TSoleArg>
			void constructor_helper1(_TSoleArg&& sole_arg) {
				/* The constructor was given exactly one parameter. If the parameter is derived from, or of this type,
				then we're going to consider the constructor a move constructor. */

				constructor_helper2(typename std::is_base_of<TXScopeOwnerPointer, _TSoleArg>::type(), MSE_FWD(sole_arg));
			}

			TXScopeOwnerPointer(const TXScopeOwnerPointer& src_cref) = delete;
			TXScopeOwnerPointer& operator=(const TXScopeOwnerPointer& _Right_cref) = delete;
			void* operator new(size_t size) { return ::operator new(size); }

			std::unique_ptr<TXScopeObj<_Ty, lt_info1> > m_ptr = nullptr;
		};

		template <class X, lifetime_info1_t lt_info1, class... Args>
		TXScopeOwnerPointer<X, lt_info1> make_xscope_owner(Args&&... args) {
			return TXScopeOwnerPointer<X, lt_info1>::make(std::forward<Args>(args)...);
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TXScopeOwnerPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeOwnerPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeOwnerPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	namespace us {
		/* (Unsafely) obtain a scope pointer to any object. */
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref) {
			return TXScopeFixedPointer<_Ty>(std::addressof(ref));
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref) {
			return TXScopeFixedConstPointer<_Ty>(std::addressof(cref));
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_pointer_to(const _Ty& cref) {
			return unsafe_make_xscope_const_pointer_to(cref);
		}
	}

	namespace rsv {
		/* Obtain a scope pointer to any object. Requires static verification. */
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> make_xscope_pointer_to(_Ty& ref) {
			return mse::us::unsafe_make_xscope_pointer_to(ref);
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> make_xscope_const_pointer_to(const _Ty& cref) {
			return mse::us::unsafe_make_xscope_const_pointer_to(cref);
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> make_xscope_pointer_to(const _Ty& cref) {
			return make_xscope_const_pointer_to(cref);
		}
	}

	namespace us {
		template<typename _TROy>
		class TXScopeUserDeclaredReturnable : public _TROy {
		public:
			MSE_USING(TXScopeUserDeclaredReturnable, _TROy);
			TXScopeUserDeclaredReturnable(const TXScopeUserDeclaredReturnable& _X) : _TROy(_X) {}
			TXScopeUserDeclaredReturnable(TXScopeUserDeclaredReturnable&& _X) : _TROy(MSE_FWD(_X)) {}
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeUserDeclaredReturnable() {
				/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
				valid_if_TROy_is_not_marked_as_unreturn_value();
				valid_if_TROy_is_an_xscope_type();
			}

			template<class _Ty2>
			TXScopeUserDeclaredReturnable& operator=(_Ty2&& _X) { _TROy::operator=(MSE_FWD(_X)); return (*this); }
			template<class _Ty2>
			TXScopeUserDeclaredReturnable& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:

			/* If _TROy is "marked" as not safe to use as a function return value, then the following member function
			will not instantiate, causing an (intended) compile error. */
			template<MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::potentially_does_not_contain_non_owning_scope_reference<_TROy>::value)
				/*&& (!std::integral_constant<bool, mse::impl::HasXScopeNotReturnableTagMethod<_TROy>::value>())*/> MSE_IMPL_EIS >
				void valid_if_TROy_is_not_marked_as_unreturn_value() const {}

			template<MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::is_potentially_xscope<_TROy>::value> MSE_IMPL_EIS >
			void valid_if_TROy_is_an_xscope_type() const {}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
	}


#ifndef MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER

#ifdef MSE_SCOPEPOINTER_DISABLED
	namespace impl {
		/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
		template<class _Ty2, class _TMemberObjectPointer>
		using make_xscope_pointer_to_member_v2_return_type1 = rsv::TXScopeFixedPointer<mse::impl::remove_reference_t<decltype(std::declval<_Ty2>().*std::declval<_TMemberObjectPointer>())> >;
	}
#endif // MSE_SCOPEPOINTER_DISABLED

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemPointerBase<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> >(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > > {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > > {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > > {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > >(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of rsv::TXScopeFixedPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		//return rsv::TXScopeFixedPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > > {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of rsv::TXScopeFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		//return rsv::TXScopeFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > > {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of rsv::TXScopeFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		//return rsv::TXScopeFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> rsv::TXScopeFixedConstPointer<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> > > {
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of rsv::TXScopeFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		//return rsv::TXScopeFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(rsv::TXScopeFixedPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(rsv::TXScopeFixedConstPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(rsv::TXScopeFixedPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(rsv::TXScopeFixedConstPointer<_Ty>(lease_pointer), member_object_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const rsv::TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const rsv::TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const rsv::TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const rsv::TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const rsv::TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const rsv::TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#endif // !MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER


	/* TXScopeStrongPointerStore et al are types that store a strong pointer (like a refcounting pointer), and let you
	obtain a corresponding scope pointer. */
	template<typename _TStrongPointer>
	class TXScopeStrongPointerStore : public mse::us::impl::XScopeTagBase
		, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::remove_reference_t<_TStrongPointer>, TXScopeStrongPointerStore<mse::impl::remove_reference_t<_TStrongPointer> > >
	{
	private:
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongPointerStore _Myt;
		typedef mse::impl::remove_reference_t<decltype(*m_stored_ptr)> target_t;

		TXScopeStrongPointerStore(const TXScopeStrongPointerStore&) = delete;
		TXScopeStrongPointerStore(TXScopeStrongPointerStore&&) = default;

		TXScopeStrongPointerStore(_TStrongPointerNR&& stored_ptr) : m_stored_ptr(std::forward<_TStrongPointerNR>(stored_ptr)) {
			mse::impl::dummy_foo(*m_stored_ptr); /* Just verifying that stored_ptr points to a valid target. */
		}
		TXScopeStrongPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {
			mse::impl::dummy_foo(*stored_ptr); /* Just verifying that stored_ptr points to a valid target. */
		}
		~TXScopeStrongPointerStore() {
			mse::impl::is_valid_if_strong_pointer<_TStrongPointerNR>::no_op();
		}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		auto xscope_ptr() const && {
			return mse::TXScopeCagedItemFixedPointerToRValue<target_t>(mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr));
		}
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedPointer<target_t>() const & {
			return m_stored_ptr;
		}
		/*
		template<class target_t2 = target_t, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::rsv::TXScopeFixedConstPointer<target_t2>, mse::TXScopeFixedPointer<target_t> >::value> MSE_IMPL_EIS >
		explicit operator mse::TXScopeFixedConstPointer<target_t2>() const & {
			return m_stored_ptr;
		}
		*/
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::value>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			)> MSE_IMPL_EIS >
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

		template<typename _TLender2> friend class TXScopeBorrowingStrongPointerStore;
	};

	template<typename _TStrongPointer>
	class TXScopeStrongConstPointerStore : public mse::us::impl::XScopeTagBase
		, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::remove_reference_t<_TStrongPointer>, TXScopeStrongConstPointerStore<mse::impl::remove_reference_t<_TStrongPointer> > >
	{
	private:
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongConstPointerStore _Myt;
		typedef mse::impl::remove_reference_t<decltype(*m_stored_ptr)> target_t;

		TXScopeStrongConstPointerStore(const TXScopeStrongConstPointerStore&) = delete;
		TXScopeStrongConstPointerStore(TXScopeStrongConstPointerStore&&) = default;

		TXScopeStrongConstPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {
			mse::impl::dummy_foo(*stored_ptr); /* Just verifying that stored_ptr points to a valid target. */
		}
		~TXScopeStrongConstPointerStore() {
			mse::impl::is_valid_if_strong_pointer<_TStrongPointerNR>::no_op();
		}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		auto xscope_ptr() const && {
			return mse::rsv::TXScopeCagedItemFixedConstPointerToRValue<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*m_stored_ptr)> > >(mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr));
		}
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedConstPointer<target_t>() const & {
			return m_stored_ptr;
		}
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::value>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			)> MSE_IMPL_EIS >
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

		template<typename _TLender2> friend class TXScopeBorrowingStrongConstPointerStore;
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_and_never_null_pointer<mse::impl::remove_reference_t<_TStrongPointer> > >
	class TXScopeStrongNotNullPointerStore : public mse::us::impl::XScopeTagBase
		, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::remove_reference_t<_TStrongPointer>, TXScopeStrongNotNullPointerStore<mse::impl::remove_reference_t<_TStrongPointer> > >
	{
	private:
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongNotNullPointerStore _Myt;
		typedef mse::impl::remove_reference_t<decltype(*m_stored_ptr)> target_t;

		TXScopeStrongNotNullPointerStore(const TXScopeStrongNotNullPointerStore&) = delete;
		TXScopeStrongNotNullPointerStore(TXScopeStrongNotNullPointerStore&&) = default;

		TXScopeStrongNotNullPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedPointer<target_t>() const & {
			return m_stored_ptr;
		}
		/*
		template<class target_t2 = target_t, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::rsv::TXScopeFixedConstPointer<target_t2>, mse::TXScopeFixedPointer<target_t> >::value> MSE_IMPL_EIS >
		explicit operator mse::TXScopeFixedConstPointer<target_t2>() const & {
			return m_stored_ptr;
		}
		*/
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::value>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			)> MSE_IMPL_EIS >
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

		template<typename _TLender2> friend class TXScopeBorrowingStrongNotNullPointerStore;
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_and_never_null_pointer<mse::impl::remove_reference_t<_TStrongPointer> > >
	class TXScopeStrongNotNullConstPointerStore : public mse::us::impl::XScopeTagBase
		, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TStrongPointer, TXScopeStrongNotNullConstPointerStore<mse::impl::remove_reference_t<_TStrongPointer> > >
	{
	private:
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongNotNullConstPointerStore _Myt;
		typedef mse::impl::remove_reference_t<decltype(*m_stored_ptr)> target_t;

		TXScopeStrongNotNullConstPointerStore(const TXScopeStrongNotNullConstPointerStore&) = delete;
		TXScopeStrongNotNullConstPointerStore(TXScopeStrongNotNullConstPointerStore&&) = default;

		TXScopeStrongNotNullConstPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedConstPointer<target_t>() const & {
			return m_stored_ptr;
		}
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::value>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			)> MSE_IMPL_EIS >
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

		template<typename _TLender2> friend class TXScopeBorrowingStrongNotNullConstPointerStore;
	};

	namespace impl {
		namespace ns_xscope_strong_pointer_store {

			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::true_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongNotNullConstPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::true_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongNotNullConstPointerStore<_TStrongPointer>(MSE_FWD(stored_ptr));
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::false_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongConstPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::false_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongConstPointerStore<_TStrongPointer>(MSE_FWD(stored_ptr));
			}

		}
	}
	template<typename _TStrongPointer>
	auto make_xscope_strong_const_pointer_store(const _TStrongPointer& stored_ptr) {
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_const_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), stored_ptr);
	}
	template<typename _TStrongPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_TStrongPointer)>
	auto make_xscope_strong_const_pointer_store(_TStrongPointer&& stored_ptr) {
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_const_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), MSE_FWD(stored_ptr));
	}

	namespace impl {
		namespace ns_xscope_strong_pointer_store {

			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::true_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongNotNullPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::true_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongNotNullPointerStore<_TStrongPointer>(MSE_FWD(stored_ptr));
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::false_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::false_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongPointerStore<_TStrongPointer>(MSE_FWD(stored_ptr));
			}

			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::true_type, const _TStrongPointer& stored_ptr) {
				return make_xscope_strong_const_pointer_store(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::true_type, _TStrongPointer&& stored_ptr) {
				return make_xscope_strong_const_pointer_store(MSE_FWD(stored_ptr));
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::false_type, const _TStrongPointer& stored_ptr) {
				typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
				return make_xscope_strong_pointer_store_helper1(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::false_type, _TStrongPointer&& stored_ptr) {
				typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
				return make_xscope_strong_pointer_store_helper1(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), MSE_FWD(stored_ptr));
			}
		}
	}
	template<typename _TStrongPointer>
	auto make_xscope_strong_pointer_store(const _TStrongPointer& stored_ptr) {
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		typedef mse::impl::remove_reference_t<decltype(*stored_ptr)> _TTargetNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_pointer_store_helper2<_TStrongPointerNR>(typename std::is_const<_TTargetNR>::type(), stored_ptr);
	}
	template<typename _TStrongPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(_TStrongPointer)>
	auto make_xscope_strong_pointer_store(_TStrongPointer&& stored_ptr) {
		typedef mse::impl::remove_reference_t<_TStrongPointer> _TStrongPointerNR;
		typedef mse::impl::remove_reference_t<decltype(*stored_ptr)> _TTargetNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_pointer_store_helper2<_TStrongPointerNR>(typename std::is_const<_TTargetNR>::type(), MSE_FWD(stored_ptr));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_strong_const_pointer_store)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_strong_pointer_store)

	template<typename _Ty> using TXScopeXScopeItemFixedStore = TXScopeStrongNotNullPointerStore<TXScopeFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeItemFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeFixedConstPointer<_Ty> >;
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty> using TXScopeXScopeFixedStore = TXScopeStrongNotNullPointerStore<TXScopeObjFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeObjFixedConstPointer<_Ty> >;
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)


	/* TXScopeBorrowingStrongPointerStore et al are types that store a strong pointer (like a refcounting pointer), and let you
	obtain a corresponding scope pointer. */
	template<typename _TLender>
	class TXScopeBorrowingStrongPointerStore : public TXScopeStrongPointerStore<_TLender>
		, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, TXScopeStrongPointerStore<_TLender>, TXScopeBorrowingStrongPointerStore<mse::impl::remove_reference_t<_TLender> > >
	{
	private:
		typedef TXScopeStrongPointerStore<_TLender> base_class;

		TXScopeBorrowingStrongPointerStore(const TXScopeBorrowingStrongPointerStore&) = delete;

		_TLender& m_src_ref;

	public:

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongPointerStore(TXScopeBorrowingStrongPointerStore&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongPointerStore(TXScopeBorrowingStrongPointerStore&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		TXScopeBorrowingStrongPointerStore(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TXScopeBorrowingStrongPointerStore(_TLender* src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		~TXScopeBorrowingStrongPointerStore() {
			m_src_ref = std::move((*this).m_stored_ptr);
		}

		MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(base_class);
		void async_not_passable_tag() const {}
	};

	template<typename _TLender>
	class TXScopeBorrowingStrongConstPointerStore : public TXScopeStrongConstPointerStore<_TLender>
		, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, TXScopeStrongConstPointerStore<_TLender>, TXScopeBorrowingStrongConstPointerStore<mse::impl::remove_reference_t<_TLender> > >
	{
	private:
		typedef TXScopeStrongConstPointerStore<_TLender> base_class;

		TXScopeBorrowingStrongConstPointerStore(const TXScopeBorrowingStrongConstPointerStore&) = delete;

		_TLender& m_src_ref;

	public:

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongConstPointerStore(TXScopeBorrowingStrongConstPointerStore&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongConstPointerStore(TXScopeBorrowingStrongConstPointerStore&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		TXScopeBorrowingStrongConstPointerStore(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TXScopeBorrowingStrongConstPointerStore(_TLender* src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		~TXScopeBorrowingStrongConstPointerStore() {
			m_src_ref = std::move((*this).m_stored_ptr);
		}

		MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(base_class);
		void async_not_passable_tag() const {}
	};

	template<typename _TLender>
	class TXScopeBorrowingStrongNotNullPointerStore : public TXScopeStrongNotNullPointerStore<_TLender>
		, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, TXScopeStrongNotNullPointerStore<_TLender>, TXScopeBorrowingStrongNotNullPointerStore<mse::impl::remove_reference_t<_TLender> > >
	{
	private:
		typedef TXScopeStrongNotNullPointerStore<_TLender> base_class;

		TXScopeBorrowingStrongNotNullPointerStore(const TXScopeBorrowingStrongNotNullPointerStore&) = delete;

		_TLender& m_src_ref;

	public:

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongNotNullPointerStore(TXScopeBorrowingStrongNotNullPointerStore&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongNotNullPointerStore(TXScopeBorrowingStrongNotNullPointerStore&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		TXScopeBorrowingStrongNotNullPointerStore(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TXScopeBorrowingStrongNotNullPointerStore(_TLender* src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		~TXScopeBorrowingStrongNotNullPointerStore() {
			m_src_ref = std::move((*this).m_stored_ptr);
		}

		MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(base_class);
		void async_not_passable_tag() const {}
	};

	template<typename _TLender>
	class TXScopeBorrowingStrongNotNullConstPointerStore : public TXScopeStrongNotNullConstPointerStore<_TLender>
		, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, TXScopeStrongNotNullConstPointerStore<_TLender>, TXScopeBorrowingStrongNotNullConstPointerStore<mse::impl::remove_reference_t<_TLender> > >
	{
	private:
		typedef TXScopeStrongNotNullConstPointerStore<_TLender> base_class;

		TXScopeBorrowingStrongNotNullConstPointerStore(const TXScopeBorrowingStrongNotNullConstPointerStore&) = delete;

		_TLender& m_src_ref;

	public:

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongNotNullConstPointerStore(TXScopeBorrowingStrongNotNullConstPointerStore&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		TXScopeBorrowingStrongNotNullConstPointerStore(TXScopeBorrowingStrongNotNullConstPointerStore&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		TXScopeBorrowingStrongNotNullConstPointerStore(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		TXScopeBorrowingStrongNotNullConstPointerStore(_TLender* src_xs_ptr) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		~TXScopeBorrowingStrongNotNullConstPointerStore() {
			m_src_ref = std::move((*this).m_stored_ptr);
		}

		MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(base_class);
		void async_not_passable_tag() const {}
	};


	namespace impl {
		namespace ns_xscope_borrowing_strong_pointer_store {

			template<typename _TLender>
			auto make_xscope_borrowing_strong_const_pointer_store_helper1(std::true_type, _TLender* src_xs_ptr) {
				return TXScopeBorrowingStrongNotNullConstPointerStore<_TLender>(src_xs_ptr);
			}
			template<typename _TLender>
			auto make_xscope_borrowing_strong_const_pointer_store_helper1(std::false_type, _TLender* src_xs_ptr) {
				return TXScopeBorrowingStrongConstPointerStore<_TLender>(src_xs_ptr);
			}
		}
	}
	template<typename _TLender>
	auto make_xscope_borrowing_strong_const_pointer_store(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) {
		typedef mse::impl::remove_reference_t<_TLender> _TStrongPointerNR;
		return impl::ns_xscope_borrowing_strong_pointer_store::make_xscope_borrowing_strong_const_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), src_xs_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TLender>
	auto make_xscope_borrowing_strong_const_pointer_store(_TLender* src_xs_ptr) {
		typedef mse::impl::remove_reference_t<_TLender> _TStrongPointerNR;
		return impl::ns_xscope_borrowing_strong_pointer_store::make_xscope_borrowing_strong_const_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), src_xs_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	namespace impl {
		namespace ns_xscope_borrowing_strong_pointer_store {

			template<typename _TLender>
			auto make_xscope_borrowing_strong_pointer_store_helper1(std::true_type, _TLender* src_xs_ptr) {
				return TXScopeBorrowingStrongNotNullPointerStore<_TLender>(src_xs_ptr);
			}
			template<typename _TLender>
			auto make_xscope_borrowing_strong_pointer_store_helper1(std::false_type, _TLender* src_xs_ptr) {
				return TXScopeBorrowingStrongPointerStore<_TLender>(src_xs_ptr);
			}
		}
	}
	template<typename _TLender>
	auto make_xscope_borrowing_strong_pointer_store(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) {
		typedef mse::impl::remove_reference_t<_TLender> _TStrongPointerNR;
		return impl::ns_xscope_borrowing_strong_pointer_store::make_xscope_borrowing_strong_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), src_xs_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TLender>
	auto make_xscope_borrowing_strong_pointer_store(_TLender* src_xs_ptr) {
		typedef mse::impl::remove_reference_t<_TLender> _TStrongPointerNR;
		return impl::ns_xscope_borrowing_strong_pointer_store::make_xscope_borrowing_strong_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), src_xs_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_borrowing_strong_const_pointer_store)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_borrowing_strong_pointer_store)


	namespace rsv {
		namespace impl {
			template<typename _Ty>
			class TContainsNonOwningScopeReferenceWrapper : public _Ty, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase {
				typedef _Ty base_class;
			public:
				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TContainsNonOwningScopeReferenceWrapper, _Ty);
				MSE_IMPL_DESTRUCTOR_PREFIX1 ~TContainsNonOwningScopeReferenceWrapper() {}
			};
		}
		template<typename _TLambda>
		auto make_xscope_reference_or_pointer_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(mse::rsv::impl::TContainsNonOwningScopeReferenceWrapper<_TLambda>(lambda));
		}
		template<typename _TLambda>
		auto make_xscope_non_reference_or_pointer_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(lambda);
		}
		template<typename _TLambda>
		auto make_xscope_capture_lambda(const _TLambda& lambda) {
			return make_xscope_non_reference_or_pointer_capture_lambda(lambda);
		}
		template<typename _TLambda>
		auto make_xscope_non_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(lambda);
		}
	}

	namespace us {
		namespace impl {
			/* The new() operator of scope objects is (often) private. The implementation of some elements (like those that
			use std::any<> or std::function<> type-erasure) may require access to the new() operator. This is just a
			transparent wrapper that doesn't "hide" its new() operator and can be used to wrap scope objects that do. */
			template<typename _TROy>
			class TNewableXScopeObj : public _TROy {
			public:
				typedef _TROy base_class;
				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TNewableXScopeObj, base_class);

				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
				MSE_DEFAULT_OPERATOR_NEW_DECLARATION

#ifdef __apple_build_version__
				auto operator&() const { return this; }
				auto operator&() { return this; }
#endif // __apple_build_version__

			};
			namespace impl {
				template<typename _Ty>
				auto make_newable_xscope_helper(std::false_type, const _Ty& arg) {
					/* Objects that don't derive from mse::us::impl::XScopeTagBase generally don't hide their new() operators
					and may not be usable as a base class. */
					return arg;
				}
				template<typename _Ty>
				auto make_newable_xscope_helper(std::false_type, _Ty&& arg) {
					/* Objects that don't derive from mse::us::impl::XScopeTagBase generally don't hide their new() operators
					and may not be usable as a base class. */
					return MSE_FWD(arg);
				}

				template<typename _Ty>
				auto make_newable_xscope_helper(std::true_type, const _Ty& arg) {
					return TNewableXScopeObj<_Ty>(arg);
				}
				template<typename _Ty>
				auto make_newable_xscope_helper(std::true_type, _Ty&& arg) {
					return TNewableXScopeObj<_Ty>(MSE_FWD(arg));
				}
			}
			template <class X>
			auto make_newable_xscope(const X& arg) {
				typedef mse::impl::remove_reference_t<X> nrX;
				return impl::make_newable_xscope_helper<nrX>(typename std::is_base_of<mse::us::impl::XScopeTagBase, nrX>::type(), arg);
			}
			template <class X>
			auto make_newable_xscope(X&& arg) {
				typedef mse::impl::remove_reference_t<X> nrX;
				return impl::make_newable_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TNewableXScopeObj>::type(), MSE_FWD(arg));
			}
		}
	}


	/* The purpose of the xscope_chosen_pointer() function is simply to take two scope pointers as input parameters and return (a copy
	of) one of them. Which of the pointers is returned is determined by a "decider" function that is passed, as the first parameter, to
	xscope_chosen_pointer(). The "decider" function needs to return a bool and take the two scope pointers as its first two parameters.
	The reason this xscope_chosen_pointer() function is needed is that (non-owning) scope pointers are, in general, not allowed to be
	used as a function return value. (Because you might accidentally return a pointer to a local scope object (which is bad)
	instead of one of the pointers given as an input parameter (which is fine).) So the xscope_chosen_pointer() template is the
	sanctioned way of creating a function that returns a non-owning scope pointer. */
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeObjFixedConstPointer<_Ty>& a, const TXScopeObjFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeObjFixedPointer<_Ty>& a, const TXScopeObjFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeObjFixedConstPointer<_Ty>& a, const TXScopeObjFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeObjFixedPointer<_Ty>& a, const TXScopeObjFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	/* Just the generalization of xscope_chosen_pointer(). */
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto chosen(const _TBoolFunction& function1, const _Ty& a, const _Ty& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen(const _TBoolFunction& function1, const _Ty& a, const _Ty& b, Args&&... args) {
		return chosen(function1, a, b, std::forward<Args>(args)...);
	}
	template<typename _Ty>
	auto chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
		return chosen(choose_the_second, a, b);
	}

	/* shorter aliases */
	template<typename _TROy> using so = TXScopeObj<_TROy>;
	template<typename _Ty> using sifp = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using sifcp = TXScopeFixedConstPointer<_Ty>;

	template<typename _TROy> using xs_obj = TXScopeObj<_TROy>;
	template<typename _Ty> using xs_fptr = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using xs_fcptr = TXScopeFixedConstPointer<_Ty>;


	namespace self_test {
		class CXScpPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class A {
				public:
					A(int x) : b(x) {}
					A(const A& _X) : b(_X.b) {}
					A(A&& _X) : b(MSE_FWD(_X).b) {}
					virtual ~A() {}
					A& operator=(A&& _X) { b = MSE_FWD(_X).b; return (*this); }
					A& operator=(const A& _X) { b = _X.b; return (*this); }

					int b = 3;
				};
				class B {
				public:
					static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
					static int foo2(mse::TXScopeFixedPointer<A> A_scope_ptr) { return A_scope_ptr->b; }
				};

				A* A_native_ptr = nullptr;

				{
					A a(7);
					mse::TXScopeObj<A> scope_a(7);
					/* mse::TXScopeObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
					in almost all cases. */

					assert(a.b == scope_a.b);
					A_native_ptr = &a;

					mse::TXScopeFixedPointer<A> A_scope_ptr1(&scope_a);
					assert(A_native_ptr->b == A_scope_ptr1->b);
					mse::TXScopeFixedPointer<A> A_scope_ptr2 = &scope_a;

					if (!A_scope_ptr2) {
						assert(false);
					}
					else if (!(A_scope_ptr2 != A_scope_ptr1)) {
						int q = B::foo2(A_scope_ptr2);
					}
					else {
						assert(false);
					}

					mse::us::impl::TPointerForLegacy<A> pfl_ptr1 = &a;
					if (!(pfl_ptr1 != nullptr)) {
						assert(false);
					}
					mse::us::impl::TPointerForLegacy<A> pfl_ptr2 = nullptr;
					if (!(pfl_ptr1 != pfl_ptr2)) {
						assert(false);
					}

					A a2 = a;
					mse::TXScopeObj<A> scope_a2 = scope_a;
					scope_a2 = a;
					scope_a2 = scope_a;

					mse::TXScopeFixedConstPointer<A> rcp = A_scope_ptr1;
					mse::TXScopeFixedConstPointer<A> rcp2 = rcp;
					const mse::TXScopeObj<A> cscope_a(11);
					mse::TXScopeFixedConstPointer<A> rfcp = &cscope_a;

					mse::TXScopeOwnerPointer<A> A_scpoptr(11);
					//B::foo2(A_scpoptr);
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

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					mse::TXScopeObj<GE> scope_ge;
					mse::TXScopeFixedPointer<GE> GE_scope_ifptr1 = &scope_ge;
					mse::TXScopeFixedPointer<E> E_scope_ifptr5 = GE_scope_ifptr1;
					mse::TXScopeFixedPointer<E> E_scope_ifptr2(&scope_ge);
					mse::TXScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_ge;
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
						static int foo2(mse::TXScopeFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
						static int foo3(mse::TXScopeFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
					};

					mse::TXScopeObj<A> a_scpobj(5);
					int res1 = (&a_scpobj)->b;
					int res2 = B::foo2(&a_scpobj);
					int res3 = B::foo3(&a_scpobj);
					mse::TXScopeOwnerPointer<A> a_scpoptr(7);
					//int res4 = B::foo2(a_scpoptr);
					int res4b = B::foo2(&(*a_scpoptr));

					/* You can use the "mse::make_xscope_pointer_to_member_v2()" function to obtain a safe pointer to a member of
					an xscope object. */
					auto s_safe_ptr1 = mse::make_xscope_pointer_to_member_v2((&a_scpobj), &A::s);
					(*s_safe_ptr1) = "some new text";
					auto s_safe_const_ptr1 = mse::make_xscope_const_pointer_to_member_v2((&a_scpobj), &A::s);
				}

				{
					int a(7);
					mse::TXScopeObj<int> scope_a(7);
					/* Use of scalar types (that can't be used as base class types) with TXScopeObj<> is not well supported. So,
					for example, rather than mse::TXScopeObj<int>, mse::TXScopeObj<mse::CInt> would be preferred. */

					auto int_native_ptr = &a;

					mse::TXScopeFixedPointer<int> int_scope_ptr1 = &scope_a;
					mse::TXScopeFixedPointer<int> int_scope_ptr2 = int_scope_ptr1;

					if (!int_scope_ptr2) {
						assert(false);
					}
					else if (!(int_scope_ptr2 != int_scope_ptr1)) {
						int q = 5;
					}
					else {
						assert(false);
					}

					int a2 = a;
					mse::TXScopeObj<int> scope_a2 = scope_a;
					scope_a2 = a;
					scope_a2 = scope_a;

					mse::TXScopeFixedConstPointer<int> rcp = int_scope_ptr1;
					mse::TXScopeFixedConstPointer<int> rcp2 = rcp;
					const mse::TXScopeObj<int> cscope_a(11);
					mse::TXScopeFixedConstPointer<int> rfcp = &cscope_a;

					mse::TXScopeOwnerPointer<int> int_scpoptr(11);
					auto int_scpptr = &*int_scpoptr;
				}

				{
					/* Polymorphic conversions. */
					class E {
					public:
						int m_b = 5;
					};

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					mse::TXScopeObj<GE> scope_ge;
					mse::TXScopeFixedPointer<GE> GE_scope_ifptr1 = &scope_ge;
					mse::TXScopeFixedPointer<E> E_scope_ptr5(GE_scope_ifptr1);
					mse::TXScopeFixedPointer<E> E_scope_ifptr2(&scope_ge);
					mse::TXScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_ge;
				}

#endif // MSE_SELF_TESTS
			}
		};
	}
}

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("_NOEXCEPT")
#pragma pop_macro("MSE_THROW")
#pragma pop_macro("_NODISCARD")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

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

#endif // MSESCOPE_H_
