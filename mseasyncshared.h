
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEASYNCSHARED_H_
#define MSEASYNCSHARED_H_

#include "mseoptional.h"
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <cassert>
#include <stdexcept>
#include <ctime>
#include <ratio>
#include <chrono>

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_ASYNCSHAREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

#ifdef MSE_ASYNCSHAREDPOINTER_DISABLED
#else /*MSE_ASYNCSHAREDPOINTER_DISABLED*/
#endif /*MSE_ASYNCSHAREDPOINTER_DISABLED*/

	/* This macro roughly simulates constructor inheritance. Originally it was used when some compilers didn't support
	constructor inheritance, but now we use it because of it's differences with standard constructor inheritance. */
#define MSE_ASYNC_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	class asyncshared_runtime_error : public std::runtime_error { public:
		using std::runtime_error::runtime_error;
	};
	class asyncshared_use_of_invalid_pointer_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

	template <class _Ty>
	class unlock_guard {
	public:
		unlock_guard(_Ty& mutex_ref) : m_mutex_ref(mutex_ref) {
			m_mutex_ref.unlock();
		}
		~unlock_guard() {
			m_mutex_ref.lock();
		}

		_Ty& m_mutex_ref;
	};

	class recursive_shared_timed_mutex : private std::shared_timed_mutex {
	public:
		typedef std::shared_timed_mutex base_class;

		void lock()
		{	// lock exclusive
			std::lock_guard<std::mutex> lock1(m_write_mutex);

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
			}
			else {
				assert((std::this_thread::get_id() != m_writelock_thread_id) || (0 == m_writelock_count));
				{
					unlock_guard<std::mutex> unlock1(m_write_mutex);
					base_class::lock();
				}
				m_writelock_thread_id = std::this_thread::get_id();
				assert(0 == m_writelock_count);
			}
			m_writelock_count += 1;
		}

		bool try_lock()
		{	// try to lock exclusive
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_write_mutex);

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
				m_writelock_count += 1;
				retval = true;
			}
			else {
				assert(0 == m_writelock_count);
				retval = base_class::try_lock();
				if (retval) {
					m_writelock_thread_id = std::this_thread::get_id();
					m_writelock_count += 1;
				}
			}
			return retval;
		}

		template<class _Rep, class _Period>
		bool try_lock_for(const std::chrono::duration<_Rep, _Period>& _Rel_time)
		{	// try to lock for duration
			return (try_lock_until(std::chrono::steady_clock::now() + _Rel_time));
		}

		template<class _Clock, class _Duration>
		bool try_lock_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time)
		{	// try to lock until time point
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_write_mutex);

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
				m_writelock_count += 1;
				retval = true;
			}
			else {
				assert(0 == m_writelock_count);
				retval = base_class::try_lock_until(_Abs_time);
				if (retval) {
					m_writelock_thread_id = std::this_thread::get_id();
					m_writelock_count += 1;
				}
			}
			return retval;
		}

		void unlock()
		{	// unlock exclusive
			std::lock_guard<std::mutex> lock1(m_write_mutex);
			assert(std::this_thread::get_id() == m_writelock_thread_id);

			if ((2 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
			}
			else {
				assert(1 == m_writelock_count);
				base_class::unlock();
			}
			m_writelock_count -= 1;
		}

		void lock_shared()
		{	// lock non-exclusive
			std::lock_guard<std::mutex> lock1(m_read_mutex);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
			}
			else {
				assert((m_thread_id_readlock_count_map.end() == found_it) || (0 == (*found_it).second));
				{
					unlock_guard<std::mutex> unlock1(m_read_mutex);
					base_class::lock_shared();
				}
				try {
					/* Things could've changed so we have to check again. */
					const auto l_found_it = m_thread_id_readlock_count_map.find(this_thread_id);
					if (m_thread_id_readlock_count_map.end() != l_found_it) {
						assert(0 <= (*l_found_it).second);
						(*l_found_it).second += 1;
					}
					else {
							std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
							m_thread_id_readlock_count_map.insert(item);
					}
				}
				catch (...) {
					base_class::unlock_shared();
					MSE_THROW(asyncshared_runtime_error("std::unordered_map<>::insert() failed? - mse::recursive_shared_timed_mutex"));
				}
			}
		}

		bool try_lock_shared()
		{	// try to lock non-exclusive
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_read_mutex);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
				retval = true;
			}
			else {
				retval = base_class::try_lock_shared();
				if (retval) {
					try {
						if (m_thread_id_readlock_count_map.end() != found_it) {
							assert(0 <= (*found_it).second);
							(*found_it).second += 1;
						}
						else {
							std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
							m_thread_id_readlock_count_map.insert(item);
						}
					}
					catch (...) {
						base_class::unlock_shared();
						MSE_THROW(asyncshared_runtime_error("std::unordered_map<>::insert() failed? - mse::recursive_shared_timed_mutex"));
					}
				}
			}
			return retval;
		}

		template<class _Rep, class _Period>
		bool try_lock_shared_for(const std::chrono::duration<_Rep, _Period>& _Rel_time)
		{	// try to lock non-exclusive for relative time
			return (try_lock_shared_until(_Rel_time + std::chrono::steady_clock::now()));
		}

		template<class _Clock, class _Duration>
		bool try_lock_shared_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time)
		{	// try to lock non-exclusive until absolute time
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_read_mutex);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
				retval = true;
			}
			else {
				retval = base_class::try_lock_shared_until(_Abs_time);
				if (retval) {
					try {
						if (m_thread_id_readlock_count_map.end() != found_it) {
							assert(0 <= (*found_it).second);
							(*found_it).second += 1;
						}
						else {
							std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
							m_thread_id_readlock_count_map.insert(item);
						}
					}
					catch (...) {
						base_class::unlock_shared();
						MSE_THROW(asyncshared_runtime_error("std::unordered_map<>::insert() failed? - mse::recursive_shared_timed_mutex"));
					}
				}
			}
			return retval;
		}

		void unlock_shared()
		{	// unlock non-exclusive
			std::lock_guard<std::mutex> lock1(m_read_mutex);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if (m_thread_id_readlock_count_map.end() != found_it) {
				if (2 <= (*found_it).second) {
					(*found_it).second -= 1;
				}
				else {
					assert(1 == (*found_it).second);
					m_thread_id_readlock_count_map.erase(found_it);
					base_class::unlock_shared();
				}
			}
			else {
				assert(false);
				base_class::unlock_shared();
			}
		}

		std::mutex m_write_mutex;
		std::mutex m_read_mutex;

		std::thread::id m_writelock_thread_id;
		int m_writelock_count = 0;
		std::unordered_map<std::thread::id, int> m_thread_id_readlock_count_map;
	};

	//typedef std::shared_timed_mutex async_shared_timed_mutex_type;
	typedef recursive_shared_timed_mutex async_shared_timed_mutex_type;

	template<typename _Ty> class TAsyncSharedReadWriteAccessRequester;
	template<typename _Ty> class TAsyncSharedReadWritePointer;
	template<typename _Ty> class TAsyncSharedReadWriteConstPointer;
	template<typename _Ty> class TAsyncSharedReadOnlyAccessRequester;
	template<typename _Ty> class TAsyncSharedReadOnlyConstPointer;

	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer;

	/* TAsyncSharedObj is intended as a transparent wrapper for other classes/objects. */
	template<typename _TROy>
	class TAsyncSharedObj : public _TROy {
	public:
		MSE_ASYNC_USING(TAsyncSharedObj, _TROy);
		virtual ~TAsyncSharedObj() {}
		using _TROy::operator=;
		//TAsyncSharedObj& operator=(TAsyncSharedObj&& _X) { _TROy::operator=(std::move(_X)); return (*this); }
		TAsyncSharedObj& operator=(typename std::conditional<std::is_const<_TROy>::value
			, std::nullptr_t, TAsyncSharedObj>::type&& _X) { _TROy::operator=(std::move(_X)); return (*this); }
		//TAsyncSharedObj& operator=(const TAsyncSharedObj& _X) { _TROy::operator=(_X); return (*this); }
		TAsyncSharedObj& operator=(const typename std::conditional<std::is_const<_TROy>::value
			, std::nullptr_t, TAsyncSharedObj>::type& _X) { _TROy::operator=(_X); return (*this); }

	private:
		TAsyncSharedObj(const TAsyncSharedObj& _X) : _TROy(_X) {}
		TAsyncSharedObj(TAsyncSharedObj&& _X) : _TROy(std::move(_X)) {}
		TAsyncSharedObj* operator&() {
			return this;
		}
		const TAsyncSharedObj* operator&() const {
			return this;
		}

		mutable async_shared_timed_mutex_type m_mutex1;

		friend class TAsyncSharedReadWriteAccessRequester<_TROy>;
		friend class TAsyncSharedReadWritePointer<_TROy>;
		friend class TAsyncSharedReadWriteConstPointer<_TROy>;
		friend class TAsyncSharedReadOnlyAccessRequester<_TROy>;
		friend class TAsyncSharedReadOnlyConstPointer<_TROy>;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_TROy>;
	};


	template<typename _Ty> class TAsyncSharedReadWriteConstPointer;

	template<typename _Ty>
	class TAsyncSharedReadWritePointer {
	public:
		TAsyncSharedReadWritePointer(const TAsyncSharedReadWritePointer& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedReadWritePointer(TAsyncSharedReadWritePointer&& src) = default;
		virtual ~TAsyncSharedReadWritePointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadWritePointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadWritePointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				shptr = nullptr;
			}
		}
		TAsyncSharedReadWritePointer<_Ty>& operator=(const TAsyncSharedReadWritePointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedReadWritePointer<_Ty>& operator=(TAsyncSharedReadWritePointer<_Ty>&& _Right) = delete;

		TAsyncSharedReadWritePointer<_Ty>* operator&() { return this; }
		const TAsyncSharedReadWritePointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedReadWriteAccessRequester<_Ty>;
		friend class TAsyncSharedReadWriteConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedReadWriteConstPointer {
	public:
		TAsyncSharedReadWriteConstPointer(const TAsyncSharedReadWriteConstPointer& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedReadWriteConstPointer(TAsyncSharedReadWriteConstPointer&& src) = default;
		TAsyncSharedReadWriteConstPointer(const TAsyncSharedReadWritePointer<_Ty>& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		virtual ~TAsyncSharedReadWriteConstPointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadWriteConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return extra_const_ptr;
		}
	private:
		TAsyncSharedReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				shptr = nullptr;
			}
		}
		TAsyncSharedReadWriteConstPointer<_Ty>& operator=(const TAsyncSharedReadWriteConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedReadWriteConstPointer<_Ty>& operator=(TAsyncSharedReadWriteConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedReadWriteConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedReadWriteConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedReadWriteAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedReadWriteAccessRequester {
	public:
		TAsyncSharedReadWriteAccessRequester(const TAsyncSharedReadWriteAccessRequester& src_cref) = default;

		TAsyncSharedReadWritePointer<_Ty> writelock_ptr() {
			return TAsyncSharedReadWritePointer<_Ty>(m_shptr);
		}
		mse::optional<TAsyncSharedReadWritePointer<_Ty>> try_writelock_ptr() {
			mse::optional<TAsyncSharedReadWritePointer<_Ty>> retval(TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return {};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::optional<TAsyncSharedReadWritePointer<_Ty>> try_writelock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::optional<TAsyncSharedReadWritePointer<_Ty>> retval(TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::optional<TAsyncSharedReadWritePointer<_Ty>> try_writelock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::optional<TAsyncSharedReadWritePointer<_Ty>> retval(TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		TAsyncSharedReadWriteConstPointer<_Ty> readlock_ptr() {
			return TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr);
		}
		mse::optional<TAsyncSharedReadWriteConstPointer<_Ty>> try_readlock_ptr() {
			mse::optional<TAsyncSharedReadWriteConstPointer<_Ty>> retval(TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::optional<TAsyncSharedReadWriteConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::optional<TAsyncSharedReadWriteConstPointer<_Ty>> retval(TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::optional<TAsyncSharedReadWriteConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::optional<TAsyncSharedReadWriteConstPointer<_Ty>> retval(TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedReadWriteAccessRequester make(Args&&... args) {
			//auto shptr = std::make_shared<TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<TAsyncSharedObj<_Ty>> shptr(new TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedReadWriteAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedReadWriteAccessRequester(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedReadWriteAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedReadWriteAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;

		friend class TAsyncSharedReadOnlyAccessRequester<_Ty>;
	};

	template <class X, class... Args>
	TAsyncSharedReadWriteAccessRequester<X> make_asyncsharedreadwrite(Args&&... args) {
		return TAsyncSharedReadWriteAccessRequester<X>::make(std::forward<Args>(args)...);
	}


	template<typename _Ty>
	class TAsyncSharedReadOnlyConstPointer {
	public:
		TAsyncSharedReadOnlyConstPointer(const TAsyncSharedReadOnlyConstPointer& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedReadOnlyConstPointer(TAsyncSharedReadOnlyConstPointer&& src) = default;
		virtual ~TAsyncSharedReadOnlyConstPointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadOnlyConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return extra_const_ptr;
		}
	private:
		TAsyncSharedReadOnlyConstPointer(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				shptr = nullptr;
			}
		}
		TAsyncSharedReadOnlyConstPointer<_Ty>& operator=(const TAsyncSharedReadOnlyConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedReadOnlyConstPointer<_Ty>& operator=(TAsyncSharedReadOnlyConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedReadOnlyConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedReadOnlyConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedReadOnlyAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedReadOnlyAccessRequester {
	public:
		TAsyncSharedReadOnlyAccessRequester(const TAsyncSharedReadOnlyAccessRequester& src_cref) = default;
		TAsyncSharedReadOnlyAccessRequester(const TAsyncSharedReadWriteAccessRequester<_Ty>& src_cref) : m_shptr(src_cref.m_shptr) {}

		TAsyncSharedReadOnlyConstPointer<_Ty> readlock_ptr() {
			return TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr);
		}
		mse::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> try_readlock_ptr() {
			mse::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> retval(TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> retval(TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> retval(TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedReadOnlyAccessRequester make(Args&&... args) {
			//auto shptr = std::make_shared<const TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr(new const TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedReadOnlyAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedReadOnlyAccessRequester(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedReadOnlyAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedReadOnlyAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedReadOnlyAccessRequester<X> make_asyncsharedreadonly(Args&&... args) {
		return TAsyncSharedReadOnlyAccessRequester<X>::make(std::forward<Args>(args)...);
	}


	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer;

	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer&& src) = default;
		virtual ~TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				shptr = nullptr;
			}
		}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& operator=(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& operator=(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>&& _Right) = delete;

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer& src) : m_shptr(src.m_shptr), m_shared_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer&& src) = default;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& src) : m_shptr(src.m_shptr), m_shared_lock(src.m_shptr->m_mutex1) {}
		virtual ~TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return extra_const_ptr;
		}
	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_for(_Rel_time)) {
				shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_until(_Abs_time)) {
				shptr = nullptr;
			}
		}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& operator=(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& operator=(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::shared_lock<async_shared_timed_mutex_type> m_shared_lock;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester& src_cref) = default;

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty> writelock_ptr() {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr);
		}
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> try_writelock_ptr() {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> try_writelock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> try_writelock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty> readlock_ptr() {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr);
		}
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> try_readlock_ptr() {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester make(Args&&... args) {
			//auto shptr = std::make_shared<TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<TAsyncSharedObj<_Ty>> shptr(new TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<X> make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadwrite(Args&&... args) {
		return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<X>::make(std::forward<Args>(args)...);
	}


	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer& src) : m_shptr(src.m_shptr), m_shared_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer&& src) = default;
		virtual ~TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return extra_const_ptr;
		}
	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_for(_Rel_time)) {
				shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_until(_Abs_time)) {
				shptr = nullptr;
			}
		}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>& operator=(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>& operator=(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
		std::shared_lock<async_shared_timed_mutex_type> m_shared_lock;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester& src_cref) = default;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>& src_cref) : m_shptr(src_cref.m_shptr) {}

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty> readlock_ptr() {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr);
		}
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> try_readlock_ptr() {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester make(Args&&... args) {
			//auto shptr = std::make_shared<const TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr(new const TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<X> make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadonly(Args&&... args) {
		return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<X>::make(std::forward<Args>(args)...);
	}


	/* For "read-only" situations when you need, or want, the shared object to be managed by std::shared_ptrs we provide a
	slightly safety enhanced std::shared_ptr wrapper. The wrapper enforces "const"ness and tries to ensure that it always
	points to a validly allocated object. Use mse::make_stdsharedimmutable<>() to construct an
	mse::TStdSharedImmutableFixedPointer. And again, beware of sharing objects with mutable members. */
	template<typename _Ty>
	class TStdSharedImmutableFixedPointer : public std::shared_ptr<const _Ty> {
	public:
		TStdSharedImmutableFixedPointer(const TStdSharedImmutableFixedPointer& src_cref) : std::shared_ptr<const _Ty>(src_cref) {}
		virtual ~TStdSharedImmutableFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::shared_ptr<const _Ty>::operator _Ty*(); }

		template <class... Args>
		static TStdSharedImmutableFixedPointer make(Args&&... args) {
			TStdSharedImmutableFixedPointer retval(std::make_shared<const _Ty>(std::forward<Args>(args)...));
			return retval;
		}

	private:
		TStdSharedImmutableFixedPointer(std::shared_ptr<const _Ty> shptr) : std::shared_ptr<const _Ty>(shptr) {}
		TStdSharedImmutableFixedPointer<_Ty>& operator=(const TStdSharedImmutableFixedPointer<_Ty>& _Right_cref) = delete;

		//TStdSharedImmutableFixedPointer<_Ty>* operator&() { return this; }
		//const TStdSharedImmutableFixedPointer<_Ty>* operator&() const { return this; }
	};

	template <class X, class... Args>
	TStdSharedImmutableFixedPointer<X> make_stdsharedimmutable(Args&&... args) {
		return TStdSharedImmutableFixedPointer<X>::make(std::forward<Args>(args)...);
	}

	/* Legacy aliases. */
	template<typename _Ty> using TReadOnlyStdSharedFixedConstPointer = TStdSharedImmutableFixedPointer<_Ty>;
	template <class X, class... Args>
	TReadOnlyStdSharedFixedConstPointer<X> make_readonlystdshared(Args&&... args) {
		return TStdSharedImmutableFixedPointer<X>::make(std::forward<Args>(args)...);
	}


#if defined(MSEREFCOUNTING_H_)
	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, TAsyncSharedReadWritePointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TAsyncSharedReadWritePointer<_Ty> &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, TAsyncSharedReadWritePointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TAsyncSharedReadWriteConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TAsyncSharedReadWriteConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TAsyncSharedReadWriteConstPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TAsyncSharedReadOnlyConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TAsyncSharedReadOnlyConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TAsyncSharedReadOnlyConstPointer<_Ty>>::make(target, lease_pointer);
	}

	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty> &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>>::make(target, lease_pointer);
	}
#endif // defined(MSEREFCOUNTING_H_)

	static void s_ashptr_test1() {
#ifdef MSE_SELF_TESTS
#endif // MSE_SELF_TESTS
	}
}

#undef MSE_THROW

#endif // MSEASYNCSHARED_H_
