
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEASYNCSHARED_H_
#define MSEASYNCSHARED_H_

#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <cassert>


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_ASYNCSHAREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

namespace mse {

#ifdef MSE_ASYNCSHAREDPOINTER_DISABLED
#else /*MSE_ASYNCSHAREDPOINTER_DISABLED*/
#endif /*MSE_ASYNCSHAREDPOINTER_DISABLED*/

	class recursive_shared_timed_mutex : private std::shared_timed_mutex {
	public:
		typedef std::shared_timed_mutex base_class;

		void lock()
		{	// lock exclusive
			m_write_mutex.lock();

			if ((1 <= m_writelock_count) && (std::this_thread::get_id() == m_writelock_thread_id)) {
			}
			else {
				assert((std::this_thread::get_id() != m_writelock_thread_id) || (0 == m_writelock_count));

				m_write_mutex.unlock();
				base_class::lock();
				m_write_mutex.lock();

				m_writelock_thread_id = std::this_thread::get_id();
				assert(0 == m_writelock_count);
			}
			m_writelock_count += 1;

			m_write_mutex.unlock();
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
		{	// lock exclusive
			m_read_mutex.lock();

			const auto this_thread_id = std::this_thread::get_id();
			const auto found_it = m_thread_id_readlock_count_map.find(this_thread_id);
			if ((m_thread_id_readlock_count_map.end() != found_it) && (1 <= (*found_it).second)) {
				(*found_it).second += 1;
			}
			else {
				assert((m_thread_id_readlock_count_map.end() == found_it) || (0 == (*found_it).second));

				m_read_mutex.unlock();
				base_class::lock_shared();
				m_read_mutex.lock();

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

			m_read_mutex.unlock();
		}
		bool try_lock_shared()
		{	// try to lock exclusive
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
					if (m_thread_id_readlock_count_map.end() != found_it) {
						assert(0 <= (*found_it).second);
						(*found_it).second += 1;
					}
					else {
						std::unordered_map<std::thread::id, int>::value_type item(this_thread_id, 1);
						m_thread_id_readlock_count_map.insert(item);
					}
				}
			}
			return retval;
		}
		void unlock_shared()
		{	// unlock exclusive
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

	template<typename _Ty> class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester;
	template<typename _Ty> class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer;
	template<typename _Ty> class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer;
	template<typename _Ty> class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester;
	template<typename _Ty> class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer;

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

		mutable async_shared_timed_mutex_type m_mutex1;

		friend class TAsyncSharedReadWriteAccessRequester<_TROy>;
		friend class TAsyncSharedReadWritePointer<_TROy>;
		friend class TAsyncSharedReadWriteConstPointer<_TROy>;
		friend class TAsyncSharedReadOnlyAccessRequester<_TROy>;
		friend class TAsyncSharedReadOnlyConstPointer<_TROy>;

		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<_TROy>;
		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_TROy>;
		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_TROy>;
		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester<_TROy>;
		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_TROy>;
	};

