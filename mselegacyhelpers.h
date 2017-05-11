
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSELEGACYHELPERS_H_
#define MSELEGACYHELPERS_H_

#include "msepoly.h"

namespace mse {
	namespace lh {

#ifdef MSE_LEGACYHELPERS_DISABLED

#define MSE_LH_DYNAMIC_ARRAY_TYPE(element_type) element_type *
#define MSE_LH_ALLOC_DYNAMIC_ARRAY(element_type, dyn_array, num_bytes) dyn_array = malloc(num_bytes)
#define MSE_LH_REALLOC_DYNAMIC_ARRAY(element_type, dyn_array, num_bytes) dyn_array = realloc(dyn_array, num_bytes)
#define MSE_LH_FREE_DYNAMIC_ARRAY(element_type, dyn_array) free(dyn_array)

#else /*MSE_LEGACYHELPERS_DISABLED*/

#define MSE_LH_DYNAMIC_ARRAY_TYPE(element_type) mse::lh::TIPointerWithBundledVector< element_type >
#define MSE_LH_ALLOC_DYNAMIC_ARRAY(element_type, dyn_array, num_bytes) mse::lh::CAllocF< MSE_LH_DYNAMIC_ARRAY_TYPE(element_type) >::allocate(dyn_array, num_bytes)
#define MSE_LH_REALLOC_DYNAMIC_ARRAY(element_type, dyn_array, num_bytes) mse::lh::CAllocF< MSE_LH_DYNAMIC_ARRAY_TYPE(element_type) >::reallocate(dyn_array, num_bytes)
#define MSE_LH_FREE_DYNAMIC_ARRAY(element_type, dyn_array) mse::lh::CAllocF< MSE_LH_DYNAMIC_ARRAY_TYPE(element_type) >::free(dyn_array)

#endif /*MSE_LEGACYHELPERS_DISABLED*/

		template <typename _Ty>
		class TOpaqueWrapper {
		public:
			TOpaqueWrapper(const _Ty& value_param) : m_value(value_param) {}
			_Ty& value() { return m_value; }
			const _Ty& value() const { return m_value; }

			_Ty m_value;
		};
		template <typename _Ty> using TVectorRefcfptrWrapper = TOpaqueWrapper<mse::TRefCountingFixedPointer<mse::msevector<_Ty>>>;

		/* This data type was motivated by the need for a direct substitute for native pointers targeting dynamically
		allocated (native) arrays, which can kind of play a dual role as a reference to the array object and/or as an
		iterator. I'm not sure about this implementation though. We could base it on an mse::ivector::ipointer
		instead... */
		template <typename _Ty>
		class TIPointerWithBundledVector : private TVectorRefcfptrWrapper<_Ty>, public mse::msevector<_Ty>::ipointer {
		public:
			typedef typename mse::msevector<_Ty>::ipointer ipointer_base_class;
			typedef typename mse::msevector<_Ty>::size_type size_type;

			TIPointerWithBundledVector() : TVectorRefcfptrWrapper<_Ty>(mse::make_refcounting<mse::msevector<_Ty>>())
				, ipointer_base_class(*vector_refcptr()) {}
			TIPointerWithBundledVector(const TIPointerWithBundledVector& src) : TVectorRefcfptrWrapper<_Ty>(src.vector_refcptr())
				, ipointer_base_class(*vector_refcptr()) {
				ipointer_base_class::operator=(src);
			}
			TIPointerWithBundledVector(_XSTD initializer_list<_Ty> _Ilist) : TVectorRefcfptrWrapper<_Ty>(mse::make_refcounting<mse::msevector<_Ty>>(_Ilist))
				, ipointer_base_class(*vector_refcptr()) {}
			explicit TIPointerWithBundledVector(size_type _N) : TVectorRefcfptrWrapper<_Ty>(mse::make_refcounting<mse::msevector<_Ty>>(_N))
				, ipointer_base_class(*vector_refcptr()) {}
			explicit TIPointerWithBundledVector(size_type _N, const _Ty& _V) : TVectorRefcfptrWrapper<_Ty>(mse::make_refcounting<mse::msevector<_Ty>>(_N, _V))
				, ipointer_base_class(*vector_refcptr()) {}
			/*
			template <class... Args>
			TIPointerWithBundledVector(Args&&... args) : TVectorRefcfptrWrapper<_Ty>(mse::make_refcounting<mse::msevector<_Ty>>(std::forward<Args>(args)...))
				, ipointer_base_class(*vector_refcptr()) {}
				*/

			size_type size() const {
				return (*vector_refcptr()).size();
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();
			}

