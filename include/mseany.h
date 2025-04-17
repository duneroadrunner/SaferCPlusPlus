#pragma once
#ifndef MSEANY_H_
#define MSEANY_H_

/* The implementation of "any" is based on the open source one from https://github.com/thelink2012/any.

Note that this (pre-C++17) implementation doesn't really support over-aligned types.
*/

//
// Implementation of N4562 std::experimental::any (merged into C++17) for C++11 compilers.
//
// See also:
//   + http://en.cppreference.com/w/cpp/any
//   + http://en.cppreference.com/w/cpp/experimental/any
//   + http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4562.html#any
//   + https://cplusplus.github.io/LWG/lwg-active.html#2509
//
//
// Copyright (c) 2016 Denilson das Merc?s Amorim
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <typeinfo>
#include <type_traits>
#include <stdexcept>

#include "msepointerbasics.h"

#ifdef MSE_HAS_CXX17
#include <any>
#endif // MSE_HAS_CXX17

#include "mseoptional.h"

#ifdef MSE_SELF_TESTS
#include <iostream>
#include <stdlib.h> // we include this after including iostream as a workaround for an apparent bug in libtooling8
#include <string>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <complex>
#include <functional>
#endif // MSE_SELF_TESTS

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#pragma push_macro("_NODISCARD")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NODISCARD
#define _NODISCARD
#endif /*_NODISCARD*/

namespace mse {

	class any;

	namespace us {
		namespace impl {
			template <typename _Ty> class TAnyPointerBaseV1;
			template <typename _Ty> class TAnyConstPointerBaseV1;
			template <typename _Ty> class TAnyRandomAccessIteratorBase;
			template <typename _Ty> class TAnyRandomAccessConstIteratorBase;

			namespace ns_any {
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
						&& (!std::is_convertible<typename std::decay<ValueType>::type *, any const *>::value)> MSE_IMPL_EIS >
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

					template<typename _Ty2> friend class mse::us::impl::TAnyPointerBaseV1;
					template<typename _Ty2> friend class mse::us::impl::TAnyConstPointerBaseV1;
					template<typename _Ty2> friend class mse::us::impl::TAnyRandomAccessIteratorBase;
					template<typename _Ty2> friend class mse::us::impl::TAnyRandomAccessConstIteratorBase;
					friend class mse::any;
				};



				namespace detail
				{
					template<typename ValueType>
					inline ValueType any_cast_move_if_true(mse::impl::remove_reference_t<ValueType>* p, std::true_type)
					{
						return std::move(*p);
					}

					template<typename ValueType>
					inline ValueType any_cast_move_if_true(mse::impl::remove_reference_t<ValueType>* p, std::false_type)
					{
						return *p;
					}
				}

				/// Performs *any_cast<add_const_t<remove_reference_t<ValueType>>>(std::addressof(operand)), or throws bad_any_cast on failure.
				template<typename ValueType>
				inline ValueType any_cast(const any& operand)
				{
					auto p = any_cast<typename std::add_const<mse::impl::remove_reference_t<ValueType> >::type>(std::addressof(operand));
					if (p == nullptr) MSE_THROW(bad_any_cast());
					return *p;
				}

				/// Performs *any_cast<remove_reference_t<ValueType>>(std::addressof(operand)), or throws bad_any_cast on failure.
				template<typename ValueType>
				inline ValueType any_cast(any& operand)
				{
					auto p = any_cast<mse::impl::remove_reference_t<ValueType>>(std::addressof(operand));
					if (p == nullptr) MSE_THROW(bad_any_cast());
					return *p;
				}

				///
				/// If ANY_IMPL_ANYCAST_MOVEABLE is not defined, does as N4562 specifies:
				///     Performs *any_cast<remove_reference_t<ValueType>>(std::addressof(operand)), or throws bad_any_cast on failure.
				///
				/// If ANY_IMPL_ANYCAST_MOVEABLE is defined, does as LWG Defect 2509 specifies:
				///     If ValueType is MoveConstructible and isn't a lvalue reference, performs
				///     std::move(*any_cast<remove_reference_t<ValueType>>(std::addressof(operand))), otherwise
				///     *any_cast<remove_reference_t<ValueType>>(std::addressof(operand)). Throws bad_any_cast on failure.
				///
				template<typename ValueType>
				inline ValueType any_cast(any&& operand)
				{
#define ANY_IMPL_ANY_CAST_MOVEABLE
#ifdef ANY_IMPL_ANY_CAST_MOVEABLE
					// https://cplusplus.github.io/LWG/lwg-active.html#2509
					using can_move = std::integral_constant<bool,
						std::is_move_constructible<ValueType>::value
						&& !std::is_lvalue_reference<ValueType>::value>;
#else
					using can_move = std::false_type;
#endif

					auto p = any_cast<mse::impl::remove_reference_t<ValueType>>(std::addressof(operand));
					if (p == nullptr) MSE_THROW(bad_any_cast());
					return detail::any_cast_move_if_true<ValueType>(p, can_move());
				}

				/// If operand != nullptr && operand->type() == typeid(ValueType), a pointer to the object
				/// contained by operand, otherwise nullptr.
				template<typename T>
				inline const T* any_cast(const any* operand) noexcept
				{
					if (operand == nullptr || !operand->is_typed(typeid(T)))
						return nullptr;
					else
						return operand->cast<T>();
				}

				/// If operand != nullptr && operand->type() == typeid(ValueType), a pointer to the object
				/// contained by operand, otherwise nullptr.
				template<typename T>
				inline T* any_cast(any* operand) noexcept
				{
					if (operand == nullptr || !operand->is_typed(typeid(T)))
						return nullptr;
					else
						return operand->cast<T>();
				}
			}

		}
	}
}

namespace std
{
	inline void swap(mse::us::impl::ns_any::any& lhs, mse::us::impl::ns_any::any& rhs) noexcept
	{
		lhs.swap(rhs);
	}
}

namespace mse
{
	typedef mse::us::impl::ns_any::bad_any_cast bad_any_cast;

	namespace impl {
		namespace ns_any {
			template<typename _Tz, typename _TTarget, typename _TPointer>
			struct points_to_helper1 : std::false_type {};
			template<typename _TTarget, typename _TPointer>
			struct points_to_helper1<std::true_type, _TTarget, _TPointer> : std::is_convertible<mse::impl::remove_reference_t<decltype(*std::declval<_TPointer>())>*, _TTarget*> {};

			template<typename _TTarget, typename _TPointer>
			struct points_to : points_to_helper1<typename mse::impl::IsDereferenceable_pb<_TPointer>::type, _TTarget, _TPointer> {};
		}
	}


	/* This macro roughly simulates constructor inheritance. */
#define MSE_ANY_USING(Derived, Base) MSE_USING_SANS_INITIALIZER_LISTS(Derived, Base)

	template <class _TLender>
	class xscope_borrowing_fixed_any;

	namespace us {
		namespace impl {
			template<class TDynamicContainer, class TElement/* = void*/> class Txscope_const_any_structure_lock_guard;
			template<class TDynamicContainer, class TElement/* = void*/> class Txscope_any_structure_lock_guard;

			namespace ns_any {
#if defined(MSE_HAS_CXX17) && defined(MSE_SCOPEPOINTER_DISABLED)
				using any_base1 = std::any;
				using bad_any_cast_base = std::bad_any_cast;
#else // defined(MSE_HAS_CXX17) && defined(MSE_SCOPEPOINTER_DISABLED)
				using any_base1 = mse::us::impl::ns_any::any;
				using bad_any_cast_base = mse::us::impl::ns_any::bad_any_cast;
#endif // defined(MSE_HAS_CXX17) && defined(MSE_SCOPEPOINTER_DISABLED)


				template<typename TAny>
				auto pointer_to_base_any_member() {
					return &TAny::m_base_any;
				}

				template<typename TAny>
				auto contained_any(const TAny& any1) -> decltype(any1.contained_any()) {
					return any1.contained_any();
				}
				template<typename TAny>
				auto contained_any(TAny&& any1) -> decltype(MSE_FWD(any1).contained_any()) {
					return MSE_FWD(any1).contained_any();
				}
				template<typename TAny>
				auto unchecked_contained_any(const TAny& any1) -> decltype(any1.unchecked_contained_any()) {
					return any1.unchecked_contained_any();
				}
				template<typename TAny>
				auto unchecked_contained_any(TAny&& any1) -> decltype(MSE_FWD(any1).unchecked_contained_any()) {
					return MSE_FWD(any1).unchecked_contained_any();
				}


				struct any_base2_const_lockable_tag {}; struct any_base2_not_const_lockable_tag {};

				template <class _TStateMutex/* = mse::non_thread_safe_shared_mutex*/, class TConstLockableIndicator/* = any_base2_not_const_lockable_tag*/>
				class any_base2
#if defined(MSE_HAS_CXX17) && defined(MSE_SCOPEPOINTER_DISABLED)
					/* When MSE_SCOPEPOINTER_DISABLED is not defined, xscope objects declare their `new()` operators as
					private (i.e. inaccessible), so some implementations of std::any may not support such xscope objects.
					So instead we use our local implementation of `any` (which is declared as a friend of xscope objects). */

