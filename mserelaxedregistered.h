
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* This file is deprecated. Migrate to the "msecregistered.h" file instead. */

/* Relaxed registered pointers are basically just like registered pointers except that unlike registered pointers, they
are able to reference target object types before those target types are (fully) defined. This is required to support
"mutually referencing" objects or "cyclic references".

The implementation difference is that with registered pointers, the "pointer tracking registry" is located in the target
object, whereas relaxed registered pointers use (thread local) global registries that track all the pointers targeting
objects of a given type. */

#pragma once
#ifndef MSERELAXEDREGISTERED_H_
#define MSERELAXEDREGISTERED_H_

#include "msecregistered.h"

namespace mse {

	template<typename _Ty> using TRelaxedRegisteredPointer MSE_DEPRECATED = TCRegisteredPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredConstPointer MSE_DEPRECATED = TCRegisteredConstPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredNotNullPointer MSE_DEPRECATED = TCRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredNotNullConstPointer MSE_DEPRECATED = TCRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredFixedPointer MSE_DEPRECATED = TCRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredFixedConstPointer MSE_DEPRECATED = TCRegisteredFixedConstPointer<_Ty>;

	template<typename _TROFLy> using TRelaxedRegisteredObj MSE_DEPRECATED = TCRegisteredObj<_TROFLy>;
	template <class _Ty, class... Args>
	MSE_DEPRECATED TRelaxedRegisteredPointer<_Ty> relaxed_registered_new(Args&&... args) {
		return cregistered_new<_Ty>(std::forward<Args>(args)...);
	}
	template <class _Ty>
	MSE_DEPRECATED void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
		cregistered_delete(regPtrRef);
	}
	template <class _Ty>
	MSE_DEPRECATED void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
		cregistered_delete(regPtrRef);
	}
	namespace us {
		template <class _Ty>
		MSE_DEPRECATED void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
			mse::relaxed_registered_delete(regPtrRef);
		}
		template <class _Ty>
		MSE_DEPRECATED void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
			mse::relaxed_registered_delete(regPtrRef);
		}
	}

	template<typename _Ty> MSE_DEPRECATED auto relaxed_registered_fptr_to(_Ty&& _X) { return cregistered_fptr_to(std::forward<decltype(_X)>(_X)); }
	template<typename _Ty> MSE_DEPRECATED auto relaxed_registered_fptr_to(const _Ty& _X) { return cregistered_fptr_to(_X); }
}

#endif // MSERELAXEDREGISTERED_H_
