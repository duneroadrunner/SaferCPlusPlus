
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSELEGACYHELPERS_H_
#define MSELEGACYHELPERS_H_

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_MSTDARRAY_DISABLED)
#define MSE_LEGACYHELPERS_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_MSTDARRAY_DISABLED)*/

#ifndef MSE_LEGACYHELPERS_DISABLED
#include "msepoly.h"
#include <cstring>
#else // !MSE_LEGACYHELPERS_DISABLED
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif // !MSE_LEGACYHELPERS_DISABLED

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4127 )
#endif /*_MSC_VER*/

#ifdef MSE_LEGACYHELPERS_DISABLED

#define MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) 
#define MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) 
#define MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size) [size]
#define MSE_LH_FIXED_ARRAY_DECLARATION(element_type, size, name) MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) element_type MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) name MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size)
#define MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(element_type) element_type *

#define MSE_LH_ALLOC_POINTER1(element_type) (element_type *)malloc(sizeof(element_type))
#define MSE_LH_ALLOC_DYN_ARRAY1(iterator_type, num_bytes) (iterator_type)malloc(num_bytes)
#define MSE_LH_REALLOC(element_type, ptr, num_bytes) (element_type *)realloc(ptr, num_bytes)
#define MSE_LH_FREE(ptr) free(ptr)

/* generally prefer MSE_LH_ALLOC_DYN_ARRAY1() or MSE_LH_ALLOC_POINTER1() over MSE_LH_ALLOC() */
#define MSE_LH_ALLOC(element_type, ptr, num_bytes) ptr = (element_type *)malloc(num_bytes)

#define MSE_LH_ARRAY_ITERATOR_TYPE(element_type) element_type *

#define MSE_LH_FREAD(ptr, size, count, stream) fread(ptr, size, count, stream)
#define MSE_LH_FWRITE(ptr, size, count, stream) fwrite(ptr, size, count, stream)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) memset(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) memset(ptr, value, num_bytes)

#define MSE_LH_ADDRESSABLE_TYPE(object_type) object_type
#define MSE_LH_POINTER_TYPE(element_type) element_type *
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) element_type *

#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE

#define MSE_LH_IF_ENABLED(x)
#define MSE_LH_IF_DISABLED(x) x

#else /*MSE_LEGACYHELPERS_DISABLED*/

#define MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) mse::lh::TNativeArrayReplacement< 
#define MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) , size >
#define MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size) 
#define MSE_LH_FIXED_ARRAY_DECLARATION(element_type, size, name) MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) element_type MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) name MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size)
#define MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(element_type) mse::lh::TStrongVectorIterator< element_type >

#define MSE_LH_ALLOC_POINTER1(element_type) mse::lh::allocate<mse::TNullableAnyPointer<element_type> >()
#define MSE_LH_ALLOC_DYN_ARRAY1(iterator_type, num_bytes) mse::lh::allocate_dyn_array1<iterator_type>(num_bytes)
#define MSE_LH_REALLOC(element_type, ptr, num_bytes) mse::lh::reallocate(ptr, num_bytes)
#define MSE_LH_FREE(ptr) mse::lh::free(ptr)

/* generally prefer MSE_LH_ALLOC_DYN_ARRAY1() or MSE_LH_ALLOC_POINTER1() over MSE_LH_ALLOC() */
#define MSE_LH_ALLOC(element_type, ptr, num_bytes) mse::lh::allocate(ptr, num_bytes)

#define MSE_LH_ARRAY_ITERATOR_TYPE(element_type) mse::TNullableAnyRandomAccessIterator< element_type >

#define MSE_LH_FREAD(ptr, size, count, stream) mse::lh::fread(ptr, size, count, stream)
#define MSE_LH_FWRITE(ptr, size, count, stream) mse::lh::fwrite(ptr, size, count, stream)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) mse::lh::memset< mse::TNullableAnyRandomAccessIterator<element_type> >::memcpy(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) mse::lh::memset< mse::TNullableAnyRandomAccessIterator<element_type> >::memset(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) mse::lh::memcpy(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) mse::lh::memset(ptr, value, num_bytes)

#define MSE_LH_ADDRESSABLE_TYPE(object_type) mse::TRegisteredObj< object_type >
#define MSE_LH_POINTER_TYPE(element_type) mse::TNullableAnyPointer< element_type >
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) mse::TRefCountingPointer< element_type >

