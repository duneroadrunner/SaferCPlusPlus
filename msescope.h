
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESCOPE_H_
#define MSESCOPE_H_

#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_SCOPEPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef NDEBUG
#define MSE_SCOPEPOINTER_DISABLED
#endif // NDEBUG


namespace mse {

	static const int sc_scp_default_cache_size = 3/* 1 + (the maximum number of pointers expected to target the object at one time) */;

#ifdef MSE_SCOPEPOINTER_DISABLED
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopePointer = _Ty*;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopeConstPointer = const _Ty*;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopeNotNullPointer = _Ty*;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopeNotNullConstPointer = const _Ty*;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopeFixedPointer = _Ty*;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopeFixedConstPointer = const _Ty*;
	template<typename _TROy, int _Tn = sc_scp_default_cache_size> using TScopeObj = _TROy;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using TScopeOwnerPointer = std::unique_ptr<TScopeObj<_Ty, _Tn>>;

	template <class X, class... Args>
	TScopeOwnerPointer<X> make_scope(Args&&... args) {
		return std::make_unique<TScopeObj<X>>(args...);
	}

#else /*MSE_SCOPEPOINTER_DISABLED*/

	/* TScpPTracker is intended to keep track of all the pointers pointing to an object. TScpPTracker objects are intended to be always
	associated with (infact, a member of) the one object that is the target of the pointers it tracks. Though at the moment, it
	doesn't need to actually know which object it is associated with. */
	template<int _Tn = sc_scp_default_cache_size>
	class TScpPTracker {
	public:
		TScpPTracker() {}
		TScpPTracker(const TScpPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		TScpPTracker(TScpPTracker&& src) { /* see above */ }
		~TScpPTracker() {
			if (!fast_mode1()) {
				delete m_ptr_to_regptr_set_ptr;
			}
		}
		TScpPTracker& operator=(const TScpPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		TScpPTracker& operator=(TScpPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const TScpPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true;
		}
		bool operator!=(const TScpPTracker& _Right_cref) const { /* see above */ return false; }

		void registerPointer(const CSaferPtrBase& sp_ref) {
			if (!fast_mode1()) {
				std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
				(*m_ptr_to_regptr_set_ptr).insert(item);
			}
			else {
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (sc_fm1_max_pointers == m_fm1_num_pointers) {
					/* Too many pointers. Initiate and switch to slow mode. */
					/* Initialize slow storage. */
					m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
					/* First copy the pointers from fast storage to slow storage. */
					for (int i = 0; i < sc_fm1_max_pointers; i += 1) {
						std::unordered_set<const CSaferPtrBase*>::value_type item(m_fm1_ptr_to_regptr_array[i]);
						(*m_ptr_to_regptr_set_ptr).insert(item);
					}
					/* Add the new pointer to slow storage. */
					std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
					(*m_ptr_to_regptr_set_ptr).insert(item);
				}
				else {
					if (1 == sc_fm1_max_pointers) {
						m_fm1_ptr_to_regptr_array[0] = (&sp_ref);
						m_fm1_num_pointers = 1;
					}
					else if (2 == sc_fm1_max_pointers) {
						if (1 == m_fm1_num_pointers) {
							m_fm1_ptr_to_regptr_array[1] = (&sp_ref);
							m_fm1_num_pointers = 2;
						}
						else {
							assert(0 == m_fm1_num_pointers);
							m_fm1_ptr_to_regptr_array[0] = (&sp_ref);
							m_fm1_num_pointers = 1;
						}
					}
					else {
						m_fm1_ptr_to_regptr_array[m_fm1_num_pointers] = (&sp_ref);
						m_fm1_num_pointers += 1;
					}
				}
			}
		}
		void unregisterPointer(const CSaferPtrBase& sp_ref) {
			if (!fast_mode1()) {
				auto res = (*m_ptr_to_regptr_set_ptr).erase(&sp_ref);
				assert(0 != res);
			}
			else {
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (1 == sc_fm1_max_pointers) {
					if (1 == m_fm1_num_pointers) {
						m_fm1_num_pointers = 0;
					}
					else { /* There are no scope pointers to be unscope. */ assert(false); }
				}
				else if (2 == sc_fm1_max_pointers){
					if (1 == m_fm1_num_pointers) {
						m_fm1_num_pointers = 0;
					}
					else if (2 == m_fm1_num_pointers) {
						if ((&sp_ref) == m_fm1_ptr_to_regptr_array[1]) {
							m_fm1_num_pointers = 1;
						}
						else {
							assert((&sp_ref) == m_fm1_ptr_to_regptr_array[0]);
							m_fm1_ptr_to_regptr_array[0] = m_fm1_ptr_to_regptr_array[1];
							m_fm1_num_pointers = 1;
						}
					}
					else { /* There are no scope pointers to be unscope. */ assert(false); }
				}
				else {
					int found_index = -1;
					for (int i = 0; i < m_fm1_num_pointers; i += 1) {
						if ((&sp_ref) == m_fm1_ptr_to_regptr_array[i]) {
							found_index = i;
							break;
						}
					}
					if (0 <= found_index) {
						m_fm1_num_pointers -= 1;
						assert(0 <= m_fm1_num_pointers);
						for (int j = found_index; j < m_fm1_num_pointers; j += 1) {
							m_fm1_ptr_to_regptr_array[j] = m_fm1_ptr_to_regptr_array[j + 1];
						}
					}
					else { assert(false); }
				}
			}
		}
		void onObjectDestruction() {
			if (!fast_mode1()) {
				for (auto sp_ref_ptr : (*m_ptr_to_regptr_set_ptr)) {
					(*sp_ref_ptr).setToNull();
				}
			}
			else {
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (1 == sc_fm1_max_pointers) {
					if (0 == m_fm1_num_pointers) {
					}
					else {
						assert(1 == m_fm1_num_pointers);
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						m_fm1_num_pointers = 0;
					}
				}
				else if (2 == sc_fm1_max_pointers) {
					if (0 == m_fm1_num_pointers) {
					}
					else if (1 == m_fm1_num_pointers) {
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						m_fm1_num_pointers = 0;
					}
					else {
						assert(2 == m_fm1_num_pointers);
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						(*(m_fm1_ptr_to_regptr_array[1])).setToNull();
						m_fm1_num_pointers = 0;
					}
				}
				else {
					for (int i = 0; i < m_fm1_num_pointers; i += 1) {
						(*(m_fm1_ptr_to_regptr_array[i])).setToNull();
					}
					m_fm1_num_pointers = 0;
				}
			}
		}

		bool fast_mode1() const { return (nullptr == m_ptr_to_regptr_set_ptr); }
		int m_fm1_num_pointers = 0;
		static const int sc_fm1_max_pointers = _Tn;
		const CSaferPtrBase* m_fm1_ptr_to_regptr_array[sc_fm1_max_pointers];

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;
	};

	template<typename _Ty, int _Tn = sc_scp_default_cache_size> class TScopeObj;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> class TScopeNotNullPointer;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> class TScopeNotNullConstPointer;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> class TScopeFixedPointer;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> class TScopeFixedConstPointer;

	/* Use TScopeFixedPointer instead. */
	template<typename _Ty, int _Tn = sc_scp_default_cache_size>
	class TScopePointer : public TSaferPtr<TScopeObj<_Ty, _Tn>> {
	public:
	private:
		TScopePointer();
		TScopePointer(TScopeObj<_Ty, _Tn>* ptr);
		TScopePointer(const TScopePointer& src_cref);
		virtual ~TScopePointer();
		TScopePointer<_Ty, _Tn>& operator=(TScopeObj<_Ty, _Tn>* ptr);
		TScopePointer<_Ty, _Tn>& operator=(const TScopePointer<_Ty, _Tn>& _Right_cref);
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const;
		explicit operator TScopeObj<_Ty, _Tn>*() const;

		TScopePointer<_Ty, _Tn>* operator&() { return this; }
		const TScopePointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TScopeNotNullPointer<_Ty, _Tn>;
	};

	/* Use TScopeFixedConstPointer instead. */
	template<typename _Ty, int _Tn = sc_scp_default_cache_size>
	class TScopeConstPointer : public TSaferPtr<const TScopeObj<_Ty, _Tn>> {
	public:
	private:
		TScopeConstPointer();
		TScopeConstPointer(const TScopeObj<_Ty, _Tn>* ptr);
		TScopeConstPointer(const TScopeConstPointer& src_cref);
		TScopeConstPointer(const TScopePointer<_Ty, _Tn>& src_cref);
		virtual ~TScopeConstPointer();
		TScopeConstPointer<_Ty, _Tn>& operator=(const TScopeObj<_Ty, _Tn>* ptr);
		TScopeConstPointer<_Ty, _Tn>& operator=(const TScopeConstPointer<_Ty, _Tn>& _Right_cref);
		TScopeConstPointer<_Ty, _Tn>& operator=(const TScopePointer<_Ty, _Tn>& _Right_cref) { return (*this).operator=(TScopeConstPointer(_Right_cref)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const;
		explicit operator const TScopeObj<_Ty, _Tn>*() const;

		TScopeConstPointer<_Ty, _Tn>* operator&() { return this; }
		const TScopeConstPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TScopeNotNullConstPointer<_Ty, _Tn>;
	};

	/* Use TScopeFixedPointer instead. */
	template<typename _Ty, int _Tn>
	class TScopeNotNullPointer : public TScopePointer<_Ty, _Tn> {
	public:
	private:
		TScopeNotNullPointer(TScopeObj<_Ty, _Tn>* ptr) : TScopePointer<_Ty, _Tn>(ptr) {}
		TScopeNotNullPointer(const TScopeNotNullPointer& src_cref) : TScopePointer<_Ty, _Tn>(src_cref) {}
		virtual ~TScopeNotNullPointer() {}
		TScopeNotNullPointer<_Ty, _Tn>& operator=(const TScopeNotNullPointer<_Ty, _Tn>& _Right_cref) {
			TScopePointer<_Ty, _Tn>::operator=(_Right_cref);
			return (*this);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TScopePointer<_Ty, _Tn>::operator _Ty*(); }
		explicit operator TScopeObj<_Ty, _Tn>*() const { return TScopePointer<_Ty, _Tn>::operator TScopeObj<_Ty, _Tn>*(); }

		TScopeNotNullPointer<_Ty, _Tn>* operator&() { return this; }
		const TScopeNotNullPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TScopeFixedPointer<_Ty, _Tn>;
	};

	/* Use TScopeFixedConstPointer instead. */
	template<typename _Ty, int _Tn>
	class TScopeNotNullConstPointer : public TScopeConstPointer<_Ty, _Tn> {
	public:
	private:
		TScopeNotNullConstPointer(const TScopeNotNullConstPointer<_Ty, _Tn>& src_cref) : TScopeConstPointer<_Ty, _Tn>(src_cref) {}
		TScopeNotNullConstPointer(const TScopeNotNullPointer<_Ty, _Tn>& src_cref) : TScopeConstPointer<_Ty, _Tn>(src_cref) {}
		virtual ~TScopeNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TScopeConstPointer<_Ty, _Tn>::operator const _Ty*(); }
		explicit operator const TScopeObj<_Ty, _Tn>*() const { return TScopeConstPointer<_Ty, _Tn>::operator const TScopeObj<_Ty, _Tn>*(); }
		TScopeNotNullConstPointer(const TScopeObj<_Ty, _Tn>* ptr) : TScopeConstPointer<_Ty, _Tn>(ptr) {}

		TScopeNotNullConstPointer<_Ty, _Tn>* operator&() { return this; }
		const TScopeNotNullConstPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TScopeFixedConstPointer<_Ty, _Tn>;
	};

	/* A TScopeFixedPointer points to a TScopeObj. Its intended for very limited use. Basically just to pass a TScopeObj
	by reference as a function parameter. TScopeFixedPointers are be obtained from TScopeObj's "&" (address of) operator. */
	template<typename _Ty, int _Tn>
	class TScopeFixedPointer : public TScopeNotNullPointer<_Ty, _Tn> {
	public:
		TScopeFixedPointer(const TScopeFixedPointer& src_cref) : TScopeNotNullPointer<_Ty, _Tn>(src_cref) {}
		virtual ~TScopeFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TScopeNotNullPointer<_Ty, _Tn>::operator _Ty*(); }
		explicit operator TScopeObj<_Ty, _Tn>*() const { return TScopeNotNullPointer<_Ty, _Tn>::operator TScopeObj<_Ty, _Tn>*(); }

	private:
		TScopeFixedPointer(TScopeObj<_Ty, _Tn>* ptr) : TScopeNotNullPointer<_Ty, _Tn>(ptr) {}
		TScopeFixedPointer<_Ty, _Tn>& operator=(const TScopeFixedPointer<_Ty, _Tn>& _Right_cref) = delete;

		TScopeFixedPointer<_Ty, _Tn>* operator&() { return this; }
		const TScopeFixedPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TScopeObj<_Ty, _Tn>;
	};

	template<typename _Ty, int _Tn>
	class TScopeFixedConstPointer : public TScopeNotNullConstPointer<_Ty, _Tn> {
	public:
		TScopeFixedConstPointer(const TScopeFixedPointer<_Ty, _Tn>& src_cref) : TScopeNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		virtual ~TScopeFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TScopeNotNullConstPointer<_Ty, _Tn>::operator const _Ty*(); }
		explicit operator const TScopeObj<_Ty, _Tn>*() const { return TScopeNotNullConstPointer<_Ty, _Tn>::operator const TScopeObj<_Ty, _Tn>*(); }

	private:
		TScopeFixedConstPointer(const TScopeObj<_Ty, _Tn>* ptr) : TScopeNotNullConstPointer<_Ty, _Tn>(ptr) {}
		TScopeFixedConstPointer<_Ty, _Tn>& operator=(const TScopeFixedConstPointer<_Ty, _Tn>& _Right_cref) = delete;

		TScopeFixedConstPointer<_Ty, _Tn>* operator&() { return this; }
		const TScopeFixedConstPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TScopeObj<_Ty, _Tn>;
	};

