
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEASYNCSHARED_H_
#define MSEASYNCSHARED_H_

#include <shared_mutex>
#include <cassert>


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_ASYNCSHAREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

namespace mse {

#ifdef MSE_ASYNCSHAREDPOINTER_DISABLED
#else /*MSE_ASYNCSHAREDPOINTER_DISABLED*/
#endif /*MSE_ASYNCSHAREDPOINTER_DISABLED*/

	template<typename _Ty> class TAsyncSharedAccessRequester;
	template<typename _Ty> class TAsyncSharedPointer;
	template<typename _Ty> class TAsyncSharedConstPointer;
	template<typename _Ty> class TAsyncSharedImmutableAccessRequester;
	template<typename _Ty> class TAsyncSharedImmutableConstPointer;

	/* TAsyncSharedObj is intended as a transparent wrapper for other classes/objects. */
	template<typename _TROy>
	class TAsyncSharedObj : public _TROy {
	public:
		virtual ~TAsyncSharedObj() {}
		using _TROy::operator=;
		//TAsyncSharedObj& operator=(TAsyncSharedObj&& _X) { _TROy::operator=(std::move(_X)); return (*this); }
		TAsyncSharedObj& operator=(typename std::conditional<std::is_const<_TROy>::value
			, std::nullptr_t, TAsyncSharedObj>::type&& _X) { _TROy::operator=(std::move(_X)); return (*this); }
		//TAsyncSharedObj& operator=(const TAsyncSharedObj& _X) { _TROy::operator=(_X); return (*this); }
		TAsyncSharedObj& operator=(const typename std::conditional<std::is_const<_TROy>::value
			, std::nullptr_t, TAsyncSharedObj>::type& _X) { _TROy::operator=(_X); return (*this); }

	private:
		MSE_USING(TAsyncSharedObj, _TROy);
		TAsyncSharedObj(const TAsyncSharedObj& _X) : _TROy(_X) {}
		TAsyncSharedObj(TAsyncSharedObj&& _X) : _TROy(std::move(_X)) {}
		TAsyncSharedObj* operator&() {
			return this;
		}
		const TAsyncSharedObj* operator&() const {
			return this;
		}

		mutable std::shared_timed_mutex m_mutex1;

		friend class TAsyncSharedAccessRequester<_TROy>;
		friend class TAsyncSharedPointer<_TROy>;
		friend class TAsyncSharedConstPointer<_TROy>;
		friend class TAsyncSharedImmutableAccessRequester<_TROy>;
		friend class TAsyncSharedImmutableConstPointer<_TROy>;
	};

	template<typename _Ty>
	class TAsyncSharedPointer {
	public:
		TAsyncSharedPointer(TAsyncSharedPointer&& src) = default;
		virtual ~TAsyncSharedPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedPointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedPointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedPointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedPointer<_Ty>& operator=(const TAsyncSharedPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedPointer<_Ty>& operator=(TAsyncSharedPointer<_Ty>&& _Right) = delete;

		TAsyncSharedPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<std::shared_timed_mutex> m_unique_lock;

		friend class TAsyncSharedAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedConstPointer {
	public:
		TAsyncSharedConstPointer(TAsyncSharedConstPointer&& src) = default;
		virtual ~TAsyncSharedConstPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedConstPointer")); }
			return m_shptr.operator bool();
		}
		const _Ty& operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedConstPointer")); }
			return (*m_shptr);
		}
		const _Ty* operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedConstPointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedConstPointer<_Ty>& operator=(const TAsyncSharedConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedConstPointer<_Ty>& operator=(TAsyncSharedConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<std::shared_timed_mutex> m_unique_lock;

		friend class TAsyncSharedAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedAccessRequester : public TSaferPtr<TAsyncSharedObj<_Ty>> {
	public:
		TAsyncSharedAccessRequester(const TAsyncSharedAccessRequester& src_cref) = default;

		TAsyncSharedPointer<_Ty> ptr() {
			return TAsyncSharedPointer<_Ty>(m_shptr);
		}
		TAsyncSharedConstPointer<_Ty> const_ptr() {
			return TAsyncSharedConstPointer<_Ty>(m_shptr);
		}

		template <class... Args>
		static TAsyncSharedAccessRequester make_asyncshared(Args&&... args) {
			//auto shptr = std::make_shared<TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<TAsyncSharedObj<_Ty>> shptr(new TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedAccessRequester(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedAccessRequester<X> make_asyncshared(Args&&... args) {
		return TAsyncSharedAccessRequester<X>::make_asyncshared(std::forward<Args>(args)...);
	}


	template<typename _Ty>
	class TAsyncSharedImmutableConstPointer {
	public:
		TAsyncSharedImmutableConstPointer(TAsyncSharedImmutableConstPointer&& src) = default;
		virtual ~TAsyncSharedImmutableConstPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedImmutableConstPointer")); }
			return m_shptr.operator bool();
		}
		const _Ty& operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedImmutableConstPointer")); }
			return (*m_shptr);
		}
		const _Ty* operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedImmutableConstPointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedImmutableConstPointer(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedImmutableConstPointer<_Ty>& operator=(const TAsyncSharedImmutableConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedImmutableConstPointer<_Ty>& operator=(TAsyncSharedImmutableConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedImmutableConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedImmutableConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<std::shared_timed_mutex> m_unique_lock;

		friend class TAsyncSharedImmutableAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedImmutableAccessRequester : public TSaferPtr<const TAsyncSharedObj<_Ty>> {
	public:
		TAsyncSharedImmutableAccessRequester(const TAsyncSharedImmutableAccessRequester& src_cref) = default;

		TAsyncSharedImmutableConstPointer<_Ty> const_ptr() {
			return TAsyncSharedImmutableConstPointer<_Ty>(m_shptr);
		}

		template <class... Args>
		static TAsyncSharedImmutableAccessRequester make_asyncsharedimmutable(Args&&... args) {
			//auto shptr = std::make_shared<const TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr(new const TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedImmutableAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedImmutableAccessRequester(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedImmutableAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedImmutableAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedImmutableAccessRequester<X> make_asyncsharedimmutable(Args&&... args) {
		return TAsyncSharedImmutableAccessRequester<X>::make_asyncsharedimmutable(std::forward<Args>(args)...);
	}


	static void s_ashptr_test1() {
	}
}

#endif // MSEASYNCSHARED_H_
