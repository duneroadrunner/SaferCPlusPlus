
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEASYNCSHARED_H_
#define MSEASYNCSHARED_H_

#include "mseoptional.h"
#include "msemsearray.h"
#include "msemsevector.h"
#ifndef MSE_ASYNCSHARED_NO_XSCOPE_DEPENDENCE
#include "msescope.h"
#endif // !MSE_ASYNCSHARED_NO_XSCOPE_DEPENDENCE
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <cassert>
#include <stdexcept>
#include <ctime>
#include <ratio>
#include <chrono>
#include <vector>

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

#ifndef _STD
#define _STD std::
#endif /*_STD*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifndef _THROW_NCEE
#define _THROW_NCEE(x, y)	MSE_THROW(x(y))
#endif /*_THROW_NCEE*/


	/* This macro roughly simulates constructor inheritance. */
#define MSE_ASYNC_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

#ifdef MSEPOINTERBASICS_H
	typedef StrongPointerTagBase AsyncSharedStrongPointerTagBase;
	class AsyncSharedStrongPointerNotAsyncShareableTagBase : public AsyncSharedStrongPointerTagBase, public NotAsyncShareableTagBase {};
#else // MSEPOINTERBASICS_H
	class AsyncSharedStrongPointerTagBase {};
	class AsyncSharedStrongPointerNotAsyncShareableTagBase : public AsyncSharedStrongPointerTagBase {};