	template<typename _Ty>
	class TAsyncSharedReadWritePointer {
	public:
		TAsyncSharedReadWritePointer(TAsyncSharedReadWritePointer&& src) = default;
		virtual ~TAsyncSharedReadWritePointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadWritePointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadWritePointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		TAsyncSharedReadWritePointer<_Ty>& operator=(const TAsyncSharedReadWritePointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedReadWritePointer<_Ty>& operator=(TAsyncSharedReadWritePointer<_Ty>&& _Right) = delete;

		TAsyncSharedReadWritePointer<_Ty>* operator&() { return this; }
		const TAsyncSharedReadWritePointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedReadWriteAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedReadWriteConstPointer {
	public:
		TAsyncSharedReadWriteConstPointer(TAsyncSharedReadWriteConstPointer&& src) = default;
		virtual ~TAsyncSharedReadWriteConstPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadWriteConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadWriteConstPointer")); }
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
		TAsyncSharedReadWriteConstPointer<_Ty>& operator=(const TAsyncSharedReadWriteConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedReadWriteConstPointer<_Ty>& operator=(TAsyncSharedReadWriteConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedReadWriteConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedReadWriteConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
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

		TAsyncSharedReadWritePointer<_Ty> ptr() {
			return TAsyncSharedReadWritePointer<_Ty>(m_shptr);
		}
		TAsyncSharedReadWritePointer<_Ty> try_ptr() {
			return TAsyncSharedReadWritePointer<_Ty>(m_shptr, std::try_to_lock);
		}
		TAsyncSharedReadWriteConstPointer<_Ty> const_ptr() {
			return TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr);
		}
		TAsyncSharedReadWriteConstPointer<_Ty> try_const_ptr() {
			return TAsyncSharedReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock);
		}

		template <class... Args>
		static TAsyncSharedReadWriteAccessRequester make_asyncsharedreadwrite(Args&&... args) {
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
		return TAsyncSharedReadWriteAccessRequester<X>::make_asyncsharedreadwrite(std::forward<Args>(args)...);
	}


	template<typename _Ty>
	class TAsyncSharedReadOnlyConstPointer {
	public:
		TAsyncSharedReadOnlyConstPointer(TAsyncSharedReadOnlyConstPointer&& src) = default;
		virtual ~TAsyncSharedReadOnlyConstPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadOnlyConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedReadOnlyConstPointer")); }
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
		TAsyncSharedReadOnlyConstPointer<_Ty>& operator=(const TAsyncSharedReadOnlyConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedReadOnlyConstPointer<_Ty>& operator=(TAsyncSharedReadOnlyConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedReadOnlyConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedReadOnlyConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedReadOnlyAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedReadOnlyAccessRequester : public TSaferPtr<const TAsyncSharedObj<_Ty>> {
	public:
		TAsyncSharedReadOnlyAccessRequester(const TAsyncSharedReadOnlyAccessRequester& src_cref) = default;
		TAsyncSharedReadOnlyAccessRequester(const TAsyncSharedReadWriteAccessRequester<_Ty>& src_cref) : m_shptr(src_cref.m_shptr) {}

		TAsyncSharedReadOnlyConstPointer<_Ty> const_ptr() {
			return TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr);
		}
		TAsyncSharedReadOnlyConstPointer<_Ty> try_const_ptr() {
			return TAsyncSharedReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock);
		}

		template <class... Args>
		static TAsyncSharedReadOnlyAccessRequester make_asyncsharedreadonly(Args&&... args) {
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
		return TAsyncSharedReadOnlyAccessRequester<X>::make_asyncsharedreadonly(std::forward<Args>(args)...);
	}


	template<typename _Ty>
	class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer {
	public:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer(TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer&& src) = default;
		virtual ~TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer")); }
			return m_shptr.operator bool();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>&, TAsyncSharedObj<_Ty>&>::type operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer")); }
			return (*m_shptr);
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TAsyncSharedObj<_Ty>*, TAsyncSharedObj<_Ty>*>::type operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer")); }
			return std::addressof(*m_shptr);
		}
	private:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1) {}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_unique_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_unique_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>& operator=(const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>& operator=(TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>&& _Right) = delete;

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>* operator&() { return this; }
		const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::unique_lock<async_shared_timed_mutex_type> m_unique_lock;

		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer {
	public:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer(TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer&& src) = default;
		virtual ~TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return extra_const_ptr;
		}
	private:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1) {}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>& operator=(const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>& operator=(TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
		std::shared_lock<async_shared_timed_mutex_type> m_shared_lock;

		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester {
	public:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester(const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester& src_cref) = default;

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty> ptr() {
			return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>(m_shptr);
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty> try_ptr() {
			return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWritePointer<_Ty>(m_shptr, std::try_to_lock);
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty> const_ptr() {
			return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>(m_shptr);
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty> try_const_ptr() {
			return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteConstPointer<_Ty>(m_shptr, std::try_to_lock);
		}

		template <class... Args>
		static TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester make_asyncsharedsimpleobjectyouaresurehasnomutablemembersreadwrite(Args&&... args) {
			//auto shptr = std::make_shared<TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<TAsyncSharedObj<_Ty>> shptr(new TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<X> make_asyncsharedsimpleobjectyouaresurehasnomutablemembersreadwrite(Args&&... args) {
		return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<X>::make_asyncsharedsimpleobjectyouaresurehasnomutablemembersreadwrite(std::forward<Args>(args)...);
	}


	template<typename _Ty>
	class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer {
	public:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer(TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer&& src) = default;
		virtual ~TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer() {}

		operator bool() const {
			//if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer")); }
			return m_shptr.operator bool();
		}
		const TAsyncSharedObj<const _Ty>& operator*() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return (*extra_const_ptr);
		}
		const TAsyncSharedObj<const _Ty>* operator->() const {
			if (!is_valid()) { throw(std::out_of_range("attempt to use invalid pointer - mse::TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer")); }
			const TAsyncSharedObj<const _Ty>* extra_const_ptr = reinterpret_cast<const TAsyncSharedObj<const _Ty>*>(std::addressof(*m_shptr));
			return extra_const_ptr;
		}
	private:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1) {}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer(std::shared_ptr<TAsyncSharedObj<_Ty>> shptr, std::try_to_lock_t) : m_shptr(shptr), m_shared_lock(shptr->m_mutex1, std::defer_lock) {
			if (!m_shared_lock.try_lock()) {
				shptr = nullptr;
			}
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>& operator=(const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>& _Right_cref) = delete;
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>& operator=(TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>&& _Right) = delete;

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>* operator&() { return this; }
		const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>* operator&() const { return this; }
		bool is_valid() const {
			/* A false return value indicates misuse. This might return false if this object has been invalidated
			by a move construction. */
			bool retval = m_shptr.operator bool();
			return retval;
		}

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
		std::shared_lock<async_shared_timed_mutex_type> m_shared_lock;

		friend class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester<_Ty>;
	};

	template<typename _Ty>
	class TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester {
	public:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester(const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester& src_cref) = default;
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester(const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadWriteAccessRequester<_Ty>& src_cref) : m_shptr(src_cref.m_shptr) {}

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty> const_ptr() {
			return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>(m_shptr);
		}
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty> try_const_ptr() {
			return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyConstPointer<_Ty>(m_shptr, std::try_to_lock);
		}

		template <class... Args>
		static TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester make_asyncsharedsimpleobjectyouaresurehasnomutablemembersreadonly(Args&&... args) {
			//auto shptr = std::make_shared<const TAsyncSharedObj<_Ty>>(std::forward<Args>(args)...);
			std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr(new const TAsyncSharedObj<_Ty>(std::forward<Args>(args)...));
			TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester retval(shptr);
			return retval;
		}

	private:
		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester(std::shared_ptr<const TAsyncSharedObj<_Ty>> shptr) : m_shptr(shptr) {}

		TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester<_Ty>* operator&() { return this; }
		const TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester<_Ty>* operator&() const { return this; }

		std::shared_ptr<const TAsyncSharedObj<_Ty>> m_shptr;
	};

	template <class X, class... Args>
	TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester<X> make_asyncsharedsimpleobjectyouaresurehasnomutablemembersreadonly(Args&&... args) {
		return TAsyncSharedSimpleObjectYouAreSureHasNoMutableMembersReadOnlyAccessRequester<X>::make_asyncsharedsimpleobjectyouaresurehasnomutablemembersreadonly(std::forward<Args>(args)...);
	}


	static void s_ashptr_test1() {
	}
}

#endif // MSEASYNCSHARED_H_