#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE MSE_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE MSE_SUPPRESS_CHECK_IN_DECLSCOPE

#define MSE_LH_IF_ENABLED(x) x
#define MSE_LH_IF_DISABLED(x)

#endif /*MSE_LEGACYHELPERS_DISABLED*/

namespace mse {
	namespace lh {

		/* This data type was motivated by the need for a direct substitute for native pointers targeting dynamically
		allocated (native) arrays, which can kind of play a dual role as a reference to the array object and/or as an
		iterator. */
		template <typename _Ty>
		class TStrongVectorIterator : public mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> {
		public:
			typedef mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TStrongVectorIterator() = default;
			TStrongVectorIterator(const std::nullptr_t& src) : TStrongVectorIterator() {}
			TStrongVectorIterator(const TStrongVectorIterator& src) = default;
			TStrongVectorIterator(TStrongVectorIterator&& src) = default;
			TStrongVectorIterator(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_Ilist), 0) {}
			TStrongVectorIterator(const base_class& src) : base_class(src) {}
			TStrongVectorIterator(const mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>>& src) : base_class(src) {}
			explicit TStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N), 0) {}
			explicit TStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N, _V), 0) {}
			/*
			template <class... Args>
			TStrongVectorIterator(Args&&... args) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(std::forward<Args>(args)...), 0) {}
			*/

			size_type size() const {
				if (vector_refcptr()) {
					return (*vector_refcptr()).size();
				}
				else {
					return 0;
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				if (!vector_refcptr()) {
					base_class::operator=(base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(), 0));
				}

				//auto old_size = size();

				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();

				/*
				if (true || (0 == old_size)) {
					(*this).set_to_beginning();
				}
				*/
			}

			TStrongVectorIterator& operator=(const std::nullptr_t& _Right_cref) {
				return operator=(TStrongVectorIterator());
			}
			TStrongVectorIterator& operator=(const TStrongVectorIterator& _Right_cref) {
				base_class::operator=(_Right_cref);
				return(*this);
			}

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TStrongVectorIterator make(Args&&... args) {
				return TStrongVectorIterator(std::forward<Args>(args)...);
			}

		private:
			auto vector_refcptr() { return (*this).target_container_ptr(); }
			auto vector_refcptr() const { return (*this).target_container_ptr(); }
		};

		template <class X, class... Args>
		TStrongVectorIterator<X> make_strong_vector_iterator(Args&&... args) {
			return TStrongVectorIterator<X>::make(std::forward<Args>(args)...);
		}

		template <typename _Ty>
		class TXScopeStrongVectorIterator : public mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> {
		public:
			typedef mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TXScopeStrongVectorIterator() = default;
			TXScopeStrongVectorIterator(const std::nullptr_t& src) : TXScopeStrongVectorIterator() {}
			TXScopeStrongVectorIterator(const TXScopeStrongVectorIterator& src) = default;
			TXScopeStrongVectorIterator(TXScopeStrongVectorIterator&& src) = default;
			TXScopeStrongVectorIterator(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_Ilist), 0) {}
			TXScopeStrongVectorIterator(const base_class& src) : base_class(src) {}
			TXScopeStrongVectorIterator(const mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>>& src) : base_class(src) {}
			explicit TXScopeStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N), 0) {}
			explicit TXScopeStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N, _V), 0) {}
			/*
			template <class... Args>
			TXScopeStrongVectorIterator(Args&&... args) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(std::forward<Args>(args)...), 0) {}
			*/

			size_type size() const {
				if (vector_refcptr()) {
					return (*vector_refcptr()).size();
				}
				else {
					return 0;
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				if (!vector_refcptr()) {
					base_class::operator=(base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(), 0));
				}

				//auto old_size = size();

				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();

				/*
				if (true || (0 == old_size)) {
					(*this).set_to_beginning();
				}
				*/
			}

			TXScopeStrongVectorIterator& operator=(const std::nullptr_t& _Right_cref) {
				return operator=(TXScopeStrongVectorIterator());
			}
			TXScopeStrongVectorIterator& operator=(const TXScopeStrongVectorIterator& _Right_cref) {
				base_class::operator=(_Right_cref);
				return(*this);
			}

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TXScopeStrongVectorIterator make(Args&&... args) {
				return TXScopeStrongVectorIterator(std::forward<Args>(args)...);
			}

		private:
			auto vector_refcptr() { return (*this).target_container_ptr(); }
			auto vector_refcptr() const { return (*this).target_container_ptr(); }
		};

		template <class X, class... Args>
		TXScopeStrongVectorIterator<X> make_xscope_strong_vector_iterator(Args&&... args) {
			return TXScopeStrongVectorIterator<X>::make(std::forward<Args>(args)...);
		}

		/* TXScopeStrongVectorIterator<> does not directly convert to mse::rsv::TFParam<mse::TXScopeCSSSXSTERandomAccessIterator<> >.
		But the following function can be used to obtain a "locking" scope iterator that does. */
		template <class X>
		auto make_xscope_locking_vector_iterator(const TXScopeStrongVectorIterator<X>& xs_strong_iter) {
			auto retval = mse::make_xscope_begin_iterator(mse::us::unsafe_make_xscope_pointer_to(*xs_strong_iter.target_container_ptr()));
			retval += xs_strong_iter.position();
			return retval;
		}
		template <class X>
		auto make_xscope_locking_vector_iterator(TXScopeStrongVectorIterator<X>&& xs_strong_iter) = delete;

		/* deprecated aliases */
		template <typename _Ty>
		using TIPointerWithBundledVector = TStrongVectorIterator<_Ty>;
		template <class X, class... Args>
		TIPointerWithBundledVector<X> make_ipointer_with_bundled_vector(Args&&... args) {
			return make_string_vector_iterator(std::forward<Args>(args)...);
		}


		template <typename _Ty, size_t _Size>
		class TNativeArrayReplacement : public mse::mstd::array<_Ty, _Size> {
		public:
			typedef mse::mstd::array<_Ty, _Size> base_class;
			using base_class::base_class;

			operator mse::TNullableAnyRandomAccessIterator<_Ty>() {
				return base_class::begin();
			}
			operator mse::TAnyRandomAccessIterator<_Ty>() {
				return base_class::begin();
			}
			operator mse::TAnyRandomAccessConstIterator<_Ty>() const {
				return base_class::cbegin();
			}
			operator typename mse::mstd::array<_Ty, _Size>::iterator() {
				return base_class::begin();
			}
			template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_same<typename mse::mstd::array<_Ty2, _Size>::const_iterator, typename mse::mstd::array<_Ty2, _Size>::iterator>::value), void>::type>
			operator typename mse::mstd::array<_Ty, _Size>::const_iterator() {
				return base_class::cbegin();
			}
			template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_const<_Ty2>::value), void>::type>
			operator mse::TNullableAnyRandomAccessIterator<const _Ty>() {
				return base_class::begin();
			}
			typename base_class::iterator operator+(typename base_class::difference_type n) { return base_class::begin() + n; }
			typename base_class::iterator operator-(typename base_class::difference_type n) { return base_class::begin() - n; }
			typename base_class::difference_type operator-(const typename base_class::iterator& _Right_cref) const { return base_class::begin() - _Right_cref; }
			typename base_class::const_iterator operator+(typename base_class::difference_type n) const { return base_class::cbegin() + n; }
			typename base_class::const_iterator operator-(typename base_class::difference_type n) const { return base_class::cbegin() - n; }
			template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_same<typename mse::mstd::array<_Ty2, _Size>::const_iterator, typename mse::mstd::array<_Ty2, _Size>::iterator>::value), void>::type>
			typename base_class::difference_type operator-(const typename base_class::const_iterator& _Right_cref) const { return base_class::cbegin() - _Right_cref; }

