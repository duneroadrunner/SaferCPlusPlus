
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "mserelaxedregistered.h"

#ifdef _MSC_VER
#include "windows.h"
#endif /*_MSC_VER*/


namespace mse {

	bool CSPTracker::registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		{
			//std::lock_guard<std::mutex> lock(m_mutex);

			/* check if the object is in "fast storage 1" first */
			for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
				if (obj_ptr == m_fs1_objects[i].m_object_ptr) {
					auto& fs1_object_ref = m_fs1_objects[i];
					if (sc_fs1_max_pointers == fs1_object_ref.m_num_pointers) {
						/* Too many pointers. We're gonna move this object to slow storage. */
						moveObjectFromFastStorage1ToSlowStorage(i);
						/* Then add the new object-pointer mapping to slow storage. */
						std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(obj_ptr, &sp_ref);
						m_obj_pointer_map.insert(item);
						return true;
					}
					else {
						/* register the object-pointer mapping in "fast storage 1" */
						fs1_object_ref.m_pointer_ptrs[fs1_object_ref.m_num_pointers] = (&sp_ref);
						fs1_object_ref.m_num_pointers += 1;
						return true;
					}
				}
			}

			/* The object was not in "fast storage 1". Check if it's in "slow storage". */
			bool object_is_in_slow_storage = false;
			if (1 <= m_obj_pointer_map.size()) {
				auto found_it = m_obj_pointer_map.find(obj_ptr);
				if (m_obj_pointer_map.end() != found_it) {
					object_is_in_slow_storage = true;
				}
			}

			if ((!object_is_in_slow_storage) && (1 <= sc_fs1_max_objects) && (1 <= sc_fs1_max_pointers)) {
				/* We'll add this object to fast storage. */
				if (sc_fs1_max_objects == m_num_fs1_objects) {
					/* Too many objects. We're gonna move the oldest object to slow storage. */
					moveObjectFromFastStorage1ToSlowStorage(0);
				}
				auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
				fs1_object_ref.m_object_ptr = obj_ptr;
				fs1_object_ref.m_pointer_ptrs[0] = &sp_ref;
				fs1_object_ref.m_num_pointers = 1;
				m_num_fs1_objects += 1;
				return true;
			}
			else {
				/* Add the mapping to slow storage. */
				std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(obj_ptr, &sp_ref);
				m_obj_pointer_map.insert(item);
			}
		}
		return true;
	}

	bool CSPTracker::unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		bool retval = false;
		{
			//std::lock_guard<std::mutex> lock(m_mutex);

			/* check if the object is in "fast storage 1" first */
			for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
				if (obj_ptr == m_fs1_objects[i].m_object_ptr) {
					auto& fs1_object_ref = m_fs1_objects[i];
					if (1 == fs1_object_ref.m_num_pointers) {
						/* Special case code just for speed. */
						if ((&sp_ref) == fs1_object_ref.m_pointer_ptrs[0]) {
							fs1_object_ref.m_num_pointers = 0;
							//removeObjectFromFastStorage1(i);
						}
						else {
							/* We should really never get here. It seems someone's trying to unregister a pointer that does not
							seem to be registered. */
							return false;
						}
						return true;
					}
					else {
						for (int j = (fs1_object_ref.m_num_pointers - 1); j >= 0; j -= 1) {
							if ((&sp_ref) == fs1_object_ref.m_pointer_ptrs[j]) {
								/* Found the mapping for the pointer. We'll now remove it. */
								for (int k = j; k < (fs1_object_ref.m_num_pointers - 1); k += 1) {
									fs1_object_ref.m_pointer_ptrs[k] = fs1_object_ref.m_pointer_ptrs[k + 1];
								}
								fs1_object_ref.m_num_pointers -= 1;

								if (0 == fs1_object_ref.m_num_pointers) {
									//removeObjectFromFastStorage1(i);
								}

								return true;
							}
						}
					}
					/* We should really never get here. It seems someone's trying to unregister a pointer that does not
					seem to be registered. */
					return false;
				}
			}

			/* The object was not in "fast storage 1". It's proably in "slow storage". */
			auto range = m_obj_pointer_map.equal_range(obj_ptr);
			if (true) {
				for (auto& it = range.first; range.second != it; it++) {
					if (((*it).second) == &sp_ref)/*we're comparing "native pointers pointing to smart pointers" here*/ {
						m_obj_pointer_map.erase(it);
						retval = true;
						break;
					}
				}
			}
		}
		return retval;
	}

	void CSPTracker::onObjectDestruction(void *obj_ptr) {
		if (nullptr == obj_ptr) { assert(false); return; }
		{
			//std::lock_guard<std::mutex> lock(m_mutex);

			/* check if the object is in "fast storage 1" first */
			for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
				if (obj_ptr == m_fs1_objects[i].m_object_ptr) {
					auto& fs1_object_ref = m_fs1_objects[i];
					for (int j = 0; j < fs1_object_ref.m_num_pointers; j += 1) {
						(*(fs1_object_ref.m_pointer_ptrs[j])).setToNull();
					}
					removeObjectFromFastStorage1(i);
					return;
				}
			}

			/* The object was not in "fast storage 1". It's proably in "slow storage". */
			auto range = m_obj_pointer_map.equal_range(obj_ptr);
			for (auto it = range.first; range.second != it; it++) {
				(*((*it).second)).setToNull();
			}
			m_obj_pointer_map.erase(obj_ptr);
		}
	}

	void CSPTracker::onObjectConstruction(void *obj_ptr) {
		if (nullptr == obj_ptr) { assert(false); return; }
		if ((1 <= sc_fs1_max_objects) && (1 <= sc_fs1_max_pointers)) {
			/* We'll add this object to fast storage. */
			if (sc_fs1_max_objects == m_num_fs1_objects) {
				/* Too many objects. We're gonna move the oldest object to slow storage. */
				moveObjectFromFastStorage1ToSlowStorage(0);
			}
			auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
			fs1_object_ref.m_object_ptr = obj_ptr;
			fs1_object_ref.m_num_pointers = 0;
			m_num_fs1_objects += 1;
			return;
		}
	}

	void CSPTracker::removeObjectFromFastStorage1(int fs1_obj_index) {
		for (int j = fs1_obj_index; j < (m_num_fs1_objects - 1); j += 1) {
			m_fs1_objects[j] = m_fs1_objects[j + 1];
		}
		m_num_fs1_objects -= 1;
	}

	void CSPTracker::moveObjectFromFastStorage1ToSlowStorage(int fs1_obj_index) {
		auto& fs1_object_ref = m_fs1_objects[fs1_obj_index];
		/* First we're gonna copy this object to slow storage. */
		for (int j = 0; j < fs1_object_ref.m_num_pointers; j += 1) {
			std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(fs1_object_ref.m_object_ptr, fs1_object_ref.m_pointer_ptrs[j]);
			m_obj_pointer_map.insert(item);
		}
		/* Then we're gonna remove the object from fast storage */
		removeObjectFromFastStorage1(fs1_obj_index);
	}

#ifdef _MSC_VER
	MSE_THREAD_ID_TYPE CSPTrackerMap::mseWindowsGetCurrentThreadId() {
		return GetCurrentThreadId();
	}
#endif /*_MSC_VER*/

	CSPTrackerMap gSPTrackerMap;
}