	/* This macro roughly simulates constructor inheritance. Originally it was used when some compilers didn't support
	constructor inheritance, but now we use it because of it's differences with standard constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base)                                 \
    template<typename ...Args,                               \
             typename = typename std::enable_if              \
             <                                               \
                std::is_constructible<Base, Args...>::value  \
			 			 			 			              >::type>                                        \
    Derived(Args &...args)                                  \
        : Base(std::forward<Args>(args)...) { }              \

	/* TScopeObj is intended as a transparent wrapper for other classes/objects with "scope lifespans". That is, objects
	that are either allocated on the stack, or constructed using mse::make_scope(). */
	template<typename _TROy, int _Tn>
	class TScopeObj : public _TROy {
	public:
		MSE_SCOPE_USING(TScopeObj, _TROy);
		//TScopeObj(const TScopeObj& _X) : _TROy(_X) {}
		//TScopeObj(TScopeObj&& _X) : _TROy(std::move(_X)) {}
		virtual ~TScopeObj() {
			mseRPManager().onObjectDestruction();
		}
		TScopeObj& operator=(const TScopeObj& _X) { _TROy::operator=(_X); return (*this); }
		TScopeFixedPointer<_TROy, _Tn> operator&() {
			return this;
		}
		TScopeFixedConstPointer<_TROy, _Tn> operator&() const {
			return this;
		}
		TScpPTracker<_Tn>& mseRPManager() const { return m_mseRPManager; }

