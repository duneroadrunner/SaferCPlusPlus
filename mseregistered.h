#ifndef MSEREGISTERED_H_
#define MSEREGISTERED_H_

#include "mseprimitives.h"
#include <unordered_set>

namespace mse {

	/* CRPTracker is intended to keep track of all the pointers pointing to an object. CRPTracker objects are intended to be always
	associated with (infact, a member of) the one object that is the target of the pointers it tracks. Though at the moment, it
	doesn't need to actually know which object it is associated with. */
	class CRPTracker {
	public:
		CRPTracker() {}
		CRPTracker(const CRPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		CRPTracker(CRPTracker&& src) { /* see above */ }
		~CRPTracker() {
			if (!fast_mode1()) {
				delete m_ptr_to_regptr_set_ptr;
			}
		}
		CRPTracker& operator=(const CRPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		CRPTracker& operator=(CRPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const CRPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true;
		}
		bool operator!=(const CRPTracker& _Right_cref) const { /* see above */ return false; }

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
					else { /* There are no registered pointers to be unregistered. */ assert(false); }
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
					else { /* There are no registered pointers to be unregistered. */ assert(false); }
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
		static const int sc_fm1_max_pointers = 2/*arbitrary*/;
		const CSaferPtrBase* m_fm1_ptr_to_regptr_array[sc_fm1_max_pointers];

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;
	};

	/* CSORPTracker is a "size optimized" (smaller and slower) version of CSPTracker. Currently not used. */
	class CSORPTracker {
	public:
		CSORPTracker() {}
		CSORPTracker(const CSORPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		CSORPTracker(CSORPTracker&& src) { /* see above */ }
		~CSORPTracker() {
			delete m_ptr_to_regptr_set_ptr;
		}
		CSORPTracker& operator=(const CSORPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		CSORPTracker& operator=(CSORPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const CSORPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true;
		}
		bool operator!=(const CSORPTracker& _Right_cref) const { /* see above */ return false; }

		void registerPointer(const CSaferPtrBase& sp_ref) {
			if (!m_ptr_to_regptr_set_ptr) {
				m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
			}
			std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
			(*m_ptr_to_regptr_set_ptr).insert(item);
		}
		void unregisterPointer(const CSaferPtrBase& sp_ref) {
			if (!m_ptr_to_regptr_set_ptr) {
				assert(false);
			}
			else {
				auto res = (*m_ptr_to_regptr_set_ptr).erase(&sp_ref);
				assert(0 != res);
			}
		}
		void onObjectDestruction() {
			if (m_ptr_to_regptr_set_ptr) {
				for (auto sp_ref_ptr : (*m_ptr_to_regptr_set_ptr)) {
					(*sp_ref_ptr).setToNull();
				}
			}
		}

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;
	};

	template<typename _Ty>
	class TRegisteredObj;

	/* TRegisteredPointer behaves similar to (and is largely compatible with) native pointers. It inherits the safety features of
	TSaferPtr (default nullptr initialization and check for null pointer dereference). In addition, when pointed at a
	TRegisteredObj, it will be checked for attempted access after destruction. It's essentially intended to be a safe pointer like
	std::shared_ptr, but that does not take ownership of the target object (i.e. does not take responsibility for deallocation).
	Because it does not take ownership, unlike std::shared_ptr, TRegisteredPointer can be used to point to objects on the stack. */
	template<typename _Ty>
	class TRegisteredPointer : public TSaferPtr<TRegisteredObj<_Ty>> {
	public:
		TRegisteredPointer();
		TRegisteredPointer(TRegisteredObj<_Ty>* ptr);
		TRegisteredPointer(const TRegisteredPointer& src_cref);
		virtual ~TRegisteredPointer();
		TRegisteredPointer<_Ty>& operator=(TRegisteredObj<_Ty>* ptr);
		TRegisteredPointer<_Ty>& operator=(const TRegisteredPointer<_Ty>& _Right_cref);
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const;

	private:
		/* If you want a pointer to a TRegisteredPointer<_Ty>, declare the TRegisteredPointer<_Ty> as a
		TRegisteredObj<TRegisteredPointer<_Ty>> instead. So for example:
		auto reg_ptr = TRegisteredObj<TRegisteredPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRegisteredPointer<_Ty>* operator&() {
			return this;
		}
		(const TRegisteredPointer<_Ty>)* operator&() const {
			return this;
		}
	};

	/* TRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TRegisteredPointers will avoid referencing destroyed objects. Note that TRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _Ty>
	class TRegisteredObj : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObj, _Ty);
		virtual ~TRegisteredObj() {
			mseRPManager().onObjectDestruction();
		}
		TRegisteredObj& operator=(TRegisteredObj&& _X) { _Ty::operator=(std::move(_X)); return (*this); }
		TRegisteredObj& operator=(const TRegisteredObj& _X) { _Ty::operator=(_X); return (*this); }
		TRegisteredPointer<_Ty> operator&() {
			return this;
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return this;
		}
		CRPTracker& mseRPManager() { return m_mseRPManager; }

		mutable CRPTracker m_mseRPManager;
	};

	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer() : TSaferPtr<TRegisteredObj<_Ty>>() {}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer(TRegisteredObj<_Ty>* ptr) : TSaferPtr<TRegisteredObj<_Ty>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer(const TRegisteredPointer& src_cref) : TSaferPtr<TRegisteredObj<_Ty>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::~TRegisteredPointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>& TRegisteredPointer<_Ty>::operator=(TRegisteredObj<_Ty>* ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
		TSaferPtr<TRegisteredObj<_Ty>>::operator=(ptr);
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
		return (*this);
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>& TRegisteredPointer<_Ty>::operator=(const TRegisteredPointer<_Ty>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty>
	TRegisteredPointer<_Ty>::operator _Ty*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}

	/* registered_new is intended to be analogous to std::make_shared */
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		return new TRegisteredObj<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredObj<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}

	static void s_regptr_test1() {

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
			static int foo2(mse::TRegisteredPointer<A> A_registered_ptr) { return A_registered_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;
		/* mse::TRegisteredPointer<> is basically a "safe" version of the native pointer. */
		mse::TRegisteredPointer<A> A_registered_ptr1;

		{
			A a;
			mse::TRegisteredObj<A> registered_a;
			/* mse::TRegisteredObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == registered_a.b);
			A_native_ptr = &a;
			A_registered_ptr1 = &registered_a;
			assert(A_native_ptr->b == A_registered_ptr1->b);

			auto A_registered_ptr2 = &registered_a;
			/* A_registered_ptr2 is actually an mse::TRegisteredPointer<A>, not a native pointer. */
			A_registered_ptr2 = nullptr;
			bool expected_exception = false;
			try {
				int i = A_registered_ptr2->b; /* this is gonna throw an exception */
			}
			catch (...) {
				//std::cerr << "expected exception" << std::endl;
				expected_exception = true;
				/* The exception is triggered by an attempt to dereference a null "registered pointer". */
			}
			assert(expected_exception);

			/* mse::TRegisteredPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_registered_ptr1);

			if (A_registered_ptr2) {
			}
			else if (A_registered_ptr2 != A_registered_ptr1) {
				A_registered_ptr2 = A_registered_ptr1;
				assert(A_registered_ptr2 == A_registered_ptr1);
			}

			A a2 = a;
			mse::TRegisteredObj<A> registered_a2 = registered_a;

			a2 = std::move(a);
			registered_a2 = std::move(registered_a);

			A a3(std::move(a2));
			mse::TRegisteredObj<A> registered_a3(std::move(registered_a2));
		}

		bool expected_exception = false;
		try {
			/* A_registered_ptr1 "knows" that the (registered) object it was pointing to has now been deallocated. */
			int i = A_registered_ptr1->b; /* So this is gonna throw an exception */
		}
		catch (...) {
			//std::cerr << "expected exception" << std::endl;
			expected_exception = true;
		}
		assert(expected_exception);

		{
			/* For heap allocations mse::registered_new is kind of analagous to std::make_shared, but again,
			mse::TRegisteredPointers don't take ownership so you are responsible for deallocation. */
			auto A_registered_ptr3 = mse::registered_new<A>();
			assert(3 == A_registered_ptr3->b);
			mse::registered_delete<A>(A_registered_ptr3);
			bool expected_exception = false;
			try {
				/* A_registered_ptr3 "knows" that the (registered) object it was pointing to has now been deallocated. */
				int i = A_registered_ptr3->b; /* So this is gonna throw an exception */
			}
			catch (...) {
				//std::cerr << "expected exception" << std::endl;
				expected_exception = true;
			}
			assert(expected_exception);
		}
	}

}

#endif // MSEREGISTERED_H_