					: private mse::impl::TOpaqueWrapper<any_base1 >{
				public:
					typedef mse::impl::TOpaqueWrapper<any_base1 > base_class;
					typedef any_base1 _MO;

				private:
					const _MO& contained_any() const& { access_guard{ m_access_mutex }; return base_class::value(); }
					//const _MO&& contained_any() const&& { access_guard{ m_access_mutex }; return base_class::value(); }
					_MO& contained_any()& { access_guard{ m_access_mutex }; return base_class::value(); }
					_MO&& contained_any()&& {
						access_guard{ m_access_mutex };
						/* We're making sure that the any is not "structure locked", because in that case it might not be
						safe to to allow the contained any to be moved from (when made movable with std::move()). */
						structure_change_guard lock1(m_structure_change_mutex);
						return std::move(base_class::value());
					}

					const _MO& unchecked_contained_any() const& { return base_class::value(); }
					//const _MO&& unchecked_contained_any() const&& { return base_class::value(); }
					_MO& unchecked_contained_any()& { return base_class::value(); }
					_MO&& unchecked_contained_any()&& { return std::move(base_class::value()); }

				public:
					using base_class::base_class;
					template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_t<any_base2, T2> >
					explicit any_base2(T2&& _X) : base_class(MSE_FWD(_X)) {}

					any_base2(const _MO& src) : base_class(src) {}
					any_base2(_MO&& src) : base_class(MSE_FWD(src)) {}

					template <class ValueType, class... Args>
					auto& emplace(Args&&... args) {
						structure_change_guard lock1(m_structure_change_mutex);
						return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
					}
					template <class ValueType, class U, class... Args>
					auto& emplace(std::initializer_list<U> il, Args&&... args) {
						structure_change_guard lock1(m_structure_change_mutex);
						return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
					}

#else // defined(MSE_HAS_CXX17) && defined(MSE_SCOPEPOINTER_DISABLED)
					: public any_base1 {
				public:
					typedef any_base1 base_class;
					typedef base_class _MO;

				private:
					const _MO& contained_any() const& { access_guard{ m_access_mutex }; return (*this); }
					const _MO& contained_any() const&& { access_guard{ m_access_mutex }; return (*this); }
					_MO& contained_any()& { access_guard{ m_access_mutex }; return (*this); }
					auto contained_any()&& -> decltype(mse::us::impl::as_ref<base_class>(std::move(*this))) {
						access_guard{ m_access_mutex };
						/* We're making sure that the any is not "structure locked", because in that case it might not be
						safe to to allow the contained any to be moved from (when made movable with std::move()). */
						structure_change_guard lock1(m_structure_change_mutex);
						return mse::us::impl::as_ref<base_class>(std::move(*this));
					}

					const _MO& unchecked_contained_any() const& { return (*this); }
					const _MO& unchecked_contained_any() const&& { return (*this); }
					_MO& unchecked_contained_any()& { return (*this); }
					auto unchecked_contained_any()&& -> decltype(mse::us::impl::as_ref<base_class>(std::move(*this))) {
						return mse::us::impl::as_ref<base_class>(std::move(*this));
					}

				public:
					using base_class::base_class;
					MSE_ANY_USING(any_base2, base_class);
					//template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_t<any_base2, T2> >
					//explicit any_base2(T2&& _X) : base_class(MSE_FWD(_X)) {}

					template <class ValueType, class... Args>
					auto& emplace(Args&&... args) {
						(*this) = ValueType(std::forward<Args>(args)...);
						auto item_ptr = any_cast<ValueType>(this);
						assert(item_ptr);
						return *item_ptr;
					}
					template <class ValueType, class U, class... Args>
					auto& emplace(std::initializer_list<U> il, Args&&... args) {
						(*this) = ValueType(il, std::forward<Args>(args)...);
						auto item_ptr = any_cast<ValueType>(this);
						assert(item_ptr);
						return *item_ptr;
					}
#endif // defined(MSE_HAS_CXX17) && defined(MSE_SCOPEPOINTER_DISABLED)
						typedef any_base2 _Myt;

					any_base2(const any_base2& src_ref) : base_class((src_ref).contained_any()) {}
					any_base2(any_base2&& src_ref) : base_class(MSE_FWD(src_ref).contained_any()) {}

					~any_base2() {
						mse::impl::destructor_lock_guard1<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					}

					_NODISCARD /*constexpr */bool has_value() const noexcept {
						return contained_any().has_value();
					}

					any_base2& operator=(const any_base2& rhs) {
						if (std::addressof(rhs) == this) { return (*this); }
						structure_change_guard lock1(m_structure_change_mutex);
						contained_any().operator=(rhs.contained_any());
						return (*this);
					}
					any_base2& operator=(any_base2&& rhs) {
						if (std::addressof(rhs) == this) { return (*this); }
						structure_change_guard lock1(m_structure_change_mutex);
						contained_any().operator=(MSE_FWD(rhs).contained_any());
						return (*this);
					}
					template<typename ...Args>
					any_base2& operator=(Args&&...args) {
						structure_change_guard lock1(m_structure_change_mutex);
						contained_any().operator=(std::forward<Args>(args)...);
						return (*this);
					}

					/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
					const std::type_info& type() const noexcept
					{
						return contained_any().type();
					}

					template<class T2>
					void swap(T2& rhs) {
						structure_change_guard lock1(m_structure_change_mutex);
						contained_any().swap(rhs.contained_any());
					}
					void reset() {
						structure_change_guard lock1(m_structure_change_mutex);
						contained_any().reset();
					}

					auto structure_change_lock() const { m_structure_change_mutex.lock(); }
					auto structure_change_unlock() const { m_structure_change_mutex.unlock(); }
					auto access_lock() const { m_access_mutex.lock(); }
					auto access_unlock() const { m_access_mutex.unlock(); }

				private:

					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					class structure_change_guard {
					public:
						structure_change_guard(_TStateMutex& _Mtx) MSE_FUNCTION_TRY : m_lock_guard(_Mtx) {}
						MSE_FUNCTION_CATCH_ANY {
							MSE_THROW(mse::structure_lock_violation_error("structure lock violation - Attempting to modify \
							the structure (size/capacity) of a container while a reference to one of its elements \
							still exists?"));
						}
					private:
						std::lock_guard<_TStateMutex> m_lock_guard;
					};

					class access_guard {
					public:
						access_guard(mse::non_thread_safe_mutex& _Mtx) MSE_FUNCTION_TRY : m_lock_guard(_Mtx) {}
						MSE_FUNCTION_CATCH_ANY{
							MSE_THROW(mse::structure_lock_violation_error("access lock violation - Attempting to access \
							the object while it is being borrowed?"));
						}
					private:
						std::lock_guard<mse::non_thread_safe_mutex> m_lock_guard;
					};

					mutable _TStateMutex m_structure_change_mutex;
					mutable mse::non_thread_safe_mutex m_access_mutex;

					template<typename TAny>
					friend auto us::impl::ns_any::contained_any(const TAny& any1) -> decltype(any1.contained_any());
					template<typename TAny>
					friend auto us::impl::ns_any::contained_any(TAny&& any1) -> decltype(MSE_FWD(any1).contained_any());
					template<typename TAny>
					friend auto us::impl::ns_any::unchecked_contained_any(const TAny& any1) -> decltype(any1.unchecked_contained_any());
					template<typename TAny>
					friend auto us::impl::ns_any::unchecked_contained_any(TAny&& any1) -> decltype(MSE_FWD(any1).unchecked_contained_any());

					template<class TDynamicContainer, class TElement>
					friend class mse::us::impl::Txscope_any_structure_lock_guard;
					template<class TDynamicContainer, class TElement>
					friend class mse::us::impl::Txscope_const_any_structure_lock_guard;
					template <class _TLender2>
					friend class mse::xscope_borrowing_fixed_any;
				};

				// 20.5.12, Specialized algorithms
				template <class _TStateMutex, class TConstLockableIndicator>
				void swap(any_base2<_TStateMutex, TConstLockableIndicator>& x, any_base2<_TStateMutex, TConstLockableIndicator>& y) noexcept(noexcept(x.swap(y)))
				{
					x.swap(y);
				}


				/// Performs *any_cast<add_const_t<remove_reference_t<ValueType>>>(std::addressof(operand)), or throws bad_any_cast on failure.
				template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
				inline ValueType any_cast(const any_base2<_TStateMutex, TConstLockableIndicator>& operand)
				{
					auto p = any_cast<typename std::add_const<mse::impl::remove_reference_t<ValueType> >::type>(std::addressof(operand));
					if (p == nullptr) MSE_THROW(bad_any_cast());
					return *p;
				}

				/// Performs *any_cast<remove_reference_t<ValueType>>(std::addressof(operand)), or throws bad_any_cast on failure.
				template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
				inline ValueType any_cast(any_base2<_TStateMutex, TConstLockableIndicator>& operand)
				{
					auto p = any_cast<mse::impl::remove_reference_t<ValueType>>(std::addressof(operand));
					if (p == nullptr) MSE_THROW(bad_any_cast());
					return *p;
				}

				///
				/// If ANY_IMPL_ANYCAST_MOVEABLE is not defined, does as N4562 specifies:
				///     Performs *any_cast<remove_reference_t<ValueType>>(std::addressof(operand)), or throws bad_any_cast on failure.
				///
				/// If ANY_IMPL_ANYCAST_MOVEABLE is defined, does as LWG Defect 2509 specifies:
				///     If ValueType is MoveConstructible and isn't a lvalue reference, performs
				///     std::move(*any_cast<remove_reference_t<ValueType>>(std::addressof(operand))), otherwise
				///     *any_cast<remove_reference_t<ValueType>>(std::addressof(operand)). Throws bad_any_cast on failure.
				///
				template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
				inline ValueType any_cast(any_base2<_TStateMutex, TConstLockableIndicator>&& operand)
				{
					return any_cast<ValueType>(unchecked_contained_any(MSE_FWD(operand)));
				}

				/// If operand != nullptr && operand->type() == typeid(ValueType), a pointer to the object
				/// contained by operand, otherwise nullptr.
				template<typename T, class _TStateMutex, class TConstLockableIndicator>
				inline const T* any_cast(const any_base2<_TStateMutex, TConstLockableIndicator>* operand) noexcept
				{
					return any_cast<T>(std::addressof(unchecked_contained_any(*operand)));
				}

				/// If operand != nullptr && operand->type() == typeid(ValueType), a pointer to the object
				/// contained by operand, otherwise nullptr.
				template<typename T, class _TStateMutex, class TConstLockableIndicator>
				inline T* any_cast(any_base2<_TStateMutex, TConstLockableIndicator>* operand) noexcept
				{
					return any_cast<T>(&(unchecked_contained_any(*operand)));
				}


				template<typename T, typename TPointerToAny>
				auto any_cast_make_xscope_pointer_helper3(std::false_type, T& target_ref, const TPointerToAny& operand) {
					return mse::us::make_xscope_syncweak(target_ref, operand);
				}
				template<typename T, typename TPointerToAny>
				auto any_cast_make_xscope_pointer_helper3(std::true_type, T& target_ref, const TPointerToAny& operand) {
					return mse::us::make_xscope_strong(target_ref, operand);
				}
				template<typename T, typename TPointerToAny>
				auto any_cast_make_pointer_helper3(std::false_type, T& target_ref, const TPointerToAny& operand) {
					return mse::us::make_syncweak(target_ref, operand);
				}
				template<typename T, typename TPointerToAny>
				auto any_cast_make_pointer_helper3(std::true_type, T& target_ref, const TPointerToAny& operand) {
					return mse::us::make_strong(target_ref, operand);
				}
				template<typename T, typename TPointerToAny>
				auto any_cast_make_maybe_pointer_helper2(std::false_type, T& target_ref, const TPointerToAny& operand) {
					return mse::make_xscope_fixed_optional(any_cast_make_xscope_pointer_helper3(typename mse::impl::is_strong_ptr<TPointerToAny>::type(), target_ref, operand));
				}
				template<typename T, typename TPointerToAny>
				auto any_cast_make_maybe_pointer_helper2(std::true_type, T& target_ref, const TPointerToAny& operand) {
					return mse::make_fixed_optional(any_cast_make_pointer_helper3(typename mse::impl::is_strong_ptr<TPointerToAny>::type(), target_ref, operand));
				}
				template<typename T, typename TPointerToAny>
				auto any_cast_make_maybe_pointer_helper1(T& target_ref, const TPointerToAny& operand) {
					return any_cast_make_maybe_pointer_helper2(typename mse::impl::is_potentially_not_xscope<TPointerToAny>::type(), target_ref, operand);
				}
				template<typename T, typename TPointerToAny>
				inline auto any_cast_make_maybe_pointer_helper01(T* cast_res, const TPointerToAny& operand) /*noexcept*/
				{
					typedef decltype(any_cast_make_maybe_pointer_helper1(*cast_res, operand)) return_t;
					if (nullptr == cast_res) {
						return return_t{};
					}
					else {
						return any_cast_make_maybe_pointer_helper1(*cast_res, operand);
					}
				}

				template<typename T, typename TPointerToAny, class _TStateMutex, class TConstLockableIndicator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::ns_any::points_to<any_base2<_TStateMutex, TConstLockableIndicator>, TPointerToAny>::value)> MSE_IMPL_EIS >
				inline auto maybe_any_cast_ptr(const TPointerToAny& operand) /*noexcept*/
				{
					auto cast_res = any_cast<T>(std::addressof((*operand).unchecked_contained_any()));
					return any_cast_make_maybe_pointer_helper01(cast_res, operand);
				}


				template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
				inline auto maybe_any_cast(const any_base2<_TStateMutex, TConstLockableIndicator>& operand)
				{
					typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_optional<ValueType>, mse::optional<ValueType> >::type retval_t;
					auto p = any_cast<typename std::add_const<mse::impl::remove_reference_t<ValueType> >::type>(std::addressof(operand));
					if (p == nullptr) {
						return retval_t{};
					}
					return retval_t(*p);
				}

				template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
				inline auto maybe_any_cast(any_base2<_TStateMutex, TConstLockableIndicator>&& operand)
				{
					typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_optional<ValueType>, mse::optional<ValueType> >::type retval_t;
					auto p = any_cast<typename std::add_const<mse::impl::remove_reference_t<ValueType> >::type>(std::addressof(unchecked_contained_any(operand)));
					if (p == nullptr) {
						return retval_t{};
					}
					return retval_t(any_cast<ValueType>(unchecked_contained_any(MSE_FWD(operand))));
				}
			}
		}
	}

	typedef typename mse::us::impl::ns_any::bad_any_cast bad_any_cast;


	template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
	inline ValueType any_cast(const mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>& operand) {
		return mse::us::impl::ns_any::any_cast<ValueType>(operand);
	}
	template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
	inline ValueType any_cast(mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>& operand) {
		return mse::us::impl::ns_any::any_cast<ValueType>(operand);
	}
	template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
	inline ValueType any_cast(mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>&& operand) {
		return mse::us::impl::ns_any::any_cast<ValueType>(MSE_FWD(operand));
	}
	template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
	inline auto maybe_any_cast(const mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>& operand) {
		return mse::us::impl::ns_any::maybe_any_cast<ValueType>(operand);
	}
	template<typename ValueType, class _TStateMutex, class TConstLockableIndicator>
	inline auto maybe_any_cast(mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>&& operand) {
		return mse::us::impl::ns_any::maybe_any_cast<ValueType>(MSE_FWD(operand));
	}
	template<typename T, typename TPointerToAny, class _TStateMutex, class TConstLockableIndicator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::ns_any::points_to<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, TPointerToAny>::value)> MSE_IMPL_EIS >
	inline auto maybe_any_cast_ptr(const TPointerToAny& operand) /*noexcept*/ {
		return mse::us::impl::ns_any::maybe_any_cast(operand);
	}
	template<typename T, typename TPointerToAny, class _TStateMutex, class TConstLockableIndicator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::ns_any::points_to<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, TPointerToAny>::value)> MSE_IMPL_EIS >
	inline auto any_cast(const TPointerToAny& operand) /*noexcept*/ {
		auto maybe_ptr = mse::maybe_any_cast<T>(operand);
		auto retval = mse::make_nullable_pointer<mse::impl::remove_reference_t<decltype(maybe_ptr.value())> >(nullptr);
		if (maybe_ptr.has_value()) {
			return mse::make_nullable_pointer(maybe_ptr.value());
		}
		return retval;
	}

	namespace impl {
		namespace ns_any {
			template<typename T, typename TPointerToAny, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TPointerToAny>::value)> MSE_IMPL_EIS >
			inline auto maybe_any_cast_ptr_test1(const TPointerToAny& operand) /*noexcept*/ {
				auto cast_res = any_cast<T>(std::addressof(mse::us::impl::ns_any::contained_any(*operand)));
				return mse::us::impl::ns_any::any_cast_make_maybe_pointer_helper01(cast_res, operand);
			}

			template<class T, class EqualTo>
			struct IsSupportedByContainedAny_any_impl
			{
				template<class U, class V>
				static auto test(U*) -> decltype((mse::us::impl::ns_any::contained_any(std::declval<U>())), mse::us::impl::ns_any::contained_any(std::declval<V>()), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
				static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
			};
			template<class T, class EqualTo = T>
			struct IsSupportedByContainedAny_any : IsSupportedByContainedAny_any_impl<
				mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

			template<class T, class EqualTo>
			struct IsSupportedByMaybeAnyCastPtrHelper1_any_impl
			{
				template<class U, class V>
				static auto test(U*) -> decltype((maybe_any_cast_ptr_test1<int>(std::declval<U>())), (*std::declval<V>()), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
				static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
			};
			template<>
			struct IsSupportedByMaybeAnyCastPtrHelper1_any_impl<void*, void*> : std::false_type {};
			template<class T, class EqualTo = T>
			struct IsSupportedByMaybeAnyCastPtrHelper1_any : IsSupportedByMaybeAnyCastPtrHelper1_any_impl<
				mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

			template<class T, class U>
			struct IsSupportedByMaybeAnyCastPtrHelper2_any : std::false_type {};
			template<class T>
			struct IsSupportedByMaybeAnyCastPtrHelper2_any<T, std::true_type> : IsSupportedByMaybeAnyCastPtrHelper1_any<T> {};

			template<class T, class U>
			struct IsSupportedByMaybeAnyCastPtrHelper3_any : std::false_type {};
			template<class T>
			struct IsSupportedByMaybeAnyCastPtrHelper3_any<T, std::true_type> : IsSupportedByMaybeAnyCastPtrHelper2_any<T, typename IsSupportedByContainedAny_any<mse::impl::remove_reference_t<decltype(*std::declval<T>())> >::type> {};

			template<class T>
			struct IsSupportedByMaybeAnyCastPtr_any : IsSupportedByMaybeAnyCastPtrHelper3_any<T, typename mse::impl::IsDereferenceable_pb<T>::type> {};
		}
	}

	template<typename T, typename TPointerToAny, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::ns_any::IsSupportedByMaybeAnyCastPtr_any<TPointerToAny>::value)> MSE_IMPL_EIS >
	inline auto maybe_any_cast_ptr(const TPointerToAny& operand) /*noexcept*/ {
		auto cast_res = any_cast<T>(std::addressof(mse::us::impl::ns_any::contained_any(*operand)));
		return mse::us::impl::ns_any::any_cast_make_maybe_pointer_helper01(cast_res, operand);
	}

	template<typename T, typename TPointerToAny, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::ns_any::IsSupportedByMaybeAnyCastPtr_any<TPointerToAny>::value)> MSE_IMPL_EIS >
	inline auto any_cast(const TPointerToAny& operand) /*noexcept*/ {
		auto maybe_ptr = mse::maybe_any_cast<T>(operand);
		auto retval = mse::make_nullable_pointer<mse::impl::remove_reference_t<decltype(maybe_ptr.value())> >(nullptr);
		if (maybe_ptr.has_value()) {
			return mse::make_nullable_pointer(maybe_ptr.value());
		}
		return retval;
	}

#define MSE_IMPL_ANY_CAST_DEFINITIONS1(any_t) \
	template<typename ValueType> \
	inline ValueType any_cast(const any_t& operand) { \
		return mse::any_cast<ValueType>(mse::us::impl::ns_any::contained_any(operand)); \
	} \
	template<typename ValueType> \
	inline ValueType any_cast(any_t& operand) { \
		return mse::any_cast<ValueType>(mse::us::impl::ns_any::contained_any(operand)); \
	} \
	template<typename ValueType> \
	inline ValueType any_cast(any_t&& operand) { \
		return mse::any_cast<ValueType>(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))); \
	} \
	template<typename ValueType> \
	inline auto maybe_any_cast(const any_t& operand) { \
		return mse::maybe_any_cast<ValueType>(mse::us::impl::ns_any::contained_any(operand)); \
	} \
	template<typename ValueType> \
	inline auto maybe_any_cast(any_t&& operand) { \
		return mse::maybe_any_cast<ValueType>(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))); \
	}

#define MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1 \
		template<typename TAny> \
		friend auto mse::us::impl::ns_any::pointer_to_base_any_member(); \
		template<typename TAny> \
		friend auto mse::us::impl::ns_any::contained_any(const TAny& any1) -> decltype(any1.contained_any()); \
		template<typename TAny> \
		friend auto mse::us::impl::ns_any::contained_any(TAny&& any1) -> decltype(MSE_FWD(any1).contained_any());

