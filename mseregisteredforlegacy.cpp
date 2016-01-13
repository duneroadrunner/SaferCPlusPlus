
#include "mseregisteredforlegacy.h"


namespace mse {
	bool CSPTracker::registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(obj_ptr, &sp_ref);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_obj_pointer_map.insert(item);
		}
		return true;
	}
	bool CSPTracker::unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		bool retval = false;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto range = m_obj_pointer_map.equal_range(obj_ptr);
			if (false) {
				/* This branch could be theoretically faster when range.first is a random access iterator. In practice though,
				it seems to be only a bidirectional iterator. */
				std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(obj_ptr, &sp_ref);
				auto found_it = std::find(range.first, range.second, item);
				if (range.second != found_it) {
					m_obj_pointer_map.erase(found_it);
					retval = true;
				}
			}
			else {
				for (auto& it = range.first; range.second != it; it++) {
					if (((*it).second) == &sp_ref)/*we're comparing pointers here*/ {
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
			std::lock_guard<std::mutex> lock(m_mutex);
			auto range = m_obj_pointer_map.equal_range(obj_ptr);
			for (auto it = range.first; range.second != it; it++) {
				(*((*it).second)).setToNull();
			}
			m_obj_pointer_map.erase(obj_ptr);
		}
	}

	CSPTracker gSPTracker;
}
