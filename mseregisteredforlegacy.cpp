
#include "mseregisteredforlegacy.h"
#include <list>


namespace mse {
	bool CSPManager::registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		std::unordered_multimap<void*, const CSaferPtrBase&>::value_type item(obj_ptr, sp_ref);
		m_obj_pointer_map.insert(item);
		return true;
	}
	bool CSPManager::unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
		if (nullptr == obj_ptr) { return true; }
		bool retval = false;
		auto range = m_obj_pointer_map.equal_range(obj_ptr);
		for (auto& it = range.first; range.second != it; it++) {
			if (&((*it).second) == &sp_ref)/*we're comparing pointers here*/ {
				m_obj_pointer_map.erase(it);
				retval = true;
				break;
			}
		}
		return retval;
	}
	void CSPManager::onObjectDestruction(void *obj_ptr) {
		if (nullptr == obj_ptr) { assert(false); return; }
		std::list<const CSaferPtrBase*> pointerRefList;
		auto range = m_obj_pointer_map.equal_range(obj_ptr);
		for (auto& it = range.first; range.second != it; it++) {
			((*it).second).setToNull();
			pointerRefList.push_back(&((*it).second));
		}
		{
			auto it = m_obj_pointer_map.find(obj_ptr);
			while (m_obj_pointer_map.end() != it) {
				m_obj_pointer_map.erase(it);
				it = m_obj_pointer_map.find(obj_ptr);
			}
		}
		for (auto item : pointerRefList) {
			registerPointer((*item), nullptr);
		}
	}

	CSPManager gSPManager;
}