#define MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(delegatee) \
		auto structure_change_lock() const { delegatee.structure_change_lock(); } \
		auto structure_change_unlock() const { delegatee.structure_change_unlock(); } \
		auto access_lock() const { delegatee.access_lock(); } \
		auto access_unlock() const { delegatee.access_unlock(); }


	class any {
	public:
		typedef any _Myt;
		/// Constructs an object of type any with an empty state.
		any() = default;

		/// Constructs an object of type any with an equivalent state as other.
		any(const any& rhs) = default;

		/// Constructs an object of type any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		any(any&& rhs) noexcept = default;

		/// Same effect as this->reset().
		~any() {}

		/// Constructs an object of type any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `any` may be copy constructed into another `any` at any time, so a copy should always be allowed.
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		any(ValueType&& value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)") : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
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
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		any& operator=(ValueType&& value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)")
		{
			static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
				"T shall satisfy the CopyConstructible requirements.");
			any(std::forward<ValueType>(value)).swap(*this);
			return *this;
		}

		template <class ValueType, class... Args>
		auto& emplace(Args&&... args) {
			return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
		}
		template <class ValueType, class U, class... Args>
		auto& emplace(std::initializer_list<U> il, Args&&... args) {
			return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
		}

		/// If not empty, destroys the contained object.
		void reset() noexcept
		{
			contained_any().reset();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

		/// Exchange the states of *this and rhs.
		void swap(any& rhs) noexcept
		{
			contained_any().swap(rhs.m_base_any);
		}

		//typedef mse::us::impl::ns_any::any_base2<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag> base_class;
		typedef mse::us::impl::ns_any::any_base2<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> base_class;

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		base_class m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(m_base_any)

		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_any_structure_lock_guard;
		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_const_any_structure_lock_guard;

		template <class _TLender2>
		friend class mse::xscope_borrowing_fixed_any;

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	any make_any(Args&&... args) {
		return any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	any make_any(std::initializer_list<U> il, Args&&... args) {
		return any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(any)


	class mt_any {
	public:
		/// Constructs an object of type mt_any with an empty state.
		mt_any() = default;

		/// Constructs an object of type mt_any with an equivalent state as other.
		mt_any(const mt_any & rhs) = default;

		/// Constructs an object of type mt_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		mt_any(mt_any && rhs) noexcept = default;

		/// Same effect as this->reset().
		~mt_any() {}

		/// Constructs an object of type mt_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `mt_any` may be copy constructed into another `mt_any` at mt_any time, so a copy should always be allowed.
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, mt_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, mt_any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			&& (mse::impl::is_marked_as_shareable_and_passable_msemsearray<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		mt_any(ValueType && value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)") : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// Has the same effect as mt_any(rhs).swap(*this). No effects if an exception is thrown.
		mt_any& operator=(const mt_any & rhs)
		{
			mt_any(rhs).swap(*this);
			return *this;
		}

		/// Has the same effect as mt_any(std::move(rhs)).swap(*this).
		///
		/// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
		/// but otherwise unspecified state.
		mt_any& operator=(mt_any && rhs) noexcept
		{
			mt_any(std::move(rhs)).swap(*this);
			return *this;
		}

		/// Has the same effect as mt_any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `mt_any` may be copy constructed into another `mt_any` at mt_any time, so a copy should always be allowed.
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, mt_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, mt_any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			&& (mse::impl::is_marked_as_shareable_and_passable_msemsearray<typename std::decay<ValueType>::type>::value)
		> MSE_IMPL_EIS >
		mt_any& operator=(ValueType && value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)")
		{
			static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
				"T shall satisfy the CopyConstructible requirements.");
			mt_any(std::forward<ValueType>(value)).swap(*this);
			return *this;
		}

		template <class ValueType, class... Args>
		auto& emplace(Args&&... args) {
			return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
		}
		template <class ValueType, class U, class... Args>
		auto& emplace(std::initializer_list<U> il, Args&&... args) {
			return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
		}

		/// If not empty, destroys the contained object.
		void reset() noexcept
		{
			contained_any().reset();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

		/// Exchange the states of *this and rhs.
		void swap(mt_any & rhs) noexcept
		{
			contained_any().swap(rhs.m_base_any);
		}

		void async_shareable_tag() const {}
		void async_passable_tag() const {}

		typedef mse::us::impl::ns_any::any_base2<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> base_class;

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		base_class m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(m_base_any)

		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_any_structure_lock_guard;
		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_const_any_structure_lock_guard;

		template <class _TLender2>
		friend class mse::xscope_borrowing_fixed_any;

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	mt_any make_mt_any(Args&&... args) {
		return mt_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	mt_any make_mt_any(std::initializer_list<U> il, Args&&... args) {
		return mt_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(mt_any)


	class st_any {
	public:
		/// Constructs an object of type st_any with an empty state.
		st_any() = default;

		/// Constructs an object of type st_any with an equivalent state as other.
		st_any(const st_any& rhs) = default;

		/// Constructs an object of type st_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		st_any(st_any&& rhs) noexcept = default;

		/// Same effect as this->reset().
		~st_any() {}

		/// Constructs an object of type st_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `st_any` may be copy constructed into another `st_any` at st_any time, so a copy should always be allowed.
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, st_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, st_any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		st_any(ValueType&& value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)") : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// Has the same effect as st_any(rhs).swap(*this). No effects if an exception is thrown.
		st_any& operator=(const st_any& rhs)
		{
			st_any(rhs).swap(*this);
			return *this;
		}

		/// Has the same effect as st_any(std::move(rhs)).swap(*this).
		///
		/// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
		/// but otherwise unspecified state.
		st_any& operator=(st_any&& rhs) noexcept
		{
			st_any(std::move(rhs)).swap(*this);
			return *this;
		}

		/// Has the same effect as st_any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `st_any` may be copy constructed into another `st_any` at st_any time, so a copy should always be allowed.
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, st_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, st_any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		st_any& operator=(ValueType&& value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)")
		{
			static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
				"T shall satisfy the CopyConstructible requirements.");
			st_any(std::forward<ValueType>(value)).swap(*this);
			return *this;
		}

		template <class ValueType, class... Args>
		auto& emplace(Args&&... args) {
			return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
		}
		template <class ValueType, class U, class... Args>
		auto& emplace(std::initializer_list<U> il, Args&&... args) {
			return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
		}

		/// If not empty, destroys the contained object.
		void reset() noexcept
		{
			contained_any().reset();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

		/// Exchange the states of *this and rhs.
		void swap(st_any& rhs) noexcept
		{
			contained_any().swap(rhs.m_base_any);
		}

		typedef mse::us::impl::ns_any::any_base2<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> base_class;

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		base_class m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(m_base_any)

		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_any_structure_lock_guard;
		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_const_any_structure_lock_guard;

		template <class _TLender2>
		friend class mse::xscope_borrowing_fixed_any;

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	st_any make_st_any(Args&&... args) {
		return st_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	st_any make_st_any(std::initializer_list<U> il, Args&&... args) {
		return st_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(st_any)
		
		
	namespace mstd {
		typedef mse::bad_any_cast bad_any_cast;

		
		class any : public mse::st_any {
		public:
			typedef mse::st_any base_class;

#ifdef MSE_HAS_CXX17
			using base_class::base_class;
			//template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_t<any, T2> >
			//explicit any(T2&& _X) : base_class(MSE_FWD(_X)) {}

			any(const base_class& src) : base_class(src) {}
			any(base_class&& src) : base_class(MSE_FWD(src)) {}

#else // MSE_HAS_CXX17
			using base_class::base_class;
			MSE_ANY_USING(any, base_class);
			//template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_t<any, T2> >
			//explicit any(T2&& _X) : base_class(MSE_FWD(_X)) {}
#endif // MSE_HAS_CXX17

			any(const any& src_ref) : base_class(static_cast<const base_class&>(src_ref)) {}

			~any() {}

		private:
			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

			template<class TDynamicContainer, class TElement>
			friend class mse::us::impl::Txscope_any_structure_lock_guard;
			template<class TDynamicContainer, class TElement>
			friend class mse::us::impl::Txscope_const_any_structure_lock_guard;
			template <class _TLender2>
			friend class mse::xscope_borrowing_fixed_any;
		};

		template <class T, class... Args>
		any make_any(Args&&... args) {
			return any(T(std::forward<Args>(args)...));
		}
		template< class T, class U, class... Args >
		any make_any(std::initializer_list<U> il, Args&&... args) {
			return any(T(il, std::forward<Args>(args)...));
		}

		MSE_IMPL_ANY_CAST_DEFINITIONS1(any)

		template<typename T, typename TPointerToAny, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::ns_any::IsSupportedByMaybeAnyCastPtr_any<TPointerToAny>::value)> MSE_IMPL_EIS >
		inline auto any_cast(const TPointerToAny& operand) /*noexcept*/ {
			auto maybe_ptr = maybe_any_cast_ptr<T>(operand);
			auto retval = mse::make_nullable_pointer<mse::impl::remove_reference_t<decltype(maybe_ptr.value())> >(nullptr);
			if (maybe_ptr.has_value()) {
				return mse::make_nullable_pointer(maybe_ptr.value());
			}
			return retval;
		}
	}


	class xscope_any : public mse::us::impl::XScopeTagBase {
	public:
		/// Constructs an object of type xscope_any with an empty state.
		xscope_any() = default;

		/// Constructs an object of type xscope_any with an equivalent state as other.
		xscope_any(const xscope_any& rhs) = default;

		/// Constructs an object of type xscope_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		xscope_any(xscope_any&& rhs) noexcept = default;

		/// Same effect as this->reset().
		~xscope_any() {}

		/// Constructs an object of type xscope_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_any` may be copy constructed into another `xscope_any` at xscope_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_any const*>::value)
			> MSE_IMPL_EIS >
		xscope_any(ValueType&& value) : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// Has the same effect as xscope_any(rhs).swap(*this). No effects if an exception is thrown.
		xscope_any& operator=(const xscope_any& rhs)
		{
			xscope_any(rhs).swap(*this);
			return *this;
		}

		/// Has the same effect as xscope_any(std::move(rhs)).swap(*this).
		///
		/// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
		/// but otherwise unspecified state.
		xscope_any& operator=(xscope_any&& rhs) noexcept
		{
			xscope_any(std::move(rhs)).swap(*this);
			return *this;
		}

		/// Has the same effect as xscope_any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_any` may be copy constructed into another `xscope_any` at xscope_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_any const*>::value)
			> MSE_IMPL_EIS >
		xscope_any& operator=(ValueType&& value)
		{
			static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
				"T shall satisfy the CopyConstructible requirements.");
			xscope_any(std::forward<ValueType>(value)).swap(*this);
			return *this;
		}

		template <class ValueType, class... Args>
		auto& emplace(Args&&... args) {
			return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
		}
		template <class ValueType, class U, class... Args>
		auto& emplace(std::initializer_list<U> il, Args&&... args) {
			return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
		}

		/// If not empty, destroys the contained object.
		void reset() noexcept
		{
			contained_any().reset();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

		/// Exchange the states of *this and rhs.
		void swap(xscope_any& rhs) noexcept
		{
			contained_any().swap(rhs.m_base_any);
		}

		typedef mse::us::impl::ns_any::any_base2<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag> base_class;

	private:

		base_class m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(m_base_any)

		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_any_structure_lock_guard;
		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_const_any_structure_lock_guard;

		template <class _TLender2>
		friend class mse::xscope_borrowing_fixed_any;

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	xscope_any make_xscope_any(Args&&... args) {
		return xscope_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	xscope_any make_xscope_any(std::initializer_list<U> il, Args&&... args) {
		return xscope_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(xscope_any)


	class xscope_mt_any : public mse::us::impl::XScopeTagBase {
	public:
		/// Constructs an object of type xscope_mt_any with an empty state.
		xscope_mt_any() = default;

		/// Constructs an object of type xscope_mt_any with an equivalent state as other.
		xscope_mt_any(const xscope_mt_any& rhs) = default;

		/// Constructs an object of type xscope_mt_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		xscope_mt_any(xscope_mt_any&& rhs) noexcept = default;

		/// Same effect as this->reset().
		~xscope_mt_any() {}

		/// Constructs an object of type xscope_mt_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_mt_any` may be copy constructed into another `xscope_mt_any` at xscope_mt_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_mt_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_mt_any const*>::value)
			&& (mse::impl::is_marked_as_xscope_shareable_and_passable_msemsearray<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		xscope_mt_any(ValueType&& value) : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// Has the same effect as xscope_mt_any(rhs).swap(*this). No effects if an exception is thrown.
		xscope_mt_any& operator=(const xscope_mt_any& rhs)
		{
			xscope_mt_any(rhs).swap(*this);
			return *this;
		}

		/// Has the same effect as xscope_mt_any(std::move(rhs)).swap(*this).
		///
		/// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
		/// but otherwise unspecified state.
		xscope_mt_any& operator=(xscope_mt_any&& rhs) noexcept
		{
			xscope_mt_any(std::move(rhs)).swap(*this);
			return *this;
		}

		/// Has the same effect as xscope_mt_any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_mt_any` may be copy constructed into another `xscope_mt_any` at xscope_mt_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_mt_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_mt_any const*>::value)
			&& (mse::impl::is_marked_as_xscope_shareable_and_passable_msemsearray<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		xscope_mt_any& operator=(ValueType&& value)
		{
			static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
				"T shall satisfy the CopyConstructible requirements.");
			xscope_mt_any(std::forward<ValueType>(value)).swap(*this);
			return *this;
		}

		template <class ValueType, class... Args>
		auto& emplace(Args&&... args) {
			return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
		}
		template <class ValueType, class U, class... Args>
		auto& emplace(std::initializer_list<U> il, Args&&... args) {
			return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
		}

		/// If not empty, destroys the contained object.
		void reset() noexcept
		{
			contained_any().reset();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

		/// Exchange the states of *this and rhs.
		void swap(xscope_mt_any& rhs) noexcept
		{
			contained_any().swap(rhs.m_base_any);
		}

		void xscope_async_shareable_tag() const {}
		void xscope_async_passable_tag() const {}

		typedef mse::us::impl::ns_any::any_base2<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> base_class;

	private:

		base_class m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(m_base_any)

		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_any_structure_lock_guard;
		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_const_any_structure_lock_guard;

		template <class _TLender2>
		friend class mse::xscope_borrowing_fixed_any;

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	xscope_mt_any make_xscope_mt_any(Args&&... args) {
		return xscope_mt_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	xscope_mt_any make_xscope_mt_any(std::initializer_list<U> il, Args&&... args) {
		return xscope_mt_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(xscope_mt_any)


	class xscope_st_any : public mse::us::impl::XScopeTagBase {
	public:
		/// Constructs an object of type xscope_st_any with an empty state.
		xscope_st_any() = default;

		/// Constructs an object of type xscope_st_any with an equivalent state as other.
		xscope_st_any(const xscope_st_any& rhs) = default;

		/// Constructs an object of type xscope_st_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		xscope_st_any(xscope_st_any&& rhs) noexcept = default;

		/// Same effect as this->reset().
		~xscope_st_any() {}

		/// Constructs an object of type xscope_st_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_st_any` may be copy constructed into another `xscope_st_any` at xscope_st_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_st_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_st_any const*>::value)
			> MSE_IMPL_EIS >
		xscope_st_any(ValueType&& value) : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// Has the same effect as xscope_st_any(rhs).swap(*this). No effects if an exception is thrown.
		xscope_st_any& operator=(const xscope_st_any& rhs)
		{
			xscope_st_any(rhs).swap(*this);
			return *this;
		}

		/// Has the same effect as xscope_st_any(std::move(rhs)).swap(*this).
		///
		/// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
		/// but otherwise unspecified state.
		xscope_st_any& operator=(xscope_st_any&& rhs) noexcept
		{
			xscope_st_any(std::move(rhs)).swap(*this);
			return *this;
		}

		/// Has the same effect as xscope_st_any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_st_any` may be copy constructed into another `xscope_st_any` at xscope_st_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_st_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_st_any const*>::value)
			> MSE_IMPL_EIS >
		xscope_st_any& operator=(ValueType&& value)
		{
			static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
				"T shall satisfy the CopyConstructible requirements.");
			xscope_st_any(std::forward<ValueType>(value)).swap(*this);
			return *this;
		}

		template <class ValueType, class... Args>
		auto& emplace(Args&&... args) {
			return contained_any().template emplace<ValueType>(std::forward<Args>(args)...);
		}
		template <class ValueType, class U, class... Args>
		auto& emplace(std::initializer_list<U> il, Args&&... args) {
			return contained_any().template emplace<ValueType>(il, std::forward<Args>(args)...);
		}

		/// If not empty, destroys the contained object.
		void reset() noexcept
		{
			contained_any().reset();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

		/// Exchange the states of *this and rhs.
		void swap(xscope_st_any& rhs) noexcept
		{
			contained_any().swap(rhs.m_base_any);
		}

		typedef mse::us::impl::ns_any::any_base2<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> base_class;

	private:

		base_class m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_LOCK_DELEGATING_DECLARATIONS1(m_base_any)

		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_any_structure_lock_guard;
		template<class TDynamicContainer, class TElement>
		friend class mse::us::impl::Txscope_const_any_structure_lock_guard;

		template <class _TLender2>
		friend class mse::xscope_borrowing_fixed_any;

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	xscope_st_any make_xscope_st_any(Args&&... args) {
		return xscope_st_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	xscope_st_any make_xscope_st_any(std::initializer_list<U> il, Args&&... args) {
		return xscope_st_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(xscope_st_any)


	class fixed_any;
	class xscope_fixed_any;

	namespace us {
		namespace impl {
			namespace ns_any {
				
				class fixed_any_base2
#ifdef MSE_HAS_CXX17
					: private mse::impl::TOpaqueWrapper<any_base1 >, private default_state_mutex {
				public:
					typedef mse::impl::TOpaqueWrapper<any_base1 > base_class;
					typedef any_base1 _MO;

				private:
					const _MO& contained_any() const& { return base_class::value(); }
					//const _MO&& contained_any() const&& { return base_class::value(); }
					_MO& contained_any()& { return base_class::value(); }
					_MO&& contained_any()&& {
						return std::move(base_class::value());
					}

				public:
					using base_class::base_class;
					//template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_t<fixed_any_base2, T2> >
					//explicit fixed_any_base2(T2&& _X) : base_class(MSE_FWD(_X)) {}

					fixed_any_base2(const _MO& src) : base_class(src) {}
					//fixed_any_base2(_MO&& src) : base_class(src) {}

#else // MSE_HAS_CXX17
					: public any_base1, private default_state_mutex {
				public:
					typedef any_base1 base_class;
					typedef base_class _MO;

				private:
					const _MO& contained_any() const& { return (*this); }
					const _MO& contained_any() const&& { return (*this); }
					_MO& contained_any()& { return (*this); }
					auto contained_any()&& -> decltype(mse::us::impl::as_ref<base_class>(std::move(*this))) {
						return mse::us::impl::as_ref<base_class>(std::move(*this));
					}

				public:
					using base_class::base_class;
					MSE_ANY_USING(fixed_any_base2, base_class);
					//template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_t<fixed_any_base2, T2> >
					//explicit fixed_any_base2(T2&& _X) : base_class(_X) {}
#endif // MSE_HAS_CXX17

					typedef default_state_mutex state_mutex_t;
					typedef state_mutex_t _TStateMutex;

					typedef fixed_any_base2 _Myt;

					fixed_any_base2(const fixed_any_base2& src_ref) : base_class((src_ref).contained_any()) {}
					//fixed_any_base2(fixed_any_base2&& src_ref) : base_class((src_ref).contained_any()) {}

					~fixed_any_base2() {
						mse::impl::destructor_lock_guard1<state_mutex_t> lock1(state_mutex1());
					}

					_NODISCARD /*constexpr */bool has_value() const noexcept {
						return contained_any().has_value();
					}

					/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
					const std::type_info& type() const noexcept
					{
						return contained_any().type();
					}

				private:

					state_mutex_t& state_mutex1()& { return (*this); }

					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					friend class mse::fixed_any;
					friend class mse::xscope_fixed_any;
				};
			}
		}
	}

	class fixed_any {
	public:
		/// Constructs an object of type fixed_any with an empty state.
		fixed_any() = default;

		/// Constructs an object of type fixed_any with an equivalent state as other.
		fixed_any(const fixed_any & rhs) = default;

		/// Constructs an object of type fixed_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		fixed_any(fixed_any && rhs) noexcept = default;

		/// Same effect as this->reset().
		~fixed_any() {}

		/// Constructs an object of type fixed_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `fixed_any` may be copy constructed into another `fixed_any` at fixed_any time, so a copy should always be allowed.
		template<typename ValueType, typename ValueTypeRR = mse::impl::remove_reference_t<ValueType>
			, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, fixed_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, fixed_any const*>::value)
			&& (mse::impl::is_potentially_not_xscope<typename std::decay<ValueType>::type>::value)
			> MSE_IMPL_EIS >
		fixed_any(ValueType && value) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(ValueTypeRR)") : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		mse::us::impl::ns_any::fixed_any_base2 m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

	template <class T, class... Args>
	fixed_any make_fixed_any(Args&&... args) {
		return fixed_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	fixed_any make_fixed_any(std::initializer_list<U> il, Args&&... args) {
		return fixed_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(fixed_any)

	class xscope_fixed_any : public mse::us::impl::XScopeTagBase
	{
	public:
		/// Constructs an object of type xscope_fixed_any with an empty state.
		xscope_fixed_any() = default;

		/// Constructs an object of type xscope_fixed_any with an equivalent state as other.
		xscope_fixed_any(const xscope_fixed_any & rhs) = default;

		/// Constructs an object of type xscope_fixed_any with a state equivalent to the original state of other.
		/// rhs is left in a valid but otherwise unspecified state.
		xscope_fixed_any(xscope_fixed_any && rhs) noexcept = default;

		/// Same effect as this->reset().
		~xscope_fixed_any() {}

		/// Constructs an object of type xscope_fixed_any that contains an object of type T direct-initialized with std::forward<ValueType>(value).
		///
		/// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
		/// This is because an `xscope_fixed_any` may be copy constructed into another `xscope_fixed_any` at xscope_fixed_any time, so a copy should always be allowed.
		template<typename ValueType, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<typename std::decay<ValueType>::type, xscope_fixed_any>::value)
			&& (!std::is_convertible<typename std::decay<ValueType>::type*, xscope_fixed_any const*>::value)
		> MSE_IMPL_EIS >
			xscope_fixed_any(ValueType && value) : m_base_any(std::forward<ValueType>(value)) {}

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return contained_any().has_value();
		}

		/// If *this has a contained object of type T, typeid(T); otherwise typeid(void).
		const std::type_info& type() const noexcept
		{
			return contained_any().type();
		}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		mse::us::impl::ns_any::fixed_any_base2 m_base_any;

		MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_base_any, contained_any)

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};
	
	template <class T, class... Args>
	xscope_fixed_any make_xscope_fixed_any(Args&&... args) {
		return xscope_fixed_any(T(std::forward<Args>(args)...));
	}
	template< class T, class U, class... Args >
	xscope_fixed_any make_xscope_fixed_any(std::initializer_list<U> il, Args&&... args) {
		return xscope_fixed_any(T(il, std::forward<Args>(args)...));
	}

	MSE_IMPL_ANY_CAST_DEFINITIONS1(xscope_fixed_any)

	template <class _TLender>
	class xscope_borrowing_fixed_any {
	public:
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		xscope_borrowing_fixed_any(xscope_borrowing_fixed_any&&) = delete;
#define MSE_IMPL_BORROWING_FIXED_OPTIONAL_CONSTRUCT_SRC_REF m_src_ref(*src_xs_ptr)
		~xscope_borrowing_fixed_any() {
			src_ref().access_unlock(); src_ref().structure_change_unlock();
		}
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		xscope_borrowing_fixed_any(xscope_borrowing_fixed_any&& src) : m_src_ptr(MSE_FWD(src).m_src_ptr) {
			src.m_src_ptr = nullptr;
		}
#define MSE_IMPL_BORROWING_FIXED_OPTIONAL_CONSTRUCT_SRC_REF m_src_ptr(std::addressof(*src_xs_ptr))
		~xscope_borrowing_fixed_any() {
			if (m_src_ptr) {
				src_ref().access_unlock(); src_ref().structure_change_unlock();
			}
		}
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		xscope_borrowing_fixed_any(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) : MSE_IMPL_BORROWING_FIXED_OPTIONAL_CONSTRUCT_SRC_REF {
			src_ref().structure_change_lock(); src_ref().access_lock();
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		xscope_borrowing_fixed_any(_TLender* src_xs_ptr) : MSE_IMPL_BORROWING_FIXED_OPTIONAL_CONSTRUCT_SRC_REF {
			src_ref().structure_change_lock(); src_ref().access_lock();
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

		_NODISCARD /*constexpr */bool has_value() const noexcept {
			return src_ref().unchecked_contained_any().has_value();
		}

	private:
#ifdef MSE_HAS_CXX17
		xscope_borrowing_fixed_any(const xscope_borrowing_fixed_any&) = delete;
#endif // MSE_HAS_CXX17

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		_TLender& m_src_ref;
		auto& src_ref() const { return m_src_ref; }
		auto& src_ref() { return m_src_ref; }
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		_TLender* m_src_ptr = nullptr;
		auto& src_ref() const { assert(m_src_ptr); return *m_src_ptr; }
		auto& src_ref() { assert(m_src_ptr); return *m_src_ptr; }
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		typedef decltype(mse::us::impl::ns_any::contained_any(std::declval<_TLender>())) contained_any_t;
		const contained_any_t& contained_any() const& { return mse::us::impl::ns_any::contained_any(src_ref()); }
		const contained_any_t& contained_any() const&& { return mse::us::impl::ns_any::contained_any(src_ref()); }
		contained_any_t& contained_any()& { return mse::us::impl::ns_any::contained_any(src_ref()); }
		auto contained_any()&& -> decltype(mse::us::impl::as_ref<contained_any_t>(std::move(mse::us::impl::ns_any::contained_any(src_ref())))) {
			return mse::us::impl::as_ref<contained_any_t>(std::move(mse::us::impl::ns_any::contained_any(src_ref())));
		}

		MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
	};

#ifdef MSE_HAS_CXX17
	/* deduction guides */
	template<class _TLender>
	xscope_borrowing_fixed_any(mse::TXScopeFixedPointer<_TLender>)->xscope_borrowing_fixed_any<_TLender>;
#endif /* MSE_HAS_CXX17 */

	template<class _TLender>
	auto make_xscope_borrowing_fixed_any(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) -> mse::TXScopeObj<xscope_borrowing_fixed_any<_TLender> > {
		return mse::TXScopeObj<xscope_borrowing_fixed_any<_TLender> >(src_xs_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TLender>
	auto make_xscope_borrowing_fixed_any(_TLender* src_xs_ptr) -> mse::TXScopeObj<xscope_borrowing_fixed_any<_TLender> > {
		return mse::TXScopeObj<xscope_borrowing_fixed_any<_TLender> >(src_xs_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
}

namespace std
{
	inline void swap(mse::any& lhs, mse::any& rhs) noexcept
	{
		lhs.swap(rhs);
	}
}


namespace mse {

	namespace us {
		namespace impl {

			/* While an instance of Txscope_any_structure_lock_guard<> exists it ensures that direct (scope) pointers to
			the element in an "any" container do not become invalid by preventing any operation (like reset(), for
			example) that might destroy the element. Any attempt to execute such an operation would result in an exception. */

			template<class TDynamicContainer, class TElement/* = void*/> class Txscope_const_any_structure_lock_guard;

			template<class TDynamicContainer, class TElement = void>
			class Txscope_any_structure_lock_guard : public mse::us::impl::XScopeStructureLockGuardTagBase {
			public:
				Txscope_any_structure_lock_guard(Txscope_any_structure_lock_guard&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) { lock_the_target(); }
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_any_structure_lock_guard(Txscope_any_structure_lock_guard<TDynamicContainer2>&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) { lock_the_target(); }

				Txscope_any_structure_lock_guard(const Txscope_any_structure_lock_guard& src) : m_stored_ptr(src.m_stored_ptr) { lock_the_target(); }
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_any_structure_lock_guard(const Txscope_any_structure_lock_guard<TDynamicContainer2>& src) : m_stored_ptr(src.m_stored_ptr) { lock_the_target(); }

				Txscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<TDynamicContainer>& owner_ptr) : m_stored_ptr(owner_ptr) {
					lock_the_target();
				}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				Txscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<TDynamicContainer>& owner_ptr) : m_stored_ptr(owner_ptr) {
					lock_the_target();
				}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
				~Txscope_any_structure_lock_guard() {
					unlock_the_target();
				}

				template<class TElement2 = TElement>
				auto xscope_ptr_to_element() const& {
					auto maybe_elem_ptr = mse::maybe_any_cast<TElement2>(m_stored_ptr);
					if (!maybe_elem_ptr.has_value()) {
						MSE_THROW(bad_any_cast());
					}
					return mse::us::unsafe_make_xscope_pointer_to(*(maybe_elem_ptr.value()));
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				operator mse::TXScopeFixedPointer<TDynamicContainer>() const& {
					return m_stored_ptr;
				}
				/*
				template<class TDynamicContainer2 = TDynamicContainer, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::TXScopeFixedConstPointer<TDynamicContainer2>, mse::TXScopeFixedPointer<TDynamicContainer> >::value> MSE_IMPL_EIS >
				explicit operator mse::TXScopeFixedConstPointer<TDynamicContainer2>() const & {
					return m_stored_ptr;
				}
				*/
				auto& operator*() const {
					return *m_stored_ptr;
				}
				auto* operator->() const {
					return std::addressof(*m_stored_ptr);
				}
				bool operator==(const Txscope_any_structure_lock_guard& rhs) const {
					return (rhs.m_stored_ptr == m_stored_ptr);
				}

				void async_not_shareable_and_not_passable_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				void lock_the_target() const {
					(*m_stored_ptr).m_structure_change_mutex.lock_shared();
				}
				void unlock_the_target() const {
					(*m_stored_ptr).m_structure_change_mutex.unlock_shared();
				}

				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				mse::TXScopeFixedPointer<TDynamicContainer> m_stored_ptr;

				friend class Txscope_const_any_structure_lock_guard<TDynamicContainer, TElement>;
			};
			template<class TDynamicContainer, class TElement = void>
			class Txscope_const_any_structure_lock_guard : public mse::us::impl::XScopeStructureLockGuardTagBase {
			public:
				Txscope_const_any_structure_lock_guard(Txscope_const_any_structure_lock_guard&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) { lock_the_target(); }
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_const_any_structure_lock_guard(Txscope_const_any_structure_lock_guard<TDynamicContainer2>&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) { lock_the_target(); }

				Txscope_const_any_structure_lock_guard(const Txscope_const_any_structure_lock_guard& src) : m_stored_ptr(src.m_stored_ptr) { lock_the_target(); }
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_const_any_structure_lock_guard(const Txscope_const_any_structure_lock_guard<TDynamicContainer2>& src) : m_stored_ptr(src.m_stored_ptr) { lock_the_target(); }

				Txscope_const_any_structure_lock_guard(Txscope_any_structure_lock_guard<TDynamicContainer>&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) { lock_the_target(); }
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_const_any_structure_lock_guard(Txscope_any_structure_lock_guard<TDynamicContainer2>&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) { lock_the_target(); }

				Txscope_const_any_structure_lock_guard(const Txscope_any_structure_lock_guard<TDynamicContainer>& src) : m_stored_ptr(src.m_stored_ptr) { lock_the_target(); }
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_const_any_structure_lock_guard(const Txscope_any_structure_lock_guard<TDynamicContainer2>& src) : m_stored_ptr(src.m_stored_ptr) { lock_the_target(); }

				Txscope_const_any_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<TDynamicContainer>& owner_ptr) : m_stored_ptr(owner_ptr) {
					lock_the_target();
				}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				Txscope_const_any_structure_lock_guard(const mse::TXScopeFixedConstPointer<TDynamicContainer>& owner_ptr) : m_stored_ptr(owner_ptr) {
					lock_the_target();
				}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
				~Txscope_const_any_structure_lock_guard() {
					unlock_the_target();
				}

				template<class TElement2 = TElement>
				auto xscope_ptr_to_element() const& {
					auto maybe_elem_ptr = mse::maybe_any_cast<TElement2>(m_stored_ptr);
					if (!maybe_elem_ptr.has_value()) {
						MSE_THROW(bad_any_cast());
					}
					return mse::us::unsafe_make_xscope_const_pointer_to(*(maybe_elem_ptr.value()));
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				operator mse::TXScopeFixedConstPointer<TDynamicContainer>() const& {
					return m_stored_ptr;
				}
				const auto& operator*() const {
					return *m_stored_ptr;
				}
				const auto* operator->() const {
					return std::addressof(*m_stored_ptr);
				}
				bool operator==(const Txscope_const_any_structure_lock_guard& rhs) const {
					return (rhs.m_stored_ptr == m_stored_ptr);
				}

				void async_not_shareable_and_not_passable_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				void lock_the_target() {
					(*m_stored_ptr).m_structure_change_mutex.lock_shared();
				}
				void unlock_the_target() const {
					(*m_stored_ptr).m_structure_change_mutex.unlock_shared();
				}

				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				mse::TXScopeFixedConstPointer<TDynamicContainer> m_stored_ptr;
			};

			/* For objects that are access controlled under an "exclusive writer" access policy, the object is immutable
			while a const pointer to the object exists. So given an "exclusive writer" const pointer to an "any"
			container, it is safe to store the pointer and provide a direct scope const pointer to its element. */
			template<class TDynamicContainer, class _TAccessMutex/* = mse::non_thread_safe_shared_mutex*/, class TElement = void>
			class Txscope_ewconst_any_structure_lock_guard : public mse::us::impl::XScopeStructureLockGuardTagBase {
			public:
				typedef mse::TXScopeAccessControlledConstPointer<TDynamicContainer, _TAccessMutex> xs_exclusive_writer_const_pointer_t;

				Txscope_ewconst_any_structure_lock_guard(Txscope_ewconst_any_structure_lock_guard&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) {}
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_ewconst_any_structure_lock_guard(Txscope_ewconst_any_structure_lock_guard<TDynamicContainer2, _TAccessMutex>&& src) : m_stored_ptr(MSE_FWD(src).m_stored_ptr) {}

				Txscope_ewconst_any_structure_lock_guard(const Txscope_ewconst_any_structure_lock_guard& src) : m_stored_ptr(src.m_stored_ptr) {}
				template<class TDynamicContainer2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value> MSE_IMPL_EIS >
				Txscope_ewconst_any_structure_lock_guard(const Txscope_ewconst_any_structure_lock_guard<TDynamicContainer2, _TAccessMutex>& src) : m_stored_ptr(src.m_stored_ptr) {}

				Txscope_ewconst_any_structure_lock_guard(const xs_exclusive_writer_const_pointer_t& owner_ptr)
					: m_stored_ptr(owner_ptr) {}
				Txscope_ewconst_any_structure_lock_guard(xs_exclusive_writer_const_pointer_t&& owner_ptr)
					: m_stored_ptr(MSE_FWD(owner_ptr)) {}

				~Txscope_ewconst_any_structure_lock_guard() {
					mse::impl::T_valid_if_is_exclusive_writer_enforcing_mutex_msemsearray<_TAccessMutex>();
				}

				template<class TElement2 = TElement>
				auto xscope_ptr_to_element() const& {
					auto maybe_elem_ptr = mse::maybe_any_cast<TElement2>(m_stored_ptr);
					if (!maybe_elem_ptr.has_value()) {
						MSE_THROW(bad_any_cast());
					}
					return mse::us::unsafe_make_xscope_const_pointer_to(*(maybe_elem_ptr.value()));
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				operator mse::TXScopeFixedConstPointer<TDynamicContainer>() const& {
					auto xs_sp_store = mse::make_xscope_strong_pointer_store(m_stored_ptr);
					return xs_sp_store.xscope_ptr();
				}
				operator xs_exclusive_writer_const_pointer_t() const {
					return m_stored_ptr;
				}
				const auto& operator*() const {
					return *m_stored_ptr;
				}
				const auto* operator->() const {
					return std::addressof(*m_stored_ptr);
				}

				void async_not_shareable_and_not_passable_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				xs_exclusive_writer_const_pointer_t m_stored_ptr;
			};

			template<class TDynamicContainer, class TBaseContainerStructureLockGuard, class TElement = void> class Txscope_const_any_structure_lock_guard_of_wrapper;

			template<class TDynamicContainer, class TBaseContainerStructureLockGuard, class TElement = void>
			class Txscope_any_structure_lock_guard_of_wrapper : public mse::us::impl::XScopeStructureLockGuardTagBase {
			public:
				Txscope_any_structure_lock_guard_of_wrapper(const Txscope_any_structure_lock_guard_of_wrapper&) = default;
				template<class TDynamicContainer2, class TBaseContainerStructureLockGuard2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value&& std::is_convertible<TBaseContainerStructureLockGuard2, TBaseContainerStructureLockGuard>::value> MSE_IMPL_EIS >
				Txscope_any_structure_lock_guard_of_wrapper(const Txscope_any_structure_lock_guard_of_wrapper<TDynamicContainer2, TBaseContainerStructureLockGuard2>& src)
					: m_stored_ptr(src.m_stored_ptr), m_MV_xscope_any_structure_lock_guard(src.m_MV_xscope_any_structure_lock_guard) {}

				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_any_structure_lock_guard_of_wrapper(const mse::TXScopeObjFixedPointer<TDynamicContainer>& owner_ptr, const TBaseContainerStructureLockGuardInitParam& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MV_xscope_any_structure_lock_guard_init_param) {}
				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_any_structure_lock_guard_of_wrapper(const mse::TXScopeObjFixedPointer<TDynamicContainer>& owner_ptr, TBaseContainerStructureLockGuardInitParam&& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MSE_FWD(MV_xscope_any_structure_lock_guard_init_param)) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_any_structure_lock_guard_of_wrapper(const mse::TXScopeFixedPointer<TDynamicContainer>& owner_ptr, const TBaseContainerStructureLockGuardInitParam& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MV_xscope_any_structure_lock_guard_init_param) {}
				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_any_structure_lock_guard_of_wrapper(const mse::TXScopeFixedPointer<TDynamicContainer>& owner_ptr, TBaseContainerStructureLockGuardInitParam&& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MSE_FWD(MV_xscope_any_structure_lock_guard_init_param)) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				template<class TElement2 = TElement>
				auto xscope_ptr_to_element() const& {
					auto maybe_elem_ptr = mse::maybe_any_cast<TElement2>(m_stored_ptr);
					if (!maybe_elem_ptr.has_value()) {
						MSE_THROW(bad_any_cast());
					}
					return mse::us::unsafe_make_xscope_pointer_to(*(maybe_elem_ptr.value()));
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				operator mse::TXScopeFixedPointer<TDynamicContainer>() const& {
					return m_stored_ptr;
				}
				/*
				template<class TDynamicContainer2 = TDynamicContainer, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::TXScopeFixedConstPointer<TDynamicContainer2>, mse::TXScopeFixedPointer<TDynamicContainer> >::value> MSE_IMPL_EIS >
				explicit operator mse::TXScopeFixedConstPointer<TDynamicContainer2>() const & {
					return m_stored_ptr;
				}
				*/
				auto& operator*() const {
					return *m_stored_ptr;
				}
				auto* operator->() const {
					return std::addressof(*m_stored_ptr);
				}
				bool operator==(const Txscope_any_structure_lock_guard_of_wrapper& rhs) const {
					return (rhs.m_stored_ptr == m_stored_ptr);
				}

				void async_not_shareable_and_not_passable_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				mse::TXScopeFixedPointer<TDynamicContainer> m_stored_ptr;
				TBaseContainerStructureLockGuard m_MV_xscope_any_structure_lock_guard;

				template<class TDynamicContainer2, class TBaseContainerStructureLockGuard2, class TElement2>
				friend class Txscope_const_any_structure_lock_guard_of_wrapper;
			};
			template<class TDynamicContainer, class TBaseContainerStructureLockGuard, class TElement/* = void*/>
			class Txscope_const_any_structure_lock_guard_of_wrapper : public mse::us::impl::XScopeStructureLockGuardTagBase {
			public:
				Txscope_const_any_structure_lock_guard_of_wrapper(const Txscope_const_any_structure_lock_guard_of_wrapper&) = default;
				template<class TDynamicContainer2, class TBaseContainerStructureLockGuard2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value&& std::is_convertible<TBaseContainerStructureLockGuard2, TBaseContainerStructureLockGuard>::value> MSE_IMPL_EIS >
				Txscope_const_any_structure_lock_guard_of_wrapper(const Txscope_const_any_structure_lock_guard_of_wrapper<TDynamicContainer2, TBaseContainerStructureLockGuard2>& src)
					: m_stored_ptr(src.m_stored_ptr), m_MV_xscope_any_structure_lock_guard(src.m_MV_xscope_any_structure_lock_guard) {}

				Txscope_const_any_structure_lock_guard_of_wrapper(const Txscope_any_structure_lock_guard_of_wrapper<TDynamicContainer, TBaseContainerStructureLockGuard>& src)
					: m_stored_ptr(src.m_stored_ptr), m_MV_xscope_any_structure_lock_guard(src.m_MV_xscope_any_structure_lock_guard) {}
				template<class TDynamicContainer2, class TBaseContainerStructureLockGuard2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<TDynamicContainer2*, TDynamicContainer*>::value&& std::is_convertible<TBaseContainerStructureLockGuard2, TBaseContainerStructureLockGuard>::value> MSE_IMPL_EIS >
				Txscope_const_any_structure_lock_guard_of_wrapper(const Txscope_any_structure_lock_guard_of_wrapper<TDynamicContainer2, TBaseContainerStructureLockGuard2>& src)
					: m_stored_ptr(src.m_stored_ptr), m_MV_xscope_any_structure_lock_guard(src.m_MV_xscope_any_structure_lock_guard) {}

				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_const_any_structure_lock_guard_of_wrapper(const mse::TXScopeObjFixedConstPointer<TDynamicContainer>& owner_ptr, const TBaseContainerStructureLockGuardInitParam& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MV_xscope_any_structure_lock_guard_init_param) {}
				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_const_any_structure_lock_guard_of_wrapper(const mse::TXScopeObjFixedConstPointer<TDynamicContainer>& owner_ptr, TBaseContainerStructureLockGuardInitParam&& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MSE_FWD(MV_xscope_any_structure_lock_guard_init_param)) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_const_any_structure_lock_guard_of_wrapper(const mse::TXScopeFixedConstPointer<TDynamicContainer>& owner_ptr, const TBaseContainerStructureLockGuardInitParam& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MV_xscope_any_structure_lock_guard_init_param) {}
				template<typename TBaseContainerStructureLockGuardInitParam>
				Txscope_const_any_structure_lock_guard_of_wrapper(const mse::TXScopeFixedConstPointer<TDynamicContainer>& owner_ptr, TBaseContainerStructureLockGuardInitParam&& MV_xscope_any_structure_lock_guard_init_param)
					: m_stored_ptr(owner_ptr), m_MV_xscope_any_structure_lock_guard(MSE_FWD(MV_xscope_any_structure_lock_guard_init_param)) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				template<class TElement2 = TElement>
				auto xscope_ptr_to_element() const& {
					auto maybe_elem_ptr = mse::maybe_any_cast<TElement2>(m_stored_ptr);
					if (!maybe_elem_ptr.has_value()) {
						MSE_THROW(bad_any_cast());
					}
					return mse::us::unsafe_make_xscope_const_pointer_to(*(maybe_elem_ptr.value()));
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				operator mse::TXScopeFixedConstPointer<TDynamicContainer>() const& {
					return m_stored_ptr;
				}
				const auto& operator*() const {
					return *m_stored_ptr;
				}
				const auto* operator->() const {
					return std::addressof(*m_stored_ptr);
				}
				bool operator==(const Txscope_const_any_structure_lock_guard_of_wrapper& rhs) const {
					return (rhs.m_stored_ptr == m_stored_ptr);
				}

				void async_not_shareable_and_not_passable_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				mse::TXScopeFixedConstPointer<TDynamicContainer> m_stored_ptr;
				TBaseContainerStructureLockGuard m_MV_xscope_any_structure_lock_guard;
			};
		}
	}

	namespace us {
		namespace impl {
			namespace ns_any {
				/* While an instance of xscope_any_structure_lock_guard exists it ensures that direct (scope) pointers to
				the element in the "any" does not become invalid by preventing any operation that might destroy the element.
				Any attempt to execute such an operation would result in an exception. */
				/* Constructing an xscope_any_structure_lock_guard involves a (shared) lock operation on the mse::us::impl::ns_any::any_base2<>'s
				mutable m_structure_change_mutex. So in cases where m_structure_change_mutex is not thread safe, neither is
				the construction of xscope_any_structure_lock_guard from a const reference. So while it's safe to expose
				generally xscope_any_structure_lock_guards constructed from non-const references, it's up to the specific
				specializations and derived classes of mse::us::impl::ns_any::any_base2<> to (anyly) expose xscope_any_structure_lock_guards
				constructed from const references and ensure their safety by either indicating that they are not eligible to
				be shared between threads (like stany<> does), or ensuring that m_structure_change_mutex is thread safe
				(like mtany<> does). */
				template <class _TStateMutex, class TConstLockableIndicator>
				class xscope_const_any_structure_lock_guard : public mse::us::impl::Txscope_const_any_structure_lock_guard<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> > {
				public:
					typedef mse::us::impl::Txscope_const_any_structure_lock_guard<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> > base_class;
					using base_class::base_class;

					operator mse::TXScopeFixedConstPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> >() const {
						return static_cast<const base_class&>(*this);
					}
					MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
				private:
					MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
				};
			}
		}
	}

	namespace impl {
		namespace ns_any {
			/* While an instance of xscope_any_structure_lock_guard exists it ensures that direct (scope) pointers to
			the element in the "any" does not become invalid by preventing any operation that might destroy the element.
			Any attempt to execute such an operation would result in an exception. */
			template <class _TStateMutex, class TConstLockableIndicator>
			class xscope_any_structure_lock_guard : public mse::us::impl::Txscope_any_structure_lock_guard<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> > {
			public:
				typedef mse::us::impl::Txscope_any_structure_lock_guard<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> > base_class;
				typedef mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> TDynamicContainer;
				using base_class::base_class;

				operator mse::TXScopeFixedPointer<TDynamicContainer>() const {
					return static_cast<const base_class&>(*this);
				}
				/*
				template<class TDynamicContainer2 = TDynamicContainer, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::TXScopeFixedConstPointer<TDynamicContainer2>, mse::TXScopeFixedPointer<TDynamicContainer> >::value> MSE_IMPL_EIS >
				explicit operator mse::TXScopeFixedConstPointer<TDynamicContainer2>() const {
					return mse::TXScopeFixedConstPointer<TDynamicContainer2>(static_cast<const base_class&>(*this));
				}
				*/
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
			};

			/* For objects that are access controlled under an "exclusive writer" access policy, the object is immutable
			while a const pointer to the object exists. So given an "exclusive writer" const pointer to an any, it is
			safe to provide a direct scope const pointer to its element. */
			template<class _TStateMutex, class TConstLockableIndicator, class _TAccessMutex/* = mse::non_thread_safe_shared_mutex*/>
			class xscope_ewconst_any_structure_lock_guard : public mse::us::impl::Txscope_ewconst_any_structure_lock_guard<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, _TAccessMutex> {
			public:
				typedef mse::us::impl::Txscope_ewconst_any_structure_lock_guard<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, _TAccessMutex> base_class;
				using base_class::base_class;

				typedef mse::TXScopeAccessControlledConstPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, _TAccessMutex> exclusive_writer_const_pointer_t;

				operator exclusive_writer_const_pointer_t() const {
					return static_cast<const base_class&>(*this);
				}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
			};
		}
	}

	/* While an instance of xscope_any_structure_lock_guard exists it ensures that direct (scope) pointers to
	the element in the "any" does not become invalid by preventing any operation that might destroy the element.
	Any attempt to execute such an operation would result in an exception. */
	template<class _TStateMutex, class TConstLockableIndicator>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> >& owner_ptr) -> mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator> {
		return mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator>(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TStateMutex, class TConstLockableIndicator>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator> >& owner_ptr) -> mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator> {
		return mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator>(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class T, class _TStateMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> >& owner_ptr) -> mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> {
		return mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, mse::us::impl::ns_any::any_base2_const_lockable_tag>(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class T, class _TStateMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedConstPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> >& owner_ptr) -> mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, mse::us::impl::ns_any::any_base2_const_lockable_tag> {
		return mse::impl::ns_any::xscope_any_structure_lock_guard<_TStateMutex, mse::us::impl::ns_any::any_base2_const_lockable_tag>(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	/* For objects that are access controlled under an "exclusive writer" access policy, the object is immutable
	while a const pointer to the object exists. So given an "exclusive writer" const pointer to an any, it is
	safe to provide a direct scope const pointer to its element. */
	template<class _TStateMutex, class TConstLockableIndicator, class _TAccessMutex/* = mse::non_thread_safe_shared_mutex*/>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator, _TAccessMutex>(owner_ptr);
	}
	template<class _TStateMutex, class TConstLockableIndicator, class _TAccessMutex/* = mse::non_thread_safe_shared_mutex*/>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<mse::us::impl::ns_any::any_base2<_TStateMutex, TConstLockableIndicator>, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<_TStateMutex, TConstLockableIndicator, _TAccessMutex>(owner_ptr);
	}

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<any>())));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<any>())));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<any>()));
	}
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<any>()));
	}


	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<mt_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename mt_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<mt_any>())));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<mt_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename mt_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<mt_any>())));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<mt_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<mt_any>()));
	}
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<mt_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<mt_any>()));
	}


	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<st_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename st_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<st_any>())));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<st_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename st_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<st_any>())));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<st_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<st_any>()));
	}
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<st_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<st_any>()));
	}


	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<mse::mstd::any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename mse::mstd::any::base_class>(owner_ptr));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<mse::mstd::any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename mse::mstd::any::base_class>(owner_ptr));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	namespace mstd {
	
		inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<mse::mstd::any >& owner_ptr) -> decltype(mse::make_xscope_any_structure_lock_guard(owner_ptr)) {
			return mse::make_xscope_any_structure_lock_guard(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	
		inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<mse::mstd::any >& owner_ptr) -> decltype(mse::make_xscope_any_structure_lock_guard(owner_ptr)) {
			return mse::make_xscope_any_structure_lock_guard(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	}


	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<xscope_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename xscope_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_any>())));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<xscope_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename xscope_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_any>())));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<xscope_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_any>()));
	}
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<xscope_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_not_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_any>()));
	}


	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<xscope_mt_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename xscope_mt_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_mt_any>())));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<xscope_mt_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename xscope_mt_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_mt_any>())));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<xscope_mt_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_mt_any>()));
	}
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<xscope_mt_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::shareable_dynamic_container_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_mt_any>()));
	}


	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeObjFixedPointer<xscope_st_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename xscope_st_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_st_any>())));
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)

	inline auto make_xscope_any_structure_lock_guard(const mse::TXScopeFixedPointer<xscope_st_any >& owner_ptr) {
		return make_xscope_any_structure_lock_guard(mse::TXScopeFixedPointer<typename xscope_st_any::base_class>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_st_any>())));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TXScopeAccessControlledConstPointer<xscope_st_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_st_any>()));
	}
	template<class _TAccessMutex>
	auto make_xscope_any_structure_lock_guard(const mse::TAccessControlledConstPointer<xscope_st_any, _TAccessMutex>& owner_ptr) -> mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex> {
		return mse::impl::ns_any::xscope_ewconst_any_structure_lock_guard<mse::non_thread_safe_shared_mutex, mse::us::impl::ns_any::any_base2_const_lockable_tag, _TAccessMutex>(
			mse::make_xscope_pointer_to_member_v2(owner_ptr, us::impl::ns_any::pointer_to_base_any_member<xscope_st_any>()));
	}

	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_structure_lock_guard)

	namespace impl {
		namespace ns_any {
			template<typename TPointerToAny>
			struct TypeInfoFromPointerToAny1 {
				/* If you get a compile error here: You may be trying to obtain a TXScopeAnyElementFixedConstPointer<> from a const
				reference to a type of any<> that does not support it, such as mse::any<> or mse::xscope_any<>. You might
				consider using another type of any<>, such as mse::xscope_st_any<> or mse::xscope_mt_any<>, that does
				support it. */
				typedef mse::impl::remove_reference_t<decltype(mse::make_xscope_any_structure_lock_guard(std::declval<TPointerToAny>()))> structure_lock_guard_t;
				//typedef typename mse::impl::remove_reference_t<decltype(*std::declval<structure_lock_guard_t>())>::value_type value_t;
			};
		}
	}

	template<typename TElement, typename TXScopeAnyPointer>
	class TXScopeAnyElementFixedConstPointer;

	template<typename TElement, typename TXScopeAnyPointer>
	class TXScopeAnyElementFixedPointer : public mse::us::TXScopeStrongFixedPointer<TElement
		, typename mse::impl::ns_any::TypeInfoFromPointerToAny1<TXScopeAnyPointer>::structure_lock_guard_t>, public mse::us::impl::StructureLockingObjectTagBase {
	public:
		typedef mse::us::TXScopeStrongFixedPointer<TElement
			, typename mse::impl::ns_any::TypeInfoFromPointerToAny1<TXScopeAnyPointer>::structure_lock_guard_t> base_class;
		typedef TElement value_t;
		typedef typename mse::impl::ns_any::TypeInfoFromPointerToAny1<TXScopeAnyPointer>::structure_lock_guard_t structure_lock_guard_t;

		TXScopeAnyElementFixedPointer(const TXScopeAnyElementFixedPointer&) = default;
		TXScopeAnyElementFixedPointer(TXScopeAnyElementFixedPointer&&) = default;

		TXScopeAnyElementFixedPointer(const TXScopeAnyPointer& src) : base_class(construction_helper1(src)) {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		static auto construction_helper1(const TXScopeAnyPointer& src) {
			auto xs_structure_lock_guard1 = mse::make_xscope_any_structure_lock_guard(src);
			auto any_base2_ptr = xs_structure_lock_guard1.target_container_ptr();
			auto elem_ref = mse::us::impl::ns_any::any_cast<TElement>(*any_base2_ptr);
			return mse::us::make_xscope_strong(elem_ref, std::move(xs_structure_lock_guard1));
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		template<typename TElement2, typename TXScopeAnyPointer2>
		friend class TXScopeAnyElementFixedConstPointer;
	};

	template<typename TElement, typename TXScopeAnyPointer>
	class TXScopeAnyElementFixedConstPointer : public mse::us::TXScopeStrongFixedConstPointer<TElement
		, typename mse::impl::ns_any::TypeInfoFromPointerToAny1<TXScopeAnyPointer>::structure_lock_guard_t>, public mse::us::impl::StructureLockingObjectTagBase {
	public:
		typedef mse::us::TXScopeStrongFixedConstPointer<TElement
			, typename mse::impl::ns_any::TypeInfoFromPointerToAny1<TXScopeAnyPointer>::structure_lock_guard_t> base_class;
		typedef TElement value_t;
		typedef typename mse::impl::ns_any::TypeInfoFromPointerToAny1<TXScopeAnyPointer>::structure_lock_guard_t structure_lock_guard_t;

		TXScopeAnyElementFixedConstPointer(const TXScopeAnyElementFixedConstPointer&) = default;
		TXScopeAnyElementFixedConstPointer(TXScopeAnyElementFixedConstPointer&&) = default;

		template<typename TXScopeAnyPointer2>
		TXScopeAnyElementFixedConstPointer(const TXScopeAnyElementFixedPointer<TElement, TXScopeAnyPointer2>& src) : base_class(src) {}
		template<typename TXScopeAnyPointer2>
		TXScopeAnyElementFixedConstPointer(TXScopeAnyElementFixedPointer<TElement, TXScopeAnyPointer2>&& src) : base_class(MSE_FWD(src)) {}

		TXScopeAnyElementFixedConstPointer(const TXScopeAnyPointer& src) : base_class(construction_helper1(src)) {}
		TXScopeAnyElementFixedConstPointer(TXScopeAnyPointer&& src) : base_class(construction_helper1(MSE_FWD(src))) {}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		static auto construction_helper1(const TXScopeAnyPointer& src) {
			auto xs_structure_lock_guard1 = mse::make_xscope_any_structure_lock_guard(src);
			auto any_base2_ptr = xs_structure_lock_guard1.target_container_ptr();
			auto elem_ref = mse::us::impl::ns_any::any_cast<const TElement>(*any_base2_ptr);
			return mse::us::make_xscope_const_strong(elem_ref, std::move(xs_structure_lock_guard1));
		}
		static auto construction_helper1(TXScopeAnyPointer&& src) {
			auto xs_structure_lock_guard1 = mse::make_xscope_any_structure_lock_guard(MSE_FWD(src));
			auto any_base2_ptr = xs_structure_lock_guard1.target_container_ptr();
			auto elem_ref = mse::us::impl::ns_any::any_cast<const TElement>(*any_base2_ptr);
			return mse::us::make_xscope_const_strong(elem_ref, std::move(xs_structure_lock_guard1));
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename TElement, typename TXScopeAnyPointer>
	auto xscope_pointer(const TXScopeAnyElementFixedPointer<TElement, TXScopeAnyPointer>& param) {
		return mse::us::unsafe_make_xscope_pointer_to(*param);
	}
	template<typename TElement, typename TXScopeAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeAnyPointer)>
	auto xscope_pointer(const TXScopeAnyElementFixedPointer<TElement, TXScopeAnyPointer>&& param) = delete;
	template<typename TElement, typename TXScopeAnyPointer>
	auto xscope_pointer(const TXScopeAnyElementFixedConstPointer<TElement, TXScopeAnyPointer>& param) {
		return mse::us::unsafe_make_xscope_const_pointer_to(*param);
	}
	template<typename TElement, typename TXScopeAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeAnyPointer)>
	auto xscope_pointer(const TXScopeAnyElementFixedConstPointer<TElement, TXScopeAnyPointer>&& param) = delete;

	template<typename TElement, typename TXScopeAnyPointer>
	auto make_xscope_any_element_pointer(const TXScopeAnyPointer& ptr) {
		return TXScopeAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TXScopeAnyPointer> >(ptr);
	}
	template<typename TElement, typename TXScopeAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeAnyPointer)>
	auto make_xscope_any_element_pointer(TXScopeAnyPointer&& ptr) {
		return TXScopeAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TXScopeAnyPointer> >(MSE_FWD(ptr));
	}
	template<typename TElement, typename TXScopeAnyPointer>
	auto make_xscope_any_element_const_pointer(const TXScopeAnyPointer& ptr) {
		return TXScopeAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TXScopeAnyPointer> >(ptr);
	}
	template<typename TElement, typename TXScopeAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeAnyPointer)>
	auto make_xscope_any_element_const_pointer(TXScopeAnyPointer&& ptr) {
		return TXScopeAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TXScopeAnyPointer> >(MSE_FWD(ptr));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	//MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_element_pointer)
	//MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_any_element_const_pointer)
	template<typename TElement, typename TXScopeAnyPointer>
	auto make_xscope_any_element_pointer(const mse::rsv::TReturnableFParam<TXScopeAnyPointer>& ptr) {
		const auto& param_base_ref = mse::us::impl::raw_reference_to<TXScopeAnyPointer>(ptr);
		return mse::rsv::as_a_returnable_fparam(TXScopeAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TXScopeAnyPointer> >(param_base_ref));
	}
	template<typename TElement, typename TXScopeAnyPointer>
	auto make_xscope_any_element_const_pointer(const mse::rsv::TReturnableFParam<TXScopeAnyPointer>& ptr) {
		const auto& param_base_ref = mse::us::impl::raw_reference_to<TXScopeAnyPointer>(ptr);
		return mse::rsv::as_a_returnable_fparam(TXScopeAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TXScopeAnyPointer> >(param_base_ref));
	}

	template<typename TElement, typename TAnyPointer>
	class TAnyElementFixedConstPointer;

	template<typename TElement, typename TAnyPointer>
	class TAnyElementFixedPointer {
	public:
		typedef TElement value_t;

		TAnyElementFixedPointer(const TAnyElementFixedPointer&) = default;
		TAnyElementFixedPointer(TAnyElementFixedPointer&&) = default;

		TAnyElementFixedPointer(const TAnyPointer& src) : m_any_ptr(src) {}
		TAnyElementFixedPointer(TAnyPointer&& src) : m_any_ptr(MSE_FWD(src)) {}

		~TAnyElementFixedPointer() {
#ifndef MSE_ANY_NO_XSCOPE_DEPENDENCE
			mse::impl::T_valid_if_not_an_xscope_type<TAnyPointer>();
#endif // !MSE_ANY_NO_XSCOPE_DEPENDENCE
		}

		value_t& operator*() const {
			typedef mse::impl::remove_reference_t<decltype(*m_any_ptr)> any_t;
			auto any_base2_ptr = mse::make_pointer_to_member_v2(m_any_ptr, mse::us::impl::ns_any::pointer_to_base_any_member<any_t>());
			auto elem_ptr = mse::us::impl::ns_any::any_cast<TElement>(std::addressof(*any_base2_ptr));
			if (nullptr == elem_ptr) {
				MSE_THROW(bad_any_cast());
			}
			return *elem_ptr;
		}
		value_t* operator->() const {
			return std::addressof(*(*this));
		}

		MSE_IMPL_POINTER_EQUALITY_COMPARISON_OPERATOR_DECLARATION(TAnyElementFixedPointer)
		MSE_IMPL_UNORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(TAnyElementFixedPointer)
		MSE_IMPL_EQUALITY_COMPARISON_WITH_ANY_POINTER_TYPE_OPERATOR_DECLARATIONS(TAnyElementFixedPointer)
		explicit operator bool() const { return true; }

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		TAnyPointer m_any_ptr;

		template<typename TElement2, typename TAnyPointer2>
		friend class TAnyElementFixedConstPointer;
	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TAnyPointer)");

	template<typename TElement, typename TAnyPointer>
	class TAnyElementFixedConstPointer {
	public:
		typedef TElement value_t;

		TAnyElementFixedConstPointer(const TAnyElementFixedConstPointer&) = default;
		TAnyElementFixedConstPointer(TAnyElementFixedConstPointer&&) = default;

		template<typename TElement2, typename TAnyPointer2>
		TAnyElementFixedConstPointer(const TAnyElementFixedPointer<TElement, TAnyPointer2>& src) : m_any_ptr(src.m_any_ptr) {}
		template<typename TElement2, typename TAnyPointer2>
		TAnyElementFixedConstPointer(TAnyElementFixedPointer<TElement, TAnyPointer2>&& src) : m_any_ptr(MSE_FWD(src).m_any_ptr) {}

		TAnyElementFixedConstPointer(const TAnyPointer& src) : m_any_ptr(src) {}
		TAnyElementFixedConstPointer(TAnyPointer&& src) : m_any_ptr(std::forward<src>(src)) {}

		~TAnyElementFixedConstPointer() {
#ifndef MSE_ANY_NO_XSCOPE_DEPENDENCE
			mse::impl::T_valid_if_not_an_xscope_type<TAnyPointer>();
#endif // !MSE_ANY_NO_XSCOPE_DEPENDENCE
		}

		const value_t& operator*() const {
			typedef mse::impl::remove_reference_t<decltype(*m_any_ptr)> any_t;
			auto any_base2_ptr = mse::make_pointer_to_member_v2(m_any_ptr, mse::us::impl::ns_any::pointer_to_base_any_member<any_t>());
			auto elem_ptr = mse::us::impl::ns_any::any_cast<TElement>(std::addressof(*any_base2_ptr));
			if (nullptr == elem_ptr) {
				MSE_THROW(bad_any_cast());
			}
			return *elem_ptr;
		}
		const value_t* operator->() const {
			return std::addressof(*(*this));
		}

		MSE_IMPL_POINTER_EQUALITY_COMPARISON_OPERATOR_DECLARATION(TAnyElementFixedConstPointer)
		MSE_IMPL_UNORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(TAnyElementFixedConstPointer)
		MSE_IMPL_EQUALITY_COMPARISON_WITH_ANY_POINTER_TYPE_OPERATOR_DECLARATIONS(TAnyElementFixedConstPointer)
		explicit operator bool() const { return true; }

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		TAnyPointer m_any_ptr;
	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TAnyPointer)");

	template<typename TElement, typename TAnyPointer>
	auto make_any_element_pointer(const TAnyPointer& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TAnyPointer)") {
		return TAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TAnyPointer> >(ptr);
	}
	template<typename TElement, typename TAnyPointer>
	auto make_any_element_pointer(TAnyPointer&& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TAnyPointer)") {
		return TAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TAnyPointer> >(MSE_FWD(ptr));
	}
	template<typename TElement, typename TAnyPointer>
	auto make_any_element_const_pointer(const TAnyPointer& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TAnyPointer)") {
		return TAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TAnyPointer> >(ptr);
	}
	template<typename TElement, typename TAnyPointer>
	auto make_any_element_const_pointer(TAnyPointer&& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TAnyPointer)") {
		return TAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TAnyPointer> >(MSE_FWD(ptr));
	}

	namespace impl {
		namespace ns_fixed_any {
			template<typename T> using is_fixed_any = mse::impl::disjunction<
				std::is_convertible<mse::impl::remove_const_t<T>*, mse::fixed_any*>
				, std::is_convertible<mse::impl::remove_const_t<T>*, mse::xscope_fixed_any*>
				, mse::impl::is_instantiation_of<mse::impl::remove_const_t<T>, mse::xscope_borrowing_fixed_any> >;
		}
	}

	template<typename TElement, typename TXScopeFixedAnyPointer>
	class TXScopeFixedAnyElementFixedConstPointer;

	template<typename TElement, typename TXScopeFixedAnyPointer>
	class TXScopeFixedAnyElementFixedPointer : public mse::us::TXScopeStrongFixedPointer<TElement, TXScopeFixedAnyPointer> {
	public:
		typedef mse::us::TXScopeStrongFixedPointer<TElement, TXScopeFixedAnyPointer> base_class;
		typedef TElement value_t;

		TXScopeFixedAnyElementFixedPointer(const TXScopeFixedAnyElementFixedPointer&) = default;
		TXScopeFixedAnyElementFixedPointer(TXScopeFixedAnyElementFixedPointer&&) = default;

		TXScopeFixedAnyElementFixedPointer(const TXScopeFixedAnyPointer& src) : base_class(element_ref(src), src) {}

		~TXScopeFixedAnyElementFixedPointer() {
			static_assert(mse::impl::ns_fixed_any::is_fixed_any<mse::impl::target_type<TXScopeFixedAnyPointer> >::value
				, "argument must be a pointer to a 'fixed any' type - TXScopeFixedAnyElementFixedPointer");
		}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		value_t& element_ref(const TXScopeFixedAnyPointer& src) const {
			auto maybe_elem_ptr = mse::maybe_any_cast_ptr<value_t>(src);
			if (!maybe_elem_ptr.has_value()) {
				MSE_THROW(bad_any_cast());
			}
			return *(maybe_elem_ptr.value());
		}

		template<typename TElement2, typename TXScopeFixedAnyPointer2>
		friend class TXScopeFixedAnyElementFixedConstPointer;
	};

	template<typename TElement, typename TXScopeFixedAnyPointer>
	class TXScopeFixedAnyElementFixedConstPointer : public mse::us::TXScopeStrongFixedConstPointer<
		TElement, TXScopeFixedAnyPointer> {
	public:
		typedef mse::us::TXScopeStrongFixedConstPointer<TElement, TXScopeFixedAnyPointer> base_class;
		typedef TElement value_t;

		TXScopeFixedAnyElementFixedConstPointer(const TXScopeFixedAnyElementFixedConstPointer&) = default;
		TXScopeFixedAnyElementFixedConstPointer(TXScopeFixedAnyElementFixedConstPointer&&) = default;

		template<typename TXScopeFixedAnyPointer2>
		TXScopeFixedAnyElementFixedConstPointer(const TXScopeFixedAnyElementFixedPointer<TElement, TXScopeFixedAnyPointer2>& src) : base_class(src) {}
		template<typename TXScopeFixedAnyPointer2>
		TXScopeFixedAnyElementFixedConstPointer(TXScopeFixedAnyElementFixedPointer<TElement, TXScopeFixedAnyPointer2>&& src) : base_class(MSE_FWD(src)) {}

		TXScopeFixedAnyElementFixedConstPointer(const TXScopeFixedAnyPointer& src) : base_class(element_ref(src), src) {}
		TXScopeFixedAnyElementFixedConstPointer(TXScopeFixedAnyPointer&& src) : base_class(element_ref(src), MSE_FWD(src)) {}

		~TXScopeFixedAnyElementFixedConstPointer() {
			static_assert(mse::impl::ns_fixed_any::is_fixed_any<mse::impl::target_type<TXScopeFixedAnyPointer> >::value
				, "argument must be a pointer to a 'fixed any' type - TXScopeFixedAnyElementFixedConstPointer");
		}
		MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

	private:
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		value_t& element_ref(const TXScopeFixedAnyPointer& src) const {
			auto maybe_elem_ptr = mse::maybe_any_cast_ptr<value_t>(src);
			if (!maybe_elem_ptr.has_value()) {
				MSE_THROW(bad_any_cast());
			}
			return *(maybe_elem_ptr.value());
		}
	};

	template<typename TElement, typename TXScopeFixedAnyPointer>
	auto xscope_pointer(const TXScopeFixedAnyElementFixedPointer<TElement, TXScopeFixedAnyPointer>& param) {
		return mse::us::unsafe_make_xscope_pointer_to(*param);
	}
	template<typename TElement, typename TXScopeFixedAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeFixedAnyPointer)>
	auto xscope_pointer(const TXScopeFixedAnyElementFixedPointer<TElement, TXScopeFixedAnyPointer>&& param) = delete;
	template<typename TElement, typename TXScopeFixedAnyPointer>
	auto xscope_pointer(const TXScopeFixedAnyElementFixedConstPointer<TElement, TXScopeFixedAnyPointer>& param) {
		return mse::us::unsafe_make_xscope_const_pointer_to(*param);
	}
	template<typename TElement, typename TXScopeFixedAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeFixedAnyPointer)>
	auto xscope_pointer(const TXScopeFixedAnyElementFixedConstPointer<TElement, TXScopeFixedAnyPointer>&& param) = delete;

	template<typename TElement, typename TXScopeFixedAnyPointer>
	auto make_xscope_fixed_any_element_pointer(const TXScopeFixedAnyPointer& ptr) {
		return TXScopeFixedAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TXScopeFixedAnyPointer> >(ptr);
	}
	template<typename TElement, typename TXScopeFixedAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeFixedAnyPointer)>
	auto make_xscope_fixed_any_element_pointer(TXScopeFixedAnyPointer&& ptr) {
		return TXScopeFixedAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TXScopeFixedAnyPointer> >(MSE_FWD(ptr));
	}
	template<typename TElement, typename TXScopeFixedAnyPointer>
	auto make_xscope_fixed_any_element_const_pointer(const TXScopeFixedAnyPointer& ptr) {
		return TXScopeFixedAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TXScopeFixedAnyPointer> >(ptr);
	}
	template<typename TElement, typename TXScopeFixedAnyPointer, class = MSE_IMPL_ENABLE_IF_NOT_RETURNABLE_FPARAM(TXScopeFixedAnyPointer)>
	auto make_xscope_fixed_any_element_const_pointer(TXScopeFixedAnyPointer&& ptr) {
		return TXScopeFixedAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TXScopeFixedAnyPointer> >(MSE_FWD(ptr));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	//MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_fixed_any_element_pointer)
	//MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_fixed_any_element_const_pointer)
	template<typename TElement, typename TXScopeFixedAnyPointer>
	auto make_xscope_fixed_any_element_pointer(const mse::rsv::TReturnableFParam<TXScopeFixedAnyPointer>& ptr) {
		const auto& param_base_ref = mse::us::impl::raw_reference_to<TXScopeFixedAnyPointer>(ptr);
		return mse::rsv::as_a_returnable_fparam(TXScopeFixedAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TXScopeFixedAnyPointer> >(param_base_ref));
	}
	template<typename TElement, typename TXScopeFixedAnyPointer>
	auto make_xscope_fixed_any_element_const_pointer(const mse::rsv::TReturnableFParam<TXScopeFixedAnyPointer>& ptr) {
		const auto& param_base_ref = mse::us::impl::raw_reference_to<TXScopeFixedAnyPointer>(ptr);
		return mse::rsv::as_a_returnable_fparam(TXScopeFixedAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TXScopeFixedAnyPointer> >(param_base_ref));
	}

	template<typename TElement, typename TFixedAnyPointer>
	using TFixedAnyElementFixedConstPointer = TAnyElementFixedConstPointer<TElement, TFixedAnyPointer>;

	template<typename TElement, typename TFixedAnyPointer>
	using TFixedAnyElementFixedPointer = TAnyElementFixedPointer<TElement, TFixedAnyPointer>;

	template<typename TElement, typename TFixedAnyPointer>
	auto make_fixed_any_element_pointer(const TFixedAnyPointer& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TFixedAnyPointer)") {
		return TFixedAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TFixedAnyPointer> >(ptr);
	}
	template<typename TElement, typename TFixedAnyPointer>
	auto make_fixed_any_element_pointer(TFixedAnyPointer&& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TFixedAnyPointer)") {
		return TFixedAnyElementFixedPointer<TElement, mse::impl::remove_reference_t<TFixedAnyPointer> >(MSE_FWD(ptr));
	}
	template<typename TElement, typename TFixedAnyPointer>
	auto make_fixed_any_element_const_pointer(const TFixedAnyPointer& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TFixedAnyPointer)") {
		return TFixedAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TFixedAnyPointer> >(ptr);
	}
	template<typename TElement, typename TFixedAnyPointer>
	auto make_fixed_any_element_const_pointer(TFixedAnyPointer&& ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(TFixedAnyPointer)") {
		return TFixedAnyElementFixedConstPointer<TElement, mse::impl::remove_reference_t<TFixedAnyPointer> >(MSE_FWD(ptr));
	}



	namespace impl {
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

		template<class _Ty, class TID = void>
		struct test_pointer {
			test_pointer() : m_ptr(nullptr) {}
			explicit test_pointer(_Ty* ptr) : m_ptr(ptr) {}
			test_pointer(const test_pointer& src) = default;
			_Ty& operator*() const { return (*m_ptr); }
			_Ty* operator->() const { return m_ptr; }
			bool operator==(const test_pointer& _Right_cref) const { return (m_ptr == _Right_cref.m_ptr); }
#ifndef MSE_HAS_CXX20
			bool operator!=(const test_pointer& _Right_cref) const { return (!((*this) == _Right_cref.m_ptr)); }
#endif // !MSE_HAS_CXX20
			explicit operator bool() const { return (m_ptr != nullptr); }
			_Ty* m_ptr = nullptr;
		};

		template<class T, class TID, class EqualTo>
		struct SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype((std::declval<U>() == std::declval<test_pointer<mse::impl::remove_reference_t<decltype(*std::declval<U>())>, TID> >())
				, (std::declval<V>() == std::declval<test_pointer<mse::impl::remove_reference_t<decltype(*std::declval<V>())>, TID> >()), bool(true));
			template<typename, typename>
			static auto test(...) -> std::false_type;

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
			/* Apr 2025: When compiling in C++17 mode, msvc2022 complained: "mseany.h(3268,45): error C1202: recursive type or function dependency context too complex".
			For now we're just going to simplify it in a way that disables the ability to compare "any" pointers to other types of pointers. */
			using type = std::true_type;
			static const bool value = true;
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
		};
		template<class TID>
		struct SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any_impl<void*, TID, void*> : std::false_type {};
		template<class T, class TID = void, class EqualTo = T>
		struct SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any : SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any_impl<
			mse::impl::remove_reference_t<T>, TID, mse::impl::remove_reference_t<EqualTo> >::type {};

		template<class T, class TPotentialBaseClass>
		using first_or_placeholder_if_subclass_of_second_mseany = mse::impl::conditional_t<std::is_base_of<TPotentialBaseClass, T>::value, mse::impl::TPlaceHolder<T, TPotentialBaseClass>, T>;
	}

	namespace us {
		namespace impl {
			template <typename _Ty> class TAnyPointerBaseV1;
			template <typename _Ty> class TAnyConstPointerBaseV1;
			template <typename _Ty> using TAnyPointerBase = TAnyPointerBaseV1<_Ty>;
			template <typename _Ty> using TAnyConstPointerBase = TAnyConstPointerBaseV1<_Ty>;
		}
	}

	namespace us {
		namespace impl {

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
					return mse::impl::operator_bool_helper1<_TPointer1>(typename mse::impl::IsExplicitlyCastableToBool_pb<_TPointer1>::type(), m_pointer);
				}

				_TPointer1 m_pointer;
			};

			template <typename _Ty>
			class TAnyPointerBaseV1 {
			public:
				typedef TAnyPointerBaseV1 _Myt;

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

#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				MSE_IMPL_POINTER_EQUALITY_COMPARISON_OPERATOR_DECLARATION(_Myt);
				MSE_IMPL_UNORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(_Myt);
				MSE_IMPL_EQUALITY_COMPARISON_WITH_ANY_POINTER_TYPE_OPERATOR_DECLARATIONS(_Myt);
#else // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) 
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value) 
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
				friend bool operator==(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					if (!bool(_Left_cref)) {
						if (!bool(_Right_cref)) {
							return true;
						} else {
							return false;
						}
					}
					else if (!bool(_Right_cref)) {
						return false;
					}
					return (std::addressof(*_Right_cref) == std::addressof(*_Left_cref));
				}
