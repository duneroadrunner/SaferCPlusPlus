Mar 2016

### Overview

A collection of safe data types that are compatible with, and can substitute for, common unsafe native C++ types. Currently these include:

- A [fast](#simple-benchmarks), [safe replacement for native pointers](#registered-pointers) that, unlike std::shared_ptr for example, does not take ownership of the target (and so can point to objects on the stack).

- An almost completely [safe implementation](#vector) of std::vector<> - bounds checked, iterator checked and memory managed.

- A couple of [other](#vectors) highly compatible vectors that address the issue of unnecessary iterator invalidation upon insert, erase or reallocation

- [replacements](#primitives) for the native "int", "size_t" and "bool" types that have default initialization values and address the "signed-unsigned mismatch" issues.

Tested with msvc2013 and g++4.8 (as of Dec 2015) and msvc2010 (as of Jan 2015).

See the file [msetl_blurb.pdf](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_blurb.pdf) for more info. Or just have a look at [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp) to see the library in action.


### Use cases

This library is appropriate for use by two groups of C++ developers - those for whom safety and security are critical, and also everybody else.  
This library can help eliminate a lot of the opportunities for inadvertently accessing invalid memory or using uninitialized values. It essentially gets you a lot of the safety that you might get from, say Java, while retaining all of the power and most of the performance of C++.  
While using the library may sometimes cost a modest performance penalty, because the library elements are [largely compatible](#compatibility-considerations) with their native counterparts they can be easily "disabled" (automatically replaced with their native counterparts) with a compile-time directive, allowing them to be used to help catch bugs in debug/test/beta modes while incurring no overhead in release mode.  
So there is really no excuse for not using the library in pretty much any situation.


### Setup and dependencies

The beauty of the library is that it is so small and simple. Using the library generally involves copying the include files you want to use into your project, and that's it. Three header files - "mseprimitives.h", "mseregistered.h" and "msemstdvector.h" - will cover most use cases. Outside of the stl, there are no other dependencies.  
A couple of notes about compling: g++(4.8) requires the -fpermissive flag. And in some cases msvc may complain about something about too many “inline” items. The error code will direct you to instructions to resolve the issue.

### Registered pointers

"Registered" pointers are intended to behave just like native C++ pointers, except that their value is (automatically) set to nullptr when the target object is destroyed. And by default they will throw an exception upon any attempt to dereference a nullptr. Because they don't take ownership like some other smart pointers, they can point to objects allocated on the stack as well as the heap. In most cases, they can be used as a compatible, direct substitute for native pointers, making it straightforward to update legacy code (to be safer).

Registered pointers come in two flavors - [TRegisteredPointer](#tregisteredpointer) and [TRelaxedRegisteredPointer](#trelaxedregisteredpointer). They are both very similar. TRegisteredPointer emphasizes speed and safety a bit more, while TRelaxedRegisteredPointer emphasizes compatibility and flexibility a bit more. If you want to undertake the task of en masse replacement of native pointers in legacy code, or need to interact with legacy native pointer interfaces, TRelaxedRegisteredPointer may be more convenient.

Note that these registered pointers cannot target types that cannot act as base classes. The primitive types like int, bool, etc. [cannot act as base classes](#compatibility-considerations). Fortunately, the library provides safer [substitutes](#primitives) for int, bool and size_t that can act as base classes. Also note that pointers that can point to the stack are inherently not thread safe. While we do not encourage the casual sharing of objects between asynchronous threads, if you need to do so you might consider using something like std::share_ptr.


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
Same as TRegisteredNotNullPointer, but cannot be re-targeted after construction (basically a "const TRegisteredNotNullPointer"). It is essentially a functional equivalent of a C++ reference and is the recommended type to be used for safe parameter passing by reference. 
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

### TRelaxedRegisteredPointer

usage example:

    #include "mserelaxedregistered.h"
    
    int main(int argc, char* argv[]) {
    
        /* One case where you may need to use mse::TRelaxedRegisteredPointer<> even when not dealing with legacy code is when
        you need a reference to a class before it is fully defined. For example, when you have two classes that mutually
        reference each other. mse::TRegisteredPointer<> does not support this.
        */
    
        class C;
    
        class D {
        public:
            virtual ~D() {}
            mse::TRelaxedRegisteredPointer<C> m_c_ptr;
        };
    
        class C {
        public:
            mse::TRelaxedRegisteredPointer<D> m_d_ptr;
        };
    
        mse::TRelaxedRegisteredObj<C> regobjfl_c;
        mse::TRelaxedRegisteredPointer<D> d_ptr = mse::relaxed_registered_new<D>();
    
        regobjfl_c.m_d_ptr = d_ptr;
        d_ptr->m_c_ptr = &regobjfl_c;
    
        mse::relaxed_registered_delete<D>(d_ptr);
    
    }


### TRelaxedRegisteredNotNullPointer

### TRelaxedRegisteredFixedPointer

### TRelaxedRegisteredConstPointer, TRelaxedRegisteredNotNullConstPointer, TRelaxedRegisteredFixedConstPointer
  
### Simple benchmarks
It would appear, from these simple benchmarks, that using mse::TRegisteredPointers with objects allocated on the stack is significantly faster than using a smart pointer, or even native pointer, that allocates it's target object on the heap. It also seems to be the case that the performance cost of doing a null_ptr check before each dereference is actually quite modest.

#### Allocation, deallocation, pointer copy and assignment:
Pointer Type | Time
------------ | ----
mse::TRegisteredPointer (stack): | 0.0270016 seconds.
native pointer (heap): | 0.0490028 seconds.
mse::TRegisteredPointer (heap): | 0.0740042 seconds.
std::shared_ptr (heap): | 0.087005 seconds.
mse::TRelaxedRegisteredPointer (heap): | 0.142008 seconds.

#### Dereferencing:
Pointer Type | Time
------------ | ----
native pointer: | 0.0100006 seconds.
mse::TRelaxedRegisteredPointer unchecked: | 0.0130008 seconds.
mse::TRelaxedRegisteredPointer (checked): | 0.016001 seconds.
std::weak_ptr: | 0.17701 seconds.

platform: msvc2013/Windows7/Haswell (Jan 2016)  
benchmark source code: [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp)

### Primitives
### CInt, CSize_t and CBool
These classes are meant to behave like, and be compatible with their native counterparts. They have default initialization values to help ensure deterministic behavior. Upon value assignment, CInt and CSize_t will check to ensure that the value fits within the type's range. CSize_t's `-=` operator checks that the operation evaluates to a positive value. And unlike its native counterpart, arithmetic operations involving CSize_t that could evaluate to a negative number are returned as a (signed) CInt.

usage example:

    #include "mseprimitives.h"
    
    int main(int argc, char* argv[]) {
    
        mse::CInt i = 5;
        i -= 17;
        mse::CSize_t szt = 5;
        szt += 3;
        auto i2 = szt + i;
        mse::CBool b = false;
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

Note: Although these types have default initialization to ensure deterministic code, for variables of these types please continue to explicitly set their value before using them, as you would with their corresponding primitive types. If you would like a type that does not require explicit initialization before use, you can just publicly derive your own type from the appropriate class in this library.  
Also note: Numeric types with more comprehensive range checking can be found here: https://github.com/robertramey/safe_numerics.  
Also see the section on "[compatibility considerations](#compatibility-considerations)".


### Vectors

We provide three vectors - [mstd::vector<>](#vector), [msevector<>](#msevector) and [ivector<>](#ivector). mstd::vector<> is simply an almost completely safe implementation of std::vector<>.
msevector<> is also quite safe. Not quite as safe as mstd::vector<>, but it requires less overhead. msevector<> also supports a new kind of iterator in addition to the standard vector iterator. This new iterator, called "ipointer", acts more like a list iterator. It's more intuitive, more useful, and isn't prone to being invalidated upon an insert or delete operation. If performance is of concern, msevector<> is probably the better choice of the three.
ivector<> is just as safe as mstd::vector<>, but drops support for the (problematic) standard vector iterators and only supports the ipointer iterators.

### vector

mstd::vector<> is simply an almost completely safe implementation of std::vector<>.

usage example:

    #include "msemstdvector.h"
    #include <vector>
    
    int main(int argc, char* argv[]) {
    
        mse::mstd::vector<int> mv;
        std::vector<int> sv;
        /* These two vectors should be completely interchangeable. The difference being that mv should throw
        an exception on any attempt to access invalid memory. */
    }

### msevector

If you're willing to forego a little theoretical safety, msevector<> is still very safe without the overhead of memory management.  
In addition to the (high performance) standard vector iterator, msevector<> also supports a new kind of iterator, called "ipointer", that acts more like a list iterator in the sense that it points to an item rather than a position, and like a list iterator, it is not invalidated by insertions or deletions occurring elsewhere in the container, even if a "reallocation" occurs. In fact, standard vector iterators are so prone to being invalidated that for algorithms involving insertion or deletion, they can be generously considered not very useful, and more prudently considered dangerous. ipointers, aside from being safe, just make sense. Algorithms that work when applied to list iterators will work when applied to ipointers. And that's important as Bjarne famously points out, for cache coherency reasons, in most cases vectors should be used in place of lists, even when lists are conceptually more appropriate.  
msevector<> also provides a safe (bounds checked) version of the standard vector iterator.

usage example:

    #include "msemsevector.h"
    
    int main(int argc, char* argv[]) {
    
        mse::msevector<int> v = { 1, 2, 3, 4 };
        mse::msevector<int>::ipointer ip_vit1(v);
        /*ip_vit1.set_to_beginning();*/ /* This would be redundant as ipointers are set to the beginning at initialization. */
        ip_vit1.advance(2);
        assert(3 == ip_vit1.item());
        auto ip_vit2 = v.ibegin(); /* ibegin() returns an ipointer */
        v.erase(ip_vit2); /* remove the first item */
        assert(3 == ip_vit1.item());
        ip_vit1.set_to_previous();
        assert(2 == ip_vit1.item());
        
        /* Btw, ipointers are compatible with stl algorithms, like any other stl iterators. */
        std::sort(v.ibegin(), v.iend());
        ip_vit1 = v.ibegin();
        
        /* And just to be clear, mse::msevector<> retains it's original (high performance) stl::vector iterators. */
        std::sort(v.begin(), v.end());
        
        /* mse::msevector<> also provides "safe" (bounds checked) versions of the original stl::vector iterators. */
        std::sort(v.ss_begin(), v.ss_end());
    }

ipointers support all the standard iterator operators, but also have member functions with "friendlier" names including:

    bool points_to_an_item() const;
    bool points_to_end_marker() const;
    bool points_to_beginning() const;
    /* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
    bool has_next_item_or_end_marker() const;
    /* has_next() is just an alias for points_to_an_item() that may be familiar to java programmers. */
    bool has_next() const;
    bool has_previous() const;
    void set_to_beginning();
    void set_to_end_marker();
    void set_to_next();
    void set_to_previous();
    void advance(difference_type n);
    void regress(difference_type n);
    reference item() const { return operator*(); }
    reference previous_item() const;
    CSize_t position() const;
    void reset();

### ivector

ivector is for cases when safety and correctness are higher priorities than compatibility and performance. ivector, like mstd::vector<>, is almost completely safe. ivector takes the further step of dropping support for the (problematic) standard vector iterator, and replacing it with [ipointer](#msevector).

usage example:

    #include "mseivector.h"
    
    int main(int argc, char* argv[]) {
    
        mse::ivector<int> iv = { 1, 2, 3, 4 };
        std::sort(iv.begin(), iv.end());
        mse::ivector<int>::ipointer ivip = iv.begin();
    }


### Compatibility considerations

People have asked why the primitive C++ types can't be used as base classes - http://stackoverflow.com/questions/2143020/why-cant-i-inherit-from-int-in-c. It turns out that really the only reason primitive types weren't made into full-fledged classes is that they inherit these "chaotic" conversion rules from C that can't be fully mimicked by C++ classes, and Bjarne thought it would be too ugly to try to make special case classes that followed different conversion rules.  
But while substitute classes cannot be 100% compatible substitutes for their corresponding primitives, they can still be mostly compatible. And if you're writing new code or maintaining existing code, it should be considered good coding practice to ensure that your code is compatible with C++'s conversion rules for classes and not dependent on the "chaotic" legacy conversion rules of primitive types.

If you are using legacy code or libraries where it's not practical to update the code, it shouldn't be a problem to continue using primitive types there and the safer substitute classes elsewhere in the code. The safer substitute classes generally have no problem interacting with primitive types, although in some cases you may need to do some explicit type casting. Registered pointers can be cast to raw pointers, and, for example, CInt can participate in arithmetic operations with regular ints.

