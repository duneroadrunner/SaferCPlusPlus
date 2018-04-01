Mar 2018

### Overview

"SaferCPlusPlus" is essentially a collection of safe data types that are compatible with, and can substitute for, common unsafe native C++ types. You can use as much or as little of the collection as you like, but it is extensive enough to enable the elimination of the large majority of potentially unsafe memory accesses.

The library includes things like:

- Drop-in replacements for [std::vector<>](#vector), [std::array<>](#array) and [std::string](#string).

- Replacements for the "use-after-free" prone [std::string_view](#nrp_string_view).

- Drop-in [replacements](#primitives) for int, size_t and bool that ensure against the use of uninitialized values and address the "signed-unsigned mismatch" issues.

- Data types for safe, simple [sharing](#asynchronously-shared-objects) of objects among asynchronous threads.

- Replacements for native pointers/references with various compatibilty and performance trade-offs. 

C++ is (famously) not a memory-safe language. But the danger is limited to a finite subset of language elements. To be sure, the subset is large and includes key elements like pointers and arrays. But if you could somehow replace those elements with memory-safe substitutes, then you could essentially turn C++ into a memory-safe language. Right? So the question becomes:

Is C++ powerful enough to enable the construction of safe, compatible substitutes for its own unsafe elements?

Since the advent of C++11, the answer appears to be largely "yes". Sort of. There are some elements whose interface cannot be fully mimicked syntactically. C++ references in particular, as overloading of the "dot" operator is not yet supported. But even in that case, it's no sweat to create an alternative with equivalent functionality, with slightly different syntax.

So it seems like a reasonable strategy to pursue memory safety in C++ by simply replacing unsafe elements with memory-safe substitutes. (Note that this includes the unsafe [`this`](#practical-limitations) pointer.) The nice thing about this strategy is that it can be adopted partially and incrementally with corresponding partial and incremental safety benefits.

An important consideration for many C++ applications is performance. Preferably high and deterministic. This library strives for minimal run-time overhead and does not resort to garbage collection. To this end, the library provides extensive support for the strategy of using [scope lifetimes](#scope-pointers) to achieve memory safety with no run-time overhead.

You can have a look at [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp) to see the library in action. You can also check out some [benchmark code](https://github.com/duneroadrunner/SaferCPlusPlus-BenchmarksGame) where you can compare traditional C++ and SaferCPlusPlus implementations of the same algorithms.

Tested with msvc2017, msvc2015, g++5.3 and clang++3.8 (as of Mar 2018). Support for versions of g++ prior to version 5 was dropped on Mar 21, 2016.


### Table of contents
1. [Overview](#overview)
2. [Use cases](#use-cases)
3. [Setup and dependencies](#setup-and-dependencies)
4. Comparisons
    1. [SaferCPlusPlus versus Clang/LLVM Sanitizers](#safercplusplus-versus-clangllvm-sanitizers)
    2. [SaferCPlusPlus versus the Core Guidelines Checkers](#safercplusplus-versus-the-core-guidelines-checkers)
    3. [SaferCPlusPlus versus Rust](#safercplusplus-versus-rust)
    4. [SaferCPlusPlus versus Checked C](#safercplusplus-versus-checked-c)
    5. [SaferCPlusPlus versus Ironclad C++](#safercplusplus-versus-ironclad-c)
5. [Getting started on safening existing code](#getting-started-on-safening-existing-code)
6. [Registered pointers](#registered-pointers)
    1. [TRegisteredPointer](#tregisteredpointer)
        1. [TRegisteredNotNullPointer](#tregisterednotnullpointer)
        2. [TRegisteredFixedPointer](#tregisteredfixedpointer)
        3. [TRegisteredConstPointer](#tregisteredconstpointer-tregisterednotnullconstpointer-tregisteredfixedconstpointer)
        4. [TRegisteredRefWrapper](#tregisteredrefwrapper)
    2. [TRelaxedRegisteredPointer](#trelaxedregisteredpointer)
7. [Simple benchmarks](#simple-benchmarks)
8. [Reference counting pointers](#reference-counting-pointers)
    1. [TRefCountingPointer](#trefcountingpointer)
        1. [TRefCountingNotNullPointer](#trefcountingnotnullpointer)
        2. [TRefCountingFixedPointer](#trefcountingfixedpointer)
        3. [TRefCountingConstPointer](#trefcountingconstpointer-trefcountingnotnullconstpointer-trefcountingfixedconstpointer)
    2. [TRefCountingOfRegisteredPointer](#trefcountingofregisteredpointer)
    3. [TRefCountingOfRelaxedRegisteredPointer](#trefcountingofrelaxedregisteredpointer)
9. [Scope pointers](#scope-pointers)
    1. [TXScopeItemFixedPointer](#txscopeitemfixedpointer)
    2. [TXScopeOwnerPointer](#txscopeownerpointer)
    3. [make_xscope_strong_pointer_store()](#make_xscope_strong_pointer_store)
    4. [xscope_ifptr_to()](#xscope_ifptr_to)
    5. [xscope_chosen_pointer()](#xscope_chosen_pointer)
    6. [Conformance helpers](#conformance-helpers)
        1. [return_value()](#return_value)
        2. [TMemberObj](#tmemberobj)
10. [make_pointer_to_member_v2()](#make_pointer_to_member_v2)
11. [Poly pointers](#poly-pointers)
    1. [TXScopePolyPointer](#txscopepolypointer-txscopepolyconstpointer)
    2. [TPolyPointer](#tpolypointer-tpolyconstpointer)
    3. [TAnyPointer](#txscopeanypointer-txscopeanyconstpointer-tanypointer-tanyconstpointer)
    4. [TAnyRandomAccessIterator](#txscopeanyrandomaccessiterator-txscopeanyrandomaccessconstiterator-tanyrandomaccessiterator-tanyrandomaccessconstiterator)
    5. [TAnyRandomAccessSection](#txscopeanyrandomaccesssection-txscopeanyrandomaccessconstsection-tanyrandomaccesssection-tanyrandomaccessconstsection)
    6. [TAnyStringSection](#txscopeanystringsection-txscopeanystringconstsection-tanystringsection-tanystringconstsection)
    7. [TAnyNRPStringSection](#txscopeanynrpstringsection-txscopeanynrpstringconstsection-tanynrpstringsection-tanynrpstringconstsection)
12. [pointer_to()](#pointer_to)
12. [Safely passing parameters by reference](#safely-passing-parameters-by-reference)
13. [Multithreading](#multithreading)
    1. [TUserDeclaredAsyncPassableObj](#tuserdeclaredasyncpassableobj)
    2. [thread](#thread)
    3. [async()](#async)
    4. [Asynchronously shared objects](#asynchronously-shared-objects)
        1. [TUserDeclaredAsyncShareableObj](#tuserdeclaredasyncshareableobj)
        2. [TAsyncSharedV2ReadWriteAccessRequester](#tasyncsharedv2readwriteaccessrequester)
        3. [TAsyncSharedV2ReadOnlyAccessRequester](#tasyncsharedv2readonlyaccessrequester)
        4. [TAsyncSharedV2ImmutableFixedPointer](#tasyncsharedv2immutablefixedpointer)
        5. [TAsyncRASectionSplitter](#tasyncrasectionsplitter)
14. [Primitives](#primitives)
    1. [CInt, CSize_t and CBool](#cint-csize_t-and-cbool)
    2. [Quarantined types](#quarantined-types)
15. [Vectors](#vectors)
    1. [mstd::vector](#vector)
    2. [nii_vector](#nii_vector)
    3. [msevector](#msevector)
    4. [ivector](#ivector)
    5. [make_xscope_vector_size_change_lock_guard()](#make_xscope_vector_size_change_lock_guard)
16. [Arrays](#arrays)
    1. [mstd::array](#array)
    2. [nii_array](#nii_array)
    3. [msearray](#msearray)
    4. [xscope_iterator](#xscope_iterator)
    5. [xscope_pointer_to_array_element()](#xscope_pointer_to_array_element)
17. [for_each() specializations](#for_each-specializations)
18. [TRandomAccessSection](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection)
19. [Strings](#strings)
    1. [mstd::string](#string)
    2. [nii_string](#nii_string)
    3. [TStringSection](#txscopestringsection-txscopestringconstsection-tstringsection-tstringconstsection)
    4. [TNRPStringSection](#txscopenrpstringsection-txscopenrpstringconstsection-tnrpstringsection-tnrpstringconstsection)
    5. [mstd::string_view](#string_view)
    6. [nrp_string_view](#nrp_string_view)
20. [optional](#optional-xscope_optional)
21. [Compatibility considerations](#compatibility-considerations)
22. [Practical limitations](#practical-limitations)
23. [Questions and comments](#questions-and-comments)


### Use cases

This library is appropriate for use by two groups of C++ developers - those for whom safety and security are critical, and also everybody else. This library can help eliminate a lot of the opportunities for inadvertently accessing invalid memory or using uninitialized values. It essentially gets you [a lot](#practical-limitations) of the memory safety that you might get from say, Java, while retaining all of the power and most of the performance of C++.  

While using the library can incur a modest performance penalty, because the library elements are [largely compatible](#compatibility-considerations) with their native counterparts, they can be easily "disabled" (automatically replaced with their native counterparts) with a compile-time directive, allowing them to be used to help catch bugs in debug/test/beta builds while incurring no overhead in release builds.  

And note that the safe components of this library can be adopted completely incrementally. New code written with these safe elements will play nicely with existing (unsafe) code, and unsafe elements can be replaced selectively without breaking the existing code. So there is really no excuse for not using the library in pretty much any situation.  

Though for real time embedded applications, note the dependence on the standard library. Also, you may want to override the default behavior upon invalid memory operations (using MSE_CUSTOM_THROW_DEFINITION(x)).  

For more information on how to use the safe smart pointers in this library for maximum memory safety, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).

### Setup and dependencies

The beauty of the library is that it is so small and simple. Using the library generally involves copying the include files you want to use into your project, and that's it. Outside of the standard library, there are no other dependencies.  

A couple of notes about compling: With g++ and clang++, you'll need to link to the pthread library (-lpthread). You may also want to use the -Wno-unused flag with g++. With 64-bit builds in msvc you may get a "[fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj](https://msdn.microsoft.com/en-us/library/8578y171(v=vs.140).aspx)". Just [add](https://msdn.microsoft.com/en-us/library/ms173499.aspx) the "/bigobj" compile flag. For more help you can try the [questions and comments](#questions-and-comments) section.

### SaferCPlusPlus versus Clang/LLVM Sanitizers

The Clang/LLVM compiler provides a set of "sanitizers" (adopted by gcc) that address C/C++ "code safety" issues. While they address many of the same bugs, the solutions provided by the SaferCPlusPlus library and the Clang/LLVM sanitizers differ in significant ways (as of Sep 2016). Namely:

- The Clang/LLVM sanitizers require modifications to the build process, not the code, whereas with SaferCPlusPlus it's the other way around.
- SaferCPlusPlus can [more completely](https://en.wikipedia.org/wiki/AddressSanitizer#Limitations) solve the problem of invalid memory access, but does so by restricting what qualifies as "proper" SaferCPlusPlus code (eg. no pointer arithmetic).
- When encountering an invalid memory operation at run-time, the Clang/LLVM sanitizers terminate the executable, where SaferCPlusPlus, by default, throws a (catchable) exception, but supports any user-defined action, including program termination. 
- SaferCPlusPlus is portable C++ code that works on any platform, whereas Clang/LLVM sanitizers are available/maintained on a finite (but at the moment, ample) set of OS-architecture combinations.
- The Clang/LLVM sanitizers cost more in terms of run-time performance. [~2x slowdown](https://github.com/google/sanitizers/wiki/AddressSanitizerPerformanceNumbers) for the AddressSanitizer, for example. SaferCPlusPlus generally has substantially [lower performance costs](#simple-benchmarks), mainly because the Clang/LLVM sanitizers cannot assume any cooperation from the source code, so they have to instrument pretty much every allocated piece of memory and check pretty well every pointer dereference.
- SaferCPlusPlus supports the mixing of "safe" and (high-performance) "unsafe" code at a granular level, where Clang/LLVM Sanitizers apply to entire modules, or as in the case of the MemorySanitizer, all modules, requiring recompilation of any linked libraries.
- Clang's ThreadSanitizer tries to detect data race bugs, while SaferCPlusPlus provides [data types](#asynchronously-shared-objects) that eliminate the possibility of data race bugs (and a superset we call "object race" bugs).

Clang/LLVM Sanitizers are intended for debugging purposes, not to be used in deployed executables. As such, by design, some of their debugging convenience features themselves introduce [opportunities](http://seclists.org/oss-sec/2016/q1/363) for malicious exploitation. SaferCPlusPlus on the other hand, is designed to be used in deployed executables, as well as for debugging and testing. And that's reflected in its performance, security and "completeness of solution". So it's not really SaferCPlusPlus "versus" Clang/LLVM Sanitizers. They are not incompatible, and there's no reason you couldn't use both simultaneously, although there would be significant redundancies.

### SaferCPlusPlus versus the Core Guidelines Checkers

At this point (Dec 2017) the key difference between the Core Guidelines and SaferCPlusPlus is that the Core Guidelines does not achieve the same degree of memory safety.

Both are based on the "subset of a superset" idea. That is, many feel that the C++ language has accumulated too much (legacy) junk and it's better to stick to a nice subset of the language, augmented by additional libraries and tools. The subset that SaferCPlusPlus chose was the one that excludes elements that can potentially access invalid (or uninitialized) memory (like native pointers, `std::shared_ptr<>`, `std::array<>`, etc.). The subset that the Core Guidelines chose does include potentially unsafe elements (like native pointers and all of the standard library). 

At one point the premise was that those unsafe elements would be made safe via a static analysis tool that would detect any unsafe uses of unsafe elements, along with a reasonable proportion of false positives. It now seems that that premise may have been a little too optimistic. 

Consider this (simplified) example where we're passing a shared object by reference to a member function:

```cpp
    #include "gsl/gsl"
    #include <mutex>
    #include <memory>
    #include <vector>
    #include <iostream>
    
    // just an example class to be used as a shared object
    class CGA {
    public:
        std::vector<int> m_vec;
    };
    
    class CGB {
    public:
        void foo1(gsl::not_null<CGA*> obj_ptr) {
            // just some code that actually uses obj_ptr
            std::cout << obj_ptr->m_vec.size();
        }
    
        // just a static function that returns an std::shared_ptr<> to a presumably (but not actually) shared object
        static auto sharedptr_to_shared_obj() {
            return std::make_shared<CGA>();
        }
    };
    
    int main()
    {
        std::mutex mutex1;
        CGB b;
        {
            /* An example of the Core Guidelines sanctioned way of passing a shared object to a function
            by reference. */
    
            // first, store a copy of the std::shared_ptr<> in local scope
            std::shared_ptr<CGA> sharedobj_ptr = CGB::sharedptr_to_shared_obj();
            // then obtain a raw pointer to the object
            auto raw_ptr = sharedobj_ptr.get();
    
            {
                /* We're locking mutex1 to make sure no other threads access the object while the function is executing. */
                std::lock_guard<std::mutex> lock_guard1(mutex1);
    
                /* The local copy of the std::shared_ptr<> will ensure that the object will not be deallocated
                before the function is finished executing. */
                b.foo1(raw_ptr);
            }
        }
    
        return 0;
    }
```

The shared object is meant to represent an object that may be shared among threads, though for brevity we don't actually use more than one thread in the example.

The code is Core Guidelines compliant (as we understand it) and the Core Guidelines static checkers (in msvc2017) have no complaints.

Now consider a scenario where a programmer decides, in the member function, to store a copy of the pointer parameter (perhaps to implement some kind of cache to increase performance or whatever).

```cpp
    #include "gsl/gsl"
    #include <mutex>
    #include <memory>
    #include <vector>
    #include <iostream>
    
    // just an example class to be used as a shared object
    class CGA {
    public:
        std::vector<int> m_vec;
    };
    
    // a class that stores a reference to the last object passed to its foo1() member function 
    class CGB {
    public:
        void foo1(gsl::not_null<CGA*> obj_ptr) {
            m_last_used = obj_ptr; // storing a reference to the object
    
            // just some code that actually uses obj_ptr
            std::cout << obj_ptr->m_vec.size();
        }
    
        // just a static function that returns an std::shared_ptr<> to a presumably (but not actually) shared object
        static auto sharedptr_to_shared_obj() {
            return std::make_shared<CGA>();
        }
    
        CGA* m_last_used = nullptr;
    };
    
    int main()
    {
        std::mutex mutex1;
        CGB b;
        {
            /* An example of the Core Guidelines sanctioned way of passing a shared object to a function
            by reference. */
    
            // first, store a copy of the std::shared_ptr<> in local scope
            std::shared_ptr<CGA> sharedobj_ptr = CGB::sharedptr_to_shared_obj();
            // then obtain a raw pointer to the object
            auto raw_ptr = sharedobj_ptr.get();
    
            {
                /* We're locking mutex1 to make sure no other threads access the object while the function is executing. */
                std::lock_guard<std::mutex> lock_guard1(mutex1);
    
                /* The local copy of the std::shared_ptr<> will ensure that the object will not be deallocated
                before the function is finished executing. */
                b.foo1(raw_ptr);
            }
        }
        if (b.m_last_used) {
            /* The problem is that the function could have stored a reference to the object that will outlive
            the object and/or the period when it is safe (from data races) to access the shared object. */
    
            std::cout << b.m_last_used->m_vec.size();
    
            /* As of Dec 2017, the (msvc2017) Core Guidelines checkers do not catch these problems. */
        }
    
        return 0;
    }
```

Unfortunately, the changes make the code unsafe. The stored (raw) pointer can be used to (invalidly) access an object after it has been deallocated. This new unsafe program is, in essence, not Core Guidelines compliant. The Core Guidelines would have the function parameter be changed from a `gsl::not_null<CGA *>` to a `gsl::not_null< std::shared_ptr<CGA> >` and the pointer stored as an `std::shared_ptr<CGA>` (or `std::weak_ptr<CGA>`) rather than a raw pointer.

The problem is that, at this point, the Core Guidelines static checkers don't issue any complaints for this new unsafe version of the program. So even if the code becomes theoretically not Core Guidelines compliant, it's easy for a programmer to not realize it.

And while using `std::shared_ptr<>`s instead of raw pointers would address the premature deallocation issue, there is still the problem that the stored pointer to the shared object could be used to access the object outside of the period when the thread holds a lock on the mutex, potentially resulting in an unsafe "data race" condition. At this point the Core Guidelines don't really offer anything concrete to address the data race issue.

We can compare all this to what happens in the corresponding SaferCPlusPlus compliant implementation:

```cpp
    #include <vector>
    #include <iostream>
    #include "msetl/msepoly.h"
    
    // just an example class to be used as a shared object
    class CMA {
    public:
        /* nii_vector<> is just a vector that is safe (from data races) to share among threads (unlike std::vector<>). */
        mse::nii_vector<int> m_vec;
    };
    /* Here ShareableCMA is just a version of the user-defined CMA class that has been declared as safe
    to share among threads. */
    typedef mse::us::TUserDeclaredAsyncShareableObj<CMA> ShareableCMA;
    
    // a class that attempts to store a reference to the last object passed to its foo1() member function 
    class CMB {
    public:
        template <class _TShareableCMAPointer>
        void foo1(_TShareableCMAPointer obj_ptr) {
            /* If obj_ptr is of "scope pointer" type, the following line will result in a compile error 
             because scope pointers cannot be safely stored in a type that can outlive the scope. */
    
            m_last_used = mse::TNullableAnyPointer<ShareableCMA>(obj_ptr); // storing a reference to the object
    
            // just some code that actually uses obj_ptr
            std::cout << obj_ptr->m_vec.size();
        }
    
        // just a static function that returns a safe pointer to a presumably (but not actually) shared object
        static auto lock_pointer_to_shared_obj() {
            auto access_requester =  mse::make_asyncsharedreadwrite<ShareableCMA>();
            return access_requester.writelock_ptr();
        }
    
        /* TNullableAnyPointer<> can essentially hold a pointer of any type, similar to the way std::any can
        hold an object of any type. */
        mse::TNullableAnyPointer<ShareableCMA> m_last_used = nullptr;
    };
    
    int main()
    {
        CMB b;
        {
            /* An example of a SaferCPlusPlus sanctioned way of passing a shared object to a function
            by reference. */
    
            /* Here we're using a "strong pointer store" to hold a copy of the (shared owner) pointer to the
            shared object, in this case a "lock pointer", in the local scope. */
            auto xscope_writelock_ptr_store = mse::make_xscope_strong_pointer_store(CMB::lock_pointer_to_shared_obj());
    
            /* Since the local copy of the "lock pointer" ensures that it will be safe to access the shared object
            for the duration of the scope, we can obtain a "zero overhead" scope pointer to the shared object. */
            auto xscp_ptr = xscope_writelock_ptr_store.xscope_ptr();
    
            b.foo1(xscp_ptr);
        }
        if (b.m_last_used) {
            std::cout << b.m_last_used->m_vec.size();
        }
    
        return 0;
    }
```

With SaferCPlusPlus, the unsafe code results in a compile error. Instead of using raw pointers, "scope pointers" (which have no extra run-time overhead) are used, which means that the pointer has "scope lifetime" and that it will not outlive its target object. So attempting the to store the scope pointer (or any scope object) in a data type that could potentially outlive the scope results in a compile error.

And if we want the program to compile and run safely:

```cpp
    #include <vector>
    #include <iostream>
    #include "msetl/msepoly.h"
    
    // just an example class to be used as a shared object
    class CMA {
    public:
        /* nii_vector<> is just a vector that is safe (from data races) to share among threads (unlike std::vector<>). */
        mse::nii_vector<int> m_vec;
    };
    /* Here ShareableCMA is just a version of the user-defined CMA class that has been declared as safe
    to share among threads. */
    typedef mse::us::TUserDeclaredAsyncShareableObj<CMA> ShareableCMA;
    
    // a class that stores a reference to the last object passed to its foo1() member function 
    class CMB {
    public:
        template <class _TShareableCMAPointer>
        void foo1(_TShareableCMAPointer obj_ptr) {
            m_last_used = mse::TNullableAnyPointer<ShareableCMA>(obj_ptr); // storing a reference to the object
    
            // just some code that actually uses obj_ptr
            std::cout << obj_ptr->m_vec.size();
        }
    
        // just a static function that returns a safe pointer to a presumably (but not actually) shared object
        static auto lock_pointer_to_shared_obj() {
            auto access_requester =  mse::make_asyncsharedreadwrite<ShareableCMA>();
            return access_requester.writelock_ptr();
        }
    
        /* TNullableAnyPointer<> can essentially hold a pointer of any type, similar to the way std::any can
        hold an object of any type. */
        mse::TNullableAnyPointer<ShareableCMA> m_last_used = nullptr;
    };
    
    int main()
    {
        CMB b;
        {
            /* An example of a SaferCPlusPlus sanctioned way of passing a shared object to a function
            by (strong) reference. */
    
            b.foo1(CMB::lock_pointer_to_shared_obj());
        }
        if (b.m_last_used) {
            /* Like all conformant SaferCPlusPlus code, the stored object reference is safe. In this case the stored
            pointer holds shared ownership of both the target object's lifespan and "write access lock". */
            std::cout << b.m_last_used->m_vec.size();
        }
    
        return 0;
    }
```

Instead of `std::shared_ptr<>`s, we use "lock pointers". Like `std::shared_ptr<>`s, lock pointers have shared ownership of their target's lifespan, but unlike `std::shared_ptr<>`s, lock pointers also hold a lock that prevents any other thread from accessing the target object in a manner that could result in a data race.  

#### The problem with `std::shared_ptr<>`
Now, let's consider the Core Guidelines' decision to standardize on `std::shared_ptr<>` with, for example, its rule "F.27":

[`F.27: Use a shared_ptr<T> to share ownership`](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rf-shared_ptr)

Uses of reference counting pointers can be divided into two categories - one where the object is shared between asynchronous threads, and one where it isn't.

For the latter case, `std::shared_ptr<>`s are unnecessarily costly due to their thread-safe reference counting mechanism. For the former case, they are insufficiently safe. While they possess shared ownership of the object's lifespan to automatically ensure that dereferences do not access deallocated memory, they have no mechanism to automatically ensure dereferences do not inappropriately access memory that is being used by another thread. (I.e. They protect against "use-after-free" bugs, but not "data race" bugs.)

So why choose a pointer type that is unsuitable for both use cases rather than two different pointer types that are more suitable for each case? Perhaps because they didn't want to force developers to make separate interfaces and implementations for each of the two use case categories? But that doesn't really make sense as there are perfectly good ways in C++ to have a single interface and implementation support different pointer types. In cases where you're only dealing with one type of pointer at a time (which would be the vast majority of cases) you can just make the function in question a function template. For other cases, you can use polymorphic pointers. (Basically the pointer specialized versions of `std::variant<>` and/or `std::any<>`.) The SaferCPlusPlus library provides such [polymorphic pointers](#poly-pointers).

The nice thing about the way SaferCPlusPlus does shared ownership is that it conforms to the "only pay for what you use" principle. Standardizing on `std::shared_ptr<>`s not only makes you pay for features that you may not be using, but it prevents you from accessing essential (safety) features when you need them, no matter how much they are worth to you.

The following table considers all pointer use cases, partitioned into relevant categories, and compares the pointer types prescribed for each use case by the Core Guidelines and SaferCPlusPlus, noting safety and performance issues. Note that the "[scope](#scope-pointers)" adjective is used to indicate that the item will be deallocated at the end of the execution scope (sometimes called "block") in which it was declared. (I.e. basically a "local variable".):

#### Pointer use case comparison table

Pointer use case | Core Guidelines | SaferCPlusPlus
----------------- | --------------- | --------------
strong pointer to mutable object shared between threads 		| shared_ptr [A]	| [lock pointer](#asynchronously-shared-objects)
strong pointer to mutable object shared within a thread 		| shared_ptr [a]	| [refcounting pointer](#reference-counting-pointers)
strong pointer to immutable object shared between threads 		| shared_ptr 		| [async immutable pointer](#tasyncsharedv2immutablefixedpointer)
strong pointer to immutable object shared within a thread 		| shared_ptr [a]	| refcounting pointer
scope reference to mutable object shared between threads 		| raw pointer [A]	| [scope pointer](#scope-pointers)
scope reference to shared object (other) 				| raw pointer		| scope pointer
non-scope (weak) reference to object shared between threads 		| weak_ptr 		| not yet supported directly
non-scope (weak) reference to object shared within a thread 		| weak_ptr [Da]		| [registered pointer](#registered-pointers) [b]
unique strong pointer with scope lifetime 				| unique_ptr [C] 	| [scope owner pointer](#txscopeownerpointer)
unique strong pointer with non-scope lifetime 				| unique_ptr [C]	| refcounting pointer
scope reference to uniquely owned object 				| raw pointer [C] 	| scope pointer
non-scope (weak) reference to uniquely owned object 			| raw pointer [BC]	| registered pointer [b]
scope reference to scope object / local variable 			| raw pointer 		| scope pointer
pointer to scope object / local variable (other) 			| raw pointer [B]	| registered pointer [b] (discouraged)

```
potential safety issues:
[A] data race
[B] use-after-free and/or use-after-scope
[C] use-after-move
[D] inadvertent use of (unsafe) raw pointer instead of (safe) weak_ptr is likely and currently not caught by checkers

performance issues:
[a] unnecessary thread-safety mechanism
[b] expensive assignment (including when done at construction)

Note it is assumed that `gsl::not_null<>` and `const` will be used where appropriate.
And in many cases (raw) references could/would be used in place of raw pointers, but the same safety issues apply.
```

It's interesting to note that, despite the fact that they were designed independently, the set of pointer types provided by SaferCPlusPlus roughly correspond to those of the [Rust](#safercplusplus-versus-rust) language. Which perhaps makes sense as both use the strategy of, as much as possible, exploiting scope lifetimes to achieve memory safety without extra run-time overhead. And both prioritize memory safety (and data race safety) without resorting to garbage collection.

### SaferCPlusPlus versus Rust

C++ and Rust differ significantly in many ways, but SaferCPlusPlus is primarily concerned with addressing memory safety so here we'll consider only that aspect. Given that, what's most notable is the similarities between SaferCPlusPlus and Rust, considering they were developed independently. 

Probably the main issue when it comes to memory safety is the relationship between pointers/references and object deallocation. So it's interesting that Rust and SaferCPlusPlus decided on roughly corresponding sets of (safe) pointer/reference types:

Rust | SaferCPlusPlus
---- | --------------
non-reassignable reference | scope pointer
reassignable (mut) reference | registered pointer
Box<> | scope owner pointer
Rc<> | reference counting pointer
Arc<> | shared immutable pointer
Arc< RwLock<> > | access requester

Indeed, if you are a Rust programmer you might be more comfortable using SaferCPlusPlus than traditional C++.

The most commonly used reference type, the non-reassignable (non-mut) reference in Rust and the scope pointer in SaferCPlusPlus, incurs no run-time overhead in both cases, which is a primary reason for the excellent performance of both solutions. 

Reassignable (mut) references occur much less frequently, but still have no run-time overhead in Rust. Since SaferCPlusPlus does not (yet) have a "borrow checker", it has to resort to run-time checks to ensure memory safety in this case. But in exchange for the overhead, SaferCPlusPlus' registered pointers get essentially all the flexibility of native pointers (minus pointer arithmetic).

Probably the biggest difference though, is that SaferCPlusPlus does not restrict the number and type of references to an object that can exist at one time (i.e. the "exclusivity of mutable references") the way Rust does. With respect to memory safety, the benefit of this restriction is that it ensures that objects with "arbitrary lifespan" (like an element in a (resizable) vector) are not deallocated while other references to that object still exist.

But most objects do not have "arbitrary lifespan". (Both Rust and SaferCPlusPlus encourage most objects to have "scope lifespan".) So most of the time, from a memory safety perspective, this restriction is not necessary. It's hard to evaluate the cost of this restriction. There's arguably some ergonomic cost, but one concrete example might be the fact that in Rust, mutable "reference counting" targets need to be wrapped in a `Cell` or `RefCell` (which introduces run-time overhead and/or the possibility of a panic). But in Rust, this restriction is about more than just memory safety, and whether or not the overall benefits of the restriction outweigh the costs is probably situation dependent.

So, perhaps as expected, you could think of the comparison between SaferCPlusPlus and Rust as essentially the comparison between C++ and Rust, with diminished discrepancies in memory safety and performance.

### SaferCPlusPlus versus Checked C

"Checked C", like SaferCPlusPlus, takes the approach of extending the language with safer elements that can directly substitute for unsafe native elements. In chapter 9 of their [spec](https://github.com/Microsoft/checkedc/releases/download/v0.5-final/checkedc-v0.5.pdf), there is an extensive survey of existing (and historical) efforts to address C/C++ memory safety. There they make the argument for the (heretofore neglected) "language extension" approach (basically citing performance, compatibility and the support for granular mixing of safe and unsafe code), that applies to SaferCPlusPlus as well.

Checked C and SaferCPlusPlus are more complementary than competitive. Checked C targets low-level system C code and basically only addresses the array bounds checking issue, including pointer arithmetic, where SaferCPlusPlus skews more toward C++ code and legacy code that would benefit from being converted to modern C++. It seems that Checked C is not yet ready for deployment (as of Sep 2016), but one could imagine both solutions being used, with little contention, in projects that have both low-level system type code and higher-level application type code.

### SaferCPlusPlus versus Ironclad C++

SaferCPlusPlus and Ironclad C++ are very similar. The main difference is probably that Ironclad uses garbage collection while SaferCPlusPlus does not. They are not incompatible, both libraries could be used in the same project. Unfortunately, Ironclad seems to be no longer under active development.  

While both solutions address the pointer/reference safety issue, SaferCPlusPlus also provides safer replacements for int and size_t, and data types for safely sharing objects between asynchronous threads.  

There is a comprehensive paper on Ironclad C++ [here](https://www.cs.rutgers.edu/~santosh.nagarakatte/papers/ironclad-oopsla2013.pdf). It's a beneficial read even for those not planning on adopting Ironclad, as the the approach has much in common with SaferCPlusPlus.  

### Getting started on safening existing code

The elements in this library are straightforward enough that a separate tutorial, beyond the examples given in the documentation, is probably not necessary. But if you're wondering how best to start, probably the easiest and most effective thing to do is to replace the vectors and arrays in your code (that aren't being shared between threads) with [`mse::mstd::vector<>`](#vector) and [`mse::mstd::array<>`](#array).

The header files you'll need to include in your source file are "`msemstdvector.h`" and "`msemstdarray.h`". Those include files have additional dependencies on "`msemsevector.h`", "`msemsearray.h`", "`msescope.h`", and possibly "`mseprimitives.h`".

Statistically speaking, doing this should already catch a significant chunk of potential memory bugs. By default, an exception will be thrown upon any attempt to access invalid memory. If your project is not using C++ exceptions, you'll probably want to override the default exception behavior by defining the `MSE_CUSTOM_THROW_DEFINITION()` preprocessor macro prior to inclusion of the header files. For example:

```cpp
    #define MSE_CUSTOM_THROW_DEFINITION(x) std::cerr << std::endl << x.what(); exit(-11)
```

will cause the error description to be written to stderr before program termination.

The next most effective thing to do, in terms of improving memory safety, is probably to replace calls to `new`/`malloc` and `delete`/`free`. The direct substitutes provided in the library (for items not shared between threads) are `mse::registered_new()` and `mse::registered_delete()`. The pointer type returned by `mse::registered_new()` is an [`mse::TRegisteredPointer<>`](#tregisteredpointer). If you need this pointer to interact with legacy interfaces, it can be explicitly cast to a corresponding native pointer. If explicit casting is too inconvenient for your situation, you may instead use `mse::relaxedregistered_new()` to obtain an [`mse::TRelaxedRegisteredPointer<>`](#trelaxedregisteredpointer), which implicitly converts to a corresponding native pointer. But ultimately you're going to want to minimize the amount of casting to (unsafe) native pointers by updating your (function) interfaces to accomodate these safe pointers directly. (See the "[Safely passing parameters by reference](#safely-passing-parameters-by-reference)" section.)

Based on reported vulnerabilities, these two things alone should catch most memory bugs.

While the library provides these direct substitutes for `new`/`malloc` and `delete`/`free`, they are usually not the optimal solution. In most cases, you can instead use [`mse::TXScopeOwnerPointer<>`](#txscopeownerpointer) or [`TRefCountingNotNullPointer<>`](#trefcountingnotnullpointer), which are faster and automatically deallocate the item for you.

For items shared between asynchronous threads, use one of the [data types designed for safe asynchronous sharing](#asynchronously-shared-objects).

After that, it's just a matter of replacing the remaining unsafe elements in your code (generally native pointers and references) with the safer substitute that works best. You might want to leave C++ references for last, because a) they seem to be empirically (if not theoretically) less prone to bugs than pointers, and b) the library does not provide a directly compatible substitute (although [`TRegisteredRefWrapper<>`](#tregisteredrefwrapper) can be used in some situations), so references generally have to be substituted with pointers, which involves the extra bit of work of changing your dots to arrows.

And if at some point you feel that these new elements involve a lot of typing, note that many of the elements have short aliases that can be used instead. Just search for "shorter aliases" in the header files. Or, of course, you can create your own to suit your preferences.

### Registered pointers

"Registered" pointers are intended to behave just like native C++ pointers, except that their value is (automatically) set to nullptr when the target object is destroyed. And by default they will throw an exception upon any attempt to dereference a nullptr. Because they don't take ownership like some other smart pointers, they can point to objects allocated on the stack as well as the heap. In most cases, they can be used as a compatible, direct substitute for native pointers, making it straightforward to update legacy code (to be safer).

Registered pointers come in two flavors - [`TRegisteredPointer<>`](#tregisteredpointer) and [`TRelaxedRegisteredPointer<>`](#trelaxedregisteredpointer). They are both very similar. `TRegisteredPointer<>` emphasizes speed and safety a bit more, while `TRelaxedRegisteredPointer<>` emphasizes compatibility and flexibility a bit more. If you want to undertake the task of en masse replacement of native pointers in legacy code, or need to interact with legacy native pointer interfaces, `TRelaxedRegisteredPointer<>` may be more convenient.

Note that these registered pointers cannot target types that cannot act as base classes. The primitive types like int, bool, etc. [cannot act as base classes](#compatibility-considerations). Fortunately, the library provides safer [substitutes](#primitives) for `int`, `bool` and `size_t` that can act as base classes. Also note that these registered pointers are not thread safe. When you need to share objects between asynchronous threads, you can use the [safe sharing data types](#asynchronously-shared-objects) in this library. For more information on how to use the safe smart pointers in this library for maximum memory safety, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).

Although registered pointers are more general and flexible, it's expected that [scope pointers](#scope-pointers) will actually be more commonly used. At least in cases where performance is important. While more restricted than registered pointers, by default they have no run-time overhead.  


### TRegisteredPointer

usage example:

```cpp
    #include "mseregistered.h"
    
    void main(int argc, char* argv[]) {
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
```

### TRegisteredNotNullPointer
Same as `TRegisteredPointer<>`, but cannot be constructed to a null value.

### TRegisteredFixedPointer
Same as `TRegisteredNotNullPointer<>`, but cannot be retargeted after construction (basically a "`const TRegisteredNotNullPointer<>`"). It is essentially a functional equivalent of a C++ reference and is a recommended type to be used for safe parameter passing by reference.  

usage example:

```cpp
    #include "mseregistered.h"
    
    void main(int argc, char* argv[]) {
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
```

### TRegisteredConstPointer, TRegisteredNotNullConstPointer, TRegisteredFixedConstPointer
`TRegisteredPointer<X>` does implicitly convert to `TRegisteredPointer<const X>`. But some prefer to think of the pointer giving "const" access to the object rather than giving access to a "const object".

### TRegisteredRefWrapper
Just a registered version of [`std::reference_wrapper<>`](http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper).  

usage example:

```cpp
    #include "mseprimitives.h"
    #include "mseregistered.h"

    void main(int argc, char* argv[]) {
        /* This example originally comes from http://www.cplusplus.com/reference/functional/reference_wrapper/. */
        mse::TRegisteredObj<mse::CInt> a(10), b(20), c(30);
        // an array of "references":
        mse::TRegisteredRefWrapper<mse::CInt> refs[] = { a,b,c };
        std::cout << "refs:";
        for (mse::CInt& x : refs) std::cout << ' ' << x;
        std::cout << '\n';
    
        mse::TRegisteredObj<mse::CInt> foo(10);
        auto bar = mse::registered_ref(foo);
        ++(mse::CInt&)bar;
        std::cout << foo << '\n';
    }
```

### TRelaxedRegisteredPointer

usage example:

```cpp
    #include "mserelaxedregistered.h"
    
    void main(int argc, char* argv[]) {
    
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
```

### TRelaxedRegisteredNotNullPointer

### TRelaxedRegisteredFixedPointer

### TRelaxedRegisteredConstPointer, TRelaxedRegisteredNotNullConstPointer, TRelaxedRegisteredFixedConstPointer

### Simple benchmarks

Just some simple microbenchmarks of the pointers. (Some less "micro" benchmarks of the library in general can be found [here](https://github.com/duneroadrunner/SaferCPlusPlus-BenchmarksGame).) We show the results for msvc2015 and msvc2013 (run on the same machine), since there are some interesting differences. The source code for these benchmarks can be found in the file [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp).

#### Allocation, deallocation, pointer copy and assignment:

##### platform: msvc2015/default optimizations/x64/Windows7/Haswell (Mar 2016):

Pointer Type | Time
------------ | ----
[mse::TRegisteredPointer](#tregisteredpointer) (stack): | 0.0317188 seconds.
native pointer (heap): | 0.0394826 seconds.
[mse::TRefCountingPointer](#trefcountingpointer) (heap): | 0.0493629 seconds.
mse::TRegisteredPointer (heap): | 0.0573699 seconds.
std::shared_ptr (heap): | 0.0692405 seconds.
[mse::TRelaxedRegisteredPointer](#trelaxedregisteredpointer) (heap): | 0.14475 seconds.

##### platform: msvc2013/default optimizations/x64/Windows7/Haswell (Jan 2016):

Pointer Type | Time
------------ | ----
mse::TRegisteredPointer (stack): | 0.0270016 seconds.
native pointer (heap): | 0.0490028 seconds.
mse::TRegisteredPointer (heap): | 0.0740042 seconds.
std::shared_ptr (heap): | 0.087005 seconds.
mse::TRelaxedRegisteredPointer (heap): | 0.142008 seconds.

Take these results with a grain of salt. The benchmarks were run on a noisy machine, and anyway don't represent realistic usage scenarios. But I'm guessing the general gist of the results is valid. Interestingly, three of the scenarios seemed to have gotten noticeably faster between msvc2013 and msvc2015.  

I'm speculating here, but it might be the case that the heap operations that occur in this benchmark may be more "cache friendly" than heap operations in real world code would be, making the "heap" results look artificially good (relative to the "stack" result).

#### Dereferencing:

##### platform: msvc2015/default optimizations/x64/Windows7/Haswell (Mar 2016):

Pointer Type | Time
------------ | ----
native pointer: | 0.0105804 seconds.
mse::TRelaxedRegisteredPointer unchecked: | 0.0136354 seconds.
mse::TRefCountingPointer (checked): | 0.0258107 seconds.
mse::TRelaxedRegisteredPointer (checked): | 0.0308289 seconds.
std::weak_ptr: | 0.179833 seconds.

##### platform: msvc2013/default optimizations/x64/Windows7/Haswell (Jan 2016):

Pointer Type | Time
------------ | ----
native pointer: | 0.0100006 seconds.
mse::TRelaxedRegisteredPointer unchecked: | 0.0130008 seconds.
mse::TRelaxedRegisteredPointer (checked): | 0.016001 seconds.
std::weak_ptr: | 0.17701 seconds.

The interesting thing here is that checking for nullptr seems to have gotten a lot slower between msvc2013 and msvc2015. But anyway, my guess is that pointer dereferencing is such a fast operation (std::weak_ptr aside) that outside of critical inner loops, the overhead of checking for nullptr would generally be probably pretty modest.  

Also note that [`mse::TRefCountingNotNullPointer<>`](#trefcountingnotnullpointer) and [`mse::TRefCountingFixedPointer<>`](#trefcountingfixedpointer) always point to a validly allocated object, so their dereferences don't need to be checked. `mse::TRegisteredPointer<>`'s safety mechanisms are not compatible with the techniques used by the benchmark to isolate dereferencing performance, but `mse::TRegisteredPointer<>`'s dereferencing performance would be expected to be essentially identical to that of `mse::TRelaxedRegisteredPointer<>`. By default, [scope pointers](#scope-pointers) have identical performance to native pointers.

### Reference counting pointers

If you're going to use pointers, then to ensure they won't be used to access invalid memory you basically have two options - detect any attempt to do so and throw an exception, or, alternatively, ensure that the pointer targets a validly allocated object. Registered pointers rely on the former, and so-called "reference counting" pointers can be used to achieve the latter. The most famous reference counting pointer is `std::shared_ptr<>`, which is notable for its thread-safe reference counting that can be handy when you're sharing an object among asynchronous threads, but is unnecessarily costly when you aren't. So we provide fast reference counting pointers that forego any thread safety mechanisms. In addition to being substantially faster (and smaller) than `std::shared_ptr<>`, they are a bit more safety oriented in that they they don't support construction from raw pointers. (Use `mse::make_refcounting<>()` instead.) "Const", "not null" and "fixed" (non-retargetable) flavors are also provided with proper conversions between them. For more information on how to use the safe smart pointers in this library for maximum memory safety, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).


### TRefCountingPointer

usage example:

```cpp
	#include "mserefcounting.h"
	
	void main(int argc, char* argv[]) {
		class A {
		public:
			A() {}
			A(const A& _X) : b(_X.b) {}
			virtual ~A() {
				int q = 3; /* just so you can place a breakpoint if you want */
			}
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
		};
		typedef std::vector<mse::TRefCountingFixedPointer<A>> CRCFPVector;
		class B {
		public:
			static int foo1(mse::TRefCountingPointer<A> A_refcounting_ptr, CRCFPVector& rcfpvector_ref) {
				rcfpvector_ref.clear();
				int retval = A_refcounting_ptr->b;
				A_refcounting_ptr = nullptr; /* Target object is destroyed here. */
				return retval;
			}
		protected:
			~B() {}
		};

		{
			CRCFPVector rcfpvector;
			{
				mse::TRefCountingFixedPointer<A> A_refcountingfixed_ptr1 = mse::make_refcounting<A>();
				rcfpvector.push_back(A_refcountingfixed_ptr1);

				/* Just to demonstrate conversion between refcounting pointer types. */
				mse::TRefCountingConstPointer<A> A_refcountingconst_ptr1 = A_refcountingfixed_ptr1;
			}
			B::foo1(rcfpvector.front(), rcfpvector);
		}
	}
```

### TRefCountingNotNullPointer

Same as `TRefCountingPointer<>`, but cannot be constructed to or assigned a null value. Because `TRefCountingNotNullPointer<>` controls the lifetime of its target it, should be always safe to assume that it points to a validly allocated object.

### TRefCountingFixedPointer

Same as `TRefCountingNotNullPointer<>`, but cannot be retargeted after construction (basically a "`const TRefCountingNotNullPointer<>`"). It is a recommended type to be used for safe parameter passing by reference.

### TRefCountingConstPointer, TRefCountingNotNullConstPointer, TRefCountingFixedConstPointer

`TRefCountingPointer<X>` actually does implicitly convert to `TRefCountingPointer<const X>`. But some prefer to think of the pointer giving "const" access to the object rather than giving access to a "const object".


### TRefCountingOfRegisteredPointer

`TRefCountingOfRegisteredPointer<>` is simply an alias for `TRefCountingPointer< TRegisteredObj<_Ty> >`. `TRegisteredObj<_Ty>` is meant to behave much like, and be compatible with a `_Ty`. The reason why we might want to use it is because the `&` ("address of") operator of `TRegisteredObj<_Ty>` returns a [`TRegisteredFixedPointer<_Ty>`](#tregisteredfixedpointer) rather than a raw pointer, and `TRegisteredPointer<>`s can serve as safe "weak pointers".  

usage example:  

```cpp
    #include "mserefcountingofregistered.h"
    
    class H {
    public:
        /* An example of a templated member function. In this case it's a static one, but it doesn't have to be.
        You might consider templating pointer parameter types to give the caller some flexibility as to which kind of
        (smart/safe) pointer they want to use. */
    
        template<typename _Tpointer, typename _Tvector>
        static int foo5(_Tpointer A_ptr, _Tvector& vector_ref) {
            int tmp = A_ptr->b;
            int retval = 0;
            vector_ref.clear();
            if (A_ptr) {
                retval = A_ptr->b;
            }
            else {
                retval = -1;
            }
            return retval;
        }
    protected:
        ~H() {}
    };
    
    void main(int argc, char* argv[]) {
        class A {
        public:
            A() {}
            A(const A& _X) : b(_X.b) {}
            virtual ~A() {
                int q = 3; /* just so you can place a breakpoint if you want */
            }
            A& operator=(const A& _X) { b = _X.b; return (*this); }

            int b = 3;
        };
        typedef std::vector<mse::TRefCountingOfRegisteredFixedPointer<A>> CRCRFPVector;
    
        {
            CRCRFPVector rcrfpvector;
            {
                mse::TRefCountingOfRegisteredFixedPointer<A> A_refcountingofregisteredfixed_ptr1 = mse::make_refcountingofregistered<A>();
                rcrfpvector.push_back(A_refcountingofregisteredfixed_ptr1);
    
                /* Just to demonstrate conversion between refcountingofregistered pointer types. */
                mse::TRefCountingOfRegisteredConstPointer<A> A_refcountingofregisteredconst_ptr1 = A_refcountingofregisteredfixed_ptr1;
            }
            int res1 = H::foo5(rcrfpvector.front(), rcrfpvector);
            assert(3 == res1);
    
            rcrfpvector.push_back(mse::make_refcountingofregistered<A>());
            /* The first parameter in this case will be a TRegisteredFixedPointer<A>. */
            int res2 = H::foo5(&(*rcrfpvector.front()), rcrfpvector);
            assert(-1 == res2);
        }
    }
```

### TRefCountingOfRegisteredNotNullPointer, TRefCountingOfRegisteredFixedPointer

### TRefCountingOfRegisteredConstPointer, TRefCountingOfRegisteredNotNullConstPointer, TRefCountingOfRegisteredFixedConstPointer

### TRefCountingOfRelaxedRegisteredPointer

`TRefCountingOfRelaxedRegisteredPointer<>` is simply an alias for `TRefCountingPointer< TRelaxedRegisteredObj<_Ty> >`. Generally you should prefer to just use `TRefCountingOfRegisteredPointer<>`, but if you need a "weak pointer" to refer to a type before it's fully defined then you can use this type. An example of such a situation is when you have so-called "cyclic references".  

usage example:  

```cpp
    #include "mserefcountingofrelaxedregistered.h"
    
    void main(int argc, char* argv[]) {
    
        /* Here we demonstrate using TRelaxedRegisteredFixedPointer<> as a safe "weak_ptr" to prevent "cyclic references" from
        becoming memory leaks. */
    
        class CRCNode {
        public:
            CRCNode(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr
                , mse::TRelaxedRegisteredPointer<CRCNode> root_ptr) : m_node_count_ptr(node_count_ptr), m_root_ptr(root_ptr) {
                (*node_count_ptr) += 1;
            }
            CRCNode(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr) : m_node_count_ptr(node_count_ptr) {
                (*node_count_ptr) += 1;
            }
            virtual ~CRCNode() {
                (*m_node_count_ptr) -= 1;
            }
            static mse::TRefCountingOfRelaxedRegisteredFixedPointer<CRCNode> MakeRoot(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr) {
                auto retval = mse::make_refcountingofrelaxedregistered<CRCNode>(node_count_ptr);
                (*retval).m_root_ptr = &(*retval);
                return retval;
            }
            mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> ChildPtr() const { return m_child_ptr; }
            mse::TRefCountingOfRelaxedRegisteredFixedPointer<CRCNode> MakeChild() {
                auto retval = mse::make_refcountingofrelaxedregistered<CRCNode>(m_node_count_ptr, m_root_ptr);
                m_child_ptr = retval;
                return retval;
            }
            void DisposeOfChild() {
                m_child_ptr = nullptr;
            }
    
        private:
            mse::TRegisteredFixedPointer<mse::CInt> m_node_count_ptr;
            mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> m_child_ptr;
            mse::TRelaxedRegisteredPointer<CRCNode> m_root_ptr;
        };
    
        mse::TRegisteredObj<mse::CInt> node_counter = 0;
        {
            mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> root_ptr = CRCNode::MakeRoot(&node_counter);
            auto kid1 = root_ptr->MakeChild();
            {
                auto kid2 = kid1->MakeChild();
                auto kid3 = kid2->MakeChild();
            }
            assert(4 == node_counter);
            kid1->DisposeOfChild();
            assert(2 == node_counter);
        }
        assert(0 == node_counter);
    }
```

### TRefCountingOfRelaxedRegisteredNotNullPointer, TRefCountingOfRelaxedRegisteredFixedPointer

### TRefCountingOfRelaxedRegisteredConstPointer, TRefCountingOfRelaxedRegisteredNotNullConstPointer, TRefCountingOfRelaxedRegisteredFixedConstPointer

### Scope pointers
Scope pointers point to scope objects. Scope objects are essentially objects that are allocated on the stack, or whose "owning" pointer is allocated on the stack. So the object is destroyed when it, or its owner, goes out of scope. In C++ at the moment, there isn't really a good way for a program to determine at compile time whether an object is allocated on the stack or not, so in order to exploit the properties of stack allocated objects, the library needs you to explicitly declare when an object is stack allocated. You do this by wrapping the type in the `mse::TXScopeObj<>` (transparent) wrapper template.

Note that you do not need to do this for all objects allocated on the stack. Just the ones for which you want to obtain a scope pointer. The reason you might want a scope pointer, as opposed to say, a registered pointer, is that, by default, scope pointers have no run-time overhead. In fact, it is expected that ultimately, scope pointers will be by far the most commonly used of the pointers provided by the library. 

In the future we expect that there will be a "compile helper tool" to verify that objects declared as scope objects are indeed allocated on the stack and used properly. For now, be careful to follow these rules:

- Objects of scope type (types whose name starts with "TXScope" or "xscope") must be global or local (non-static) automatic variables.
	- Basically global or allocated on the stack.
- Note that scope pointers are themselves scope objects and must adhere to the same restrictions.
- Do not use scope types as members of classes or structs.
	- Note that you can use the [`mse::make_pointer_to_member()`](#make_pointer_to_member) function to obtain a scope pointer to a member of a scope object. So it's generally not necessary for any class/struct member to be declared as a scope object.
	- In the uncommon cases that you really want to use a scope type as a member of a class or struct, that class or struct must itself be a scope type. User defined scope types must adhere to the [rules](#defining-your-own-scope-types) of scope types.
- Do not use scope types as base classes.
	- There probably isn't much motivation to do this anyway.
	- In the uncommon cases that you really want to use a scope type as a base class/struct, the derived class/struct must itself be a scope type. User defined scope types must adhere to the [rules](#defining-your-own-scope-types) of scope types.
- Do not use scope types as function return types.
	- In the uncommon cases that you really want to use a scope type as a function return type, it must be wrapped in the [`mse::TXScopeReturnValue<>`](#txscopereturnable) transparent template wrapper.
	- `mse::TXScopeReturnValue<>` will not accept non-owning scope pointer types. Pretty much the only time you would legitimately want to return a non-owning pointer to a scope object is when that pointer is one of the function's input parameters. In those cases you can use the [`xscope_chosen_pointer()`](#xscope_chosen_pointer) function.

Failure to adhere to the rules for scope objects could result in unsafe code. Currently, most, but not all, inadvertent misuses of scope objects should result in compile errors. Again, at some point the restrictions will be fully enforced at compile-time, but for now hopefully these rules are intuitive enough that adherence should be fairly natural. Just remember that the safety of scope pointers is premised on the fact that scope objects are never deallocated before the end of the scope in which they are declared, and (non-owning) scope pointers (and any copies of them) never survive beyond the scope in which they are declared, so that a scope pointer cannot outlive its target scope object.

In lieu of full compile-time enforcement, run-time checking is available to enforce safety and help detect misuses of scope pointers. Run-time checking in debug mode is enabled by defining `MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED`. Additionally defining `MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED` will enable them in non-debug modes as well. And as with registered pointers, scope pointers cannot target types that cannot act as a base class. For `int`, `bool` and `size_t` use the safer [substitutes](#primitives) that can act as base classes. 

Generally, there are two types of scope pointers you might use, [`TXScopeOwnerPointer<>`](#txscopeownerpointer) and [`TXScopeItemFixedPointer<>`](#txscopeitemfixedpointer). `TXScopeOwnerPointer<>` is similar to `boost::scoped_ptr<>` in functionality (but more limited in intended use). It creates an instance of a given class on the heap and destroys that instance in its destructor. (We use "scope" to mean "execution scope", where in boost it seems to also include "declaration scope".)
`TXScopeItemFixedPointer<>` is a "non-owning" pointer to scope objects. It is (intentionally) limited in its functionality, and is primarily intended for the purpose of passing scope objects by reference as function arguments. 

### TXScopeItemFixedPointer
`TXScopeItemFixedPointer<>` is primarily intended to be used to pass scope objects by reference as function arguments. It should not be used as a function return type, as that could be unsafe. And as with any other scope object, it should not be used as a member of any class or struct that is not itself a scope object (though attempting to do so would generally produce a compile error).  

usage example:

```cpp
    #include "msescope.h"
    
    void main(int argc, char* argv[]) {
        class A {
        public:
            A(int x) : b(x) {}
            A(const A& _X) : b(_X.b) {}
            virtual ~A() {}
            A& operator=(const A& _X) { b = _X.b; return (*this); }

            int b = 3;
        };
        class B {
        public:
            static int foo2(mse::TXScopeItemFixedPointer<A> A_scpifptr) { return A_scpifptr->b; }
            static int foo3(mse::TXScopeItemFixedConstPointer<A> A_scpifcptr) { return A_scpifcptr->b; }
        protected:
            ~B() {}
        };
    
        mse::TXScopeObj<A> a_scpobj(5);
        int res1 = (&a_scpobj)->b;
        int res2 = B::foo2(&a_scpobj);
        int res3 = B::foo3(&a_scpobj);
    }
```

### TXScopeItemFixedConstPointer

### TXScopeOwnerPointer
`TXScopeOwnerPointer<>` is similar to `boost::scoped_ptr<>` in functionality, but more limited in intended use. In particular, as a scope object, `TXScopeOwnerPointer<>` should not be used as a member of any class or struct that is not iself a scope object. Use it when you want to give scope lifetime to objects that are too large to be declared directly on the stack. Also, instead of its constructor taking a native pointer pointing to the already allocated object, it allocates the object itself and passes its contruction arguments to the object's constructor.  

usage example:

```cpp
    #include "msescope.h"
    
    void main(int argc, char* argv[]) {
        class A {
        public:
            A(int x) : b(x) {}
            A(const A& _X) : b(_X.b) {}
            virtual ~A() {}
            A& operator=(const A& _X) { b = _X.b; return (*this); }

            int b = 3;
        };
        class B {
        public:
            static int foo2(mse::TXScopeItemFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
            static int foo3(mse::TXScopeItemFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
        protected:
            ~B() {}
        };
    
        mse::TXScopeOwnerPointer<A> xscp_a_ownerptr(7);
        int res4 = B::foo2(xscp_a_ownerptr);
        int res4b = B::foo2(&(*xscp_a_ownerptr));
    }
```

### TXScopeFixedPointer
`TXScopeFixedPointer<>` is the actual type of the pointer value returned by the `&` (ampersand) operator of an object declared as a "scope" object (by virtue of being wrapped in the `TXScopeObj<>` transparent wrapper template). Generally, you don't need to use this type directly. `TXScopeFixedPointer<>` implicitly converts to a `TXScopeItemFixedPointer<>`, which can point to both explicitly declared and implicit scope objects. So generally you would just use the latter.

### TXScopeFixedConstPointer

### make_xscope_strong_pointer_store()

`make_xscope_strong_pointer_store()` returns a scope object that holds a copy of the given strong pointer and allows you to obtain a corresponding scope pointer. Currently supported strong pointers include [reference counting pointers](#reference-counting-pointers) and pointers to [asynchronously shared objects](#asynchronously-shared-objects) (and scope pointers themselves for the sake of completeness).

usage example:

```cpp
    #include "msescope.h"
    #include "mserefcounting.h"
    
    void main(int argc, char* argv[]) {
        class A {
        public:
            A(int x) : b(x) {}
            A(const A& _X) : b(_X.b) {}
            virtual ~A() {}
            A& operator=(const A& _X) { b = _X.b; return (*this); }

            int b = 3;
        };
        class B {
        public:
            static int foo2(mse::TXScopeItemFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
            static int foo3(mse::TXScopeItemFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
        protected:
            ~B() {}
        };
    
        /* Using mse::make_xscope_strong_pointer_store(), you can obtain a scope pointer from a refcounting pointer. */
        /* Let's make it a const refcounting pointer, just for variety. */
        mse::TRefCountingFixedConstPointer<A> refc_cptr1 = mse::make_refcounting<A>(11);
        auto xscp_refc_cstore = mse::make_xscope_strong_pointer_store(refc_cptr1);
        auto xscp_cptr1 = xscp_refc_cstore.xscope_ptr();
        int res6 = B::foo3(xscp_cptr1);
        mse::TXScopeItemFixedConstPointer<A> xscp_cptr2 = xscp_cptr1;
        A res7 = *xscp_cptr2;
    }
```

### xscope_ifptr_to()

Scope pointers cannot (currently) be retargeted after construction. If you need a pointer that will point to multiple different scope objects over its lifespan, you can use a registered pointer. This means that the target objects will also need to be registered objects. If the object is a registered scope object, then the '&' operator will will return a registered pointer. But at some point we're going to need a scope pointer to the base scope object. A convenient way to get one is to use the xscope_ifptr_to() function. 

usage example:

```cpp
    #include "msescope.h"
    #include "mseregistered.h"
    #include "msemsestring.h"
    
    void main(int argc, char* argv[]) {
        typedef mse::TXScopeObj<mse::nii_string> xscp_nstring_t;
        typedef mse::TXScopeItemFixedPointer<mse::nii_string> xscp_nstring_ptr_t;
        class CB {
        public:
            static void foo1(xscp_nstring_ptr_t xscope_ptr1) {
                std::cout << *xscope_ptr1;
            }
        };
        typedef mse::TRegisteredObj< xscp_nstring_t > regxscp_nstring_t;
        typedef mse::TRegisteredPointer< xscp_nstring_t > regxscp_nstring_ptr_t;
        regxscp_nstring_t regxscp_nstring1("some text");
        regxscp_nstring_ptr_t registered_ptr1 = &regxscp_nstring1;

        auto xscope_ptr1 = mse::xscope_ifptr_to(*registered_ptr1);
        CB::foo1(xscope_ptr1);

        regxscp_nstring_t regxscp_nstring2("some other text");
        registered_ptr1 = &regxscp_nstring2;
        CB::foo1(mse::xscope_ifptr_to(*registered_ptr1));

        {
            regxscp_nstring_t regxscp_nstring3("other text");
            registered_ptr1 = &regxscp_nstring3;
            CB::foo1(mse::xscope_ifptr_to(*registered_ptr1));
        }
        /* Attempting to dereference registered_ptr1 here would result in an exception. */
        //*registered_ptr1;
    }
```

### xscope_chosen_pointer()

Currently there's a rule against using non-owning scope pointers as function return values due to the possibility of inadvertently returning an invalid pointer to a local scope object. You could imagine that this rule might be relaxed in the future when a static code analyzer becomes available to catch any attempts to return an invalid scope pointer. But in the meantime, when you feel the need to return a non-owning scope pointer, you can use the `xscope_chosen_pointer()` function instead.

In essence, the `xscope_chosen_pointer()` function simply takes a bool and two scope pointers as input parameters and returns one of the pointers. If the bool is false then the first scope pointer is returned, otherwise the second is returned.

So consider, for example, a "min" function that takes two scope pointers and returns a scope pointer to the lesser of the two target (scope) objects. The implementation of this function would be straightforward if returning non-owning scope pointers was permitted. The following example demonstrates the same functionality using `xscope_chosen_pointer()` instead. 

```cpp
    #include "msescope.h"
    
    void main(int argc, char* argv[]) {
    
        class A {
        public:
            A(int x) : b(x) {}
            A(const A& _X) : b(_X.b) {}
            virtual ~A() {}
            bool operator<(const A& _X) const { return (b < _X.b); }

            int b = 3;
        };
    
        mse::TXScopeObj<A> a_scpobj(5);
        mse::TXScopeOwnerPointer<A> xscp_a_ownerptr(7);
    
        /* Technically, you're not allowed to return a non-owning scope pointer from a function. (The return_value() function
        wrapper enforces this.) Pretty much the only time you'd legitimately want to do this is when the returned pointer
        is one of the input parameters. An example might be a "min(a, b)" function which takes two objects by reference and
        returns the reference to the lesser of the two objects. The library provides the xscope_chosen_pointer() function
        which takes a bool and two scope pointers, and returns one of the scope pointers depending on the value of the
        bool. You could use this function to implement the equivalent of a min(a, b) function like so: */
        auto xscp_a_ptr5 = &a_scpobj;
        auto xscp_a_ptr6 = &(*xscp_a_ownerptr);
        auto xscp_min_ptr1 = mse::xscope_chosen_pointer((xscp_a_ptr6 < xscp_a_ptr5), xscp_a_ptr5, xscp_a_ptr6);
        assert(5 == xscp_min_ptr1->b);
    }
```

### Conformance helpers

As mentioned, in the future we expect that there will be a "compile helper tool" to verify that scope objects are not misused. Until then, a couple of "conformance helpers" are provided that can be used to help catch inadvertent misuse.

### return_value()

The safety of non-owning scope pointers is premised on the fact that they will not outlive the scope in which they are declared. So returning a non-owning scope pointer, or any object that contains or owns a non-owning scope pointer, from a function would be potentially unsafe. However, it could be safe to return a scope object if that object does not contain or own any non-owning scope pointers.

The `return_value()` function just returns its argument and verifies that it is of a type that is safe to return from a function (basically, doesn't contain any scope pointers). If not it will induce a compile error. Functions that do or could return scope types should wrap their return value with this function. 

`TReturnValue<>` is a transparent template wrapper that verifies that the type is safe to use as a function return type. If not it will induce a compile error. Functions that do or could return scope types and do not use the "auto" return type should wrap their return type with this function. Alternatively, you can use `TXScopeReturnValue<>` which additionally ensures that the return type is a scope type. 

usage example:

```cpp
    #include "msescope.h"
    #include "mseregistered.h"
    #include "msemstdstring.h"
    #include "mseoptional.h"
    
    class J {
    public:
        template<typename _TParam>
        static auto foo10(_TParam param) {
            auto l_obj = param;
            /* Functions that could return a scope type need to wrap their return value with the return_value() function. */
            return mse::return_value(mse::pointer_to(l_obj));
        }
    };
    
    void main() {
        class CB {
        public:
            /* It's generally not necessary for a function return type to be a scope type. Even if the return value
            is of a scope type, you can usually just use the underlying (non-scope) type of the scope object as the
            return type. */
            static mse::mstd::string foo1() {
                mse::TXScopeObj<mse::mstd::string> xscp_string1("some text");
                return mse::return_value(xscp_string1);
            }
    
            /* In the less common case where the scope type doesn't have an underlying non-scope type, it may be safe
            to return the scope object. But in order to use a scope type as a function return value, it must be
            wrapped in the transparent mse::TReturnValue<> or mse::TXScopeReturnValue<> wrapper template, which will
            induce a compile error if it deems the scope type potentially unsafe to use as a return type. */
            static mse::TXScopeReturnValue<mse::xscope_optional<mse::mstd::string> > foo2() {
                mse::xscope_optional<mse::mstd::string> xscp_returnable_obj1(mse::mstd::string("some text"));
                return mse::return_value(xscp_returnable_obj1);
            }
    
            /* "auto" return types don't need to be wrapped, but the return value needs to be wrapped with the
            return_value() function. */
            static auto foo3() {
                mse::xscope_optional<mse::mstd::string> xscp_returnable_obj1(mse::mstd::string("some text"));
                return mse::return_value(xscp_returnable_obj1);
            }
        };
    
        mse::TXScopeObj<mse::mstd::string> xscp_res1(CB::foo1());
        mse::xscope_optional<mse::mstd::string> xscp_res2(CB::foo2());
    
        typedef mse::TXScopeObj<mse::mstd::string> xscope_string_t;
        xscope_string_t xscp_str1 = "some text";
        /* TXScopeReturnValue<> deems xscope_string_t to be an acceptable return type because it doesn't contain
        any scope pointers. */
        mse::TXScopeReturnValue<xscope_string_t> xscpr_str1("some text");
        auto xscp_rstr1 = mse::return_value(xscp_str1);
    
        typedef decltype(&xscp_str1) xscope_string_ptr_t;
        /* TXScopeReturnValue<> deems xscope_string_ptr_t to be an unsafe return type because it is (or contains)
        a scope pointer. So the next line would result in a compile error. */
        //mse::TXScopeReturnValue<xscope_string_ptr_t> xscpr_sfptr1 = &xscp_str1;
        //auto xscp_rstr_ptr1 = mse::return_value(&xscp_str1);
    
        mse::TRegisteredObj<mse::mstd::string> reg_str1 = "some text";
        auto reg_ptr_res1 = J::foo10(reg_str1);
        //auto xscp_ptr_res1 = J::foo10(xscp_str1); // <-- would induce a compile error inside J::foo10() 
    }
```

### TMemberObj

Scope types have built in protection that prevents them from being allocated dynamically. But those protections are circumvented if a scope type is used as a member of a class or struct. So `TMemberObj<>` is a transparent wrapper that can be used to wrap class/struct member types to ensure that they are not scope types. This is particularly relevant in cases when the member type is, or is derived from, a template parameter.

### Defining your own scope types

example:
```cpp
#include "msescope.h"
#include "mseoptional.h"

void main(int argc, char* argv[]) {

		/* Defining your own scope types. */

		/* It is (intended to be) uncommon to need to define your own scope types. In general, if you want to use a
		type as a scope type, you can just wrap it with the mse::TXScopeObj<> template. */

		/* But in cases where you're going to use a scope type as a member of a class or struct, that class or
		struct must itself be a scope type. Improperly defining a scope type could result in unsafe code. */

		/* Scope types need to publicly inherit from mse::XScopeTagBase. And by convention, be named with a prefix
		indicating that it's a scope type. */
		class xscope_my_type1 : public mse::XScopeTagBase {
		public:
			xscope_my_type1(const mse::xscope_optional<mse::mstd::string>& xscp_maybe_string)
				: m_xscp_maybe_string1(xscp_maybe_string) {}

			/* If your scope type does not contain any non-owning scope pointers, then it should be safe to use
			as a function return type. You can "mark" it as such by adding the following member function. If the
			type does contain non-owning scope pointers, then doing so could result in unsafe code. */
			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

			mse::xscope_optional<mse::mstd::string> m_xscp_maybe_string1;
		};

		/* If your type contains or owns any non-owning scope pointers, then it must also publicly inherit
		from mse::ContainsNonOwningScopeReferenceTagBase. If your type contains or owns any item that can be
		independently targeted by scope pointers (i.e. basically has a '&' ("address of" operator) that yeilds
		a scope pointer), then it must also publicly inherit from mse::ReferenceableByScopePointerTagBase.
		Failure to do so could result in unsafe code. */
		class xscope_my_type2 : public mse::XScopeTagBase, public mse::ContainsNonOwningScopeReferenceTagBase
			, public mse::ReferenceableByScopePointerTagBase
		{
		public:
			typedef mse::TXScopeItemFixedConstPointer<mse::mstd::string> xscope_string_ptr_t;

			xscope_my_type2(const mse::xscope_optional<xscope_string_ptr_t>& xscp_maybe_string_ptr) : m_xscp_maybe_string_ptr(xscp_maybe_string_ptr) {}

			/* This item (potentially) contains a non-owning scope pointer. */
			mse::xscope_optional<xscope_string_ptr_t> m_xscp_maybe_string_ptr;

			/* This item owns an object that can be independently targeted by scope pointers. That is,
			&(*m_xscp_string_owner_ptr) yields a scope pointer. */
			mse::TXScopeOwnerPointer<mse::mstd::string> m_xscp_string_owner_ptr;
		};
}
```

### make_pointer_to_member_v2()
If you need a safe pointer to a member of a class/struct, you could declare the member itself to be a registered object (or a reference counting pointer). But often a preferable option is to use `make_pointer_to_member_v2()`. This function takes a safe pointer to the containing class/struct and a "[pointer-to-member](http://en.cppreference.com/w/cpp/language/pointer#Pointers_to_members)" indicating the member you want to target, and combines them to create a safe pointer to the member. The actual type of the returned pointer varies depending on the types of the parameters passed.

usage example:

```cpp
    /* Including "msepoly.h" is not required to use mse::make_pointer_to_member_v2(). It just happens to include all the
    other include files used by this example. */
    #include "msepoly.h"
    
    class H {
    public:
        /* A member function that provides a safe pointer/reference to a class/struct member is going to need to
        take a safe version of the "this" pointer as a parameter. */
        template<class this_type>
        static auto safe_pointer_to_member_string1(this_type safe_this) {
            return mse::make_pointer_to_member_v2(safe_this, &H::m_string1);
        }
    
        mse::nii_string m_string1 = "initial text";
    };
    
    void main() {
        /* To demonstrate, first we'll declare some objects such that we can obtain safe pointers to those
        objects. For better or worse, this library provides a bunch of different safe pointers types. */
        mse::TXScopeObj<H> h_scpobj;
        auto h_refcptr = mse::make_refcounting<H>();
        mse::TRegisteredObj<H> h_regobj;
        mse::TRelaxedRegisteredObj<H> h_rlxregobj;
    
        /* Safe iterators are a type of safe pointer too. */
        mse::mstd::vector<H> h_mstdvec;
        h_mstdvec.resize(1);
        auto h_mstdvec_iter = h_mstdvec.begin();
        mse::us::msevector<H> h_msevec;
        h_msevec.resize(1);
        auto h_msevec_ipointer = h_msevec.ibegin();
        auto h_msevec_ssiter = h_msevec.ss_begin();
    
        /* And don't forget the safe async sharing pointers. */
        auto h_access_requester = mse::make_asyncsharedv2readwrite<ShareableH>();
        auto h_writelock_ptr = h_access_requester.writelock_ptr();
        auto h_stdshared_const_ptr = mse::make_stdsharedimmutable<H>();
    
        {
            /* So here's how you get a safe pointer to a member of the object using mse::make_pointer_to_member_v2(). */
            auto h_string1_scpptr = mse::make_pointer_to_member_v2(&h_scpobj, &H::m_string1);
            (*h_string1_scpptr) = "some new text";
            auto h_string1_scp_const_ptr = mse::make_const_pointer_to_member_v2(&h_scpobj, &H::m_string1);
    
            auto h_string1_refcptr = mse::make_pointer_to_member_v2(h_refcptr, &H::m_string1);
            (*h_string1_refcptr) = "some new text";
    
            auto h_string1_regptr = mse::make_pointer_to_member_v2(&h_regobj, &H::m_string1);
            (*h_string1_regptr) = "some new text";
    
            auto h_string1_rlxregptr = mse::make_pointer_to_member_v2(&h_rlxregobj, &H::m_string1);
            (*h_string1_rlxregptr) = "some new text";
    
            auto h_string1_mstdvec_iter = mse::make_pointer_to_member_v2(h_mstdvec_iter, &H::m_string1);
            (*h_string1_mstdvec_iter) = "some new text";
    
            auto h_string1_msevec_ipointer = mse::make_pointer_to_member_v2(h_msevec_ipointer, &H::m_string1);
            (*h_string1_msevec_ipointer) = "some new text";
    
            auto h_string1_msevec_ssiter = mse::make_pointer_to_member_v2(h_msevec_ssiter, &H::m_string1);
            (*h_string1_msevec_ssiter) = "some new text";
    
            auto h_string1_writelock_ptr = mse::make_pointer_to_member_v2(h_writelock_ptr, &H::m_string1);
            (*h_string1_writelock_ptr) = "some new text";
    
            auto h_string1_stdshared_const_ptr = mse::make_pointer_to_member_v2(h_stdshared_const_ptr, &H::m_string1);
            //(*h_string1_stdshared_const_ptr) = "some new text";
        }
    
        {
            /* Though the type of the safe pointer to the object member varies depending on how the object was
            declared, you can make a (templated) accessor function that will return a safe pointer of the
            appropriate type. */
            auto h_string1_scpptr = H::safe_pointer_to_member_string1(&h_scpobj);
            (*h_string1_scpptr) = "some new text";
    
            auto h_string1_refcptr = H::safe_pointer_to_member_string1(h_refcptr);
            (*h_string1_refcptr) = "some new text";
    
            auto h_string1_regptr = H::safe_pointer_to_member_string1(&h_regobj);
            (*h_string1_regptr) = "some new text";
    
            auto h_string1_rlxregptr = H::safe_pointer_to_member_string1(&h_rlxregobj);
            (*h_string1_rlxregptr) = "some new text";
    
            auto h_string1_mstdvec_iter = H::safe_pointer_to_member_string1(h_mstdvec_iter);
            (*h_string1_mstdvec_iter) = "some new text";
    
            auto h_string1_msevec_ipointer = H::safe_pointer_to_member_string1(h_msevec_ipointer);
            (*h_string1_msevec_ipointer) = "some new text";
    
            auto h_string1_msevec_ssiter = H::safe_pointer_to_member_string1(h_msevec_ssiter);
            (*h_string1_msevec_ssiter) = "some new text";
    
            auto h_string1_writelock_ptr = H::safe_pointer_to_member_string1(h_writelock_ptr);
            (*h_string1_writelock_ptr) = "some new text";
    
            auto h_string1_stdshared_const_ptr = H::safe_pointer_to_member_string1(h_stdshared_const_ptr);
            //(*h_string1_stdshared_const_ptr) = "some new text";
        }
    }
```

### Poly pointers
Poly pointers are "chameleon" pointers that can be constructed from, and retain the safety features of many of the pointer types in this library. If you're writing a function and you'd like it to be able to accept different types of safe pointer parameters, you can "templatize" your function. Alternatively, you can declare your pointer parameters as poly pointers.  

Note that poly pointers support only basic facilities common to all the covered pointer and iterator types, providing essentially the functionality of a C++ reference. For example, this means no assignment operator, and no `operator bool()`. Where null pointer values are desired you might consider using [`mse::mstd::optional<>`](#optional-xscope_optional) or `std::optional<>` instead.  

### TXScopePolyPointer, TXScopePolyConstPointer
Scope poly pointers are primarily intended to be used in function parameter declarations. In particular, as they can be constructed from a scope pointer (`TXScopeFixedPointer<>` or `TXScopeFixedConstPointer<>`), they must observe the same usage restrictions.

usage example:

```cpp
    #include "msepoly.h"
    
    void main() {
        class A {
        public:
            A() {}
            A(std::string x) : b(x) {}
            virtual ~A() {}

            std::string b = "some text ";
        };
        class D : public A {
        public:
            D(std::string x) : A(x) {}
        };
        class B {
        public:
            static std::string foo1(mse::TXScopePolyPointer<A> ptr) {
                std::string retval = ptr->b;
                return retval;
            }
            static std::string foo2(mse::TXScopePolyConstPointer<A> ptr) {
                std::string retval = ptr->b;
                return retval;
            }
            static std::string foo3(mse::TXScopePolyPointer<std::string> ptr) {
                std::string retval = (*ptr) + (*ptr);
                return retval;
            }
            static std::string foo4(mse::TXScopePolyConstPointer<std::string> ptr) {
                std::string retval = (*ptr) + (*ptr);
                return retval;
            }
        protected:
            ~B() {}
        };
    
        /* To demonstrate, first we'll declare some objects such that we can obtain safe pointers to those
        objects. For better or worse, this library provides a bunch of different safe pointers types. */
        mse::TXScopeObj<A> a_scpobj;
        auto a_refcptr = mse::make_refcounting<A>();
        mse::TRegisteredObj<A> a_regobj;
        mse::TRelaxedRegisteredObj<A> a_rlxregobj;
    
        /* Safe iterators are a type of safe pointer too. */
        mse::mstd::vector<A> a_mstdvec;
        a_mstdvec.resize(1);
        auto a_mstdvec_iter = a_mstdvec.begin();
        mse::us::msevector<A> a_msevec;
        a_msevec.resize(1);
        auto a_msevec_ipointer = a_msevec.ibegin();
        auto a_msevec_ssiter = a_msevec.ss_begin();
    
        /* And don't forget the safe async sharing pointers. */
        auto a_access_requester = mse::make_asyncsharedreadwrite<A>();
        auto a_writelock_ptr = a_access_requester.writelock_ptr();
        auto a_stdshared_const_ptr = mse::make_stdsharedimmutable<A>();
    
        /* And note that safe pointers to member elements need to be wrapped in an mse::TXScopeAnyPointer<> for
        mse::TXScopePolyPointer<> to accept them. */
        auto b_member_a_refc_anyptr = mse::TXScopeAnyPointer<std::string>(mse::make_pointer_to_member(a_refcptr->b, a_refcptr));
        auto b_member_a_reg_anyptr = mse::TXScopeAnyPointer<std::string>(mse::make_pointer_to_member(a_regobj.b, &a_regobj));
        auto b_member_a_mstdvec_iter_anyptr = mse::TXScopeAnyPointer<std::string>(mse::make_pointer_to_member(a_mstdvec_iter->b, a_mstdvec_iter));
    
        {
            /* All of these safe pointer types happily convert to an mse::TXScopePolyPointer<>. */
            auto res_using_scpptr = B::foo1(&a_scpobj);
            auto res_using_refcptr = B::foo1(a_refcptr);
            auto res_using_regptr = B::foo1(&a_regobj);
            auto res_using_rlxregptr = B::foo1(&a_rlxregobj);
            auto res_using_mstdvec_iter = B::foo1(a_mstdvec_iter);
            auto res_using_msevec_ipointer = B::foo1(a_msevec_ipointer);
            auto res_using_msevec_ssiter = B::foo1(a_msevec_ssiter);
            auto res_using_writelock_ptr = B::foo1(a_writelock_ptr);
            auto res_using_member_refc_anyptr = B::foo3(b_member_a_refc_anyptr);
            auto res_using_member_reg_anyptr = B::foo3(b_member_a_reg_anyptr);
            auto res_using_member_mstdvec_iter_anyptr = B::foo3(b_member_a_mstdvec_iter_anyptr);
    
            /* Or an mse::TXScopePolyConstPointer<>. */
            auto res_using_scpptr_via_const_poly = B::foo2(&a_scpobj);
            auto res_using_refcptr_via_const_poly = B::foo2(a_refcptr);
            auto res_using_regptr_via_const_poly = B::foo2(&a_regobj);
            auto res_using_rlxregptr_via_const_poly = B::foo2(&a_rlxregobj);
            auto res_using_mstdvec_iter_via_const_poly = B::foo2(a_mstdvec_iter);
            auto res_using_msevec_ipointer_via_const_poly = B::foo2(a_msevec_ipointer);
            auto res_using_msevec_ssiter_via_const_poly = B::foo2(a_msevec_ssiter);
            auto res_using_writelock_ptr_via_const_poly = B::foo2(a_writelock_ptr);
            auto res_using_stdshared_const_ptr_via_const_poly = B::foo2(a_stdshared_const_ptr);
            auto res_using_member_refc_anyptr_via_const_poly = B::foo4(b_member_a_refc_anyptr);
            auto res_using_member_reg_anyptr_via_const_poly = B::foo4(b_member_a_reg_anyptr);
            auto res_using_member_mstdvec_iter_anyptr_via_const_poly = B::foo4(b_member_a_mstdvec_iter_anyptr);
        }
    }
```

### TPolyPointer, TPolyConstPointer
These poly pointers do not support construction from scope pointers, and thus are not bound by the same usage restrictions. For example, these poly pointers may be used as a member of a class or struct.

### TXScopeAnyPointer, TXScopeAnyConstPointer, TAnyPointer, TAnyConstPointer
"Any" pointers are also “chameleon” pointers that behave similarly to poly pointers. One difference is that unlike poly pointers which can only be directly constructed from a finite set of pointer types, "any" pointers can be constructed from almost any kind of pointer. But poly pointers can be constructed from "any" pointers, so indirectly, via "any" pointers, pretty much any type of pointer converts to a poly pointer too. In particular, if you wanted to pass a pointer generated by [`make_pointer_to_member()`](#make_pointer_to_member) to a function that takes a poly pointer, you would first need to wrap it an "any" pointer. This is demonstrated in the [scope poly pointer](#txscopepolypointer-txscopepolyconstpointer) usage example.  

"Any" pointers can also be used as function arguments. The choice between using poly pointers versus "any" pointers is similar to the choice between [`std::variant` and `std::any`](http://www.boost.org/doc/libs/1_63_0/doc/html/variant/misc.html#variant.versus-any). 

### TXScopeAnyRandomAccessIterator, TXScopeAnyRandomAccessConstIterator, TAnyRandomAccessIterator, TAnyRandomAccessConstIterator

In modern C++ (and SaferCPlusPlus), arrays of different sizes are actually different types, with incompatible iterators. So, for example, if you wanted to make a function that accepts the iterators of arrays of varying size, you would generally do that by "templatizing" the function. Alternatively, you could use an "any random access iterator" which is a "chameleon" iterator that can be constructed from basically any iterator that supports `operator[]` (the "square bracket" operator).

### TXScopeAnyRandomAccessSection, TXScopeAnyRandomAccessConstSection, TAnyRandomAccessSection, TAnyRandomAccessConstSection

`TAnyRandomAccessSection<_Ty>` is essentially just an alias for `TRandomAccessSection<TAnyRandomAccessIterator<_Ty> >`. Analogous to [`TAnyRandomAccessIterator<>`](#txscopeanyrandomaccessiterator-txscopeanyrandomaccessconstiterator-tanyrandomaccessiterator-tanyrandomaccessconstiterator), it can be used to enable a function to accept, as a parameter, any type of "[random access section](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection)".

usage example:

```cpp
    #include "msepoly.h"
    
    void main(int argc, char* argv[]) {
        mse::mstd::array<int, 4> array1 { 1, 2, 3, 4 };
        mse::mstd::array<int, 5> array2 { 5, 6, 7, 8, 9 };
        mse::mstd::vector<int> vec1 { 10, 11, 12, 13, 14 };
        class B {
        public:
            static void foo1(mse::TXScopeAnyRandomAccessIterator<int> ra_iter1) {
                ra_iter1[1] = 15;
            }
            static int foo2(mse::TXScopeAnyRandomAccessConstIterator<int> const_ra_iter1) {
                const_ra_iter1 += 2;
                --const_ra_iter1;
                const_ra_iter1--;
                return const_ra_iter1[2];
            }
            static void foo3(mse::TXScopeAnyRandomAccessSection<int> ra_section) {
                for (mse::TXScopeAnyRandomAccessSection<int>::size_type i = 0; i < ra_section.size(); i += 1) {
                    ra_section[i] = 0;
                }
            }
            static int foo4(mse::TXScopeAnyRandomAccessConstSection<int> const_ra_section) {
                int retval = 0;
                for (mse::TXScopeAnyRandomAccessSection<int>::size_type i = 0; i < const_ra_section.size(); i += 1) {
                    retval += const_ra_section[i];
                }
                return retval;
            }
            static int foo5(mse::TXScopeAnyRandomAccessConstSection<int> const_ra_section) {
                int retval = 0;
                for (const auto& const_item : const_ra_section) {
                    retval += const_item;
                }
                return retval;
            }
        };
    
        auto array_iter1 = array1.begin();
        array_iter1++;
        auto res1 = B::foo2(array_iter1);
        B::foo1(array_iter1);
    
        auto array_const_iter2 = array2.cbegin();
        array_const_iter2 += 2;
        auto res2 = B::foo2(array_const_iter2);
    
        auto res3 = B::foo2(vec1.cbegin());
        B::foo1(++vec1.begin());
        auto res4 = B::foo2(vec1.begin());
    
        mse::TXScopeAnyRandomAccessSection<int> ra_section1(array_iter1, 2);
        B::foo3(ra_section1);
    
        mse::TXScopeAnyRandomAccessSection<int> ra_section2(++vec1.begin(), 3);
        auto res5 = B::foo5(ra_section2);
        B::foo3(ra_section2);
        auto res6 = B::foo4(ra_section2);
    }
```

### TXScopeAnyStringSection, TXScopeAnyStringConstSection, TAnyStringSection, TAnyStringConstSection

`TAnyStringSection<_Ty>` is essentially just an alias for `TAnyStringSection<TAnyRandomAccessIterator<_Ty> >`. Like [`TAnyRandomAccessSection<_Ty>`](#txscopeanyrandomaccesssection-txscopeanyrandomaccessconstsection-tanyrandomaccesssection-tanyrandomaccessconstsection), it can be used to enable a function to accept, as a parameter, any type of "[string section](#txscopestringsection-txscopestringconstsection-tstringsection-tstringconstsection)".

usage example:

```cpp
    #include "msepoly.h" // TAnyStringSection<> is defined in this header
    #include "msemstdstring.h"
    #include "msemsestring.h"
    #include "mseregistered.h"
    
    void main(int argc, char* argv[]) {
    
        /* "Any" string sections are basically polymorphic string sections that can hold the value of any string
        section type. They can be used as function parameter types to enable functions to accept any type of string
        section. */
    
        mse::mstd::string mstring1("some text");
        mse::TAnyStringSection<char> any_string_section1(mstring1.begin()+5, 3);
    
        auto string_literal = "some text";
        mse::TAnyStringConstSection<char> any_string_const_section2(string_literal+5, 3);
    
        typedef mse::TRegisteredObj<mse::nii_string> reg_nii_string_t;
        reg_nii_string_t reg_nii_string3("some other text");
        /* This is a different type of (safe) iterator to a different type of string. */
        auto iter = reg_nii_string_t::ss_begin(&reg_nii_string3);
    
        /* Resulting in a different type of string section. */
        auto string_section3 = mse::make_string_section(iter+11, 3);
    
        mse::TAnyStringSection<char> any_string_section3 = string_section3;
        assert(any_string_section1 == any_string_section3);
        assert(any_string_section1.front() == 't');
        assert(any_string_section1.back() == 'x');
        any_string_section1 = string_section3;
        any_string_section1[1] = 'E';
    }
```

### TXScopeAnyNRPStringSection, TXScopeAnyNRPStringConstSection, TAnyNRPStringSection, TAnyNRPStringConstSection

`TAnyNRPStringSection<>` is just a version of [`TAnyStringSection<>`](#txscopeanystringsection-txscopeanystringconstsection-tanystringsection-tanystringconstsection) that, for enhanced safety, doesn't support construction from unsafe raw pointer iterators or (unsafe) `std::string` iterators.

### pointer_to()

`pointer_to(X)` simply returns `&X`, unless the type of `&X` is a native pointer (and the library's safe pointers have not been disabled). In that case a compiler error will be induced. It can be used in place of the `&` operator to help avoid inadvertent use of native pointers.

### Safely passing parameters by reference
As has been shown, you can use [registered pointers](#registered-pointers), [reference counting pointers](#reference-counting-pointers), [scope pointers](#scope-pointers) and/or various iterators to safely pass parameters by reference. When writing a function for general use that takes parameters by reference, you can either require a specific (safe) reference type for its reference parameters, or allow the caller some flexibility as to which reference type they use. 

One way to allow your function to accept any reference type is to make your function into a function template. The benefits of this approach are that it requires no extra run-time overhead, and it intrinsically supports legacy (unsafe, native) pointer types when needed. This approach is demonstrated in the [`TRefCountingOfRegisteredPointer<>`](#trefcountingofregisteredpointer) usage example. Or you can read a slightly out-of-date article about it [here](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).

Another option is to use [poly pointers](#poly-pointers) instead. They can also enable your function to accept a variety of reference types, without "templatizing" your function, but with a small run-time overhead.

Another choice is to require that reference parameters be passed using scope pointers. This approach, by default, has no more run-time overhead than using native pointers/references. And note that scope pointers can be obtained from reference counting pointers and pointers to shared objects (using [`make_xscope_strong_pointer_store()`](#make_xscope_strong_pointer_store)), and registered pointers when they are pointing to scope objects. Legacy (native) pointers though, would not be supported. (Although if you're in a pinch, `mse::us::unsafe_make_xscope_pointer_to()` can produce an (unsafe) scope pointer to any object.) Generally, you would use scope pointer parameters in cases where you are adopting a "scopecentric" style of programming (similar to the Rust language), and trying to avoid use of (unsafe) legacy elements.

And of course the library remains perfectly compatible with (the less safe) traditional C++ references if you prefer. 

### Multithreading

Note that the library requires and enforces that objects shared or passed between threads may only be of types identified as safe for such operations. Also, remember that any input to, or output from a function not via its interface (i.e function parameters or return value) is technically unsafe. Specifically, directly accessing global variables, or accessing variables via lambda capture (by reference or otherwise). This particularly applies when the function in question is executed asynchronously.

### TUserDeclaredAsyncPassableObj

When passing an argument to a function that will be executed in another thread using the library, the argument must be of a type identified as being safe to do so. If not, a compiler error will be induced. The library knows which of its own types and the standard types are and aren't safely passable to another thread, but can't automatically deduce whether or not a user-defined type is safe to pass. So in order to pass a user-defined type, you need to "declare" that it is safely passable by wrapping it with the `us::TUserDeclaredAsyncPassableObj<>` template. Otherwise you'll get a compile error. A type that is safe to pass should have no indirect members (i.e. pointers/references) whose target is not protected by a thread-safety mechanism. (Mis)using `us::TUserDeclaredAsyncPassableObj<>` to indicate that a user-defined type is safely passable when that type does not meet these criteria could result in unsafe code.

### thread

`mstd::thread` is just an implementation of `std::thread` that verifies that the arguments passed are of a type that is designated as safe to pass between threads. 

### async()

`mstd::async()` is just an implementation of `std::async()` that verifies that the arguments and return value passed are of a type that is designated as safe to pass between threads. 

### Asynchronously shared objects
One situation where safety mechanisms are particularly important is when sharing objects between asynchronous threads. In particular, while one thread is modifying an object, you want to ensure that no other thread accesses it. But you also want to do it in a way that allows for maximum utilization of the shared object. To this end the library provides "access requesters". Access requesters provide "lock pointers" on demand that are used to safely access the shared object.

In cases where the object you want to share is "immutable" (i.e. not modifiable), no access control is necessary. For these cases the library provides "immutable fixed pointers", which can be thought of as sort of a safer version `std::shared_ptr<>`.

In order to ensure safety, shared objects can only be accessed through lock pointers or immutable fixed pointers. If you have an existing object that you only want to share part of the time, you can swap (using `std::swap()` for example) the object with a shared object when it's time to share it, and swap it back when you're done sharing.

Note that not all types are safe to share between threads. For example, because of its iterators, `mstd::vector<int>` is not safe to share between threads. (And neither is `std::vector<int>`.) `nii_vector<int>` on the other hand is. Trying to share the former using access requesters or immutable fixed pointers would result in a compile error.

### TUserDeclaredAsyncShareableObj

As with passing objects between threads, when using the library to share an object among threads, the object must be of a type identified as being safe to do so. If not, a compiler error will be induced. The library knows which of its own types and the standard types are and aren't safely shareable, but can't automatically deduce whether or not a user-defined type is safe to share. So in order to share a user-defined type, you need to "declare" that it is safely shareable by wrapping it with the `us::TUserDeclaredAsyncShareableObj<>` template.

As with objects that are passed between threads, a type that is safe to share should have no indirect members (i.e. pointers/references) whose target is not protected by a thread-safety mechanism. 

In addition, safely shareable types should not have any `mutable` qualified members that are not protected by a thread-safety mechanism.

And currently, any type declared as safely shareable must also satisfy the criteria for being safely passable. That is, safe shareability must imply safe passability.

(Mis)using `us::TUserDeclaredAsyncShareableObj<>` to indicate that a user-defined type is safely shareable when that type does not meet these criteria could result in unsafe code.

### TAsyncSharedV2ReadWriteAccessRequester

Use the `writelock_ptr()` and `readlock_ptr()` member functions to obtain pointers to the shared object. Those functions will block until they can obtain the needed lock on the shared object. The obtained pointers will hold on to their lock for as long as they exist. Their locks are released when the pointers are destroyed (generally when they go out of scope).  

Use `mse::make_asyncsharedv2readwrite<>()` to obtain a `TAsyncSharedV2ReadWriteAccessRequester<>`. `TAsyncSharedV2ReadWriteAccessRequester<>` can be copied and passed-by-value as a parameter (to another thread, generally).

Non-blocking `try_writelock_ptr()` and `try_readlock_ptr()` member functions are also available. As are the limited-blocking `try_writelock_ptr_for()`, `try_readlock_ptr_for()`, `try_writelock_ptr_until()` and `try_readlock_ptr_until()`.

Note that while a "write-lock" pointer will not simultaneously co-exist with any lock pointer to the same shared object in any other thread, it can co-exist with (read- and/or write-) lock pointers in the same thread. This means that lock pointers have ["upgrade lock"](http://www.boost.org/doc/libs/1_65_1/doc/html/thread/synchronization.html#thread.synchronization.mutex_concepts.upgrade_lockable) functionality. That is, for example, a thread that holds a read lock on a shared object (via read-lock pointer) can, at some later point, additionally obtain a write lock (via write-lock pointer) without surrendering the original read lock. It can then release the write lock (by allowing the write-lock pointer to go out of scope), again without surrendering the original read lock. Systems based on traditional "readers-writer" locks would require you to surrender the read lock before attempting to obtain a write lock, allowing another thread to potentially (and undesirably) obtain a write lock in between.

One caveat is that this introduces a new possible deadlock scenario where two threads hold read locks and both are blocked indefinitely waiting for write locks. Prudent practice would avoid deadlock by using the non-blocking `try_writelock_ptr()`, or time-out limited `try_writelock_ptr_for()` member functions to obtain the write-lock pointer. Currently, this dead-lock scenario is not detected by the access requester (or its underlying mutex). It is intended that in the near future, this dead-lock scenario will be detected and an exception will be thrown (or whatever user-specified behavior).

### TAsyncSharedV2ReadOnlyAccessRequester
Same as `TAsyncSharedV2ReadWriteAccessRequester<>`, but only supports `readlock_ptr()`, not `writelock_ptr()`. You can use `mse::make_asyncsharedv2readonly<>()` to obtain a `TAsyncSharedV2ReadOnlyAccessRequester<>`. `TAsyncSharedV2ReadOnlyAccessRequester<>` can also be copy constructed from a `TAsyncSharedV2ReadWriteAccessRequester<>`.

### TAsyncSharedV2ImmutableFixedPointer
In cases where the object you want to share is "immutable" (i.e. not modifiable), no access control is necessary. For these cases you can use `TAsyncSharedV2ImmutableFixedPointer<>`, which can be thought of as sort of a safer version `std::shared_ptr<>`. Use `mse::make_asyncsharedv2immutable<>()` to obtain a `TAsyncSharedV2ImmutableFixedPointer<>`.

usage example:

```cpp
	#include "mseasyncshared.h"
	#include <ctime>
	#include <ratio>
	#include <chrono>
	#include <future>
	
	class H {
	public:
		template<class _TAsyncSharedReadWriteAccessRequester>
		static double foo7(_TAsyncSharedReadWriteAccessRequester A_ashar) {
			auto t1 = std::chrono::high_resolution_clock::now();
			/* A_ashar.readlock_ptr() will block until it can obtain a read lock. */
			auto ptr1 = A_ashar.readlock_ptr(); // while ptr1 exists it holds a (read) lock on the shared object
			auto t2 = std::chrono::high_resolution_clock::now();
			std::this_thread::sleep_for(std::chrono::seconds(1));
			auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			auto timespan_in_seconds = time_span.count();
			auto thread_id = std::this_thread::get_id();
			return timespan_in_seconds;
		}
	protected:
		~H() {}
	};
	
	void main(int argc, char* argv[]) {
		/* The TAsyncShared data types are used to safely share objects between asynchronous threads. */
	
		class A {
		public:
			A(int x) : b(x) {}
			virtual ~A() {}
	
			int b = 3;
			mse::nii_string s = "some text ";
		};
		/* User-defined classes need to be declared as (safely) shareable in order to be accepted by the access requesters. */
		typedef mse::us::TUserDeclaredAsyncShareableObj<A> ShareableA;
	
		class B {
		public:
			static double foo1(mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA> A_ashar) {
				auto t1 = std::chrono::high_resolution_clock::now();
				/* mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA>::writelock_ptr() will block until it can obtain a write lock. */
				auto ptr1 = A_ashar.writelock_ptr(); // while ptr1 exists it holds a (write) lock on the shared object
				auto t2 = std::chrono::high_resolution_clock::now();
				std::this_thread::sleep_for(std::chrono::seconds(1));
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				auto timespan_in_seconds = time_span.count();
				auto thread_id = std::this_thread::get_id();
				//std::cout << "thread_id: " << thread_id << ", time to acquire write pointer: " << timespan_in_seconds << " seconds.";
				//std::cout << std::endl;
	
				ptr1->s = std::to_string(timespan_in_seconds);
				return timespan_in_seconds;
			}
			static int foo2(mse::TAsyncSharedV2ImmutableFixedPointer<ShareableA> A_immptr) {
				return A_immptr->b;
			}
		protected:
			~B() {}
		};
	
		std::cout << std::endl;
		std::cout << "AsyncShared test output:";
		std::cout << std::endl;
	
		{
			/* This block contains a simple example demonstrating the use of mse::TAsyncSharedReadWriteAccessRequester
			to safely share an object between threads. */
	
			std::cout << "TAsyncSharedReadWrite:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readwrite<ShareableA>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			{
				auto ptr1 = ash_access_requester.writelock_ptr();
				auto ptr2 = ash_access_requester.writelock_ptr();
			}
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(B::foo1, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire write pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
	
			/* Btw, mse::TAsyncSharedV2ReadOnlyAccessRequester<>s can be copy constructed from
			mse::TAsyncSharedV2ReadWriteAccessRequester<>s */
			mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA> ash_read_only_access_requester(ash_access_requester);
		}
		{
			std::cout << "TAsyncSharedReadOnly:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readonly<ShareableA>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(J::foo7<mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedV2ReadWriteAccessRequester:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readwrite<ShareableA>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			{
				auto ptr3 = ash_access_requester.readlock_ptr();
				auto ptr1 = ash_access_requester.writelock_ptr();
				auto ptr2 = ash_access_requester.writelock_ptr();
			}
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(J::foo7<mse::TAsyncSharedV2ReadWriteAccessRequester<ShareableA>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedV2ReadOnlyAccessRequester:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedv2readonly<ShareableA>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(J::foo7<mse::TAsyncSharedV2ReadOnlyAccessRequester<ShareableA>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			/* Just demonstrating the existence of the "try" versions. */
			auto access_requester = mse::make_asyncsharedv2readwrite<mse::nii_string>("some text");
			auto writelock_ptr1 = access_requester.try_writelock_ptr();
			if (writelock_ptr1) {
				// lock request succeeded
				int q = 5;
			}
			auto readlock_ptr2 = access_requester.try_readlock_ptr_for(std::chrono::seconds(1));
			auto writelock_ptr3 = access_requester.try_writelock_ptr_until(std::chrono::steady_clock::now() + std::chrono::seconds(1));
		}
		{
			/* For scenarios where the shared object is immutable (i.e. is never modified), you can get away without using locks
			or access requesters. */
			auto A_immptr = mse::make_asyncsharedv2immutable<ShareableA>(5);
			int res1 = A_immptr->b;
			std::shared_ptr<const ShareableA> A_shptr(A_immptr);
	
			std::list<std::future<int>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(mse::mstd::async(B::foo2, A_immptr));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				int res2 = (*it).get();
			}
	
			auto A_b_safe_cptr = mse::make_const_pointer_to_member(A_immptr->b, A_immptr);
		}
	}
```

### TAsyncRASectionSplitter

`TAsyncRASectionSplitter<>` is used for situations where you want to allow multiple threads to concurrently access and/or modify different parts of an array or vector. You specify how you want the array/vector partitioned, and the `TAsyncRASectionSplitter<>` will provide a set of access requesters used to obtain access to each partition. Instead of the usual "lock pointers", these access requesters return "lock [random access section](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection)s".

usage example:
```cpp
#include "msemstdvector.h"
#include "mseasyncshared.h"

class J {
public:
	/* This function takes a "random access section" (which is like an "array_view" or gsl::span) as its parameter. */
	template<class _TStringRASection>
	static void foo8(_TStringRASection ra_section) {
		size_t delay_in_milliseconds = 3000/*arbitrary*/;
		if (1 <= ra_section.size()) {
			delay_in_milliseconds /= ra_section.size();
		}
		for (size_t i = 0; i < ra_section.size(); i += 1) {
			auto now1 = std::chrono::system_clock::now();
			auto tt = std::chrono::system_clock::to_time_t(now1);

			/* Just trying to obtain a string with the current time and date. The standard library doesn't yet
			seem to provide a safe, portable way to do this. */
#ifdef _MSC_VER
			static const size_t buffer_size = 64;
			char buffer[buffer_size];
			buffer[0] = '\0';
			ctime_s(buffer, buffer_size, &tt);
#else /*_MSC_VER*/
			auto buffer = ctime(&tt);
#endif /*_MSC_VER*/

			std::string now_str(buffer);
			ra_section[i] = now_str;

			std::this_thread::sleep_for(std::chrono::milliseconds(delay_in_milliseconds));
		}
	}

	/* This function just obtains a writelock_ra_section from the given "splitter access requester" and calls the given
	function with the writelock_ra_section as the first argument. */
	template<class _TAsyncSplitterRASectionReadWriteAccessRequester, class _TFunction, class... Args>
	static void invoke_with_writelock_ra_section1(_TAsyncSplitterRASectionReadWriteAccessRequester ar, _TFunction function1, Args&&... args) {
		function1(ar.writelock_ra_section(), args...);
	}
};

void main(int argc, char* argv[]) {
	/* This block demonstrates safely allowing different threads to (simultaneously) modify different
	sections of a vector. (We use vectors in this example, but it works just as well with arrays.) */

	static const size_t num_sections = 10;
	static const size_t section_size = 5;
	const size_t num_elements = num_sections * section_size;

	typedef mse::nii_vector<mse::nii_string> async_shareable_vector1_t;
	typedef mse::mstd::vector<mse::nii_string> nonshareable_vector1_t;
	/* Let's say we have a vector. */
	nonshareable_vector1_t vector1;
	vector1.resize(num_elements);
	{
		size_t count = 0;
		for (auto& item_ref : vector1) {
			count += 1;
			item_ref = "text" + std::to_string(count);
		}
	}

	/* Only access controlled objects can be shared with other threads, so we'll make an access controlled vector and
	(temporarily) swap it with our original one. */
	auto ash_access_requester = mse::make_asyncsharedv2readwrite<async_shareable_vector1_t>();
	std::swap(vector1, (*(ash_access_requester.writelock_ptr())));

	{
		/* Now, we're going to use the access requester to obtain two new access requesters that provide access to
		(newly created) "random access section" objects which are used to access (disjoint) sections of the vector.
		We need to specify the position where we want to split the vector. Here we specify that it be split at index
		"num_elements / 2", right down the middle. */
		mse::TAsyncRASectionSplitter<decltype(ash_access_requester)> ra_section_split1(ash_access_requester, num_elements / 2);
		auto ar1 = ra_rection_split1.first_ra_section_access_requester();
		auto ar2 = ra_rection_split1.second_ra_section_access_requester();

		/* The J::foo8 template function is just an example function that operates on containers of strings. In our case the
		containers will be the random access sections we just created. We'll create an instance of the function here. */
		auto& my_foo8_function_ref = J::foo8<decltype(ar1.writelock_ra_section())>;
		typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;

		/* We want to execute the my_foo8 function in a separate thread. The function takes a "random access section"
		as an argument. But as we're not allowed to pass random access sections between threads, we must pass an
		access requester instead. The "J::invoke_with_writelock_ra_section1" template function is just a helper
		function that will obtain a (writelock) random access section from the access requester, then call the given
		function, in this case my_foo8, with that random access section. So here we'll use it to create a proxy
		function that we can execute directly in a separate thread and will accept an access requester as a
		parameter. */
		auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar1), my_foo8_function_type>;

		std::list<mse::mstd::thread> threads;
		/* So this thread will modify the first section of the vector. */
		threads.emplace_back(mse::mstd::thread(my_foo8_proxy_function_ref, ar1, my_foo8_function_ref));
		/* While this thread modifies the other section. */
		threads.emplace_back(mse::mstd::thread(my_foo8_proxy_function_ref, ar2, my_foo8_function_ref));

		{
			int count = 1;
			for (auto it = threads.begin(); threads.end() != it; it++, count++) {
				(*it).join();
			}
		}
	}
	{
		/* Ok, now let's do it again, but instead of splitting the vector into two sections, let's split it into more sections: */
		/* First we create a list of a the sizes of each section. We'll use a vector here, but any iteratable container will work. */
		mse::mstd::vector<size_t> section_sizes;
		for (size_t i = 0; i < num_sections; i += 1) {
			section_sizes.push_back(section_size);
		}

		/* Just as before, TAsyncRASectionSplitter<> will generate a new access requester for each section. */
		mse::TAsyncRASectionSplitter<decltype(ash_access_requester)> ra_section_split1(ash_access_requester, section_sizes);
		auto ar0 = ra_rection_split1.ra_section_access_requester(0);

		auto& my_foo8_function_ref = J::foo8<decltype(ar0.writelock_ra_section())>;
		typedef std::remove_reference<decltype(my_foo8_function_ref)>::type my_foo8_function_type;
		auto& my_foo8_proxy_function_ref = J::invoke_with_writelock_ra_section1<decltype(ar0), my_foo8_function_type>;

		std::list<mse::mstd::thread> threads;
		for (size_t i = 0; i < num_sections; i += 1) {
			auto ar = ra_rection_split1.ra_section_access_requester(i);
			threads.emplace_back(mse::mstd::thread(my_foo8_proxy_function_ref, ar, my_foo8_function_ref));
		}

		{
			int count = 1;
			for (auto it = threads.begin(); threads.end() != it; it++, count++) {
				(*it).join();
			}
		}
	}

	/* Now that we're done sharing the (controlled access) vector, we can swap it back to our original vector. */
	std::swap(vector1, (*(ash_access_requester.writelock_ptr())));
	auto first_element_value = vector1[0];
	auto last_element_value = vector1.back();
}
```

### Primitives

### CInt, CSize_t and CBool
These classes are meant to behave like, and be compatible with their native counterparts. In debug mode, they check for "use before initialization", and in release mode, they use default initialization to help ensure deterministic behavior. Upon value assignment, `CInt` and `CSize_t` will check to ensure that the value fits within the type's range. `CSize_t`'s `-=` operator checks that the operation evaluates to a positive value. And unlike its native counterpart, arithmetic operations involving `CSize_t` that could evaluate to a negative number are returned as a (signed) `CInt`.

usage example:

```cpp
    #include "mseprimitives.h"
    
    void main(int argc, char* argv[]) {
    
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
```

Note that while `CInt` and `CSize_t` have no problem interacting with native signed integers, they do not implicitly play well with `size_t` or native unsigned integers. We'd be generally wary of using native unsigned integer types due to the implicit conversion/promotion rules between signed and unsigned native integers. But if you need to obtain a `size_t` from a `CSize_t`, you can do so explicitly using the `mse::as_a_size_t()` function. If you want to construct a `CSize_t` (or `CInt`) from a native unsigned integer type, you'd need to first cast it to a `size_t`, or a signed integer.  

Also see the section on "[compatibility considerations](#compatibility-considerations)".

### Quarantined types

Quarantined types are meant to hold values that are obtained from user input or some other untrusted source (like a media file for example). These are not yet available in the library, but are an important concept with respect to safe programming. Values obtained from untrusted sources are the main attack vector of malicious actors and should be handled with special care. For example, the so-called "stagefright" vulnerability in the Android OS is the result of a specially crafted media file causing the sum of integers to overflow.  

It is intended that these types will appropriately handle "extreme" values (at some run-time cost if necessary), and ensure that their values are in an appropriate range when converted to their (high-performance) native counterparts.

### CQuarantinedInt, CQuarantinedSize_t, CQuarantinedVector, CQuarantinedString

Not yet available.

Integer types with more comprehensive range checking can be found here: https://github.com/robertramey/safe_numerics.

### Vectors

The library provides a number of vector types. Probably the two most essential are [`mstd::vector<>`](#vector) and [`nii_vector<>`](#nii_vector). `mstd::vector<>` is simply a memory-safe drop-in replacement for `std::vector<>`. Due to their iterators, vectors are not, in general, safe to share among threads. `nii_vector<>` is designed for safe sharing among asynchronous threads.

The standard library vector iterators are designed so that they can be (unsafely) implemented as just pointers. But this makes them prone to being invalidated as a side effect of insertion, deletion and resize operations on the vector. This also means that they behave differently from list iterators, so algorithms that work on lists won't necessarily work on vectors. So the library includes [`ivector<>`](#ivector), whose iterators behave like list iterators. That is, they don't get invalidated by insert/delete/resize vector operations unless the element they were pointing to is deleted, and after any such operation, they will continue to point to the same item, which may then be in a different position in the vector.

And finally, for those whose are willing to sacrifice some safety for performance there is [`us::msevector<>`](#msevector). This vector is not memory-safe in the way that the other vectors are. It may be useful in cases where you want more control over the safety-performance trade-off. It supports a variety of iterator types - the traditional (unsafe) iterators, a bounds-checked version of the traditional iterator, and iterators that, like `ivector<>`'s iterators, behave like list iterators.

The vectors also support scope iterators which have the same syntax and behavior as the arrays' [scope iterators](#xscope_iterator). And remember that you can use [`TRandomAccessSection<>`](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection) to provide access to a subsection of any vector or array.

### vector

`mstd::vector<>` is a memory-safe drop-in replacement for `std::vector<>`.

usage example:

```cpp
    #include "msemstdvector.h"
    #include <vector>
    
    void main(int argc, char* argv[]) {
    
        mse::mstd::vector<int> mv;
        std::vector<int> sv;
        /* These two vectors should be completely interchangeable. The difference being that mv should throw
        an exception on any attempt to access invalid memory. */
        
        
        /* mse::msevector is not quite as safe as mse::mstd::vector in the following way: */
        
        std::vector<int>::iterator sv1_it;
        mse::us::msevector<int>::ss_iterator_type msev1_it; // bounds checked iterator just like mse::mstd::vector<int>::iterator
        mse::mstd::vector<int>::iterator mv1_it;
        {
            std::vector<int> sv1 = { 1, 2, 3 };
            sv1_it = sv1.begin();
            
            mse::us::msevector<int> msev1 = { 1, 2, 3 };
            msev1_it = msev1.ss_begin();
            
            mse::mstd::vector<int> mv1 = { 1, 2, 3 };
            mv1_it = mv1.begin();
        }
        
        // (*sv1_it) = 4; // not good
        // (*msev1_it) = 4; // not good
        
        try {
            (*mv1_it) = 4; // ok
        } catch(...) {
            // At present, this won't even result in an exception. It'll just work.
            // In the future an exception may be throw in debug builds.
        }
    }
```

### nii_vector

Due to their iterators, vectors are not, in general, safe to share among threads. `nii_vector<>` is designed to be safely shareable between asynchronous threads. To that end, it does not support "implicit" iterators. That is, in order to obtain an iterator, you must explicitly provide a (safe) pointer to the `nii_vector<>`. So for example, instead of a `begin()` member function that takes no parameters, `nii_vector<>` has an `ss_begin(...)` (static template) member function that actually requires a pointer to the vector to passed as a parameter.  

Note that in cases when you only need the vector to be shared between threads part of the time, you can swap between, for example, (non-shareable) `mstd::vector<>`s and (shareable) `nii_vector<>`s when you need.  

Also note that an `nii_vector<>` will be (automatically) marked as [safely shareable](#asynchronously-shared-objects) only if its element type is known or declared to be safely shareable.

usage example:

```cpp
    #include "msemsevector.h"
    #include "mseregistered.h"
    
    void main(int argc, char* argv[]) {
    
        /* nii_vector<> is a safe vector designed for safe sharing between asynchronous threads. */
    
        typedef mse::nii_vector<mse::nii_string> nii_vector1_t;
    
        mse::TRegisteredObj<nii_vector1_t> rg_vo1;
        for (size_t i = 0; i < 5; i += 1) {
            rg_vo1.push_back("some text");
        }
        mse::TRegisteredPointer<nii_vector1_t> vo1_regptr1 = &rg_vo1;
    
        /* nii_vector<> does not have member functions like "begin(void)" that return "implicit" iterators. It does have
        (template) member functions like "ss_begin" which take a (safe) pointer to the nii_vector<> as a parameter and
        return a (safe) iterator. */
        auto iter1 = rg_vo1.ss_begin(vo1_regptr1);
        auto citer1 = rg_vo1.ss_cend(vo1_regptr1);
        citer1 = iter1;
        rg_vo1.emplace(citer1, "some other text");
        rg_vo1.insert(citer1, "some other text");
        mse::nii_string str1 = "some other text";
        rg_vo1.insert(citer1, str1);
    
        class A {
        public:
            A() {}
            int m_i;
        };
        /* Here we're declaring that A can be safely shared between asynchronous threads. */
        typedef mse::TUserDeclaredAsyncShareableObj<A> shareable_A_t;
    
        /* When the element type of an nii_vector<> is marked as "async shareable", the nii_vector<> itself is
        (automatically) marked as async shareable as well and can be safely shared between asynchronous threads
        using "access requesters". */
        auto access_requester1 = mse::make_asyncsharedv2readwrite<mse::nii_vector<shareable_A_t>>();
        auto access_requester2 = mse::make_asyncsharedv2readwrite<nii_vector1_t>();
    
        /* If the element type of an nii_vector<> is not marked as "async shareable", then neither is the
        nii_vector<> itself. So attempting to create an "access requester" using it would result in a compile
        error. */
        //auto access_requester3 = mse::make_asyncsharedv2readwrite<mse::nii_vector<A>>();
        //auto access_requester4 = mse::make_asyncsharedv2readwrite<mse::nii_vector<mse::mstd::string>>();
    
        typedef mse::mstd::vector<mse::nii_string> vector1_t;
        vector1_t vo2 = { "a", "b", "c" };
        /* mstd::vector<>s, for example, are not safely shareable between threads. But if its element type is
        safely shareable, then the contents of the mse::mstd::vector<>, can be swapped with a corresponding
        shareable nii_vector<>. Note that vector swaps are intrinsically fast operations. */
        vo2.swap(*(access_requester2.writelock_ptr()));
    }
```

### msevector

`us::msevector<>` is not memory-safe in the way that the other vectors are. It can be used in cases where you want more control over the safety-performance trade-off.  

In addition to the (high performance) standard vector iterator, `us::msevector<>` also supports a new kind of iterator, called `ipointer`, that acts more like a list iterator in the sense that it points to an item rather than a position, and like a list iterator, it is not invalidated by insertions or deletions occurring elsewhere in the container, even if a "reallocation" occurs. Algorithms that work when applied to list iterators will work when applied to ipointers. This can be useful as Bjarne famously [points out](https://www.youtube.com/watch?v=YQs6IC-vgmo), for cache-coherency reasons, in most cases vectors should be used in place of lists, even when lists are conceptually more appropriate. You can read a short article comparing ipointers with some existing alternatives [here](http://www.codeproject.com/Articles/1087021/Stable-Iterators-for-Cplusplus-Vectors-and-Why-You).  

`us::msevector<>` also provides a safer bounds-checked version of the standard vector iterator. Note that none of these iterators are safe against the situation where the vector is deleted before an iterator is finished using it.

usage example:

```cpp
    #include "msemsevector.h"
    
    void main(int argc, char* argv[]) {
        
        mse::us::msevector<int> v1 = { 1, 2, 3, 4 };
        mse::us::msevector<int> v = v1;
        {
            mse::us::msevector<int>::ipointer ip1 = v.ibegin();
            ip1 += 2;
            assert(3 == (*ip1));
            auto ip2 = v.ibegin(); /* ibegin() returns an ipointer */
            v.erase(ip2); /* remove the first item */
            assert(3 == (*ip1)); /* ip1 continues to point to the same item, not the same position */
            ip1--;
            assert(2 == (*ip1));
            for (mse::us::msevector<int>::cipointer cip = v.cibegin(); v.ciend() != cip; cip++) {
                /* You might imagine what would happen if cip were a regular vector iterator. */
                v.insert(v.ibegin(), (*cip));
            }
        }
        v = v1;
        {
            /* This code block is equivalent to the previous code block, but uses ipointer's more "readable" interface
            that might make the code a little more clear to those less familiar with C++ syntax. */
            mse::us::msevector<int>::ipointer ip_vit1 = v.ibegin();
            ip_vit1.advance(2);
            assert(3 == ip_vit1.item());
            auto ip_vit2 = v.ibegin();
            v.erase(ip_vit2);
            assert(3 == ip_vit1.item());
            ip_vit1.set_to_previous();
            assert(2 == ip_vit1.item());
            mse::us::msevector<int>::cipointer cip(v);
            for (cip.set_to_beginning(); cip.points_to_an_item(); cip.set_to_next()) {
                v.insert_before(v.ibegin(), (*cip));
            }
        }
    
        /* Btw, ipointers are compatible with stl algorithms, like any other stl iterators. */
        std::sort(v.ibegin(), v.iend());
    
        /* And just to be clear, mse::us::msevector<> retains its original (high performance) stl vector iterators. */
        std::sort(v.begin(), v.end());
    
        /* mse::us::msevector<> also provides "safe" (bounds checked) versions of the original stl vector iterators. */
        std::sort(v.ss_begin(), v.ss_end());
    }
```

`ipointer`s support all the standard iterator operators, but also have member functions with "friendlier" names including:

```cpp
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
```

### ivector

`ivector<>` is for cases when safety and correctness are higher priorities than compatibility and performance. `ivector<>` drops support for the (problematic) standard vector iterator, replacing it with [`ipointer`](#msevector).

usage example:

```cpp
    #include "mseivector.h"
    
    void main(int argc, char* argv[]) {
    
        mse::ivector<int> iv = { 1, 2, 3, 4 };
        std::sort(iv.begin(), iv.end());
        mse::ivector<int>::ipointer ivip = iv.begin();
    }
```

### make_xscope_vector_size_change_lock_guard()

The `make_xscope_vector_size_change_lock_guard()` function is used, indirectly, to obtain a scope pointer to a vector element. The challenge with scope pointers to vector elements is that any operation that resizes or increases the capacity of the vector could cause the scope pointer to become invalid. So before obtaining a scope pointer, the vector needs to be "locked" to ensure that no such operation occurs. To this end, you can use the `make_xscope_vector_size_change_lock_guard()` function to create an `xscope_structure_change_lock_guard` object. You can obtain scope pointers to elements in the corresponding vector via its `xscope_ptr_to_element()` member function. While the object exists, any attempt to execute an operation that would cause the size of the vector to change (or capacity to increase) will cause an exception. `mstd::vector<>`, `ivector<>` and `us::msevector<>` are supported. `nii_vector<>` is not supported because the mechanism required to ensure memory safety would either compromise thread safety or require costly synchronization operations.

usage example:

```cpp
    #include "msemstdvector.h"
    
    void main(int argc, char* argv[]) {
    
        /* Here we're declaring an vector as a scope object. */
        mse::TXScopeObj<mse::mstd::vector<int>> vector1_scpobj = mse::mstd::vector<int>{ 1, 2, 3 };
        
        {
            /* In order to obtain a direct scope pointer to a vector element, you first need to instantiate a "structure lock"
            object, which "locks" the vector to ensure that no resize (or reserve) operation that might cause a scope pointer
            to become invalid is performed. */
            auto xscp_vector1_change_lock_guard = mse::mstd::make_xscope_vector_size_change_lock_guard(&vector1_scpobj);
            auto scp_ptr1 = xscp_vector1_change_lock_guard.xscope_ptr_to_element(2);
            auto res4 = *scp_ptr1;
        }
        // the vector is no longer "size change locked"
        vector1_scpobj.push_back(4);
    }
```

### Arrays

The library provides a few array types - [`mstd::array<>`](#array), [`nii_array<>`](#nii_array) and [`us::msearray<>`](#msearray) - which have properties similar to their corresponding [vector](#vectors) types. `mstd::array<>` is simply a memory-safe drop-in replacement for `std::array<>`. `nii_array<>` is designed to be safely shared between asynchronous threads. And `us::msearray<>` is not memory-safe in the way the other arrays are, and is provided for cases where more control over the safety-preformance trade-off is desired.

Note that these arrays currently do not support using [scope](#scope-pointers) types as the element type even when the array itself is declared as a scope object. It's expected that this will be supported in the future. The (few) cases where this would be an issue is when you want the element type to be a scope pointer or a type with scope pointer members. In those cases, you might use registered and/or refcounting pointers instead. 

And remember that you can use [`TRandomAccessSection<>`](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection) to provide access to a subsection of any vector or array.

### array

`mstd::array<>` is a memory-safe drop-in replacement for `std::array<>`. Note that the current implementation requires "`mseregistered.h`".  

usage example:

```cpp
    #include "msemstdarray.h"
    #include "msemsearray.h"
    #include <array>
    
    void main(int argc, char* argv[]) {
    
        mse::mstd::array<int, 3> ma;
        std::array<int, 3> sa;
        /* These two arrays should be completely interchangeable. The difference being that ma should throw
        an exception on any attempt to access invalid memory. */
    
    
        /* mse::msearray is not quite as safe as mse::mstd::array in the following way: */
    
        std::array<int, 3>::iterator sa1_it;
        mse::us::msearray<int, 3>::ss_iterator_type msea1_it; // bounds checked iterator just like mse::mstd::array::iterator
        mse::mstd::array<int, 3>::iterator ma1_it;
        {
            std::array<int, 3> sa1 = { 1, 2, 3 };
            sa1_it = sa1.begin();
    
            mse::us::msearray<int, 3> msea1 = { 1, 2, 3 };
            msea1_it = msea1.ss_begin();
    
            mse::mstd::array<int, 3> ma1 = { 1, 2, 3 };
            ma1_it = ma1.begin();
        }
    
        // (*sa1_it) = 4; // not good - undefined behavior
        // (*msea1_it) = 4; // not good - undefined behavior
    
        try {
            (*ma1_it) = 4; // not undefined behavior - will throw an exception
        } catch(...) {
            // expected exception
        }
    }
```

### nii_array

`nii_array<>` is just the corresponding array version of [`nii_vector<>`](#nii_vector). It is designed such that it can be safely shared between asynchronous threads.

### msearray

`us::msearray<>`, like `us::msevector<>`, is not memory-safe in the way that the other arrays are. And like `us::msevector<>`, `us::msearray<>` provides a safer iterator, in addition to the (high performance) standard iterator. Like `us::msevector<>`, `us::msearray<>`'s safe iterator also supports the more "readable" interface. In cases where the msearray is declared as a scope object, you can also use a "scope" version of the safe iterator. The restrictions on when and how scope iterators can be used ensure that they won't be used to access the array after it's been deallocated.  

usage example:

```cpp
    #include "msemsearray.h"
    #include <array>
    
    void main(int argc, char* argv[]) {
        mse::us::msearray<int, 3> a1 = { 1, 2, 3 };
        mse::us::msearray<int, 3> a2 = { 11, 12, 13 };
        
        //bool bres1 = (a1.begin() == a2.end());
        /* The previous commented out line would result in "undefined behavior. */
        
        try {
            /* The behavior of the next line is not "undefined". It's going to throw an exception. */
            bool bres2 = (a1.ss_begin() == a2.ss_end());
        }
        catch (...) {
            std::cerr << "expected exception" << std::endl;
        }
        
        auto ss_cit1 = a1.ss_cbegin();
        /* These safe iterators support traditional and "friendly" iterator operation syntax. */
        ss_cit1++;
        ss_cit1.set_to_next(); /*same as previous line*/
        ss_cit1.set_to_beginning();
        bool bres3 = ss_cit1.has_previous();
        ss_cit1.set_to_end_marker();
        bool bres4 = ss_cit1.points_to_an_item();
    
        {
            /* A "scope" version of the safe iterators can be used when the array is declared as a scope
            object. There are limitations on when thay can be used, but unlike the other msearray iterators,
            those restrictions ensure that they won't be used to access the array after it's been deallocated. */
            
            mse::TXScopeObj<mse::us::msearray<int, 3>> array1_scpobj = mse::us::msearray<int, 3>{ 1, 2, 3 };
            
            auto scp_ss_iter1 = mse::make_xscope_iterator(&array1_scpobj);
            scp_ss_iter1.set_to_beginning();
            auto scp_ss_iter2 = mse::make_xscope_iterator(&array1_scpobj);
            scp_ss_iter2.set_to_end_marker();
            
            std::sort(scp_ss_iter1, scp_ss_iter2);
            
            auto scp_ss_citer3 = mse::make_xscope_const_iterator(&array1_scpobj);
            scp_ss_citer3 = scp_ss_iter1;
            scp_ss_citer3 = array1_scpobj.ss_cbegin();
            scp_ss_citer3 += 2;
            auto res1 = *scp_ss_citer3;
            auto res2 = scp_ss_citer3[0];
            
            /* Here we demonstrate the case where the array is a member of a class/struct declared as a
            scope object. */
            class CContainer1 {
            public:
                CContainer1() : m_array({ 1, 2, 3 }) {}
                
                mse::us::msearray<int, 3> m_array;
            };
            mse::TXScopeObj<CContainer1> container1_scpobj;
            auto container1_m_array_scpptr = mse::make_pointer_to_member(container1_scpobj.m_array, &container1_scpobj);
            auto scp_ss_citer4 = mse::make_xscope_iterator(container1_m_array_scpptr);
            scp_ss_citer4++;
            auto res3 = *scp_ss_citer4;
        }
    }
```

### xscope_iterator

The implementation of, for example, `mstd::array<>` iterators uses [registered pointers](#registered-pointers) to ensure that iterators are not used to access array elements after the array has been deallocated. This incurs a slight run-time cost. So just as the library provides [scope pointers](#scope-pointers) without run-time cost, scope iterators for arrays are also provided. Scope iterators have usage restrictions similar to scope pointers. For example, they can only target arrays declared as scope objects, and may not be used as a member of any class or struct that is not itself a scope object, and may not be used as a function return value. `mstd::array<>`, `nii_array<>` and `us::msearray<>` all support scope iterators.

usage example:

```cpp
    #include "msemstdarray.h"
    
    void main(int argc, char* argv[]) {
        /* If the array is declared as a "scope" object (which basically indicates that it is declared
        on the stack), then you can use "scope" iterators. While there are limitations on when they can
        be used, scope iterators would be the preferred iterator type where performance is a priority
        as they don't require extra run time overhead to ensure that the array has not been deallocated. */
        
        /* Here we're declaring an array as a scope object. */
        mse::TXScopeObj<mse::mstd::array<int, 3>> array1_scpobj = mse::mstd::array<int, 3>{ 1, 2, 3 };
        
        /* Here we're obtaining a scope iterator to the array. */
        auto scp_array_iter1 = mse::mstd::make_xscope_iterator(&array1_scpobj);
        scp_array_iter1 = array1_scpobj.begin();
        auto scp_array_iter2 = mse::mstd::make_xscope_iterator(&array1_scpobj);
        scp_array_iter2 = array1_scpobj.end();
        
        std::sort(scp_array_iter1, scp_array_iter2);
        
        auto scp_array_citer3 = mse::mstd::make_xscope_const_iterator(&array1_scpobj);
        scp_array_citer3 = scp_array_iter1;
        scp_array_citer3 = array1_scpobj.cbegin();
        scp_array_citer3 += 2;
        auto res1 = *scp_array_citer3;
        auto res2 = scp_array_citer3[0];
        
        /* Here we demonstrate the case where the array is a member of a class/struct declared as a
        scope object. */
        class CContainer1 {
        public:
            CContainer1() : m_vector({ 1, 2, 3 }) {}
            mse::mstd::array<int, 3> m_array;
        };
        mse::TXScopeObj<CContainer1> container1_scpobj;
        auto container1_m_array_scpptr = mse::mstd::make_pointer_to_member(container1_scpobj.m_array, &container1_scpobj);
        auto scp_iter4 = mse::mstd::make_xscope_iterator(container1_m_array_scpptr);
        scp_iter4++;
        auto res3 = *scp_iter4;
    }
```

### xscope_pointer_to_array_element()

You can use this function to obtain a scope pointer to an array element. You can pass it ethier an xscope_iterator or a scope pointer to an array and an index. `mstd::array<>`, `nii_array<>` and `us::msearray<>` are supported.

usage example:

```cpp
    #include "msemstdarray.h"
    
    void main(int argc, char* argv[]) {
    
        /* Here we're declaring an array as a scope object. */
        mse::TXScopeObj<mse::mstd::array<int, 3>> array1_scpobj = mse::mstd::array<int, 3>{ 1, 2, 3 };
        
        /* Here we're obtaining a scope iterator to the array. */
        auto scp_array_iter1 = mse::mstd::make_xscope_iterator(&array1_scpobj);
        scp_array_iter1 = array1_scpobj.begin();
        
        /* You can also obtain a corresponding scope pointer from a scope iterator. */
        auto scp_ptr1 = mse::mstd::xscope_pointer_to_array_element<int, 3>(scp_array_iter1);
        auto res1 = *scp_ptr1;
        /* Or with a scope pointer to the array and an index. */
        auto scp_cptr2 = mse::mstd::xscope_const_pointer_to_array_element<int, 3>(&array1_scpobj, 2/*element index*/);
        auto res2 = *scp_cptr2;
    }
```

### for_each() specializations

`std::for_each()` template specializations for the library's safe iterators are not yet implemented, but are hopefully coming soon. In theory, using `std::for_each()` could provide a performance benefit over regular "ranged-based loops", as it eliminates the need for bounds checking of the loop iterator. Note that the specialization for `mstd::vector<>` will hold a "[size change lock guard](#make_xscope_vector_size_change_lock_guard)", preventing the size of the vector from being changed during iteration.

Also, parallel execution policies, will only be supported for specializations using the async shared iterators provided in the library.

### TXScopeRandomAccessSection, TXScopeRandomAccessConstSection, TRandomAccessSection, TRandomAccessConstSection

A "random access section" is basically a convenient interface to access a (contiguous) subsection of an existing array or vector. (Essentially an "array view" or "span" if you're familiar with those.) You construct them, using the `make_random_access_section()` functions, by specifying an iterator to the start of the section, and the length of the section. Random access sections support most of the member functions and operators that [std::basic_string_view](http://en.cppreference.com/w/cpp/string/basic_string_view) does, except that the "[substr()](http://en.cppreference.com/w/cpp/string/basic_string_view/substr)" member function is named "subsection()".

Note that for convenience, random access sections can be constructed from just a (safe) pointer to a supported container object, but in some cases the exact type of the resulting random access section may not be obvious. Constructing instead from a specified iterator and length should avoid any ambiguity.

usage example:

```cpp
    #include "msemsearray.h" //random access sections are defined in this file
    #include "msemstdarray.h"
    #include "msemstdvector.h"
    
    class J {
    public:
        template<class _TRASection>
        static void foo13(_TRASection ra_section) {
            for (_TRASection::size_type i = 0; i < ra_section.size(); i += 1) {
                ra_section[i] = 0;
            }
        }
        template<class _TRAConstSection>
        static int foo14(_TRAConstSection const_ra_section) {
            int retval = 0;
            for (_TRAConstSection::size_type i = 0; i < const_ra_section.size(); i += 1) {
                retval += const_ra_section[i];
            }
            return retval;
        }
        template<class _TRAConstSection>
        static int foo15(_TRAConstSection const_ra_section) {
            int retval = 0;
            for (const auto& const_item : const_ra_section) {
                retval += const_item;
            }
            return retval;
        }
    };

    void main(int argc, char* argv[]) {
        mse::mstd::array<int, 4> mstd_array1{ 1, 2, 3, 4 };
        mse::mstd::vector<int> mstd_vec1{ 10, 11, 12, 13, 14 };

        auto xscp_ra_section1 = mse::make_xscope_random_access_section(mstd_array1.begin(), 2);
        J::foo13(xscp_ra_section1);

        auto ra_const_section2 = mse::make_random_access_const_section(++mstd_vec1.begin(), 3);
        auto res6 = J::foo15(ra_const_section2);
        auto res7 = J::foo14(ra_const_section2);

        auto xscp_ra_section1_xscp_iter1 = xscp_ra_section1.xscope_begin();
        auto xscp_ra_section1_xscp_iter2 = xscp_ra_section1.xscope_end();
        auto res8 = xscp_ra_section1_xscp_iter2 - xscp_ra_section1_xscp_iter1;
        bool res9 = (xscp_ra_section1_xscp_iter1 < xscp_ra_section1_xscp_iter2);
        
        /* Like non-owning scope pointers, scope sections may not be used as a function return value. (The return_value()
        function wrapper enforces this.) Pretty much the only time you'd legitimately want to do this is when the
        returned section is constructed from one of the input parameters. Let's consider a simple example of a
        "first_half()" function that takes a scope section and returns a scope section spanning the first half of the
        section. The library provides the random_access_subsection() function which takes a random access section and a
        tuple containing a start index and a length and returns a random access section spanning the indicated
        subsection. You could use this function to implement the equivalent of a "first_half()" function like so: */
        
        auto xscp_ra_section3 = mse::random_access_subsection(xscp_ra_section1, std::make_tuple(0, xscp_ra_section1.length()/2));
        assert(xscp_ra_section3.length() == 1);
    }
```

### Strings

From an interface perspective, you might think of strings roughly as glorified vectors of characters, and thus they are given similar treatment in the library. A couple of string types are provided that correspond to their [vector](#vectors) counterparts. [`mstd::string`](#string) is simply a memory-safe drop-in replacement for std::string. Due to their iterators, strings are not, in general, safe to share among threads. [`nii_string`](#nii_string) is designed for safe sharing among asynchronous threads. 

### string

`mstd::string` is a memory-safe drop-in replacement for `std::string`. As with the standard library, `mstd::string` is defined as an alias for `mstd::basic_string<char>`. The `mstd::wstring`, `mstd::u16string` and `mstd::u32string` aliases are also present.

### nii_string

`nii_string` is a string type designed to be safely shareable between asynchronous threads. See the corresponding [`nii_vector<>`](#nii_vector) for more information. Like `mstd::string`, `nii_string` is defined as an alias of `nii_basic_string<char>`. The `nii_wstring`, `nii_u16string` and `nii_u32string` aliases are also present.

### TXScopeStringSection, TXScopeStringConstSection, TStringSection, TStringConstSection

"String sections" are string specialized versions of "[random access sections](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection)". 

usage example:

```cpp
    #include "msemsestring.h" // make_string_section() is defined in this header
    #include "msemstdstring.h"
    
    void main(int argc, char* argv[]) {

        /* "String sections" are the string specialized versions of "random access sections", basically providing the
        functionality of std::string_view but supporting construction from any (safe) iterator type, not just raw
        pointer iterators. */
    
        mse::mstd::string mstring1("some text");
        auto string_section1 = mse::make_string_section(mstring1.begin() + 1, 7);
        auto string_section2 = string_section1.substr(4, 3);
        assert(string_section2.front() == 't');
        assert(string_section2.back() == 'x');
    
        /* Unlike std::string_view, string sections are available in "non-const" versions. */
        string_section2[0] = 'T';
        std::cout << string_section2;
        assert(mstring1 == "some Text");
    }
```

### TXScopeNRPStringSection, TXScopeNRPStringConstSection, TNRPStringSection, TNRPStringConstSection

`TNRPStringSection<>` is just a version of [`TStringSection<>`](#txscopestringsection-txscopestringconstsection-tstringsection-tstringconstsection) that, for enhanced safety, does not support construction from unsafe raw pointer iterators or (unsafe) `std::string` iterators. Use the `make_nrp_string_section()` functions to create them.

### string_view

`std::string_view` is, in a way, a problematic addition to the standard library in the sense that it has an intrinsically unsafe interface. That is, its constructors support only (unsafe) raw pointer iterator parameters. In contrast, the standard library generally uses iterator types which allow for the option of a memory safe implementation. So to enable memory safe use, this library's version, `mstd::string_view`, generalizes the interface to support construction from safe iterator types. So while technically `mstd::string_view` can act as a drop-in replacement for `std::string_view`, it is designed to be used with safe iterator types, not unsafe raw pointer iterators.

Like `std::string_view`, `mstd::string_view` is defined as an alias for `mstd::basic_string_view<char>`. The `mstd::wstring_view`, `mstd::u16string_view` and `mstd::u32string_view` aliases are also present. Note that `mstd::basic_string_view<>` is in fact just a slightly augmented version of [`TAnyStringConstSection<>`](#txscopeanystringsection-txscopeanystringconstsection-tanystringsection-tanystringconstsection).

usage example:

```cpp
    #include "msepoly.h" // mstd::string_view is defined in this header
    #include "msemstdstring.h"
    
    void main(int argc, char* argv[]) {
    
        /* std::string_view stores an (unsafe) pointer iterator into its target string. mse::mstd::string_view can
        instead store any type of string iterator, including memory safe iterators. So for example, when assigned
        from an mse::mstd::string, mse::mstd::string_view will hold one of mse::mstd::string's safe (strong) iterators
        (obtained with a call to the string's cbegin() member function). Consequently, the mse::mstd::string_view will
        be safe against "use-after-free" bugs to which std::string_view is so prone. */
    
        mse::mstd::string_view msv1;
        {
            mse::mstd::string mstring1("some text");
            msv1 = mstring1;
        }
        try {
            /* This is not undefined (or unsafe) behavior. Either an exception will be thrown or it will just work. */
            auto ch1 = msv1[3];
            assert('e' == ch1);
        }
        catch (...) {
            /* At present, no exception will be thrown. Instead, the lifespan of the string data is extended to match
            that of the mstd::string_view. In the future, an exception may be thrown in debug builds. */
            std::cerr << "potentially expected exception" << std::endl;
        }
    
        mse::mstd::string mstring2("some other text");
        /* With std::string_view, you specify a string subrange with a raw pointer iterator and a length. With
        mse::mstd::string_view you are not restricted to (unsafe) raw pointer iterators. You can use memory safe
        iterators like those provided by mse::mstd::string. */
        auto msv2 = mse::mstd::string_view(mstring2.cbegin()+5, 7);
        assert(msv2 == "other t");
    }
```

### nrp_string_view

`mse::nrp_string_view` is just a version of [`mse::mstd::string_view`](#string_view) that, for enhanced safety, does not support construction from unsafe raw pointer iterators or (unsafe) `std::string` iterators.

### optional, xscope_optional

`mse::mstd::optional<>` is simply a safe implementation of `std::optional<>`. `mse::xscope_optional<>` is the scope version which is subject to the restrictions of all scope objects. The (uncommon) reason you might need to use `mse::xscope_optional<>` rather than just `mse::TXScopeObj<mse::mstd::optional<> >` is that `mse::xscope_optional<>` supports using scope types (including scope pointer types) as its element type. 

### Compatibility considerations
People have [asked](http://stackoverflow.com/questions/2143020/why-cant-i-inherit-from-int-in-c) why the primitive C++ types can't be used as base classes. It turns out that really the only reason primitive types weren't made into full-fledged classes is that they inherit these "chaotic" conversion rules from C that can't be fully mimicked by C++ classes, and Bjarne thought it would be too ugly to try to make special case classes that followed different conversion rules.  

But while substitute classes cannot be 100% compatible substitutes for their corresponding primitives, they can still be mostly compatible. And if you're writing new code or maintaining existing code, it should be considered good coding practice to ensure that your code is compatible with C++'s conversion rules for classes and not dependent on the "chaotic" legacy conversion rules of primitive types.

If you are using legacy code or libraries where it's not practical to update the code, it shouldn't be a problem to continue using primitive types there and the safer substitute classes elsewhere in the code. The safer substitute classes generally have no problem interacting with primitive types, although in some cases you may need to do some explicit type casting. [Registered pointers](#registered-pointers) can be cast to raw pointers, and, for example, [`CInt`](#primitives) can participate in arithmetic operations with regular `int`s.

### Practical limitations

The degree of memory safety that can be achieved is a function of the degree to which use of C++'s (memory) unsafe elements is avoided. Unfortunately, there is not yet a tool to automatically identify such uses. But if, in the future, there is significant demand for such a tool, it shouldn't be a particulary difficult thing to develop. Certainly trivial compared to some of the existing static analysis tools.

Note that one of C++'s more subtle unsafe elements is the implicit `this` pointer when accessing member variables from member functions. Consider this example:

```cpp
    #include "msescope.h"
    #include "msemstdvector.h"
    
    class CI {
    public:
        template<class safe_vector_pointer_type>
        void foo1(safe_vector_pointer_type vec_ptr) {
            vec_ptr->clear();
    
            /* These next two lines are equivalent and technically unsafe. */
            m_i += 1;
            this->m_i += 1;
        }
    
        int m_i = 0;
    };
    
    void main() {
        mse::TXScopeObj<mse::mstd::vector<CI>> vec1;
        vec1.resize(1);
        auto iter = vec1.begin();
        iter->foo1(&vec1);
    }
```

The above example contains unchecked accesses to deallocated memory via an implicit and explicit `this` pointer. The `this` pointer (implicit or explicit) is a native pointer, and like any other native pointer, is unsafe and can/should be replaced with a safer substitute:

```cpp
    #include "msescope.h"
    #include "msemstdvector.h"
    
    class CI {
    public:
        template<class safe_this_type, class safe_vector_pointer_type>
        static void foo2(safe_this_type safe_this, safe_vector_pointer_type vec_ptr) {
            vec_ptr->clear();
    
            /* The safe_this pointer will catch the attempted invalid memory access. */
            safe_this->m_i += 2;
        }
    
        int m_i = 0;
    };
    
    void main() {
        mse::TXScopeObj<mse::mstd::vector<CI>> vec1;
        vec1.resize(1);
        auto iter = vec1.begin();
        iter->foo2(iter, &vec1);
    }
```

So, technically, achieving complete memory safety requires passing a safe `this` pointer parameter as an argument to every member function that accesses a member variable. (I.e. Make your member functions `static`.)

And also, SaferCPlusPlus does not yet provide safer substitutes for all of the standard library containers, just the ones responsible for the most problems (vector and array). So be careful with your maps, sets, etc. In many cases lists can be replaced with [`ivector<>`](#ivector)s that support list-style iterators, often with a performance benefit.

### Questions and comments
If you have questions or comments you can create a post in the [issues section](https://github.com/duneroadrunner/SaferCPlusPlus/issues).
