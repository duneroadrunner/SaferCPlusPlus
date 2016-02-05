Jan 2016

A collection of safe data types that are compatible with, and can substitute for, common unsafe native c++ types. Currently these include:

- A [fast](#simple-benchmarks), [safe replacement for native pointers](#registered-pointers) that, unlike std::shared_ptr for example, does not take ownership of the target (and so can point to objects on the stack).

- An almost completely safe implementation of std::vector<> - bounds checked, iterator checked and memory managed.

- A couple of other highly compatible vectors that address the issue of unnecessary iterator invalidation upon insert, erase or reallocation

- [replacements](#cint-csize_t-and-cbool) for the native "int", "size_t" and "bool" types that have default initialization values and address the "signed-unsigned mismatch" issues.

Tested with msvc2013 and g++4.8 (as of Dec 2015) and msvc2010 (as of Jan 2015).

See the file [msetl_blurb.pdf](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_blurb.pdf) for more info. Or just have a look at [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp) to see the library in action.



### Registered Pointers

"Registered" pointers are intended to behave just like native C++ pointers, except that their value is (automatically) set to nullptr when the target object is destroyed. And by default they will throw an exception upon any attempt to dereference a nullptr. Because they don't take ownership like some other smart pointers, they can point to objects allocated on the stack as well as the heap. In most cases, they can be used as a compatible, direct substitute for native pointers, making it straightforward to update legacy code (to be safer).

Registered pointers come in two flavors - TRegisteredPointer and TRegisteredPointerForLegacy. They are both very similar. TRegisteredPointer emphasizes speed and safety a bit more, while TRegisteredPointerForLegacy emphasizes compatibility and flexibility a bit more. If you want to undertake the task of en masse replacement of native pointers in legacy code, or need to interact with legacy native pointer interfaces, TRegisteredPointerForLegacy may be more convenient.

Note that these registered pointers cannot target types that cannot act as base classes. The primitive types like int, bool, etc. cannot act as base classes. Fortunately, the library provides safer substitutes for int, bool and size_t that can act as base classes.


### TRegisteredPointer

usage example:

    #include "mseregistered.h"
    
    int main(int argc, char* argv[]) {
        class CA {
        public:
            CA(int x) : m_x(x) {}
            int m_x;
        };
    
        mse::TRegisteredPointer<CA> a_ptr;
        CA a2_obj(2);
        {
            // mse::TRegisteredObj<CA> is a class publicly derived from CA
    
            mse::TRegisteredObj<CA> a_obj(1); // a_obj is entirely on the stack
    
            a_ptr = &a_obj;
            a2_obj = (*a_ptr);
        }
        if (a_ptr) {
            assert(false);
        } else {
            try {
                a2_obj = (*a_ptr);
            }
            catch (...) {
                // expected exception
            }
        }
    
        a_ptr = mse::registered_new<CA>(3); // heap allocation
        mse::registered_delete<CA>(a_ptr);
    }


### TRegisteredNotNullPointer
Same as TRegisteredPointer, but cannot be constructed to a null_ptr value.

### TRegisteredFixedPointer
Same as TRegisteredNotNullPointer, but cannot be re-targeted after construction. This is the recommended type to be used for safe parameter passing by reference.

usage example:

    #include "mseregistered.h"
    
    int main(int argc, char* argv[]) {
        class CA {
        public:
            CA(std::string str) : m_str(str) {}
            std::string m_str;
        };
    
        class CB {
        public:
            static void foo(mse::TRegisteredFixedConstPointer<CA> input1_fc_ptr, mse::TRegisteredFixedConstPointer<CA> 
                input2_fc_ptr, mse::TRegisteredFixedPointer<CA> output_f_ptr) {
                output_f_ptr->m_str = "output from " + input1_fc_ptr->m_str + " and " + input2_fc_ptr->m_str;
                return;
            }
        };
    
        mse::TRegisteredObj<CA> in1_obj("input1");
        mse::TRegisteredPointer<CA> in2_reg_ptr = mse::registered_new<CA>("input2");
        mse::TRegisteredObj<CA> out_obj("");
    
        CB::foo(&in1_obj, &(*in2_reg_ptr), &out_obj);
    
        mse::registered_delete<CA>(in2_reg_ptr);
    }

### TRegisteredConstPointer, TRegisteredNotNullConstPointer, TRegisteredFixedConstPointer
Just the "const" version of the references.

### TRegisteredPointerForLegacy

usage example:

    #include "mseregisteredforlegacy.h"
    
    int main(int argc, char* argv[]) {
    
        /* One case where you may need to use mse::TRegisteredPointerForLegacy<> even when not dealing with legacy code is when
        you need a reference to a class before it is fully defined. For example, when you have two classes that mutually
        reference each other. mse::TRegisteredPointer<> does not support this.
        */
    
        class C;
    
        class D {
        public:
            virtual ~D() {}
            mse::TRegisteredPointerForLegacy<C> m_c_ptr;
        };
    
        class C {
        public:
            mse::TRegisteredPointerForLegacy<D> m_d_ptr;
        };
    
        mse::TRegisteredObjForLegacy<C> regobjfl_c;
        mse::TRegisteredPointerForLegacy<D> d_ptr = mse::registered_new_for_legacy<D>();
    
        regobjfl_c.m_d_ptr = d_ptr;
        d_ptr->m_c_ptr = &regobjfl_c;
    
        mse::registered_delete_for_legacy<D>(d_ptr);
    
    }


### TRegisteredNotNullPointerForLegacy

### TRegisteredFixedPointerForLegacy

### TRegisteredConstPointerForLegacy, TRegisteredNotNullConstPointerForLegacy, TRegisteredFixedConstPointerForLegacy
  
### Simple benchmarks

#### Allocation, deallocation, pointer copy and assignment:
Pointer Type | Time
------------ | ----
mse::TRegisteredPointer (stack): | 0.0270016 seconds.
native pointer (heap): | 0.0490028 seconds.
mse::TRegisteredPointer (heap): | 0.0740042 seconds.
std::shared_ptr (heap): | 0.087005 seconds.
mse::TRegisteredPointerForLegacy (heap): | 0.142008 seconds.

#### Dereferencing:
Pointer Type | Time
------------ | ----
native pointer: | 0.0100006 seconds.
mse::TRegisteredObjForLegacy unchecked: | 0.0130008 seconds.
mse::TRegisteredObjForLegacy (checked): | 0.016001 seconds.
std::weak_ptr: | 0.17701 seconds.

platform: msvc2013/Windows7/Haswell  
benchmark source code: [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp)

### CInt CSize_t and CBool

usage example:

    #include "mseprimitives.h"
    
    int main(int argc, char* argv[]) {
    
        mse::CInt i = 5;
        i -= 17;
        mse::CSize_t szt = 5;
        szt += 3;
        auto i2 = szt + i;
        CBool b = false;
        if (-4 == i2) {
            b = true;
        }
        if (b) {
            try {
                szt -= 20; // out of range result - this is going to throw an exception
            }
            catch (...) {
                // expected exception
            }
        }
    }