	private:
		TScopeObj& operator=(TScopeObj&& _X) = delete;
		mutable TScpPTracker<_Tn> m_mseRPManager;
	};

	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>::TScopePointer() : TSaferPtr<TScopeObj<_Ty, _Tn>>() {}
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>::TScopePointer(TScopeObj<_Ty, _Tn>* ptr) : TSaferPtr<TScopeObj<_Ty, _Tn>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>::TScopePointer(const TScopePointer& src_cref) : TSaferPtr<TScopeObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>::~TScopePointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>& TScopePointer<_Ty, _Tn>::operator=(TScopeObj<_Ty, _Tn>* ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
		TSaferPtr<TScopeObj<_Ty, _Tn>>::operator=(ptr);
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
		return (*this);
	}
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>& TScopePointer<_Ty, _Tn>::operator=(const TScopePointer<_Ty, _Tn>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>::operator _Ty*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}
	/* This cast operator, if possible, should not be used. It is meant to be used exclusively by scope_delete<>(). */
	template<typename _Ty, int _Tn>
	TScopePointer<_Ty, _Tn>::operator TScopeObj<_Ty, _Tn>*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}


	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::TScopeConstPointer() : TSaferPtr<const TScopeObj<_Ty, _Tn>>() {}
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::TScopeConstPointer(const TScopeObj<_Ty, _Tn>* ptr) : TSaferPtr<const TScopeObj<_Ty, _Tn>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::TScopeConstPointer(const TScopeConstPointer& src_cref) : TSaferPtr<const TScopeObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::TScopeConstPointer(const TScopePointer<_Ty, _Tn>& src_cref) : TSaferPtr<const TScopeObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::~TScopeConstPointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>& TScopeConstPointer<_Ty, _Tn>::operator=(const TScopeObj<_Ty, _Tn>* ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
		TSaferPtr<const TScopeObj<_Ty, _Tn>>::operator=(ptr);
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
		return (*this);
	}
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>& TScopeConstPointer<_Ty, _Tn>::operator=(const TScopeConstPointer<_Ty, _Tn>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::operator const _Ty*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}
	/* This cast operator, if possible, should not be used. It is meant to be used exclusively by scope_delete<>(). */
	template<typename _Ty, int _Tn>
	TScopeConstPointer<_Ty, _Tn>::operator const TScopeObj<_Ty, _Tn>*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}

	template<typename _Ty, int _Tn = sc_scp_default_cache_size>
	class TScopeOwnerPointer {
	public:
		virtual ~TScopeOwnerPointer() {
			assert(m_ptr);
			delete m_ptr;
		}

		TScopeObj<_Ty, _Tn>& operator*() const {
			return (*m_ptr);
		}
		TScopeObj<_Ty, _Tn>* operator->() const {
			return m_ptr;
		}

		template <class... Args>
		static TScopeOwnerPointer make_scope(Args&&... args) {
			auto new_ptr = new TScopeObj<_Ty, _Tn>(args...);
			TScopeOwnerPointer retval(new_ptr);
			return retval;
		}

	private:
		TScopeOwnerPointer(TScopeObj<_Ty, _Tn>* ptr) : m_ptr(ptr) {}
		TScopeOwnerPointer<_Ty, _Tn>& operator=(const TScopeOwnerPointer<_Ty, _Tn>& _Right_cref) = delete;

		TScopeObj<_Ty, _Tn>* m_ptr = nullptr;
	};

	template <class X, class... Args>
	TScopeOwnerPointer<X> make_scope(Args&&... args) {
		return TScopeOwnerPointer<X>::make_scope(args...);
	}

