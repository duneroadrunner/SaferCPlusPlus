#ifndef MSEREGISTEREDFORLEGACY_H_
#define MSEREGISTEREDFORLEGACY_H_

#include "mseprimitives.h"
#include <unordered_map>
#include <mutex>

#ifdef _MSC_VER
/* Jan 2016: For some reason std::this_thread::get_id() seems to be really slow in windows. So we're replacing it with the
native GetCurrentThreadId(). */
#define MSE_THREAD_ID_TYPE unsigned long /*DWORD*/
#define MSE_GET_CURRENT_THREAD_ID CSPTrackerMap::mseWindowsGetCurrentThreadId
#else /*_MSC_VER*/
#include <thread>         // std::thread, MSE_THREAD_ID_TYPE, MSE_GET_CURRENT_THREAD_ID
#define MSE_THREAD_ID_TYPE std::thread::id
#define MSE_GET_CURRENT_THREAD_ID std::this_thread::get_id
#endif /*_MSC_VER*/

namespace mse {

	/* CSPTracker is intended to keep track of all pointers, objects and their lifespans in order to ensure that pointers don't
	end up pointing to deallocated objects. */
	class CSPTracker {
	public:
		CSPTracker() {}
		~CSPTracker() {}
		bool registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		bool unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		void onObjectDestruction(void *obj_ptr);
		void onObjectConstruction(void *obj_ptr);
		bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_obj_pointer_map.size())); }

		/* So this tracker stores the object-pointer mappings in either "fast storage1" or "slow storage". The code for
		"fast storage1" is ugly. The code for "slow storage" is more readable. */
		void removeObjectFromFastStorage1(int fs1_obj_index);
		void moveObjectFromFastStorage1ToSlowStorage(int fs1_obj_index);
		static const int sc_fs1_max_pointers = 3/* must be at least 1 */;
		class CFS1Object {
		public:
			void* m_object_ptr;
			const CSaferPtrBase* m_pointer_ptrs[sc_fs1_max_pointers];
			int m_num_pointers = 0;
		};
		static const int sc_fs1_max_objects = 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */;
		CFS1Object m_fs1_objects[sc_fs1_max_objects];
		int m_num_fs1_objects = 0;

		/* "slow storage" */
		std::unordered_multimap<void*, const CSaferPtrBase*> m_obj_pointer_map;

		//std::mutex m_mutex;
	};

	class CSPTrackerMap {
	public:
		CSPTrackerMap() {
			m_first_thread_id = MSE_GET_CURRENT_THREAD_ID();
			m_first_sp_tracker_pointer = new CSPTracker();
			//std::unordered_map<MSE_THREAD_ID_TYPE, CSPTracker*>::value_type item(m_first_thread_id, m_first_sp_tracker_pointer);
			//auto insert_retval = m_tracker_map.insert(item);
		}
		~CSPTrackerMap() {
			delete m_first_sp_tracker_pointer; m_first_sp_tracker_pointer = nullptr;
			for (auto it = m_tracker_map.begin(); m_tracker_map.end() != it; it++) {
				delete (*it).second; (*it).second = nullptr;
			}
		}
		CSPTracker& SPTrackerRef(const MSE_THREAD_ID_TYPE &thread_id_cref) {
			if (thread_id_cref == m_first_thread_id) {
				return (*m_first_sp_tracker_pointer);
			}
			else {
				std::lock_guard<std::mutex> lock(m_mutex);

				auto found_it = m_tracker_map.find(thread_id_cref);
				if (m_tracker_map.end() == found_it) {
					auto new_sp_tracker_ptr = new CSPTracker();
					std::unordered_map<MSE_THREAD_ID_TYPE, CSPTracker*>::value_type item(thread_id_cref, new_sp_tracker_ptr);
					auto insert_retval = m_tracker_map.insert(item);
					number_of_added_trackers_since_last_pruning += 1;
					if (10000/*arbitrary*/ < number_of_added_trackers_since_last_pruning) {
						remove_empty_trackers();
						number_of_added_trackers_since_last_pruning = 0;
					}

					found_it = insert_retval.first;
				}

				m_first_thread_id = thread_id_cref;
				m_first_sp_tracker_pointer = ((*found_it).second);

				return (*((*found_it).second));
			}
		}
		void remove_empty_trackers() {
			for (auto it = m_tracker_map.begin(); m_tracker_map.end() != it; it++) {
				if ((*it).second->isEmpty()) {
					delete ((*it).second); (*it).second = nullptr;
					m_tracker_map.erase(it);
				}
			}
		}
#ifdef _MSC_VER
		static MSE_THREAD_ID_TYPE mseWindowsGetCurrentThreadId();
#endif /*_MSC_VER*/


		MSE_THREAD_ID_TYPE m_first_thread_id;
		CSPTracker* m_first_sp_tracker_pointer = nullptr;
		std::unordered_map<MSE_THREAD_ID_TYPE, CSPTracker*> m_tracker_map;
		int number_of_added_trackers_since_last_pruning = 0;
		std::mutex m_mutex;
	};

	extern CSPTrackerMap gSPTrackerMap;

	template<typename _Ty>
	class TRegisteredObjForLegacy;

	/* TRegisteredPointerForLegacy is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, it may be the case that fewer code changes
	(explicit casts) will be required when using this template. */
	template<typename _Ty>
	class TRegisteredPointerForLegacy : public TSaferPtrForLegacy<_Ty> {
	public:
		TRegisteredPointerForLegacy() : TSaferPtrForLegacy<_Ty>() {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID()));
		}
		TRegisteredPointerForLegacy(_Ty* ptr) : TSaferPtrForLegacy<_Ty>(ptr) {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID()));
			m_might_not_point_to_a_TRegisteredObjForLegacy = true;
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
		}
		/* The CSPTracker* parameter is actually kind of redundant. We include it to remove ambiguity in the overloads. */
		TRegisteredPointerForLegacy(CSPTracker* sp_tracker_ptr, TRegisteredObjForLegacy<_Ty>* ptr) : TSaferPtrForLegacy<_Ty>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
		}
		TRegisteredPointerForLegacy(const TRegisteredPointerForLegacy& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID()));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			m_might_not_point_to_a_TRegisteredObjForLegacy = src_cref.m_might_not_point_to_a_TRegisteredObjForLegacy;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		virtual ~TRegisteredPointerForLegacy() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			(*m_sp_tracker_ptr).onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(_Ty* ptr) {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
			m_might_not_point_to_a_TRegisteredObjForLegacy = true;
			return (*this);
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(const TRegisteredPointerForLegacy<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(_Right_cref);
			(*m_sp_tracker_ptr).registerPointer((*this), _Right_cref);
			assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			m_might_not_point_to_a_TRegisteredObjForLegacy = _Right_cref.m_might_not_point_to_a_TRegisteredObjForLegacy;
			return (*this);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
		}
		/* This function, if possible, should not be used. It is meant to be used exclusively by registered_delete_for_legacy<>(). */
		TRegisteredObjForLegacy<_Ty>* asANativePointerToTRegisteredObjForLegacy() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			/* It should be ok to hard cast to (TRegisteredObjForLegacy<_Ty>*) even if (*this).m_ptr points to a _Ty and not a
			TRegisteredObjForLegacy<_Ty>, because TRegisteredObjForLegacy<_Ty> doesn't have any extra data members that _Ty
			doesn't. */
			return (TRegisteredObjForLegacy<_Ty>*)((*this).m_ptr);
		}
		/*Ideally these "address of" operators shouldn't be used. If you want a pointer to a TRegisteredPointerForLegacy<_Ty>,
		declare the TRegisteredPointerForLegacy<_Ty> as a TRegisteredObjForLegacy<TRegisteredPointerForLegacy<_Ty>> instead. So
		for example:
		auto reg_ptr = TRegisteredObjForLegacy<TRegisteredPointerForLegacy<_Ty>>(mse::registered_new_for_legacy<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRegisteredPointerForLegacy<TRegisteredPointerForLegacy<_Ty>> operator&() {
			return this;
		}
		TRegisteredPointerForLegacy<const TRegisteredPointerForLegacy<_Ty>> operator&() const {
			return this;
		}

		CSPTracker* m_sp_tracker_ptr = nullptr;
		bool m_might_not_point_to_a_TRegisteredObjForLegacy = false;
	};

	class CTrackerNotifier {
	public:
		CTrackerNotifier() {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID()));
			(*m_sp_tracker_ptr).onObjectConstruction(this);
		}
		~CTrackerNotifier() {
			(*m_sp_tracker_ptr).onObjectDestruction(this);
		}
		CSPTracker* trackerPtr() const { return m_sp_tracker_ptr; }

		CSPTracker* m_sp_tracker_ptr = nullptr;
	};

	/* See TRegisteredObj. */
	template<typename _Ty>
	class TRegisteredObjForLegacy : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObjForLegacy, _Ty);
		virtual ~TRegisteredObjForLegacy() {
			//gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID()).onObjectDestruction(this);
		}
		TRegisteredObjForLegacy& operator=(TRegisteredObjForLegacy&& _X) { _Ty::operator=(std::move(_X)); return (*this); }
		TRegisteredObjForLegacy& operator=(const TRegisteredObjForLegacy& _X) { _Ty::operator=(_X); return (*this); }
		TRegisteredPointerForLegacy<_Ty> operator&() {
			return TRegisteredPointerForLegacy<_Ty>(trackerPtr(), this);
			//return this;
		}
		TRegisteredPointerForLegacy<const _Ty> operator&() const {
			return TRegisteredPointerForLegacy<_Ty>(trackerPtr(), this);
			//return this;
		}
		CSPTracker* trackerPtr() const { return m_tracker_notifier.trackerPtr(); }

		CTrackerNotifier m_tracker_notifier;
	};

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TRegisteredPointerForLegacy<_Ty> registered_new_for_legacy(Args&&... args) {
		auto a = new TRegisteredObjForLegacy<_Ty>(args...);
		return TRegisteredPointerForLegacy<_Ty>((*a).trackerPtr(), a);
	}
	template <class _Ty>
	void registered_delete_for_legacy(const TRegisteredPointerForLegacy<_Ty>& regPtrRef) {
		//auto a = dynamic_cast<TRegisteredObjForLegacy<_Ty> *>((_Ty*)regPtrRef);
		//auto a = (TRegisteredObjForLegacy<_Ty>*)regPtrRef;
		if (regPtrRef.m_might_not_point_to_a_TRegisteredObjForLegacy) {
			/* It would be a very strange case to arrive here. For (aggressive) compatibility reasons we allow
			TRegisteredPointerForLegacy<_Ty> to point to a _Ty instead of a TRegisteredObjForLegacy<_Ty>. But in those
			situations it doesn't make sense that someone would be calling this delete function. */
			//_Ty* a = &regPtrRef;
			_Ty* a = regPtrRef.m_ptr;
			delete a;
		}
		else {
			auto a = regPtrRef.asANativePointerToTRegisteredObjForLegacy();
			delete a;
		}
	}
}


#endif // MSEREGISTEREDFORLEGACY_H_