			TIPointerWithBundledVector& operator=(const TIPointerWithBundledVector& _Right_cref) {
				if (_Right_cref.vector_refcptr() == vector_refcptr()) {
					ipointer_base_class::operator=(_Right_cref);
				}
				else {
					~TIPointerWithBundledVector();
					::new (this) TIPointerWithBundledVector(_Right_cref);
				}
				return(*this);
			}

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TIPointerWithBundledVector make(Args&&... args) {
				return TIPointerWithBundledVector(std::forward<Args>(args)...);
			}

		private:
			mse::TRefCountingFixedPointer<mse::msevector<_Ty>>& vector_refcptr() { return (*this).value(); }
			const mse::TRefCountingFixedPointer<mse::msevector<_Ty>>& vector_refcptr() const { return (*this).value(); }
			//mse::TRefCountingFixedPointer<mse::msevector<_Ty>> m_vector_refcptr;
		};

		template <class X, class... Args>
		TIPointerWithBundledVector<X> make_ipointer_with_bundled_vector(Args&&... args) {
			return TIPointerWithBundledVector<X>::make(std::forward<Args>(args)...);
		}

		template <typename _Ty, size_t _Size>
		class TNativeArrayReplacement : public mse::mstd::array<_Ty, _Size> {
		public:
			typedef mse::mstd::array<_Ty, _Size> base_class;
			using base_class::base_class;

			operator mse::TNullableAnyRandomAccessIterator<_Ty>() {
				return begin();
			}
			operator mse::TAnyRandomAccessIterator<_Ty>() {
				return begin();
			}
			operator mse::TAnyRandomAccessConstIterator<_Ty>() const {
				return cbegin();
			}
			operator typename mse::mstd::array<_Ty, _Size>::iterator() {
				return begin();
			}
			operator typename mse::mstd::array<_Ty, _Size>::const_iterator() {
				return cbegin();
			}
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
		class CAllocF<mse::lh::TIPointerWithBundledVector<_Ty>> {
		public:
			static void free(mse::lh::TIPointerWithBundledVector<_Ty>& ptr) {
				ptr.resize(0);
			}
			static void allocate(mse::lh::TIPointerWithBundledVector<_Ty>& ptr, size_t num_bytes) {
				ptr.resize(num_bytes / sizeof(_Ty));
			}
			static void reallocate(mse::lh::TIPointerWithBundledVector<_Ty>& ptr, size_t num_bytes) {
				ptr.resize(num_bytes / sizeof(_Ty));
			}
		};

		template<class _Ty>
		class CFileF {
		public:
			static size_t fread(_Ty ptr, size_t size, size_t count, FILE * stream);
			static size_t fwrite(_Ty ptr, size_t size, size_t count, FILE * stream);
		};
		template<class _Ty>
		class CFileF<_Ty*> {
		public:
			static size_t fread(_Ty* ptr, size_t size, size_t count, FILE * stream) {
				return ::fread(ptr, size, count, stream);
			}
			static size_t fwrite(_Ty ptr, size_t size, size_t count, FILE * stream) {
				return ::fwrite(ptr, size, count, stream);
			}
		};
		template<class _Ty>
		class CFileF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
		public:
			static size_t fread(mse::TNullableAnyRandomAccessIterator<_Ty> ptr, size_t size, size_t count, FILE * stream) {
				std::vector<unsigned char> v(size * count);
				auto num_bytes_read = ::fread(v.data(), size, count, stream);
				auto num_items_read = num_bytes_read / sizeof(_Ty);
				size_t uc_index = 0;
				size_t Ty_index = 0;
				for (; Ty_index < num_items_read; uc_index += sizeof(_Ty), Ty_index += 1) {
					unsigned char* uc_ptr = &(v[uc_index]);
					_Ty* Ty_ptr = (_Ty*)uc_ptr;
					ptr[Ty_index] = (*Ty_ptr);
				}
				return num_bytes_read;
			}
			static size_t fwrite(mse::TNullableAnyRandomAccessIterator<_Ty> ptr, size_t size, size_t count, FILE * stream) {
				std::vector<unsigned char> v(size * count);
				auto num_items_to_write = size * count / sizeof(_Ty);
				size_t uc_index = 0;
				size_t Ty_index = 0;
				for (; Ty_index < num_items_to_write; uc_index += sizeof(_Ty), Ty_index += 1) {
					unsigned char* uc_ptr = &(v[uc_index]);
					_Ty* Ty_ptr = (_Ty*)uc_ptr;
					(*Ty_ptr) = ptr[Ty_index];
				}
				return ::fwrite(v.data(), size, count, stream);
			}
		};

	}
}

#endif // MSELEGACYHELPERS_H_