#endif /*MSE_SCOPEPOINTER_DISABLED*/

	/* shorter aliases */
	//template<typename _Ty, int _Tn = sc_scp_default_cache_size> using scpp = TScopePointer<_Ty, _Tn>;
	//template<typename _Ty, int _Tn = sc_scp_default_cache_size> using scpcp = TScopeConstPointer<_Ty, _Tn>;
	//template<typename _Ty, int _Tn = sc_scp_default_cache_size> using scpnnp = TScopeNotNullPointer<_Ty, _Tn>;
	//template<typename _Ty, int _Tn = sc_scp_default_cache_size> using scpnncp = TScopeNotNullConstPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using scpfp = TScopeFixedPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_scp_default_cache_size> using scpfcp = TScopeFixedConstPointer<_Ty, _Tn>;
	template<typename _TROy, int _Tn = sc_scp_default_cache_size> using scpo = TScopeObj<_TROy, _Tn>;


	static void s_scpptr_test1() {
		class A {
		public:
			A() {}
			A(const A& _X) : b(_X.b) {}
			A(A&& _X) : b(std::move(_X.b)) {}
			virtual ~A() {}
			A& operator=(A&& _X) { b = std::move(_X.b); return (*this); }
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TScopeFixedPointer<A> A_scope_ptr) { return A_scope_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;

		{
			A a;
			mse::TScopeObj<A> scope_a;
			/* mse::TScopeObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == scope_a.b);
			A_native_ptr = &a;
			mse::TScopeFixedPointer<A> A_scope_ptr1 = &scope_a;
			assert(A_native_ptr->b == A_scope_ptr1->b);

			mse::TScopeFixedPointer<A> A_scope_ptr2 = &scope_a;

			/* mse::TScopeFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_scope_ptr1);

			if (A_scope_ptr2) {
			}
			else if (A_scope_ptr2 != A_scope_ptr1) {
				int q = B::foo2(A_scope_ptr2);
			}

			A a2 = a;
			mse::TScopeObj<A> scope_a2 = scope_a;

			mse::TScopeFixedConstPointer<A> rcp = A_scope_ptr1;
			mse::TScopeFixedConstPointer<A> rcp2 = rcp;
			const mse::TScopeObj<A> cscope_a;
			mse::TScopeFixedConstPointer<A> rfcp = &cscope_a;

			auto A_scpoptr = mse::make_scope<A>();
			B::foo2(&*A_scpoptr);
			if (A_scpoptr->b == (&*A_scpoptr)->b) {
			}
		}
	}
}

#endif // MSESCOPE_H_