#endif // MSEPOINTERBASICS_H

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

	/* todo: Detect the case where two threads each have a "read" lock and are (indefinitely) blocking on obtaining a "write"
	lock (in addition to the "read" locks they already hold). This is a deadlock case. We should throw an exception when the
	second thread requests a "write" lock (by calling "lock()"). */
	/* Note that this "recursive_shared_timed_mutex" allows a thread to hold "read" (shared) locks and "write" locks at the
	same time. It also provides "nonrecursive_lock()" member functions to obtain a lock that is exclusive within the thread
	as well as between threads. */
	class recursive_shared_timed_mutex : private std::shared_timed_mutex {
	public:
		typedef std::shared_timed_mutex base_class;

		void lock()
		{	// lock exclusive
			std::lock_guard<std::mutex> lock1(m_mutex1);

			if ((1 <= m_readlock_count) && (!m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock)) {
				assert(0 == m_writelock_count);
				const auto this_thread_id = std::this_thread::get_id();
				const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
				if (m_thread_id_readlock_count_map.end() != found_it) {
					assert(1 <= (*found_it).second);
					/* This thread currently holds a shared_lock on the underlying mutex. We'll release it (and note that
					we did so) so as not to prevent the exclusive_lock from being acquired. */
					base_class::unlock_shared();
					m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = true;
				}
			}

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
				if (m_writelock_is_nonrecursive) {
					MSE_THROW(std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur)));
				}
			}
			else {
				assert((0 == m_writelock_count) || (std::this_thread::get_id() != m_writelock_thread_id));
				{
					unlock_guard<std::mutex> unlock1(m_mutex1);
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
			std::lock_guard<std::mutex> lock1(m_mutex1);

			if ((1 <= m_readlock_count) && (!m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock)) {
				assert(0 == m_writelock_count);
				const auto this_thread_id = std::this_thread::get_id();
				const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
				if (m_thread_id_readlock_count_map.end() != found_it) {
					if ((*found_it).second == m_readlock_count) {
						/* All of the (one or more) readlocks are owned by this thread. So we should be able to release
						the shared_lock on the underlying mutex and immediately acquire an exclusive_lock. */
						base_class::unlock_shared();
						base_class::lock();
						assert(0 == m_writelock_count);
						m_writelock_thread_id = std::this_thread::get_id();
						m_writelock_count += 1;
						m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = true;
						retval = true;
					}
				}
			}

			if (!retval) {
				if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
					if (m_writelock_is_nonrecursive) {
						retval = false;
					}
					else {
						m_writelock_count += 1;
						retval = true;
					}
				}
				else {
					retval = base_class::try_lock();
					if (retval) {
						assert(0 == m_writelock_count);
						m_writelock_thread_id = std::this_thread::get_id();
						m_writelock_count += 1;
					}
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
			std::lock_guard<std::mutex> lock1(m_mutex1);

			if ((1 <= m_readlock_count) && (!m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock)) {
				assert(0 == m_writelock_count);
				const auto this_thread_id = std::this_thread::get_id();
				const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
				if (m_thread_id_readlock_count_map.end() != found_it) {
					if ((*found_it).second == m_readlock_count) {
						/* All of the (one or more) readlocks are owned by this thread. So we should be able to release
						the shared_lock on the underlying mutex and immediately acquire an exclusive_lock. */
						base_class::unlock_shared();
						base_class::lock();
						assert(0 == m_writelock_count);
						m_writelock_thread_id = std::this_thread::get_id();
						m_writelock_count += 1;
						m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = true;
						retval = true;
					}
				}
			}

			if (!retval) {
				if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
					if (m_writelock_is_nonrecursive) {
						retval = false;
					}
					else {
						m_writelock_count += 1;
						retval = true;
					}
				}
				else {
					{
						unlock_guard<std::mutex> unlock1(m_mutex1);
						retval = base_class::try_lock_until(_Abs_time);
					}
					if (retval) {
						assert(0 == m_writelock_count);
						m_writelock_thread_id = std::this_thread::get_id();
						m_writelock_count += 1;
					}
				}
			}
			return retval;
		}

		void unlock()
		{	// unlock exclusive
			std::lock_guard<std::mutex> lock1(m_mutex1);
			assert(std::this_thread::get_id() == m_writelock_thread_id);

			if ((2 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
			}
			else {
				assert(1 == m_writelock_count);
				base_class::unlock();
				m_writelock_is_nonrecursive = false;
				if (m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock) {
					/* We need to reacquire the shared_lock that was suspended to make way for the exclusive_lock we
					just released. */
					base_class::lock_shared();
					m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = false;
				}
			}
			m_writelock_count -= 1;
		}

		void nonrecursive_lock()
		{	// lock nonrecursive
			std::lock_guard<std::mutex> lock1(m_mutex1);

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
				MSE_THROW(std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur)));
			}
			else {
				if (1 <= m_readlock_count) {
					const auto this_thread_id = std::this_thread::get_id();
					const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
					if (m_thread_id_readlock_count_map.end() != found_it) {
						assert(1 <= (*found_it).second);
						MSE_THROW(std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur)));
					}
				}

				assert((std::this_thread::get_id() != m_writelock_thread_id) || (0 == m_writelock_count));
				{
					unlock_guard<std::mutex> unlock1(m_mutex1);
					base_class::lock();
				}
				m_writelock_thread_id = std::this_thread::get_id();
				assert(0 == m_writelock_count);
			}
			m_writelock_count += 1;
			m_writelock_is_nonrecursive = true;
		}

		bool try_nonrecursive_lock()
		{	// try to lock nonrecursive
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_mutex1);

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
				retval = false;
			}
			else {
				if (1 <= m_readlock_count) {
					const auto this_thread_id = std::this_thread::get_id();
					const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
					if (m_thread_id_readlock_count_map.end() != found_it) {
						assert(1 <= (*found_it).second);
						return false;
					}
				}

				retval = base_class::try_lock();
				if (retval) {
					assert(0 == m_writelock_count);
					m_writelock_thread_id = std::this_thread::get_id();
					m_writelock_count += 1;
					m_writelock_is_nonrecursive = true;
				}
			}
			return retval;
		}

		template<class _Rep, class _Period>
		bool try_nonrecursive_lock_for(const std::chrono::duration<_Rep, _Period>& _Rel_time)
		{	// try to nonrecursive lock for duration
			return (try_nonrecursive_lock_until(std::chrono::steady_clock::now() + _Rel_time));
		}

		template<class _Clock, class _Duration>
		bool try_nonrecursive_lock_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time)
		{	// try to nonrecursive lock until time point
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_mutex1);

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
				retval = false;
			}
			else {
				if (1 <= m_readlock_count) {
					const auto this_thread_id = std::this_thread::get_id();
					const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
					if (m_thread_id_readlock_count_map.end() != found_it) {
						assert(1 <= (*found_it).second);
						return false;
					}
				}

				{
					unlock_guard<std::mutex> unlock1(m_mutex1);
					retval = base_class::try_lock_until(_Abs_time);
				}
				if (retval) {
					assert(0 == m_writelock_count);
					m_writelock_thread_id = std::this_thread::get_id();
					m_writelock_count += 1;
					m_writelock_is_nonrecursive = true;
				}
			}
			return retval;
		}

		void nonrecursive_unlock()
		{	// unlock nonrecursive
			std::lock_guard<std::mutex> lock1(m_mutex1);
			assert(std::this_thread::get_id() == m_writelock_thread_id);
			assert((m_writelock_is_nonrecursive) && (1 == m_writelock_count));
			assert(0 == m_readlock_count);

			base_class::unlock();
			m_writelock_is_nonrecursive = false;
			m_writelock_count -= 1;
		}

		void lock_shared()
		{	// lock non-exclusive
			std::lock_guard<std::mutex> lock1(m_mutex1);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
			}
			else if ((1 <= m_writelock_count) && (this_thread_id == m_writelock_thread_id) && (!m_writelock_is_nonrecursive)) {
				assert((m_thread_id_readlock_count_map.end() == found_it) || (0 == (*found_it).second));
				assert(!m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock);
				m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = true;
				try {
					std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
					m_thread_id_readlock_count_map.insert(item);
				}
				catch (...) {
					m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = false;
					MSE_THROW(asyncshared_runtime_error("std::unordered_map<>::insert() failed? - mse::recursive_shared_timed_mutex"));
				}
			}
			else {
				assert((m_thread_id_readlock_count_map.end() == found_it) || (0 == (*found_it).second));
				{
					unlock_guard<std::mutex> unlock1(m_mutex1);
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
			m_readlock_count += 1;
		}

		bool try_lock_shared()
		{	// try to lock non-exclusive
			bool retval = false;
			std::lock_guard<std::mutex> lock1(m_mutex1);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
				m_readlock_count += 1;
				retval = true;
			}
			else if ((1 <= m_writelock_count) && (this_thread_id == m_writelock_thread_id) && (!m_writelock_is_nonrecursive)) {
				assert((m_thread_id_readlock_count_map.end() == found_it) || (0 == (*found_it).second));
				assert(!m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock);
				m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = true;
				try {
					std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
					m_thread_id_readlock_count_map.insert(item);
					m_readlock_count += 1;
					retval = true;
				}
				catch (...) {
					m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = false;
					MSE_THROW(asyncshared_runtime_error("std::unordered_map<>::insert() failed? - mse::recursive_shared_timed_mutex"));
				}
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
						m_readlock_count += 1;
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
			std::lock_guard<std::mutex> lock1(m_mutex1);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
				m_readlock_count += 1;
				retval = true;
			}
			else if ((1 <= m_writelock_count) && (this_thread_id == m_writelock_thread_id) && (!m_writelock_is_nonrecursive)) {
				assert((m_thread_id_readlock_count_map.end() == found_it) || (0 == (*found_it).second));
				assert(!m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock);
				m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = true;
				try {
					std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
					m_thread_id_readlock_count_map.insert(item);
					m_readlock_count += 1;
					retval = true;
				}
				catch (...) {
					m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = false;
					MSE_THROW(asyncshared_runtime_error("std::unordered_map<>::insert() failed? - mse::recursive_shared_timed_mutex"));
				}
			}
			else {
				{
					unlock_guard<std::mutex> unlock1(m_mutex1);
					retval = base_class::try_lock_shared_until(_Abs_time);
				}
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
						m_readlock_count += 1;
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
			std::lock_guard<std::mutex> lock1(m_mutex1);

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if (m_thread_id_readlock_count_map.end() != found_it) {
				if (2 <= (*found_it).second) {
					(*found_it).second -= 1;
				}
				else {
					assert(1 == (*found_it).second);
					m_thread_id_readlock_count_map.erase(found_it);
					if (m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock) {
						m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = false;
					}
					else {
						base_class::unlock_shared();
					}
				}
			}
			else {
				assert(false);
				MSE_THROW(asyncshared_runtime_error("unpaired unlock_shared() call? - mse::recursive_shared_timed_mutex"));
				//base_class::unlock_shared();
			}
			m_readlock_count -= 1;
		}

		//std::mutex m_write_mutex;
		//std::mutex m_read_mutex;
		std::mutex m_mutex1;

		std::thread::id m_writelock_thread_id;
		int m_writelock_count = 0;
		bool m_writelock_is_nonrecursive = false;
		std::unordered_map<std::thread::id, int> m_thread_id_readlock_count_map;
		int m_readlock_count = 0;
		bool m_a_shared_lock_is_suspended_to_allow_an_exclusive_lock = false;
	};

	//typedef std::shared_timed_mutex async_shared_timed_mutex_type;
	typedef recursive_shared_timed_mutex async_shared_timed_mutex_type;


	template<typename _TAccessLease> class TAsyncSharedV2XWPReadWriteAccessRequester;
	template<typename _TAccessLease> class TAsyncSharedV2ReadWritePointer;
	template<typename _TAccessLease> class TAsyncSharedV2ReadWriteConstPointer;
	template<typename _TAccessLease> class TAsyncSharedV2ExclusiveReadWritePointer;
	template<typename _TAccessLease> class TAsyncSharedV2XWPReadOnlyAccessRequester;
	template<typename _TAccessLease> class TAsyncSharedV2ReadOnlyConstPointer;

	template <typename _TAccessLease>
	class TAsyncSharedXWPAccessLeaseObj {
	public:
		TAsyncSharedXWPAccessLeaseObj(_TAccessLease&& access_lease)
			: m_access_lease(std::forward<_TAccessLease>(access_lease)) {}
		const _TAccessLease& cref() const {
			return m_access_lease;
		}
		async_shared_timed_mutex_type& mutex_ref() const {
			return m_mutex1;
		}
	private:
		_TAccessLease m_access_lease;

		mutable async_shared_timed_mutex_type m_mutex1;

		friend class TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>;
		friend class TAsyncSharedV2ReadWritePointer<_TAccessLease>;
		friend class TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>;
		friend class TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>;
		friend class TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease>;
		friend class TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>;
	};

	template<typename _TAccessLease> class TAsyncSharedV2ReadWriteConstPointer;

	template<typename _TAccessLease>
	class TAsyncSharedV2ReadWritePointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedV2ReadWritePointer(const TAsyncSharedV2ReadWritePointer& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedV2ReadWritePointer(TAsyncSharedV2ReadWritePointer&& src) = default;
		virtual ~TAsyncSharedV2ReadWritePointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typedef std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr_t;
		/* gcc5 crashes if you uncomment the explicit return type declaration (Nov 2017). */
		auto& operator*() const/* -> typename std::add_lvalue_reference<decltype(*((*std::declval<m_shptr_t>()).cref()))>::type*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return (*((*m_shptr).cref()));
		}
		auto operator->() const/* -> decltype(std::addressof(*((*std::declval<m_shptr_t>()).cref())))*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return std::addressof(*((*m_shptr).cref()));
		}
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	private:
		TAsyncSharedV2ReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedV2ReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedV2ReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedV2ReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
			}
		}
		TAsyncSharedV2ReadWritePointer<_TAccessLease>& operator=(const TAsyncSharedV2ReadWritePointer<_TAccessLease>& _Right_cref) = delete;
		TAsyncSharedV2ReadWritePointer<_TAccessLease>& operator=(TAsyncSharedV2ReadWritePointer<_TAccessLease>&& _Right) = delete;

		TAsyncSharedV2ReadWritePointer<_TAccessLease>* operator&() { return this; }
		const TAsyncSharedV2ReadWritePointer<_TAccessLease>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>;
		friend class TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>;
	};

	template<typename _TAccessLease>
	class TAsyncSharedV2ReadWriteConstPointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedV2ReadWriteConstPointer(const TAsyncSharedV2ReadWriteConstPointer& src) : m_shptr(src.m_shptr), m_shared_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedV2ReadWriteConstPointer(TAsyncSharedV2ReadWriteConstPointer&& src) = default;
		TAsyncSharedV2ReadWriteConstPointer(const TAsyncSharedV2ReadWritePointer<_TAccessLease>& src) : m_shptr(src.m_shptr), m_shared_lock(src.m_shptr->m_mutex1) {}
		virtual ~TAsyncSharedV2ReadWriteConstPointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWriteConstPointer")); }
			return m_shptr.operator bool();
		}
		typedef std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr_t;
		const auto& operator*() const/* -> typename std::add_const<typename std::add_lvalue_reference<decltype(*((*std::declval<m_shptr_t>()).cref()))>::type>::type*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return (*((*m_shptr).cref()));
		}
		const auto operator->() const/* -> typename std::add_const<decltype(std::addressof(*((*std::declval<m_shptr_t>()).cref())))>::type*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return std::addressof(*((*m_shptr).cref()));
		}
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	private:
		TAsyncSharedV2ReadWriteConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1) {}
		TAsyncSharedV2ReadWriteConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock()) {
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedV2ReadWriteConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedV2ReadWriteConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
			}
		}
		TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>& operator=(const TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>& _Right_cref) = delete;
		TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>& operator=(TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>&& _Right) = delete;

		TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>* operator&() { return this; }
		const TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr;
		std::shared_lock<async_shared_timed_mutex_type> m_shared_lock;

		friend class TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>;
	};

	template<typename _TAccessLease>
	class TAsyncSharedV2ExclusiveReadWritePointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedV2ExclusiveReadWritePointer(const TAsyncSharedV2ExclusiveReadWritePointer& src) = delete;
		TAsyncSharedV2ExclusiveReadWritePointer(TAsyncSharedV2ExclusiveReadWritePointer&& src) = default;
		virtual ~TAsyncSharedV2ExclusiveReadWritePointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ExclusiveReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typedef std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr_t;
		auto& operator*() const/* -> typename std::add_lvalue_reference<decltype(*((*std::declval<m_shptr_t>()).cref()))>::type*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return (*((*m_shptr).cref()));
		}
		auto operator->() const/* -> decltype(std::addressof(*((*std::declval<m_shptr_t>()).cref())))*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return std::addressof(*((*m_shptr).cref()));
		}
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	private:
		TAsyncSharedV2ExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedV2ExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedV2ExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedV2ExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
			}
		}
		TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>& operator=(const TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>& _Right_cref) = delete;
		TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>& operator=(TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>&& _Right) = delete;

		TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>* operator&() { return this; }
		const TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr;
		unique_nonrecursive_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>;
		//friend class TAsyncSharedXWPReadWriteExclusiveConstPointer<_TAccessLease>;
	};

	template<typename _TAccessLease>
	class TAsyncSharedV2XWPReadWriteAccessRequester {
	public:
		TAsyncSharedV2XWPReadWriteAccessRequester(const TAsyncSharedV2XWPReadWriteAccessRequester& src_cref) = default;
		TAsyncSharedV2XWPReadWriteAccessRequester(_TAccessLease&& exclusive_write_pointer) {
			m_shptr = std::make_shared<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>>(std::forward<_TAccessLease>(exclusive_write_pointer));
		}

		TAsyncSharedV2ReadWritePointer<_TAccessLease> writelock_ptr() {
			return TAsyncSharedV2ReadWritePointer<_TAccessLease>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedV2ReadWritePointer<_TAccessLease>> try_writelock_ptr() {
			mse::mstd::optional<TAsyncSharedV2ReadWritePointer<_TAccessLease>> retval(TAsyncSharedV2ReadWritePointer<_TAccessLease>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedV2ReadWritePointer<_TAccessLease>> try_writelock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedV2ReadWritePointer<_TAccessLease>> retval(TAsyncSharedV2ReadWritePointer<_TAccessLease>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedV2ReadWritePointer<_TAccessLease>> try_writelock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedV2ReadWritePointer<_TAccessLease>> retval(TAsyncSharedV2ReadWritePointer<_TAccessLease>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		TAsyncSharedV2ReadWriteConstPointer<_TAccessLease> readlock_ptr() {
			return TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>> try_readlock_ptr() {
			mse::mstd::optional<TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>> retval(TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>> retval(TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>> retval(TAsyncSharedV2ReadWriteConstPointer<_TAccessLease>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		/* Note that an exclusive_writelock_ptr cannot coexist with any other lock_ptrs (targeting the same object), including ones in
		the same thread. Thus, using exclusive_writelock_ptrs without sufficient care introduces the potential for exceptions (in a way
		that sticking to (regular) writelock_ptrs doesn't). */
		TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease> exclusive_writelock_ptr() {
			return TAsyncSharedV2ExclusiveReadWritePointer<_TAccessLease>(m_shptr);
		}

		static TAsyncSharedV2XWPReadWriteAccessRequester make(_TAccessLease&& exclusive_write_pointer) {
			return TAsyncSharedV2XWPReadWriteAccessRequester(std::make_shared<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>>(std::forward<_TAccessLease>(exclusive_write_pointer)));
		}

		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		TAsyncSharedV2XWPReadWriteAccessRequester(const std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>>& shptr) : m_shptr(shptr) {}
		TAsyncSharedV2XWPReadWriteAccessRequester(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>>&& shptr) : m_shptr(std::forward<decltype(shptr)>(shptr)) {}

		TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>* operator&() { return this; }
		const TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr;

		friend class TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease>;
	};

	template<typename _TAccessLease>
	TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease> make_asyncsharedv2xwpreadwrite(_TAccessLease&& exclusive_write_pointer) {
		return TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>::make(std::forward<_TAccessLease>(exclusive_write_pointer));
	}

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedV2ReadWriteStore = TXScopeStrongNotNullPointerStore<TAsyncSharedV2ReadWritePointer<_Ty> >;
	template<typename _Ty> using TXScopeAsyncSharedV2ReadWriteConstStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedV2ReadWriteConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedV2ReadWriteStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedV2ReadWritePointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedV2ReadWriteStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeAsyncSharedV2ReadWriteConstStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedV2ReadWriteConstPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedV2ReadWriteConstStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_


	template<typename _TAccessLease>
	class TAsyncSharedV2ReadOnlyConstPointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedV2ReadOnlyConstPointer(const TAsyncSharedV2ReadOnlyConstPointer& src) : m_shptr(src.m_shptr), m_shared_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedV2ReadOnlyConstPointer(TAsyncSharedV2ReadOnlyConstPointer&& src) = default;
		virtual ~TAsyncSharedV2ReadOnlyConstPointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadOnlyConstPointer")); }
			return m_shptr.operator bool();
		}
		typedef std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr_t;
		const auto& operator*() const/* -> typename std::add_const<typename std::add_lvalue_reference<decltype(*((*std::declval<m_shptr_t>()).cref()))>::type>::type*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return (*((*m_shptr).cref()));
		}
		const auto operator->() const/* -> typename std::add_const<decltype(std::addressof(*((*std::declval<m_shptr_t>()).cref())))>::type*/ {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedV2ReadWritePointer")); }
			return std::addressof(*((*m_shptr).cref()));
		}
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	private:
		TAsyncSharedV2ReadOnlyConstPointer(std::shared_ptr<const TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1) {}
		TAsyncSharedV2ReadOnlyConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock()) {
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedV2ReadOnlyConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedV2ReadOnlyConstPointer(std::shared_ptr<TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
			}
		}
		TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>& operator=(const TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>& _Right_cref) = delete;
		TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>& operator=(TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>&& _Right) = delete;

		TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>* operator&() { return this; }
		const TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<const TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr;
		std::shared_lock<async_shared_timed_mutex_type> m_shared_lock;

		friend class TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease>;
	};

	template<typename _TAccessLease>
	class TAsyncSharedV2XWPReadOnlyAccessRequester {
	public:
		TAsyncSharedV2XWPReadOnlyAccessRequester(const TAsyncSharedV2XWPReadOnlyAccessRequester& src_cref) = default;
		TAsyncSharedV2XWPReadOnlyAccessRequester(const TAsyncSharedV2XWPReadWriteAccessRequester<_TAccessLease>& src_cref) : m_shptr(src_cref.m_shptr) {}

		TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease> readlock_ptr() {
			return TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>> try_readlock_ptr() {
			mse::mstd::optional<TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>> retval(TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>> retval(TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>> retval(TAsyncSharedV2ReadOnlyConstPointer<_TAccessLease>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedV2XWPReadOnlyAccessRequester make(Args&&... args) {
			return TAsyncSharedV2XWPReadOnlyAccessRequester(std::make_shared<const TAsyncSharedXWPAccessLeaseObj<_TAccessLease>>(std::forward<Args>(args)...));
		}

		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		TAsyncSharedV2XWPReadOnlyAccessRequester(std::shared_ptr<const TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> shptr) : m_shptr(shptr) {}

		TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease>* operator&() { return this; }
		const TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease>* operator&() const { return this; }

		std::shared_ptr<const TAsyncSharedXWPAccessLeaseObj<_TAccessLease>> m_shptr;
	};

	template<typename _TAccessLease>
	TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease> make_asyncsharedv2xwpreadonly(_TAccessLease&& exclusive_write_pointer) {
		return TAsyncSharedV2XWPReadOnlyAccessRequester<_TAccessLease>::make(std::forward<_TAccessLease>(exclusive_write_pointer));
	}

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedV2ReadOnlyConstStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedV2ReadOnlyConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedV2ReadOnlyConstStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedV2ReadOnlyConstPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedV2ReadOnlyConstStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_


	template <typename _Ty>
	class TAsyncSharedV2ReadWriteAccessRequester : public TAsyncSharedV2XWPReadWriteAccessRequester<std::shared_ptr<_Ty>> {
	public:
		typedef TAsyncSharedV2XWPReadWriteAccessRequester<std::shared_ptr<_Ty>> base_class;
		typedef decltype(std::declval<base_class>().writelock_ptr()) writelock_ptr_t;
		typedef decltype(std::declval<base_class>().readlock_ptr()) readlock_ptr_t;

		TAsyncSharedV2ReadWriteAccessRequester(const TAsyncSharedV2ReadWriteAccessRequester& src_cref) = default;

		~TAsyncSharedV2ReadWriteAccessRequester() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			valid_if_Ty_is_marked_as_shareable();
			valid_if_Ty_is_not_an_xscope_type();
		}

		template <class... Args>
		static TAsyncSharedV2ReadWriteAccessRequester make(Args&&... args) {
			return TAsyncSharedV2ReadWriteAccessRequester(std::make_shared<_Ty>(std::forward<Args>(args)...));
		}

		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		/* If _Ty is not "marked" as safe to share among threads (via the presence of the "async_shareable_tag()" member
		function), then the following member function will not instantiate, causing an (intended) compile error. User-defined
		objects can be marked safe to share by wrapping them with us::TUserDeclaredAsyncShareableObj<>. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>()), void>::type>
		void valid_if_Ty_is_marked_as_shareable() const {}

		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void valid_if_Ty_is_not_an_xscope_type() const {}

		//TAsyncSharedV2ReadWriteAccessRequester(const std::shared_ptr<_Ty>& shptr) : base_class(make_asyncsharedv2xwpreadwrite(std::shared_ptr<_Ty>(shptr))) {}
		TAsyncSharedV2ReadWriteAccessRequester(std::shared_ptr<_Ty>&& shptr) : base_class(make_asyncsharedv2xwpreadwrite(std::forward<decltype(shptr)>(shptr))) {}

		TAsyncSharedV2ReadWriteAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedV2ReadWriteAccessRequester<_Ty>* operator&() const { return this; }
	};

	template <class X, class... Args>
	TAsyncSharedV2ReadWriteAccessRequester<X> make_asyncsharedv2readwrite(Args&&... args) {
		return TAsyncSharedV2ReadWriteAccessRequester<X>::make(std::forward<Args>(args)...);
	}

	template <typename _Ty>
	class TAsyncSharedV2ReadOnlyAccessRequester : public TAsyncSharedV2XWPReadOnlyAccessRequester<std::shared_ptr<_Ty>> {
	public:
		typedef TAsyncSharedV2XWPReadOnlyAccessRequester<std::shared_ptr<_Ty>> base_class;
		typedef decltype(std::declval<base_class>().readlock_ptr()) readlock_ptr_t;

		TAsyncSharedV2ReadOnlyAccessRequester(const TAsyncSharedV2ReadOnlyAccessRequester& src_cref) = default;
		TAsyncSharedV2ReadOnlyAccessRequester(const TAsyncSharedV2ReadWriteAccessRequester<_Ty>& src_cref) : base_class(src_cref) {}

		~TAsyncSharedV2ReadOnlyAccessRequester() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			valid_if_Ty_is_marked_as_shareable();
			valid_if_Ty_is_not_an_xscope_type();
		}

		template <class... Args>
		static TAsyncSharedV2ReadOnlyAccessRequester make(Args&&... args) {
			return TAsyncSharedV2ReadOnlyAccessRequester(std::make_shared<_Ty>(std::forward<Args>(args)...));
		}

		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		/* If _Ty is not "marked" as safe to share among threads (via the presence of the "async_shareable_tag()" member
		function), then the following member function will not instantiate, causing an (intended) compile error. User-defined
		objects can be marked safe to share by wrapping them with us::TUserDeclaredAsyncShareableObj<>. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>()), void>::type>
		void valid_if_Ty_is_marked_as_shareable() const {}

		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void valid_if_Ty_is_not_an_xscope_type() const {}

		//TAsyncSharedV2ReadOnlyAccessRequester(const std::shared_ptr<_Ty>& shptr) : base_class(make_asyncsharedv2xwpreadonly(std::shared_ptr<_Ty>(shptr))) {}
		TAsyncSharedV2ReadOnlyAccessRequester(std::shared_ptr<_Ty>&& shptr) : base_class(make_asyncsharedv2xwpreadonly(std::forward<decltype(shptr)>(shptr))) {}

		TAsyncSharedV2ReadOnlyAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedV2ReadOnlyAccessRequester<_Ty>* operator&() const { return this; }
	};

	template <class X, class... Args>
	TAsyncSharedV2ReadOnlyAccessRequester<X> make_asyncsharedv2readonly(Args&&... args) {
		return TAsyncSharedV2ReadOnlyAccessRequester<X>::make(std::forward<Args>(args)...);
	}


	/* For situations where the shared object is immutable (i.e. is never modified), you don't even need locks or access requesters. */
	template<typename _Ty>
	class TAsyncSharedV2ImmutableFixedPointer : public AsyncSharedStrongPointerTagBase {
	public:
		TAsyncSharedV2ImmutableFixedPointer(const TAsyncSharedV2ImmutableFixedPointer& src_cref) = default;
		virtual ~TAsyncSharedV2ImmutableFixedPointer() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			valid_if_Ty_is_marked_as_shareable();
			valid_if_Ty_is_not_an_xscope_type();
		}

		operator bool() const {
			return m_shptr.operator bool();
		}
		const auto& operator*() const {
			return (*m_shptr);
		}
		const auto operator->() const {
			return std::addressof(*m_shptr);
		}

		/* This operator is just for compatibility with existing/legacy code. */
		explicit operator std::shared_ptr<const _Ty>() const { return m_shptr; }

		template <class... Args>
		static TAsyncSharedV2ImmutableFixedPointer make(Args&&... args) {
			return TAsyncSharedV2ImmutableFixedPointer(std::make_shared<const _Ty>(std::forward<Args>(args)...));
		}

		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		/* If _Ty is not "marked" as safe to share among threads (via the presence of the "async_shareable_tag()" member
		function), then the following member function will not instantiate, causing an (intended) compile error. User-defined
		objects can be marked safe to share by wrapping them with us::TUserDeclaredAsyncShareableObj<>. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>()), void>::type>
		void valid_if_Ty_is_marked_as_shareable() const {}

		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void valid_if_Ty_is_not_an_xscope_type() const {}

		TAsyncSharedV2ImmutableFixedPointer(std::shared_ptr<const _Ty> shptr) : m_shptr(shptr) {}
		TAsyncSharedV2ImmutableFixedPointer<_Ty>& operator=(const TAsyncSharedV2ImmutableFixedPointer<_Ty>& _Right_cref) = delete;

		TAsyncSharedV2ImmutableFixedPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedV2ImmutableFixedPointer<_Ty>* operator&() const { return this; }

		std::shared_ptr<const _Ty> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedV2ImmutableFixedPointer<X> make_asyncsharedv2immutable(Args&&... args) {
		return TAsyncSharedV2ImmutableFixedPointer<X>::make(std::forward<Args>(args)...);
	}

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedV2ImmutableFixedStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedV2ImmutableFixedPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedV2ImmutableFixedStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedV2ImmutableFixedPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedV2ImmutableFixedStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_

#if defined(MSEPOINTERBASICS_H)
	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::writelock_ptr_t> make_pointer_to_member(_TTargetType& target, const typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::writelock_ptr_t &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::writelock_ptr_t>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::readlock_ptr_t> make_const_pointer_to_member(const _TTargetType& target, const typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::readlock_ptr_t &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::readlock_ptr_t>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, typename TAsyncSharedV2ReadOnlyAccessRequester<_Ty>::readlock_ptr_t> make_const_pointer_to_member(const _TTargetType& target, const typename TAsyncSharedV2ReadOnlyAccessRequester<_Ty>::readlock_ptr_t &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, typename TAsyncSharedV2ReadOnlyAccessRequester<_Ty>::readlock_ptr_t>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TAsyncSharedV2ImmutableFixedPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TAsyncSharedV2ImmutableFixedPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TAsyncSharedV2ImmutableFixedPointer<_Ty>>::make(target, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	static auto make_pointer_to_member_v2(const typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::writelock_ptr_t &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedPointer<_TTarget, typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::writelock_ptr_t>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	static auto make_const_pointer_to_member_v2(const typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::readlock_ptr_t &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, typename TAsyncSharedV2ReadWriteAccessRequester<_Ty>::readlock_ptr_t>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	static auto make_const_pointer_to_member_v2(const typename TAsyncSharedV2ReadOnlyAccessRequester<_Ty>::readlock_ptr_t &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, typename TAsyncSharedV2ReadOnlyAccessRequester<_Ty>::readlock_ptr_t>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	static auto make_const_pointer_to_member_v2(const TAsyncSharedV2ImmutableFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TAsyncSharedV2ImmutableFixedPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
#endif // defined(MSEPOINTERBASICS_H)

	namespace us {
		namespace impl {

			template <typename _TRAIterator>
			class TAsyncSplitterRandomAccessSection
				: public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TRAIterator>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TAsyncSplitterRandomAccessSection<_TRAIterator> > >::type
			{
			public:
				typedef typename std::remove_reference<decltype(std::declval<_TRAIterator>()[0])>::type element_t;
				typedef decltype(std::declval<_TRAIterator>()[0]) reference_t;
				typedef typename std::add_lvalue_reference<typename std::add_const<element_t>::type>::type const_reference_t;
				typedef typename mse::us::msearray<element_t, 0>::size_type size_type;
				typedef decltype(std::declval<_TRAIterator>() - std::declval<_TRAIterator>()) difference_t;

				TAsyncSplitterRandomAccessSection(const _TRAIterator& start_iter, size_type count) : m_start_iter(start_iter), m_count(count) {}

				reference_t operator[](size_type _P) const {
					if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - reference_t operator[](size_type _P) - TAsyncSplitterRandomAccessSection")); }
					return m_start_iter[difference_t(_P)];
				}
				size_type size() const {
					return m_count;
				}

				/* We will mark this type as safely "async shareable" if the elements it contains are also "async shareable"
				and _TRAIterator is marked as "strong". This is technically unsafe as those criteria may not be sufficient
				to ensure safe "async shareability". */
				template<class element_t2 = element_t, class = typename std::enable_if<(std::is_same<element_t2, element_t>::value)
					&& ((std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<element_t2>::Has>()) || (std::is_arithmetic<element_t2>::value))
					&& (std::is_base_of<StrongPointerTagBase, _TRAIterator>::value)
					, void>::type>
					void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

			private:
				TAsyncSplitterRandomAccessSection(const TAsyncSplitterRandomAccessSection& src) = default;
				template <typename _TRAIterator1>
				TAsyncSplitterRandomAccessSection(const TAsyncSplitterRandomAccessSection<_TRAIterator1>& src) : m_start_iter(src.m_start_iter), m_count(src.m_count) {}

				TAsyncSplitterRandomAccessSection<_TRAIterator>* operator&() { return this; }
				const TAsyncSplitterRandomAccessSection<_TRAIterator>* operator&() const { return this; }

				_TRAIterator m_start_iter;
				const size_type m_count = 0;

				template <typename _TExclusiveWritelockPtr>
				friend class TAsyncRASectionSplitterXWP;
			};
		}
	}

	template <typename _TRAIterator>
	class TAsyncSplitterRASectionReadWriteAccessRequester {
	public:
		typedef mse::us::impl::TAsyncSplitterRandomAccessSection<_TRAIterator> splitter_ra_section_t;
		typedef decltype(std::declval<splitter_ra_section_t>().size()) size_type;

		TAsyncSplitterRASectionReadWriteAccessRequester(const TAsyncSplitterRASectionReadWriteAccessRequester& src_cref) = default;

		typedef TRandomAccessSection<TRAIterator<TAsyncSharedV2ReadWritePointer<std::shared_ptr<splitter_ra_section_t> > > > rw_ra_section_t;
		rw_ra_section_t writelock_ra_section() {
			return rw_ra_section_t(TRAIterator<decltype(m_splitter_ra_section_access_requester.writelock_ptr())>(m_splitter_ra_section_access_requester.writelock_ptr()), m_count);
		}
		mse::mstd::optional<rw_ra_section_t> try_writelock_ra_section() {
			auto maybe_wl_ptr = m_splitter_ra_section_access_requester.try_writelock_ptr();
			if (!(maybe_wl_ptr.has_value())) {
				return{};
			}
			auto& wl_ptr = maybe_wl_ptr.value();
			return mse::mstd::optional<rw_ra_section_t>(rw_ra_section_t(TRAIterator<decltype(wl_ptr)>(wl_ptr), wl_ptr->size()));
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<rw_ra_section_t> try_writelock_ra_section_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			auto abs_time = std::chrono::system_clock::now() + _Rel_time;
			return try_writelock_ra_section_until(abs_time);
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<rw_ra_section_t> try_writelock_ra_section_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			auto maybe_wl_ptr = m_splitter_ra_section_access_requester.try_writelock_ptr_until(_Abs_time);
			if (!(maybe_wl_ptr.has_value())) {
				return{};
			}
			auto& wl_ptr = maybe_wl_ptr.value();
			return mse::mstd::optional<rw_ra_section_t>(rw_ra_section_t(TRAIterator<decltype(wl_ptr)>(wl_ptr), wl_ptr->size()));
		}

		typedef TRandomAccessSection<TRAIterator<TAsyncSharedV2ReadWriteConstPointer<splitter_ra_section_t> > > rwc_ra_section_t;
		rwc_ra_section_t readlock_ra_section() {
			return rwc_ra_section_t(TRAIterator<decltype(m_splitter_ra_section_access_requester.writelock_ptr())>(m_splitter_ra_section_access_requester.readlock_ptr()), m_count);
		}
		mse::mstd::optional<rwc_ra_section_t> try_readlock_ra_section() {
			auto maybe_rl_ptr = m_splitter_ra_section_access_requester.try_readlock_ptr();
			if (!(maybe_rl_ptr.has_value())) {
				return{};
			}
			auto& rl_ptr = maybe_rl_ptr.value();
			return mse::mstd::optional<rwc_ra_section_t>(rwc_ra_section_t(TRAIterator<decltype(rl_ptr)>(rl_ptr), rl_ptr->size()));
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<rwc_ra_section_t> try_readlock_ra_section_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			auto abs_time = std::chrono::system_clock::now() + _Rel_time;
			return try_readlock_ra_section_until(abs_time);
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<rwc_ra_section_t> try_readlock_ra_section_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			auto maybe_rl_ptr = m_splitter_ra_section_access_requester.try_readlock_ptr_until(_Abs_time);
			if (!(maybe_rl_ptr.has_value())) {
				return{};
			}
			auto& rl_ptr = maybe_rl_ptr.value();
			return mse::mstd::optional<rwc_ra_section_t>(rwc_ra_section_t(TRAIterator<decltype(rl_ptr)>(rl_ptr), rl_ptr->size()));
		}

		/* Note that an exclusive_writelock_ra_section cannot coexist with any other lock_ra_sections (targeting the same object), including ones in
		the same thread. Thus, using exclusive_writelock_ra_sections without sufficient care introduces the potential for exceptions (in a way
		that sticking to (regular) writelock_ra_sections doesn't). */
		typedef TRandomAccessSection<TRAIterator<TAsyncSharedV2ExclusiveReadWritePointer<splitter_ra_section_t> > > xrw_ra_section_t;
		xrw_ra_section_t exclusive_writelock_ra_section() {
			return xrw_ra_section_t(TRAIterator<decltype(m_splitter_ra_section_access_requester.writelock_ptr())>(m_splitter_ra_section_access_requester.exclusive_writelock_ptr()), m_count);
		}

		template <class... Args>
		static TAsyncSplitterRASectionReadWriteAccessRequester make(Args&&... args) {
			return TAsyncSplitterRASectionReadWriteAccessRequester(std::forward<Args>(args)...);
		}

		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:

		TAsyncSplitterRASectionReadWriteAccessRequester(const _TRAIterator& start_iter, size_type count) : m_count(count)
			, m_splitter_ra_section_access_requester(make_asyncsharedv2readwrite<splitter_ra_section_t>(start_iter, count)) {}

		TAsyncSplitterRASectionReadWriteAccessRequester<_TRAIterator>* operator&() { return this; }
		const TAsyncSplitterRASectionReadWriteAccessRequester<_TRAIterator>* operator&() const { return this; }

		const size_type m_count = 0;
		mse::TAsyncSharedV2ReadWriteAccessRequester<splitter_ra_section_t> m_splitter_ra_section_access_requester;
	};

	template <class X, class... Args>
	TAsyncSplitterRASectionReadWriteAccessRequester<X> make_asyncsplitterrasectionreadwrite(Args&&... args) {
		return TAsyncSplitterRASectionReadWriteAccessRequester<X>::make(std::forward<Args>(args)...);
	}

	template <typename _TAccessLease>
	class TSplitterAccessLeaseObj {
	public:
		TSplitterAccessLeaseObj(_TAccessLease&& access_lease)
			: m_access_lease(std::forward<_TAccessLease>(access_lease)) {}
		const _TAccessLease& cref() const {
			return m_access_lease;
		}
	private:
		_TAccessLease m_access_lease;
	};

	template <typename _TExclusiveWritelockPtr>
	class TAsyncRASectionSplitterXWP {
	public:
		typedef _TExclusiveWritelockPtr exclusive_writelock_ptr_t;
		typedef typename std::remove_reference<decltype(*(std::declval<exclusive_writelock_ptr_t>()))>::type _TContainer;
		typedef typename std::remove_reference<decltype(std::declval<_TContainer>()[0])>::type element_t;
		typedef mse::TRAIterator<_TContainer*> ra_iterator_t;
		typedef decltype(mse::make_strong_iterator(std::declval<ra_iterator_t>(), std::declval<std::shared_ptr<TSplitterAccessLeaseObj<exclusive_writelock_ptr_t> > >())) strong_ra_iterator_t;
		typedef TAsyncSplitterRASectionReadWriteAccessRequester<strong_ra_iterator_t> ras_ar_t;

		template<typename _TList>
		TAsyncRASectionSplitterXWP(exclusive_writelock_ptr_t&& exclusive_writelock_ptr, const _TList& section_sizes)
			: m_access_lease_obj_shptr(std::make_shared<TSplitterAccessLeaseObj<exclusive_writelock_ptr_t> >(std::forward<exclusive_writelock_ptr_t>(exclusive_writelock_ptr))) {
			size_t cummulative_size = 0;
			//auto section_begin_it = m_access_lease_obj_shptr->cref()->begin();
			auto section_begin_it = ra_iterator_t(std::addressof(*(m_access_lease_obj_shptr->cref())));
			for (const auto& section_size : section_sizes) {
				if (0 > section_size) { MSE_THROW(std::range_error("invalid section size - TAsyncRASectionSplitterXWP() - TAsyncRASectionSplitterXWP")); }

				auto it1 = mse::make_strong_iterator(section_begin_it, m_access_lease_obj_shptr);
				auto ras_ar1 = mse::make_asyncsplitterrasectionreadwrite<strong_ra_iterator_t>(it1, section_size);
				m_ra_sections.push_back(ras_ar1);

				cummulative_size += section_size;
				section_begin_it += section_size;
			}
			if (m_access_lease_obj_shptr->cref()->size() > cummulative_size) {
				auto section_size = m_access_lease_obj_shptr->cref()->size() - cummulative_size;
				auto it1 = mse::make_strong_iterator(section_begin_it, m_access_lease_obj_shptr);
				auto ras_ar1 = mse::make_asyncsplitterrasectionreadwrite<strong_ra_iterator_t>(it1, section_size);
				m_ra_sections.push_back(ras_ar1);
			}
		}
		TAsyncRASectionSplitterXWP(exclusive_writelock_ptr_t&& exclusive_writelock_ptr, size_t split_index)
			: TAsyncRASectionSplitterXWP(std::forward<exclusive_writelock_ptr_t>(exclusive_writelock_ptr), std::array<size_t, 1>{ {split_index}}) {}
		ras_ar_t ra_section_access_requester(size_t index) const {
			return m_ra_sections.at(index);
		}
		ras_ar_t first_ra_section_access_requester() const {
			return m_ra_sections.at(0);
		}
		ras_ar_t second_ra_section_access_requester() const {
			return m_ra_sections.at(1);
		}
	private:
		std::shared_ptr<TSplitterAccessLeaseObj<exclusive_writelock_ptr_t> > m_access_lease_obj_shptr;
		std::vector<ras_ar_t> m_ra_sections;
	};

	template <typename _TAccessRequester>
	class TAsyncRASectionSplitter : public TAsyncRASectionSplitterXWP<decltype(std::declval<_TAccessRequester>().exclusive_writelock_ptr())> {
	public:
		typedef TAsyncRASectionSplitterXWP<decltype(std::declval<_TAccessRequester>().exclusive_writelock_ptr())> base_class;

		template<typename _TList>
		TAsyncRASectionSplitter(_TAccessRequester& ar, const _TList& section_sizes) : base_class(ar.exclusive_writelock_ptr(), section_sizes) {}

		TAsyncRASectionSplitter(_TAccessRequester& ar, size_t split_index) : base_class(ar.exclusive_writelock_ptr(), split_index) {}
	};


	/* TAsyncSharedReadWriteAccessRequester, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester,
	and TStdSharedImmutableFixedPointer are deprecated. */

	template<typename _Ty> class TAsyncSharedReadWriteAccessRequester;
	template<typename _Ty> class TAsyncSharedReadWritePointer;
	template<typename _Ty> class TAsyncSharedReadWriteConstPointer;
	template<typename _Ty> class TAsyncSharedExclusiveReadWritePointer;
	template<typename _Ty> class TAsyncSharedReadOnlyAccessRequester;
	template<typename _Ty> class TAsyncSharedReadOnlyConstPointer;

	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester;
	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer;

	/* TAsyncSharedObj is intended as a transparent wrapper for other classes/objects. */
	template<typename _TROy>
	class TAsyncSharedObj : public _TROy {
	public:
		MSE_ASYNC_USING(TAsyncSharedObj, _TROy);
		using _TROy::operator=;
		TAsyncSharedObj& operator=(TAsyncSharedObj&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		//TAsyncSharedObj& operator=(typename std::conditional<std::is_const<_TROy>::value, std::nullptr_t, TAsyncSharedObj>::type&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		TAsyncSharedObj& operator=(const TAsyncSharedObj& _X) { _TROy::operator=(_X); return (*this); }
		//TAsyncSharedObj& operator=(const typename std::conditional<std::is_const<_TROy>::value, std::nullptr_t, TAsyncSharedObj>::type& _X) { _TROy::operator=(_X); return (*this); }

	private:
		TAsyncSharedObj(const TAsyncSharedObj& _X) : _TROy(_X) {}
		TAsyncSharedObj(TAsyncSharedObj&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
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
		friend class TAsyncSharedExclusiveReadWritePointer<_TROy>;
		friend class TAsyncSharedReadOnlyAccessRequester<_TROy>;
		friend class TAsyncSharedReadOnlyConstPointer<_TROy>;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<_TROy>;
		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_TROy>;
	};


	template<typename _Ty> class TAsyncSharedReadWriteAccessRequester;
	template<typename _Ty> class TAsyncSharedReadWriteConstPointer;

	template<typename _Ty>
	class TAsyncSharedReadWritePointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedReadWritePointer(const TAsyncSharedReadWriteAccessRequester<_Ty>& src);
		TAsyncSharedReadWritePointer(const TAsyncSharedReadWritePointer& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}
		TAsyncSharedReadWritePointer(TAsyncSharedReadWritePointer&& src) = default; /* Note, the move constructor is only safe when std::move() is prohibited. */
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
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
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
	class TAsyncSharedReadWriteConstPointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
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
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
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
	class TAsyncSharedExclusiveReadWritePointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedExclusiveReadWritePointer(const TAsyncSharedExclusiveReadWritePointer& src) = delete;
		TAsyncSharedExclusiveReadWritePointer(TAsyncSharedExclusiveReadWritePointer&& src) = default;
		virtual ~TAsyncSharedExclusiveReadWritePointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedExclusiveReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedExclusiveReadWritePointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedExclusiveReadWritePointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
			}
		}
		TAsyncSharedExclusiveReadWritePointer<_Ty>& operator=(const TAsyncSharedExclusiveReadWritePointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedExclusiveReadWritePointer<_Ty>& operator=(TAsyncSharedExclusiveReadWritePointer<_Ty>&& _Right) = delete;

		TAsyncSharedExclusiveReadWritePointer<_Ty>* operator&() { return this; }
		const TAsyncSharedExclusiveReadWritePointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		unique_nonrecursive_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedReadWriteAccessRequester<_Ty>;
		//friend class TAsyncSharedReadWriteExclusiveConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedReadWriteAccessRequester {
	public:
		TAsyncSharedReadWriteAccessRequester(const TAsyncSharedReadWriteAccessRequester& src_cref) = default;

		TAsyncSharedReadWritePointer<_Ty> writelock_ptr() {
			return TAsyncSharedReadWritePointer<_Ty>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedReadWritePointer<_Ty>> try_writelock_ptr() {
			mse::mstd::optional<TAsyncSharedReadWritePointer<_Ty>> retval(TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedReadWritePointer<_Ty>> try_writelock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedReadWritePointer<_Ty>> retval(TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedReadWritePointer<_Ty>> try_writelock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedReadWritePointer<_Ty>> retval(TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		TAsyncSharedReadWriteConstPointer<_Ty> readlock_ptr() {
			return TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedReadWriteConstPointer<_Ty>> try_readlock_ptr() {
			mse::mstd::optional<TAsyncSharedReadWriteConstPointer<_Ty>> retval(TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedReadWriteConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedReadWriteConstPointer<_Ty>> retval(TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedReadWriteConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedReadWriteConstPointer<_Ty>> retval(TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		/* Note that an exclusive_writelock_ptr cannot coexist with any other lock_ptrs (targeting the same object), including ones in
		the same thread. Thus, using exclusive_writelock_ptrs without sufficient care introduces the potential for exceptions (in a way
		that sticking to (regular) writelock_ptrs doesn't). */
		TAsyncSharedExclusiveReadWritePointer<_Ty> exclusive_writelock_ptr() {
			return TAsyncSharedExclusiveReadWritePointer<_Ty>(m_shptr);
		}

		template <class... Args>
		static TAsyncSharedReadWriteAccessRequester make(Args&&... args) {
			return TAsyncSharedReadWriteAccessRequester(std::make_shared<TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...));
		}

	private:
		TAsyncSharedReadWriteAccessRequester(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedReadWriteAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedReadWriteAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;

		friend class TAsyncSharedReadOnlyAccessRequester<_Ty>;
		friend class TAsyncSharedReadWritePointer<_Ty>;
	};

	template <class X, class... Args>
	TAsyncSharedReadWriteAccessRequester<X> make_asyncsharedreadwrite(Args&&... args) {
		return TAsyncSharedReadWriteAccessRequester<X>::make(std::forward<Args>(args)...);
	}

	template<typename _Ty>
	TAsyncSharedReadWritePointer<_Ty>::TAsyncSharedReadWritePointer(const TAsyncSharedReadWriteAccessRequester<_Ty>& src) : m_shptr(src.m_shptr), m_unique_lock(src.m_shptr->m_mutex1) {}

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedReadWriteStore = TXScopeStrongNotNullPointerStore<TAsyncSharedReadWritePointer<_Ty> >;
	template<typename _Ty> using TXScopeAsyncSharedReadWriteConstStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedReadWriteConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedReadWriteStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedReadWritePointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedReadWriteStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeAsyncSharedReadWriteConstStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedReadWriteConstPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedReadWriteConstStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_


	template<typename _Ty>
	class TAsyncSharedReadOnlyConstPointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
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
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
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
		mse::mstd::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> try_readlock_ptr() {
			mse::mstd::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> retval(TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> retval(TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedReadOnlyConstPointer<_Ty>> retval(TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedReadOnlyAccessRequester make(Args&&... args) {
			return TAsyncSharedReadOnlyAccessRequester(std::make_shared<const TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...));
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

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedReadOnlyConstStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedReadOnlyConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedReadOnlyConstStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedReadOnlyConstPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedReadOnlyConstStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_


	template<typename _Ty> class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer;

	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
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
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
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
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
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
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
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
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer& src) = delete;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer&& src) = default;
		virtual ~TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer() {}

		operator bool() const {
			//assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			assert(is_valid()); //{ MSE_THROW(asyncshared_use_of_invalid_pointer_error("attempt to use invalid pointer - mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
			}
		}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>& operator=(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>& operator=(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>&& _Right) = delete;

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		unique_nonrecursive_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>;
		//friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteExclusiveConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester {
	public:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester& src_cref) = default;

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty> writelock_ptr() {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> try_writelock_ptr() {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> try_writelock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> try_writelock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty> readlock_ptr() {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr);
		}
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> try_readlock_ptr() {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		/* Note that an exclusive_writelock_ptr cannot coexist with any other lock_ptrs (targeting the same object), including ones in
		the same thread. Thus, using exclusive_writelock_ptrs without sufficient care introduces the potential for exceptions (in a way
		that sticking to (regular) writelock_ptrs doesn't). */
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty> exclusive_writelock_ptr() {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesExclusiveReadWritePointer<_Ty>(m_shptr);
		}

		template <class... Args>
		static TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester make(Args&&... args) {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester(std::make_shared<TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...));
		}

	private:
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;

		friend class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<_Ty>;
	};

	template <class X, class... Args>
	TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<X> make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadwrite(Args&&... args) {
		return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<X>::make(std::forward<Args>(args)...);
	}

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteStore = TXScopeStrongNotNullPointerStore<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty> >;
	template<typename _Ty> using TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWritePointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteConstStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_


	template<typename _Ty>
	class TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer : public AsyncSharedStrongPointerNotAsyncShareableTagBase {
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
				m_shptr = nullptr;
			}
		}
		template<class _Rep, class _Period>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::duration<_Rep, _Period>& _Rel_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_for(_Rel_time)) {
				m_shptr = nullptr;
			}
		}
		template<class _Clock, class _Duration>
		TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t, const std::chrono::time_point<_Clock, _Duration>& _Abs_time) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock_until(_Abs_time)) {
				m_shptr = nullptr;
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
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> try_readlock_ptr() {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Rep, class _Period>
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> try_readlock_ptr_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Rel_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}
		template<class _Clock, class _Duration>
		mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> try_readlock_ptr_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) {
			mse::mstd::optional<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>> retval(TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock, _Abs_time));
			if (!((*retval).is_valid())) {
				return{};
			}
			return retval;
		}

		template <class... Args>
		static TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester make(Args&&... args) {
			return TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester(std::make_shared<const TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...));
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

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstStore = TXScopeStrongNotNullConstPointerStore<TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstStore<_Ty> make_xscope_strong_pointer_store(const TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstPointer<_Ty>& stored_ptr) {
		return TXScopeAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyConstStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_

	/* For "read-only" situations when you need, or want, the shared object to be managed by std::shared_ptrs we provide a
	slightly safety enhanced std::shared_ptr wrapper. The wrapper enforces "const"ness and tries to ensure that it always
	points to a validly allocated object. Use mse::make_stdsharedimmutable<>() to construct an
	mse::TStdSharedImmutableFixedPointer. And again, beware of sharing objects with mutable members. */
	template<typename _Ty>
	class TStdSharedImmutableFixedPointer : public std::shared_ptr<const _Ty>, public AsyncSharedStrongPointerNotAsyncShareableTagBase {
	public:
		TStdSharedImmutableFixedPointer(const TStdSharedImmutableFixedPointer& src_cref) : std::shared_ptr<const _Ty>(src_cref) {}
		virtual ~TStdSharedImmutableFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::shared_ptr<const _Ty>::operator _Ty*(); }

		template <class... Args>
		static TStdSharedImmutableFixedPointer make(Args&&... args) {
			return TStdSharedImmutableFixedPointer(std::make_shared<const _Ty>(std::forward<Args>(args)...));
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

#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeStdSharedImmutableFixedStore = TXScopeStrongNotNullConstPointerStore<TStdSharedImmutableFixedPointer<_Ty> >;

	template<typename _Ty>
	TXScopeStdSharedImmutableFixedStore<_Ty> make_xscope_strong_pointer_store(const TStdSharedImmutableFixedPointer<_Ty>& stored_ptr) {
		return TXScopeStdSharedImmutableFixedStore<_Ty>(stored_ptr);
	}
#endif // MSESCOPE_H_


	/* Legacy aliases. */
	template<typename _Ty> using TReadOnlyStdSharedFixedConstPointer = TStdSharedImmutableFixedPointer<_Ty>;
	template <class X, class... Args>
	TReadOnlyStdSharedFixedConstPointer<X> make_readonlystdshared(Args&&... args) {
		return TStdSharedImmutableFixedPointer<X>::make(std::forward<Args>(args)...);
	}


#if defined(MSEPOINTERBASICS_H)
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
#endif // defined(MSEPOINTERBASICS_H)

	/*
	static void s_ashptr_test1() {
#ifdef MSE_SELF_TESTS
#endif // MSE_SELF_TESTS
	}
	*/
}

#undef MSE_THROW

#endif // MSEASYNCSHARED_H_