#ifdef MSE_LEGACYHELPERS_DISABLED
			TNativeArrayReplacement(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::nii_array<_Ty, _Size>(_Ilist)) {}
#endif // MSE_LEGACYHELPERS_DISABLED

		};

		template<class _Ty>
		class CAllocF {
		public:
			static void free(_Ty& ptr);
			static void allocate(_Ty& ptr, size_t num_bytes);
			static void reallocate(_Ty& ptr, size_t num_bytes);
		};
		template<class _Ty>
		class CAllocF<_Ty*> {
		public:
			static void free(_Ty* ptr) {
				::free(ptr);
			}
			static void allocate(_Ty*& ptr, size_t num_bytes) {
				ptr = ::malloc(num_bytes);
			}
			static void reallocate(_Ty*& ptr, size_t num_bytes) {
				ptr = ::realloc(ptr, num_bytes);
			}
		};
		template<class _Ty>
		class CAllocF<mse::lh::TStrongVectorIterator<_Ty>> {
		public:
			static void free(mse::lh::TStrongVectorIterator<_Ty>& ptr) {
				ptr = mse::lh::TStrongVectorIterator<_Ty>();
			}
			static void allocate(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				mse::lh::TStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
				ptr = tmp;
			}
			static void reallocate(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				ptr.resize(num_bytes / sizeof(_Ty));
			}
		};
		template<class _Ty>
		class CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
		public:
			static void free(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr) {
				ptr = mse::lh::TStrongVectorIterator<_Ty>();
			}
			static void allocate(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) {
				mse::lh::TStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
				ptr = tmp;
			}
			//static void reallocate(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes);
		};
		template<class _Ty>
		class CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>> {
		public:
			static void free(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr) {
				ptr = mse::lh::TXScopeStrongVectorIterator<_Ty>();
			}
			static void allocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				mse::lh::TXScopeStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
				ptr = tmp;
			}
			static void reallocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				ptr.resize(num_bytes / sizeof(_Ty));
			}
		};
		template<class _Ty>
		class CAllocF<mse::TRefCountingPointer<_Ty>> {
		public:
			static void free(mse::TRefCountingPointer<_Ty>& ptr) {
				ptr = nullptr;
			}
			static void allocate(mse::TRefCountingPointer<_Ty>& ptr, size_t num_bytes) {
				ptr = mse::TRefCountingPointer<_Ty>();
			}
			static void reallocate(mse::TRefCountingPointer<_Ty>& ptr, size_t num_bytes) {
				if (0 == num_bytes) {
					ptr = nullptr;
				}
				else {
					assert(false);
				}
			}
		};
		template<class _Ty>
		class CAllocF<mse::TNullableAnyPointer<_Ty>> {
		public:
			static void free(mse::TNullableAnyPointer<_Ty>& ptr) {
				ptr = nullptr;
			}
			static void allocate(mse::TNullableAnyPointer<_Ty>& ptr, size_t num_bytes) {
				if (0 == num_bytes) {
					ptr = nullptr;
				}
				else if (sizeof(_Ty) == num_bytes) {
					ptr = mse::make_refcounting<_Ty>();
				}
				else {
					assert(false);
					ptr = mse::make_refcounting<_Ty>();
					//MSE_THROW(std::bad_alloc("the given allocation size is not supported for this pointer type - CAllocF<mse::TNullableAnyPointer<_Ty>>::allocate()"));
				}
			}
			//static void reallocate(mse::TNullableAnyPointer<_Ty>& ptr, size_t num_bytes);
		};

		template<class _TPointer>
		_TPointer allocate() {
			_TPointer ptr;
			auto num_bytes = sizeof(decltype(*ptr));
			MSE_TRY{
				CAllocF<_TPointer>::allocate(ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TPointer();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate_dyn_array1(size_t num_bytes) {
			_TDynArrayIter ptr;
			MSE_TRY{
				CAllocF<_TDynArrayIter>::allocate(ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TDynArrayIter();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter reallocate(const _TDynArrayIter& ptr2, size_t num_bytes) {
			_TDynArrayIter ptr = ptr2;
			MSE_TRY{
				CAllocF<_TDynArrayIter>::reallocate(ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TDynArrayIter();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate(_TDynArrayIter& ptr, size_t num_bytes) {
			MSE_TRY{
				CAllocF<_TDynArrayIter>::allocate(ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TDynArrayIter();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		void free(_TDynArrayIter& ptr) {
			CAllocF<_TDynArrayIter>::free(ptr);
		}

		/* Memory safe approximation of fread(). */
		template<class _TIter>
		size_t fread(_TIter ptr, size_t size, size_t count, FILE* stream) {
			typedef typename std::remove_reference<decltype((ptr)[0])>::type element_t;
			thread_local std::vector<unsigned char> v;
			v.resize(size * count);
			auto num_bytes_read = ::fread(v.data(), size, count, stream);
			auto num_items_read = num_bytes_read / sizeof(element_t);
			size_t uc_index = 0;
			size_t element_index = 0;
			for (; element_index < num_items_read; uc_index += sizeof(element_t), element_index += 1) {
				unsigned char* uc_ptr = &(v[uc_index]);
				if (false) {
					element_t* element_ptr = reinterpret_cast<element_t*>(uc_ptr);
					ptr[element_index] = (*element_ptr);
				}
				else {
					long long int adjusted_value = 0;
					static_assert(sizeof(adjusted_value) >= sizeof(element_t), "The (memory safe) lh::fread() function does not support element types larger than `long long int`. ");
					for (size_t i = 0; i < sizeof(element_t); i += 1) {
						adjusted_value |= ((uc_ptr[i]) << (8 * ((sizeof(element_t) - 1) - i)));
					}
					/* Only element types that are (or can be constructed from) integral types are supported. */
					ptr[element_index] = element_t(adjusted_value);
				}
			}
			v.resize(0);
			v.shrink_to_fit();
			return num_bytes_read;
		}
		/* Memory safe approximation of fwrite(). */
		template<class _TIter>
		size_t fwrite(_TIter ptr, size_t size, size_t count, FILE* stream) {
			typedef typename std::remove_reference<decltype((ptr)[0])>::type element_t;
			auto num_items_to_write = size * count / sizeof(element_t);
			thread_local std::vector<unsigned char> v;
			v.resize(size * count);
			//assert(num_items_to_write * sizeof(element_t) == size * count);
			size_t uc_index = 0;
			size_t element_index = 0;
			for (; element_index < num_items_to_write; uc_index += sizeof(element_t), element_index += 1) {
				unsigned char* uc_ptr = &(v[uc_index]);
				if (false) {
					typedef typename std::remove_const<element_t>::type non_const_element_t;
					non_const_element_t* element_ptr = reinterpret_cast<non_const_element_t*>(uc_ptr);
					(*element_ptr) = ptr[element_index];
				}
				else {
					/* Only element types that are (or convert to) integral types are supported. */
					long long int value(ptr[element_index]);
					static_assert(sizeof(value) >= sizeof(element_t), "The (memory safe) lh::write() function does not support element types larger than `long long int`. ");
					for (size_t i = 0; i < sizeof(element_t); i += 1) {
						uc_ptr[i] = ((value >> (8 * ((sizeof(element_t) - 1) - i))) & 0xff);
					}
				}
			}
			auto res = ::fwrite(v.data(), size, count, stream);
			v.resize(0);
			v.shrink_to_fit();
			return res;
		}

		/* Memory safe approximation of memcpy(). */
		template<class _TIter>
		void memcpy(_TIter destination, _TIter source, size_t num_bytes) {
			typedef typename std::remove_reference<decltype((destination)[0])>::type element_t;
			auto num_items = num_bytes / sizeof(element_t);
			//assert(num_items * sizeof(element_t) == num_bytes);
			for (size_t i = 0; i < num_items; i += 1) {
				destination[i] = source[i];
			}
		}
		/* Memory safe approximation of memset(). */
		template<class _TIter>
		void memset(_TIter iter, int value, size_t num_bytes) {
			typedef typename std::remove_reference<decltype((iter)[0])>::type element_t;
			value &= 0xff;
			long long int adjusted_value = value;
			if (sizeof(adjusted_value) >= sizeof(element_t)) {
				for (size_t i = 1; i < sizeof(element_t); i += 1) {
					adjusted_value |= (value << (8 * i));
				}
			}
			else {
				/* In this case, if the value being set is non-zero, then it's likely that this function won't
				faithfully emulate the standard memset() function. */
				assert(0 == value);
			}
			/* Only element types that are (or can be constructed from) integral types are supported. */
			const auto element_value = element_t(adjusted_value);
			auto num_items = num_bytes / sizeof(element_t);
			//assert(num_items * sizeof(element_t) == num_bytes);
			for (size_t i = 0; i < num_items; i += 1) {
				iter[i] = element_value;
			}
		}
	}
}

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSELEGACYHELPERS_H_