#endif // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)

			protected:
				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				const TCommonPointerInterface<_Ty>* common_pointer_interface_ptr() const {
					auto retval = static_cast<const TCommonPointerInterface<_Ty>*>(m_any_pointer.storage_address());
					assert(nullptr != retval);
					return retval;
				}

				typedef mse::us::impl::ns_any::any base_class;
				base_class m_any_pointer;

				MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_any_pointer, contained_any)

				friend class TAnyConstPointerBaseV1<_Ty>;
				MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
			};

			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(const TAnyPointerBaseV1<_Ty>& operand) {
				return mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(operand)).m_pointer;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyPointerBaseV1<_Ty>& operand) {
				return mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(operand)).m_pointer;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyPointerBaseV1<_Ty>&& operand) {
				return mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_pointer;
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(const TAnyPointerBaseV1<_Ty>& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType>>(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_pointer };
				}
				return retval_t{};
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(TAnyPointerBaseV1<_Ty>&& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType> >(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedPointer<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_pointer };
				}
				return retval_t{};
			}
		}
	}

	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(const mse::us::impl::TAnyPointerBaseV1<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyPointerBaseV1<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyPointerBaseV1<_Ty>&& operand) {
		return mse::us::impl::any_cast<ValueType>(MSE_FWD(operand));
	}
	template<typename ValueType, typename _Ty/* = mse::impl::target_type<ValueType> */>
	inline auto maybe_any_cast(const mse::us::impl::TAnyPointerBaseV1<_Ty>& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline auto maybe_any_cast(mse::us::impl::TAnyPointerBaseV1<_Ty>&& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(MSE_FWD(operand));
	}

	namespace us {
		namespace impl {
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
					return mse::impl::operator_bool_helper1<_TConstPointer1>(typename mse::impl::IsExplicitlyCastableToBool_pb<_TConstPointer1>::type(), m_const_pointer);
				}

				_TConstPointer1 m_const_pointer;
			};

			template <typename _Ty>
			class TAnyConstPointerBaseV1 {
			public:
				typedef TAnyConstPointerBaseV1 _Myt;

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

#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				MSE_IMPL_POINTER_EQUALITY_COMPARISON_OPERATOR_DECLARATION(_Myt);
				MSE_IMPL_UNORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(_Myt);
				MSE_IMPL_EQUALITY_COMPARISON_WITH_ANY_POINTER_TYPE_OPERATOR_DECLARATIONS(_Myt);
#else // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
				/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) || ((!std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyConstPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || ((!std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyConstPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
				friend bool operator!=(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
					return !(_Left_cref == _Right_cref);
				}

				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) || ((!std::is_base_of<TAnyPointerBaseV1<_Ty>, TLHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyConstPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TLHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TLHSPointer_ecwapt>::value))
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || ((!std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) && (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value))
						|| (std::is_base_of<TAnyConstPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value))
				> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
				template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
					(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value)
					&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) && (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
					&& ((std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<_Ty>, TRHSPointer_ecwapt>::value) || (!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryPointerTypes_any<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSPointer_ecwapt, _Myt>, _Myt>::value)
						|| (std::is_base_of<TAnyConstPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value) || (std::is_base_of<TAnyPointerBaseV1<mse::impl::remove_const_t<_Ty> >, TRHSPointer_ecwapt>::value))
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

			protected:
				MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

				const TCommonPointerInterface<_Ty>* common_pointer_interface_const_ptr() const {
					/* This use of mse::us::impl::ns_any::any::storage_address() brings to mind the fact that the (pre-C++17) implementation
					of mse::us::impl::ns_any::any that we're using does not support over-aligned types. (And therefore neither does this
					template.) Though it's hard to imagine a reason why a pointer would be declared an over-aligned type. */
					auto retval = static_cast<const TCommonPointerInterface<_Ty>*>(m_any_const_pointer.storage_address());
					assert(nullptr != retval);
					return retval;
				}

				typedef mse::us::impl::ns_any::any base_class;
				base_class m_any_const_pointer;

				MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_any_const_pointer, contained_any)

				MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1
			};

			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(const TAnyConstPointerBaseV1<_Ty>& operand) {
				return mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_const_pointer;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyConstPointerBaseV1<_Ty>& operand) {
				return mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(operand)).m_const_pointer;
			}
			template<typename ValueType, typename _Ty>
			inline ValueType any_cast(TAnyConstPointerBaseV1<_Ty>&& operand) {
				return mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType>>(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_const_pointer;
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(const TAnyConstPointerBaseV1<_Ty>& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType>>(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(operand)).m_const_pointer };
				}
				return retval_t{};
			}
			template<typename ValueType, typename _Ty>
			inline auto maybe_any_cast(TAnyConstPointerBaseV1<_Ty>&& operand) {
				auto any1 = mse::us::impl::ns_any::contained_any(operand);
				auto cast_ptr = mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType>>(std::addressof(any1));
				typedef typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type retval_t;
				if (cast_ptr) {
					return retval_t{ mse::us::impl::ns_any::any_cast<TCommonizedConstPointer<_Ty, ValueType> >(mse::us::impl::ns_any::contained_any(MSE_FWD(operand))).m_const_pointer };
				}
				return retval_t{};
			}
		}
	}

	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(const mse::us::impl::TAnyConstPointerBaseV1<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyConstPointerBaseV1<_Ty>& operand) {
		return mse::us::impl::any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline ValueType any_cast(mse::us::impl::TAnyConstPointerBaseV1<_Ty>&& operand) {
		return mse::us::impl::any_cast<ValueType>(MSE_FWD(operand));
	}
	template<typename ValueType, typename _Ty/* = mse::impl::target_type<ValueType> */>
	inline auto maybe_any_cast(const mse::us::impl::TAnyConstPointerBaseV1<_Ty>& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(operand);
	}
	template<typename ValueType, typename _Ty>
	inline auto maybe_any_cast(mse::us::impl::TAnyConstPointerBaseV1<_Ty>&& operand) {
		return mse::us::impl::maybe_any_cast<ValueType>(MSE_FWD(operand));
	}


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /*__GNUC__*/
#endif /*__clang__*/

	namespace self_test {
		class CAnyTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				{
					std::cout << std::boolalpha;

					// any type
					//mse::mstd::any a = 1;
					mse::TRegisteredObj<mse::mstd::any> a = 1;
					std::cout << a.type().name() << ": " << mse::mstd::any_cast<int>(a) << '\n';
					a = 3.14;
					std::cout << a.type().name() << ": " << mse::mstd::any_cast<double>(a) << '\n';
					a = true;
					std::cout << a.type().name() << ": " << mse::mstd::any_cast<bool>(a) << '\n';

					// bad cast
					MSE_TRY
					{
						a = 1;
						std::cout << mse::mstd::any_cast<float>(a) << '\n';
					}
					MSE_CATCH (const mse::mstd::bad_any_cast& e)
					{
						MSE_IF_EXCEPTIONS_ENABLED( std::cout << e.what() << '\n'; )
					}

					// has value
					a = 2;
					if (a.has_value())
					{
						std::cout << a.type().name() << ": " << mse::mstd::any_cast<int>(a) << '\n';
					}

					// reset
					a.reset();
					if (!a.has_value())
					{
						std::cout << "no value\n";
					}

					// pointer to contained data
					a = 3;
					//int* i = mse::mstd::any_cast<int>(&a);
					auto i = mse::mstd::any_cast<int>(&a);
					std::cout << *i << "\n";
				}
				{
					auto a = mse::make_registered(mse::mstd::any(12));

					std::cout << mse::mstd::any_cast<int>(a) << '\n';

					MSE_TRY {
						std::cout << mse::mstd::any_cast<std::string>(a) << '\n';
					}
					MSE_CATCH (const mse::mstd::bad_any_cast& e) {
						MSE_IF_EXCEPTIONS_ENABLED( std::cout << e.what() << '\n'; )
					}

					// pointer example

					if (/*int* */auto i = mse::mstd::any_cast<int>(&a)) {
						std::cout << "a is int: " << *i << '\n';
					}
					else if (/*std::string* */auto s = mse::mstd::any_cast<std::string>(&a)) {
						std::cout << "a is std::string: " << *s << '\n';
					}
					else {
						std::cout << "a is another type or unset\n";
					}

					// advanced example

					a = std::string("hello");

					auto& ra = mse::mstd::any_cast<std::string&>(a); //< reference
					ra[1] = 'o';

					std::cout << "a: "
						<< mse::mstd::any_cast<const std::string&>(a) << '\n'; //< const reference

					auto b = mse::mstd::any_cast<std::string&&>(std::move(a)); //< rvalue reference

					// Note: 'b' is a move-constructed std::string,
					// 'a' is left in valid but unspecified state

					std::cout << "a: " << *mse::mstd::any_cast<std::string>(&a) //< pointer
						<< "b: " << b << '\n';
				}
				{
					class Star
					{
						std::string name;
						int id;

					public:
						Star(std::string name, int id) : name{ name }, id{ id }
						{
							std::cout << "Star::Star(string, int)\n";
						}

						void print() const
						{
							std::cout << "Star{ \"" << name << "\" : " << id << " };\n";
						}
					};

					mse::TRegisteredObj<mse::mstd::any> celestial;
					// (1) emplace( Args&&... args );
					celestial.emplace<Star>("Procyon", 2943);
					const /*auto* */auto star = mse::mstd::any_cast<Star>(&celestial);
					star->print();

					mse::TRegisteredObj<mse::mstd::any> av;
					// (2) emplace( std::initializer_list<U> il, Args&&... args );
					av.emplace<std::vector<char>>({ 'C', '+', '+', '1', '7' } /* no args */);
					std::cout << av.type().name() << '\n';
					const /*auto* */auto va = mse::mstd::any_cast<std::vector<char>>(&av);
					std::for_each(va->cbegin(), va->cend(), [](char const& c) { std::cout << c; });
					std::cout << '\n';
				}
				{
					std::boolalpha(std::cout);

					mse::mstd::any a0;
					std::cout << "a0.has_value(): " << a0.has_value() << "\n";

					mse::mstd::any a1 = 42;
					std::cout << "a1.has_value(): " << a1.has_value() << '\n';
					std::cout << "a1 = " << mse::mstd::any_cast<int>(a1) << '\n';
					a1.reset();
					std::cout << "a1.has_value(): " << a1.has_value() << '\n';

					auto a2 = mse::mstd::make_any<std::string>("Milky Way");
					std::cout << "a2.has_value(): " << a2.has_value() << '\n';
					std::cout << "a2 = \"" << mse::mstd::any_cast<std::string&>(a2) << "\"\n";
					a2.reset();
					std::cout << "a2.has_value(): " << a2.has_value() << '\n';
				}
				{
					auto a0 = mse::mstd::make_any<std::string>("Hello, mse::mstd::any!\n");
					auto a1 = mse::mstd::make_any<std::complex<double>>(0.1, 2.3);

					std::cout << mse::mstd::any_cast<std::string&>(a0);
					std::cout << mse::mstd::any_cast<std::complex<double>&>(a1) << '\n';

					using lambda = std::function<void(void)>;

					// Put a lambda into mse::mstd::any. Attempt #1 (failed).
					mse::mstd::any a2 = [] { std::cout << "Lambda #1.\n"; };
					std::cout << "a2.type() = \"" << a2.type().name() << "\"\n";

					// any_cast casts to <void(void)> but actual type is not
					// a std::function..., but ~ main::{lambda()#1}, and it is
					// unique for each lambda. So, this throws...
					MSE_TRY {
						mse::mstd::any_cast<lambda>(a2)();
					}
					MSE_CATCH (mse::mstd::bad_any_cast const& ex) {
						MSE_IF_EXCEPTIONS_ENABLED( std::cout << ex.what() << '\n'; )
					}

					// Put a lambda into mse::mstd::any. Attempt #2 (successful).
					auto a3 = mse::mstd::make_any<lambda>([] { std::cout << "Lambda #2.\n"; });
					std::cout << "a3.type() = \"" << a3.type().name() << "\"\n";
					mse::mstd::any_cast<lambda>(a3)();
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

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#pragma pop_macro("_NODISCARD")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#endif // MSEANY_H_
