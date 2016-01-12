
#include "mseregisteredforlegacy.h"


namespace mse {
	bool CSPTracker::registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		std::unordered_multimap<void*, const CSaferPtrBase&>::value_type item(obj_ptr, sp_ref);
		m_obj_pointer_map.insert(item);
		return true;
	}
	bool CSPTracker::unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		bool retval = false;
		//auto lower_bound = m_obj_pointer_map.lower_bound(obj_ptr);
		auto range = m_obj_pointer_map.equal_range(obj_ptr);
		//for (auto it = lower_bound; m_obj_pointer_map.end() != it; it++) {
		for (auto& it = range.first; range.second != it; it++) {
			if (&((*it).second) == &sp_ref)/*we're comparing pointers here*/ {
				m_obj_pointer_map.erase(it);
				retval = true;
				break;
			}
		}
		return retval;
	}
	void CSPTracker::onObjectDestruction(void *obj_ptr) {
		if (nullptr == obj_ptr) { assert(false); return; }
		auto range = m_obj_pointer_map.equal_range(obj_ptr);
		for (auto it = range.first; range.second != it; it++) {
			((*it).second).setToNull();
		}
		m_obj_pointer_map.erase(obj_ptr);
	}

	CSPTracker gSPTracker;
}
