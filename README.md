Feb 2017

### Overview

"SaferCPlusPlus" is essentially a collection of safe data types that are compatible with, and can substitute for, common unsafe native C++ types. Currently these include:

- A [fast](#simple-benchmarks), safe general [replacement for native pointers](#registered-pointers).

- A faster, smaller, safer [reference counting pointer](#reference-counting-pointers) that can substitute for std::shared_ptr in situations where the target is not shared between asynchronous threads. Including [safe parameter passing](#safely-passing-parameters-by-reference) by reference.

- A "[scope pointer](#scope-pointers)" for target objects allocated on the stack, or whose "owning" pointer is allocated on the stack. By default, not quite as safe as the other smart pointers in this library, but with zero runtime overhead.

- An almost completely [safe implementation](#vector) of std::vector<> - bounds checked, iterator checked and memory managed.

- A couple of [other](#vectors) highly compatible vectors that address the issue of unnecessary iterator invalidation upon insert, erase or reallocation.

- An almost completely [safe implementation](#array) of std::array<> - bounds checked, iterator checked and "lifespan aware".

- [Replacements](#primitives) for the native "int", "size_t" and "bool" types that ensure against the use of uninitialized values and address the "signed-unsigned mismatch" issues.

- Data types for safe, simple [sharing](#asynchronously-shared-objects) of objects among asynchronous threads.

Tested with msvc2015, g++5.3 and clang++3.8 (as of Jan 2017). Support for versions of g++ prior to version 5 was dropped on Mar 21, 2016.

You can have a look at [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp) to see the library in action. You can also check out some [benchmark code](https://github.com/duneroadrunner/SaferCPlusPlus-BenchmarksGame) where you can compare traditional C++ and SaferCPlusPlus implementations of the same algorithms.


### Table of contents
1. [Overview](#overview)
2. [Use cases](#use-cases)
3. [Setup and dependencies](#setup-and-dependencies)
4. Comparisons
    1. [SaferCPlusPlus versus Clang/LLVM Sanitizers](#safercplusplus-versus-clangllvm-sanitizers)
    2. [SaferCPlusPlus versus Checked C](#safercplusplus-versus-checked-c)
    3. [SaferCPlusPlus versus Ironclad C++](#safercplusplus-versus-ironclad-c)
    4. [SaferCPlusPlus versus Rust](#safercplusplus-versus-rust)
    5. [SaferCPlusPlus versus the Core Guidelines Checkers](#safercplusplus-versus-the-core-guidelines-checkers)
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
    1. [TXScopeFixedPointer](#txscopefixedpointer)
    2. [TXScopeOwnerPointer](#txscopeownerpointer)
10. [make_pointer_to_member()](#make_pointer_to_member)
11. [Poly pointers](#poly-pointers)
    1. [TXScopePolyPointer](#txscopepolypointer-txscopepolyconstpointer)
    2. [TPolyPointer](#tpolypointer-tpolyconstpointer)
    3. [TAnyPointer](#txscopeanypointer-txscopeanyconstpointer-tanypointer-tanyconstpointer)
    4. [TAnyRandomAccessIterator](#txscopeanyrandomaccessiterator-txscopeanyrandomaccessconstiterator-tanyrandomaccessiterator-tanyrandomaccessconstiterator)
    5. [TRandomAccessSection](#txscoperandomaccesssection-txscoperandomaccessconstsection-trandomaccesssection-trandomaccessconstsection)
12. [Safely passing parameters by reference](#safely-passing-parameters-by-reference)
13. [Asynchronously shared objects](#asynchronously-shared-objects)
    1. [TAsyncSharedReadWriteAccessRequester](#tasyncsharedreadwriteaccessrequester)
        1. [TAsyncSharedReadOnlyAccessRequester](#tasyncsharedreadonlyaccessrequester)
    2. [TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester](#tasyncsharedobjectthatyouaresurehasnounprotectedmutablesreadwriteaccessrequester)
    3. [TStdSharedImmutableFixedPointer](#tstdsharedimmutablefixedpointer)
14. [Primitives](#primitives)
    1. [CInt, CSize_t and CBool](#cint-csize_t-and-cbool)
    2. [Quarantined types](#quarantined-types)
15. [Vectors](#vectors)
    1. [mstd::vector](#vector)
    2. [msevector](#msevector)
    3. [ivector](#ivector)
16. [Arrays](#arrays)
    1. [mstd::array](#array)
    2. [msearray](#msearray)
17. [Compatibility considerations](#compatibility-considerations)
18. [On thread safety](#on-thread-safety)
19. [Practical limitations](#practical-limitations)
20. [Questions and comments](#questions-and-comments)



### Use cases

This library is appropriate for use by two groups of C++ developers - those for whom safety and security are critical, and also everybody else. This library can help eliminate a lot of the opportunities for inadvertently accessing invalid memory or using uninitialized values. It essentially gets you [a lot](#practical-limitations) of the memory safety that you might get from say, Java, while retaining all of the power and most of the performance of C++.  

While using the library can incur a modest performance penalty, because the library elements are [largely compatible](#compatibility-considerations) with their native counterparts, they can be easily "disabled" (automatically replaced with their native counterparts) with a compile-time directive, allowing them to be used to help catch bugs in debug/test/beta builds while incurring no overhead in release builds.  

And note that the safe components of this library can be adopted completely incrementally. New code written with these safe elements will play nicely with existing (unsafe) code, and unsafe elements can be replaced selectively without breaking the existing code. So there is really no excuse for not using the library in pretty much any situation.  

Though for real time embedded applications, note the dependence on the standard library. Also, you may want to override the default behavior upon invalid memory operations (using MSE_CUSTOM_THROW_DEFINITION(x)) and read the notes in the [array](#array) section.  

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

### SaferCPlusPlus versus Checked C

"Checked C", like SaferCPlusPlus, takes the approach of extending the language with safer elements that can directly substitute for unsafe native elements. In chapter 9 of their [spec](https://github.com/Microsoft/checkedc/releases/download/v0.5-final/checkedc-v0.5.pdf), there is an extensive survey of existing (and historical) efforts to address C/C++ memory safety. There they make the argument for the (heretofore neglected) "language extension" approach (basically citing performance, compatibility and the support for granular mixing of safe and unsafe code), that applies to SaferCPlusPlus as well.

Checked C and SaferCPlusPlus are more complementary than competitive. Checked C targets low-level system C code and basically only addresses the array bounds checking issue, including pointer arithmetic, where SaferCPlusPlus skews more toward C++ code and legacy code that would benefit from being converted to modern C++. It seems that Checked C is not yet ready for deployment (as of Sep 2016), but one could imagine both solutions being used, with little contention, in projects that have both low-level system type code and higher-level application type code.

### SaferCPlusPlus versus Ironclad C++

SaferCPlusPlus and Ironclad C++ are very similar. The main difference is probably that Ironclad uses garbage collection while SaferCPlusPlus does not. SaferCPlusPlus is not yet as complete as Ironclad (for example, SaferCPlusPlus does not yet have a static validator to verify that "scope" pointers are being used properly), but Ironclad seems to be no longer under active development. They are not incompatible, both libraries could be used in the same project. Rather than thinking of them as competing solutions, you could think of them combined as one solution, sometimes with multiple options for achieving the same thing.  

While both solutions address the pointer/reference safety issue, SaferCPlusPlus also provides safer replacements for int and size_t, and data types for safely sharing objects between asynchronous threads.  

If you're considering one or the other solution (or both), I would suggest starting out with SaferCPlusPlus, as adopting it is essentially risk free. It's basically just a collection of header files written in portable C++. And can be easily disabled (i.e. its elements can be automatically aliased to their native counterparts) with a compile-time directive. Whereas because of its dependence on its garbage collector, Ironclad doesn't have quite the same properties. And if later you decide to adopt the Ironclad solution, or parts of it, the transition should be painless.  

There is a comprehensive paper on Ironclad C++ [here](https://www.cs.rutgers.edu/~santosh.nagarakatte/papers/ironclad-oopsla2013.pdf). It's a beneficial read even for those not planning on adopting Ironclad, as the the approach has much in common with SaferCPlusPlus.  

### SaferCPlusPlus versus Rust

SaferCPlusPlus and Rust both rely on a combination of compile-time code restrictions and run-time checks to achieve memory safety. Rust leans heavily toward the former and SaferCPlusPlus a little more toward the latter. It's probably the similarities between SaferCPlusPlus and Rust that's most notable, considering they were developed independently. Indeed, if you are a Rust programmer you might be more comfortable using SaferCPlusPlus than traditional C++ once you realize the (loose) correspondence between Rust and SaferCPlusPlus elements:

Rust | SaferCPlusPlus
---- | --------------
non-reassignable reference | scope pointer
reassignable reference | registered pointer
Box<> | scope owner pointer
Rc<> | reference counting pointer
Arc<> | shared immutable pointer
Arc< Mutex<> > | access requester

Probably the main difference between Rust and SaferCPlusPlus is that SaferCPlusPlus does not restrict the number and type of references to an object that can exist at one time (i.e. the exclusivity of mutable references) the way Rust does. Rust uses this restriction to (among other things) help ensure that dynamic objects are not deallocated while other references to that object still exist. SaferCPlusPlus, on the other hand, deals with this issue by having the pointer/reference itself "know" if its target dynamic object is still valid. This can result in a little run-time overhead that a Rust implementation might tend to avoid. But perhaps unintuitively, this tends to have little effect on performance in practice. In part because there is often opportunity for the compiler optimizer to discard redundant run-time overhead, but mainly because with both SaferCPlusPlus and Rust, it's usually the case that the vast majority of pointer/reference instances end up spending their entire existence targeting a single object with "scope lifetime" (which incurs no run-time overhead in either solution). In practice, we observe that in both cases, the largest contributor to run-time overhead actually tends to be bounds checking of vectors and arrays.

A sample [subset of "The Computer Language Benchmark Game"](https://github.com/duneroadrunner/SaferCPlusPlus-BenchmarksGame) showed SaferCPlusPlus translations of C++ implementations to be, on average, about 30% slower than the original, with wide variation. At the time of this writing (Feb 2017), the Rust implementations of the same subset were reported as also, on average, about 30% slower than the C++ implementations, with very wide variation. For various reasons, these benchmarks should not be considered an accurate measure of intrinsic language performance. (The underperformance versus C++ probably being overstated in both cases.) But we can probably take away two things from these results - i) SaferCPlusPlus and Rust are both at least within the same order of magnitude of C++, in terms of performance, and ii) any theoretical performance difference between SaferCPlusPlus and Rust due to extra run-time checks on pointers/references is, in practice, not significant relative to the observed performance variation due to other factors.

So, perhaps as expected, you could think of the comparison between SaferCPlusPlus and Rust as essentially the comparison between C++ and Rust, with diminished discrepancies in memory safety and performance.  

### SaferCPlusPlus versus the Core Guidelines Checkers

At the time of this writing (Nov 2016), the Core Guidelines Checkers were still a work in progress and so didn't yet provide any memory safety guarantees for your code. But the goal is that at some point the Checkers (the Lifetimes Checker in particular) will be able to detect all potentially unsafe memory operations, with a reasonable proportion of false positives (apparently "under 10%" is the goal). That should be a big boon for C++ memory safety and performance if/when that is achieved. But even then there will still be the issue of how best to address the false (and true) positive concerns of the checkers. Often, appeasing static code verifiers will seem to require major code changes and/or a significant performance penalty. In those cases, SaferCPlusPlus is well suited to ensure memory safety with straightforward code modifications and minimal performance cost.

In the mean time, SaferCPlusPlus is, in general, not a substitute for, or incompatible with static analyzers. You are encouraged to use both.  

### Getting started on safening existing code

The elements in this library are straightforward enough that a separate tutorial, beyond the examples given in the documentation, is probably not necessary. But if you're wondering how best to start, probably the easiest and most effective thing to do is to replace the vectors and arrays in your code (that aren't being shared between threads) with [mse::mstd::vector](#vector) and [mse::mstd::array](#array).

The header files you'll need to include in your source file are "msemstdvector.h" and "msemstdarray.h". Those include files have additional dependencies on "msemsevector.h", "msemsearray.h", and possibly "mseprimitives.h".

Statistically speaking, doing this should already catch a significant chunk of potential memory bugs. By default, an exception will be thrown upon any attempt to access invalid memory. If your project is not using C++ exceptions, you'll probably want to override the default exception behavior by defining the MSE_CUSTOM_THROW_DEFINITION() preprocessor macro prior to inclusion of the header files. For example:

    #define MSE_CUSTOM_THROW_DEFINITION(x) std::cerr << std::endl << x.what(); exit(-11)

will cause the error description to be written to stderr before program termination.

The next most effective thing to do, in terms of improving memory safety, is probably to replace calls to new/malloc and delete/free. The direct substitutes provided in the library (for items not shared between threads) are mse::registered_new() and mse::registered_delete(). The pointer type returned by mse::registered_new() is an [mse::TRegisteredPointer<>](#tregisteredpointer). If you need this pointer to interact with legacy interfaces, it can be explicitly cast to a corresponding native pointer. If explicit casting is too inconvenient for your situation, you may instead use mse::relaxedregistered_new() to obtain an [mse::TRelaxedRegisteredPointer<>](#trelaxedregisteredpointer), which implicitly converts to a corresponding native pointer. But ultimately you're going to want to minimize the amount of casting to (unsafe) native pointers by updating your (function) interfaces to accomodate these safe pointers directly. (See the "[Safely passing parameters by reference](#safely-passing-parameters-by-reference)" section.)

Based on reported vulnerabilities, these two things alone should catch most memory bugs.

While the library provides these direct substitutes for new/malloc and delete/free, they are usually not the optimal solution. In most cases, you can instead use [mse::TXScopeOwnerPointer<>](#txscopeownerpointer) or [TRefCountingNotNullPointer<>](#trefcountingnotnullpointer), which are faster and automatically deallocate the item for you.

For items shared between asynchronous threads, use one of the [data types designed for safe asynchronous sharing](#asynchronously-shared-objects).

After that, it's just a matter of replacing the remaining unsafe elements in your code (generally native pointers and references) with the safer substitute that works best. You might want to leave C++ references for last, because a) they seem to be empirically (if not theoretically) less prone to bugs than pointers, and b) the library does not provide a directly compatible substitute (although [TRegisteredRefWrapper<>](#tregisteredrefwrapper) can be used in some situations), so references generally have to be substituted with pointers, which involves the extra bit of work of changing your dots to arrows.

And if at some point you feel that these new elements involve a lot of typing, note that many of the elements have short aliases that can be used instead. Just search for "shorter aliases" in the header files. Or, of course, you can create your own to suit your preferences.

### Registered pointers

"Registered" pointers are intended to behave just like native C++ pointers, except that their value is (automatically) set to nullptr when the target object is destroyed. And by default they will throw an exception upon any attempt to dereference a nullptr. Because they don't take ownership like some other smart pointers, they can point to objects allocated on the stack as well as the heap. In most cases, they can be used as a compatible, direct substitute for native pointers, making it straightforward to update legacy code (to be safer).

Registered pointers come in two flavors - [TRegisteredPointer](#tregisteredpointer) and [TRelaxedRegisteredPointer](#trelaxedregisteredpointer). They are both very similar. TRegisteredPointer emphasizes speed and safety a bit more, while TRelaxedRegisteredPointer emphasizes compatibility and flexibility a bit more. If you want to undertake the task of en masse replacement of native pointers in legacy code, or need to interact with legacy native pointer interfaces, TRelaxedRegisteredPointer may be more convenient.

Note that these registered pointers cannot target types that cannot act as base classes. The primitive types like int, bool, etc. [cannot act as base classes](#compatibility-considerations). Fortunately, the library provides safer [substitutes](#primitives) for int, bool and size_t that can act as base classes. Also note that these registered pointers are not thread safe. While we [do not encourage](#on-thread-safety) the casual sharing of objects between asynchronous threads, if you need to do so consider using the [safe sharing data types](#asynchronously-shared-objects) in this library. For more information on how to use the safe smart pointers in this library for maximum memory safety, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).

Although registered pointers are more general and flexible, it's expected that [scope pointers](#scope-pointers) will actually be more commonly used. At least in cases where performance is important. While more restricted than registered pointers, by default they have no run-time overhead.  


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
Same as TRegisteredPointer, but cannot be constructed to a null value.

### TRegisteredFixedPointer
Same as TRegisteredNotNullPointer, but cannot be retargeted after construction (basically a "const TRegisteredNotNullPointer"). It is essentially a functional equivalent of a C++ reference and is a recommended type to be used for safe parameter passing by reference.  

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
TRegisteredPointer&lt;X&gt; does implicitly convert to TRegisteredPointer&lt;const X&gt;. But some prefer to think of the pointer giving "const" access to the object rather than giving access to a "const object".

### TRegisteredRefWrapper
Just a registered version of [std::reference_wrapper](http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper).  

usage example:

    #include "mseprimitives.h"
    #include "mseregistered.h"

    int main(int argc, char* argv[]) {
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

Also note that [mse::TRefCountingNotNullPointer](#trefcountingnotnullpointer) and [mse::TRefCountingFixedPointer](#trefcountingfixedpointer) always point to a validly allocated object, so their dereferences don't need to be checked. mse::TRegisteredPointer's safety mechanisms are not compatible with the techniques used by the benchmark to isolate dereferencing performance, but mse::TRegisteredPointer's dereferencing performance would be expected to be essentially identical to that of mse::TRelaxedRegisteredPointer. By default, [scope pointers](#scope-pointers) have identical performance to native pointers.

### Reference counting pointers

If you're going to use pointers, then to ensure they won't be used to access invalid memory you basically have two options - detect any attempt to do so and throw an exception, or, alternatively, ensure that the pointer targets a validly allocated object. Registered pointers rely on the former, and so-called "reference counting" pointers can be used to achieve the latter. The most famous reference counting pointer is std::shared_ptr, which is notable for its thread-safe reference counting that can be handy when you're sharing an object among asynchronous threads, but is unnecessarily costly when you aren't. So we provide fast reference counting pointers that [forego](#on-thread-safety) any thread safety mechanisms. In addition to being substantially faster (and smaller) than std::shared_ptr, they are a bit more safety oriented in that they they don't support construction from raw pointers. (Use mse::make_refcounting&lt;&gt;() instead.) "Const", "not null" and "fixed" (non-retargetable) flavors are also provided with proper conversions between them. For more information on how to use the safe smart pointers in this library for maximum memory safety, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).


### TRefCountingPointer

usage example:

	#include "mserefcounting.h"
	
	int main(int argc, char* argv[]) {
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


### TRefCountingNotNullPointer

Same as TRefCountingPointer, but cannot be constructed to or assigned a null value. Because TRefCountingNotNullPointer controls the lifetime of its target it, should be always safe to assume that it points to a validly allocated object.

### TRefCountingFixedPointer

Same as TRefCountingNotNullPointer, but cannot be retargeted after construction (basically a "const TRefCountingNotNullPointer"). It is a recommended type to be used for safe parameter passing by reference.

### TRefCountingConstPointer, TRefCountingNotNullConstPointer, TRefCountingFixedConstPointer

TRefCountingPointer&lt;X&gt; actually does implicitly convert to TRefCountingPointer&lt;const X&gt;. But some prefer to think of the pointer giving "const" access to the object rather than giving access to a "const object".


### TRefCountingOfRegisteredPointer

TRefCountingOfRegisteredPointer is simply an alias for TRefCountingPointer&lt;TRegisteredObj&lt;_Ty&gt;&gt;. TRegisteredObj&lt;_Ty&gt; is meant to behave much like, and be compatible with a _Ty. The reason why we might want to use it is because the &amp; ("address of") operator of TRegisteredObj&lt;_Ty&gt; returns a [TRegisteredFixedPointer&lt;_Ty&gt;](#tregisteredfixedpointer) rather than a raw pointer, and TRegisteredPointers can serve as safe "weak pointers".  

usage example:  

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
    
    int main(int argc, char* argv[]) {
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

### TRefCountingOfRegisteredNotNullPointer, TRefCountingOfRegisteredFixedPointer

### TRefCountingOfRegisteredConstPointer, TRefCountingOfRegisteredNotNullConstPointer, TRefCountingOfRegisteredFixedConstPointer

### TRefCountingOfRelaxedRegisteredPointer

TRefCountingOfRelaxedRegisteredPointer is simply an alias for TRefCountingPointer&lt;TRelaxedRegisteredObj&lt;_Ty&gt;&gt;. Generally you should prefer to just use TRefCountingOfRegisteredPointer, but if you need a "weak pointer" to refer to a type before it's fully defined then you can use this type. An example of such a situation is when you have so-called "cyclic references".  

usage example:  

    #include "mserefcountingofrelaxedregistered.h"
    
    int main(int argc, char* argv[]) {
    
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

### TRefCountingOfRelaxedRegisteredNotNullPointer, TRefCountingOfRelaxedRegisteredFixedPointer

### TRefCountingOfRelaxedRegisteredConstPointer, TRefCountingOfRelaxedRegisteredNotNullConstPointer, TRefCountingOfRelaxedRegisteredFixedConstPointer

### Scope pointers
Scope pointers are different from other smart pointers in the library in that, by default, they have no runtime safety enforcement mechanism, and the compile-time safety mechanisms aren't (yet) quite sufficient to ensure that they will be used in an intrinsically safe manner. Scope pointers point to scope objects. Scope objects are objects that are allocated on the stack, or whose "owning" pointer is allocated on the stack. So basically the object is destroyed when it, or its owner, goes out of scope. The purpose of scope pointers and objects is to identify a class of situations that are simple and deterministic enough that no (runtime) safety mechanisms are necessary. In theory, a tool could be constructed to verify that scope pointers are used in a safe manner at compile-time. But in the mean time we provide the option of using a relaxed registered pointer as the scope pointer's base class for enhanced safety and to help catch misuse. Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause relaxed registered pointers to be used in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause them to be used in non-debug modes as well. And as with registered pointers, scope pointers cannot target types that cannot act as a base class. For int, bool and size_t use the safer [substitutes](#primitives) that can act as base classes.

There are two types of scope pointers, [TXScopeFixedPointer](#txscopefixedpointer) and [TXScopeOwnerPointer](#txscopeownerpointer). TXScopeOwnerPointer is similar to boost::scoped_ptr in functionality (but more limited in intended use). It creates an instance of a given class on the heap and destroys that instance in its destructor. (We use "scope" to mean "execution scope", where in boost it seems to refer to "declaration scope".) TXScopeFixedPointer is a "non-owning" (or "weak") pointer to a scope object. It is (intentionally) limited in its functionality, and is intended pretty much for the sole purpose of passing scope objects by reference as function arguments. For more information on how to use the safe smart pointers in this library for maximum memory safety, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).


### TXScopeFixedPointer
TXScopeFixedPointer is intended to be used to pass scope objects by reference as function arguments. It is not intended to be used as a member of any class or struct (this would generally produce a compile error) or as a function return type.  

usage example:

    #include "msescope.h"
    
    int main(int argc, char* argv[]) {
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
            static int foo2(mse::TXScopeFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
            static int foo3(mse::TXScopeFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
        protected:
            ~B() {}
        };
    
        mse::TXScopeObj<A> a_scpobj(5);
        int res1 = (&a_scpobj)->b;
        int res2 = B::foo2(&a_scpobj);
        int res3 = B::foo3(&a_scpobj);
    }

### TXScopeFixedConstPointer

### TXScopeOwnerPointer
TXScopeOwnerPointer is similar to boost::scoped_ptr in functionality, but more limited in intended use. In particular, TXScopeOwnerPointer is not intended to be used as a member of any class or struct. Use it when you want to give scope lifetime to objects that are too large to be declared directly on the stack. Also, instead of its constructor taking a native pointer pointing to the already allocated object, it allocates the object itself and passes its contruction arguments to the object's constructor.  

usage example:

    #include "msescope.h"
    
    int main(int argc, char* argv[]) {
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
            static int foo2(mse::TXScopeFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
            static int foo3(mse::TXScopeFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
        protected:
            ~B() {}
        };
    
        mse::TXScopeOwnerPointer<A> a_scpoptr(7);
        int res4 = B::foo2(&(*a_scpoptr));
    }


### make_pointer_to_member()
If you need a safe pointer to a member of a class/struct, you could declare the member itself to be a registered object (or a reference counting pointer). But often a preferable option is to use make_pointer_to_member(). This function takes the member you want to target, and a safe pointer to the containing class/struct, and combines them to create a safe pointer to the member. The actual type of the returned pointer varies depending on the types of the parameters passed.

usage example:

    /* Including "msepoly.h" is not required to use mse::make_pointer_to_member(). It just happens to include all the
    other include files used by this example. */
    #include "msepoly.h"
    
    class H {
    public:
        /* A member function that provides a safe pointer/reference to a class/struct member is going to need to
        take a safe version of the "this" pointer as a parameter. */
        template<class this_type>
        static auto safe_pointer_to_member_string1(this_type safe_this) -> decltype(mse::make_pointer_to_member(safe_this->m_string1, safe_this)) {
            return mse::make_pointer_to_member(safe_this->m_string1, safe_this);
        }
    
        std::string m_string1 = "initial text";
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
        mse::msevector<H> h_msevec;
        h_msevec.resize(1);
        auto h_msevec_ipointer = h_msevec.ibegin();
        auto h_msevec_ssiter = h_msevec.ss_begin();
    
        /* And don't forget the safe async sharing pointers. */
        auto h_access_requester = mse::make_asyncsharedreadwrite<H>();
        auto h_writelock_ptr = h_access_requester.writelock_ptr();
        auto h_stdshared_const_ptr = mse::make_stdsharedimmutable<H>();
    
        {
            /* So here's how you get a safe pointer to a member of the object using mse::make_pointer_to_member(). */
            auto h_string1_scpptr = mse::make_pointer_to_member(h_scpobj.m_string1, &h_scpobj);
            (*h_string1_scpptr) = "some new text";
            auto h_string1_scp_const_ptr = mse::make_const_pointer_to_member(h_scpobj.m_string1, &h_scpobj);
    
            auto h_string1_refcptr = mse::make_pointer_to_member(h_refcptr->m_string1, h_refcptr);
            (*h_string1_refcptr) = "some new text";
    
            auto h_string1_regptr = mse::make_pointer_to_member(h_regobj.m_string1, &h_regobj);
            (*h_string1_regptr) = "some new text";
    
            auto h_string1_rlxregptr = mse::make_pointer_to_member(h_rlxregobj.m_string1, &h_rlxregobj);
            (*h_string1_rlxregptr) = "some new text";
    
            auto h_string1_mstdvec_iter = mse::make_pointer_to_member(h_mstdvec_iter->m_string1, h_mstdvec_iter);
            (*h_string1_mstdvec_iter) = "some new text";
    
            auto h_string1_msevec_ipointer = mse::make_pointer_to_member(h_msevec_ipointer->m_string1, h_msevec_ipointer);
            (*h_string1_msevec_ipointer) = "some new text";
    
            auto h_string1_msevec_ssiter = mse::make_pointer_to_member(h_msevec_ssiter->m_string1, h_msevec_ssiter);
            (*h_string1_msevec_ssiter) = "some new text";
    
            auto h_string1_writelock_ptr = mse::make_pointer_to_member(h_writelock_ptr->m_string1, h_writelock_ptr);
            (*h_string1_writelock_ptr) = "some new text";
    
            auto h_string1_stdshared_const_ptr = mse::make_pointer_to_member(h_stdshared_const_ptr->m_string1, h_stdshared_const_ptr);
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


### Poly pointers
Poly pointers are "chameleon" pointers that can be constructed from, and retain the safety features of many of the pointer types in this library. If you're writing a function and you'd like it to be able to accept different types of safe pointer parameters, you can "templatize" your function. Alternatively, you can declare your pointer parameters as poly pointers.  

Note that poly pointers support only basic facilities common to all the covered pointer and iterator types, providing essentially the functionality of a C++ reference. For example, this means no assignment operator, and no "operator bool()". Where null pointer values are desired you might consider using mse::optional<> or std::optional<> instead.  

### TXScopePolyPointer, TXScopePolyConstPointer
Scope poly pointers are primarily intended to be used in function parameter declarations. In particular, as they can be constructed from a scope pointer (TXScopeFixedPointer or TXScopeFixedConstPointer), they must observe the same usage restrictions.

usage example:

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
        mse::msevector<A> a_msevec;
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

### TPolyPointer, TPolyConstPointer
These poly pointers do not support construction from scope pointers, and thus are not bound by the same usage restrictions. For example, these poly pointers may be used as a member of a class or struct.

### TXScopeAnyPointer, TXScopeAnyConstPointer, TAnyPointer, TAnyConstPointer
"Any" pointers are also “chameleon” pointers that behave similarly to poly pointers. One difference is that unlike poly pointers which can only be directly constructed from a finite set of pointer types, "any" pointers can be constructed from almost any kind of pointer. But poly pointers can be constructed from "any" pointers, so indirectly, via "any" pointers, pretty much any type of pointer converts to a poly pointer too. In particular, if you wanted to pass a pointer generated by [make_pointer_to_member()](#make_pointer_to_member) to a function that takes a poly pointer, you would first need to wrap it an "any" pointer. This is demonstrated in the scope poly pointer usage example.  

"Any" pointers can also be used as function arguments. The choice between using poly pointers versus "any" pointers is similar to the choice between [std::variant and std::any](http://www.boost.org/doc/libs/1_63_0/doc/html/variant/misc.html#variant.versus-any). 

### TXScopeAnyRandomAccessIterator, TXScopeAnyRandomAccessConstIterator, TAnyRandomAccessIterator, TAnyRandomAccessConstIterator

In modern C++ (and SaferCPlusPlus), arrays of different sizes are actually different types, with incompatible iterators. So, for example, if you wanted to make a function that accepts the iterators of arrays of varying size, you would generally do that by "templatizing" the function. Alternatively, you could use an "any random access iterator" which is a "chameleon" iterator that can be constructed from basically any iterator that supports operator\[\] (the "square bracket" operator).

### TXScopeRandomAccessSection, TXScopeRandomAccessConstSection, TRandomAccessSection, TRandomAccessConstSection

A "random access section" is basically a convenient interface to access a (contiguous) subsection of an existing array or vector. (Also monikered as "array view" or "span" if you're familiar with those.) It's constructed by specifying an iterator to the start of the section, and the length of the section.

usage example:

    #include "msepoly.h"
    
    int main(int argc, char* argv[]) {
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
            static void foo3(mse::TXScopeRandomAccessSection<int> ra_section) {
                for (mse::TXScopeRandomAccessSection<int>::size_type i = 0; i < ra_section.size(); i += 1) {
                    ra_section[i] = 0;
                }
            }
            static int foo4(mse::TXScopeRandomAccessConstSection<int> const_ra_section) {
                int retval = 0;
                for (mse::TXScopeRandomAccessSection<int>::size_type i = 0; i < const_ra_section.size(); i += 1) {
                    retval += const_ra_section[i];
                }
                return retval;
            }
            static int foo5(mse::TXScopeRandomAccessConstSection<int> const_ra_section) {
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
    
        mse::TXScopeRandomAccessSection<int> ra_section1(array_iter1, 2);
        B::foo3(ra_section1);
    
        mse::TXScopeRandomAccessSection<int> ra_section2(++vec1.begin(), 3);
        auto res5 = B::foo5(ra_section2);
        B::foo3(ra_section2);
        auto res6 = B::foo4(ra_section2);
    }

### Safely passing parameters by reference
As has been shown, you can use [registered pointers](#registered-pointers), [reference counting pointers](#reference-counting-pointers) and [scope pointers](#scope-pointers) to safely pass parameters by reference. (Well, scope pointers aren't completely safe yet, but "safer" anyway.) If you're writing a function for general use, we recommend that you "templatize" the function so that it can accept any type of pointer. This is demonstrated in the [TRefCountingOfRegisteredPointer](#trefcountingofregisteredpointer) usage example. Or you can read an article about it [here](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu). If for some reason you can't or don't want to templatize the function, but still want to give the caller some flexibility in terms of pointer reference parameters then you can use a [poly pointer](#poly-pointers). And of course the library remains perfectly compatible with (the less safe) traditional C++ references if you prefer. 


### Asynchronously shared objects
One situation where safety mechanisms are particularly important is when sharing objects between asynchronous threads. In particular, when one party (thread) is modifying an object, you want to ensure that no other party accesses it. So we provide TAsyncSharedReadWriteAccessRequester that (like std::shared_ptr) possesses shared ownership of an object to be shared among asynchronous threads, and provides (const and non-const smart) pointers that can be used to safely access the object. For more information and examples, see [this article](http://www.codeproject.com/Articles/1106491/Sharing-Objects-Between-Threads-in-Cplusplus-the-S). At the moment, these data types cannot target types that cannot act as a base class. If you want to share an int, bool or size_t, use the [safer substitutes](#primitives) that can act as base classes. And as a rule of thumb, if you have to share data between asynchronous threads, prefer the simplest possible packaging of that data (or one specifically designed for asynchronous sharing), even when that means foregoing the use of some of the elements in this library. Ideally, prefer a POD ("plain old data") data type with no member functions and no mutable members.

### TAsyncSharedReadWriteAccessRequester
Use the writelock_ptr() and readlock_ptr() member functions to obtain pointers to the shared object. Those functions will block until they can obtain the needed lock on the shared object. The obtained pointers will hold on to their lock for as long as they exist. Their locks are released when the pointers are destroyed (generally when they go out of scope).  

Use mse::make_asyncsharedreadwrite<>() to obtain a TAsyncSharedReadWriteAccessRequester. TAsyncSharedReadWriteAccessRequester can be copied and passed-by-value as a parameter (to another thread, generally).

Non-blocking try_writelock_ptr() and try_readlock_ptr() member functions are also available. As are the limited-blocking try_writelock_ptr_for(), try_readlock_ptr_for(), try_writelock_ptr_until() and try_readlock_ptr_until().

### TAsyncSharedReadOnlyAccessRequester
Same as TAsyncSharedReadWriteAccessRequester, but only supports readlock_ptr(), not writelock_ptr(). You can use mse::make_asyncsharedreadonly<>() to obtain a TAsyncSharedReadOnlyAccessRequester. TAsyncSharedReadOnlyAccessRequester can also be copy constructed from a TAsyncSharedReadWriteAccessRequester.

### TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester
A peculiarity of C++ is that a "const" object is not necessarily guaranteed to be unmodifiable. Specifically in cases where the object has "mutable" members. So, out of an abundance of prudence TAsyncSharedReadWriteAccessRequester and TAsyncSharedReadOnlyAccessRequester do not allow for the simultaneous existence of multiple "readlock_ptr"s. But sometimes you really want to allow for multiple simultaneous readers. So we provide these versions with unwieldy names to remind you of the potential dangers of shared objects with mutable members. Ideally, at some point in the future, we'd be able to determine at compile-time whether or not a type has mutable members.

### TStdSharedImmutableFixedPointer
For "read-only" situations when you need, or want, the shared object to be managed by std::shared_ptrs we provide a slightly safety-enhanced wrapper for std::shared_ptr. The wrapper enforces "const"ness and tries to ensure that it points to a validly allocated object. Use mse::make_stdsharedimmutable<>() to construct an mse::TStdSharedImmutableFixedPointer. And again, beware of sharing objects with mutable members.  

usage example:

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
	
	int main(int argc, char* argv[]) {
		/* The TAsyncShared data types are used to safely share objects between asynchronous threads. */
	
		class A {
		public:
			A(int x) : b(x) {}
			A(const A& _X) : b(_X.b) {}
			virtual ~A() {}
			A& operator=(const A& _X) { b = _X.b; return (*this); }
	
			int b = 3;
			std::string s = "some text ";
		};
		class B {
		public:
			static double foo1(mse::TAsyncSharedReadWriteAccessRequester<A> A_ashar) {
				auto t1 = std::chrono::high_resolution_clock::now();
				/* mse::TAsyncSharedReadWriteAccessRequester<A>::writelock_ptr() will block until it can obtain a write lock. */
				auto ptr1 = A_ashar.writelock_ptr(); // while ptr1 exists it holds a (write) lock on the shared object
				auto t2 = std::chrono::high_resolution_clock::now();
				std::this_thread::sleep_for(std::chrono::seconds(1));
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				auto timespan_in_seconds = time_span.count();
				auto thread_id = std::this_thread::get_id();
	
				ptr1->s = std::to_string(timespan_in_seconds);
				return timespan_in_seconds;
			}
			static int foo2(std::shared_ptr<const A> A_shptr) {
				return A_shptr->b;
			}
		protected:
			~B() {}
		};
	
		std::cout << std::endl;
		std::cout << "AsyncShared test output:";
		std::cout << std::endl;
	
		{
			std::cout << "TAsyncSharedReadWrite:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedreadwrite<A>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(B::foo1, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire write pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
	
			/* Btw, mse::TAsyncSharedReadOnlyAccessRequester<>s can be copy constructed from 
			mse::TAsyncSharedReadWriteAccessRequester<>s */
			mse::TAsyncSharedReadOnlyAccessRequester<A> ash_read_only_access_requester(ash_access_requester);
		}
		{
			std::cout << "TAsyncSharedReadOnly:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedreadonly<A>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(H::foo7<mse::TAsyncSharedReadOnlyAccessRequester<A>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWrite:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadwrite<A>(7);
			ash_access_requester.writelock_ptr()->b = 11;
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(H::foo7<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester<A>>, ash_access_requester));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				std::cout << "thread: " << count << ", time to acquire read pointer: " << (*it).get() << " seconds.";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		{
			std::cout << "TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnly:";
			std::cout << std::endl;
			auto ash_access_requester = mse::make_asyncsharedobjectthatyouaresurehasnounprotectedmutablesreadonly<A>(7);
			int res1 = ash_access_requester.readlock_ptr()->b;
	
			std::list<std::future<double>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(H::foo7<mse::TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester<A>>, ash_access_requester));
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
			auto access_requester = mse::make_asyncsharedreadwrite<std::string>("some text");
			auto writelock_ptr1 = access_requester.try_writelock_ptr();
			if (writelock_ptr1) {
				// lock request succeeded
				int q = 5;
			}
			auto readlock_ptr2 = access_requester.try_readlock_ptr_for(std::chrono::seconds(10));
			auto writelock_ptr3 = access_requester.try_writelock_ptr_until(std::chrono::steady_clock::now() + std::chrono::seconds(10));
		}
		{
			/* For simple "read-only" scenarios where you need, or want, the shared object to be managed by std::shared_ptrs,
			TStdSharedImmutableFixedPointer is a "safety enhanced" wrapper for std::shared_ptr. And again, beware of
			sharing objects with mutable members. */
			auto read_only_sh_ptr = mse::make_stdsharedimmutable<A>(5);
			int res1 = read_only_sh_ptr->b;
	
			std::list<std::future<int>> futures;
			for (size_t i = 0; i < 3; i += 1) {
				futures.emplace_back(std::async(B::foo2, read_only_sh_ptr));
			}
			int count = 1;
			for (auto it = futures.begin(); futures.end() != it; it++, count++) {
				int res2 = (*it).get();
			}
		}
	}


### Primitives

### CInt, CSize_t and CBool
These classes are meant to behave like, and be compatible with their native counterparts. In debug mode, they check for "use before initialization", and in release mode, they use default initialization to help ensure deterministic behavior. Upon value assignment, CInt and CSize_t will check to ensure that the value fits within the type's range. CSize_t's `-=` operator checks that the operation evaluates to a positive value. And unlike its native counterpart, arithmetic operations involving CSize_t that could evaluate to a negative number are returned as a (signed) CInt.

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

Note that while CInt and CSize_t have no problem interacting with native signed integers, they do not implicitly play well with size_t or native unsigned integers. We'd be generally wary of using native unsigned integer types due to the implicit conversion/promotion rules between signed and unsigned native integers. But if you need to obtain a size_t from a CSize_t, you can do so explicitly using the mse::as_a_size_t() function. If you want to construct a CSize_t (or CInt) from a native unsigned integer type, you'd need to first cast it to a size_t, or a signed integer.  

Also see the section on "[compatibility considerations](#compatibility-considerations)".

### Quarantined types

Quarantined types are meant to hold values that are obtained from user input or some other untrusted source (like a media file for example). These are not yet available in the library, but are an important concept with respect to safe programming. Values obtained from untrusted sources are the main attack vector of malicious actors and should be handled with special care. For example, the so-called "stagefright" vulnerability in the Android OS is the result of a specially crafted media file causing the sum of integers to overflow.  

It is often the case that untrusted values are obtained through intrinsically slow communication mediums (i.e. file system, internet, UI, etc.), so it often makes no perceptible difference whether the code that processes those untrusted values into "trusted" internal values is optimized for performance or not. So don't hesitate to use whatever safety methods are called for. In particular, integer types with more comprehensive range checking can be found here: https://github.com/robertramey/safe_numerics.

### CQuarantinedInt, CQuarantinedSize_t, CQuarantinedVector, CQuarantinedString

Not yet available.

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
        
        
        /* mse::msevector is not quite as safe as mse::mstd::vector in the following way: */
        
        std::vector<int>::iterator sv1_it;
        mse::msevector<int>::ss_iterator_type msev1_it; // bounds checked iterator just like mse::mstd::vector<int>::iterator
        mse::mstd::vector<int>::iterator mv1_it;
        {
            std::vector<int> sv1 = { 1, 2, 3 };
            sv1_it = sv1.begin();
            
            mse::msevector<int> msev1 = { 1, 2, 3 };
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
            // Still debating whether it'd be better to throw an exception though.
        }
    }

Important note: As a general rule, avoid sharing mse::mstd::vector<>s among asynchronous threads.  

The mechanism mse::mstd::vector<> uses to track its iterators is not thread safe (for performance reasons). Technically there is no issue as long as you don't obtain, release, move or copy any associated iterators from asyncronous threads. But there's no way to enforce that, so it's generally better just to follow the SaferCPlusPlus rule of thumb: If you have to share data between asynchronous threads, prefer the simplest possible packaging of that data (or one specifically designed for asynchronous sharing). Ideally a POD ("plain old data") data type with no member functions and no mutable members. mse::mstd::vector<> doesn't really qualify. std::vector<>, while perhaps still not ideal, is much more appropriate for asyncronous sharing. And of course, remember to use SaferCPlusPlus [asyncronous sharing data types](#asynchronously-shared-objects) when appropriate.  

Also, keep in mind that dynamic data structures, like vectors, are a primary source of memory access bugs, so unsafe native references and pointers to members of dynamic data structures should particularly avoided. See the note in the "[Practical limitations](#practical-limitations)" section about implicit "this" pointers.

### msevector

If you're willing to forego a little theoretical safety, msevector<> is still very safe without the overhead of memory management.  
In addition to the (high performance) standard vector iterator, msevector<> also supports a new kind of iterator, called "ipointer", that acts more like a list iterator in the sense that it points to an item rather than a position, and like a list iterator, it is not invalidated by insertions or deletions occurring elsewhere in the container, even if a "reallocation" occurs. In fact, standard vector iterators are so prone to being invalidated that for algorithms involving insertion or deletion, they can be generously considered not very useful, and more prudently considered dangerous. ipointers, aside from being safe, just make sense. Algorithms that work when applied to list iterators will work when applied to ipointers. And that's important as Bjarne famously [points out](https://www.youtube.com/watch?v=YQs6IC-vgmo), for cache coherency reasons, in most cases vectors should be used in place of lists, even when lists are conceptually more appropriate. You can read a short article comparing ipointers with some existing alternatives [here](http://www.codeproject.com/Articles/1087021/Stable-Iterators-for-Cplusplus-Vectors-and-Why-You).  

msevector<> also provides a safe (bounds checked) version of the standard vector iterator.

usage example:

    #include "msemsevector.h"
    
    int main(int argc, char* argv[]) {
        
        mse::msevector<int> v1 = { 1, 2, 3, 4 };
        mse::msevector<int> v = v1;
        {
            mse::msevector<int>::ipointer ip1 = v.ibegin();
            ip1 += 2;
            assert(3 == (*ip1));
            auto ip2 = v.ibegin(); /* ibegin() returns an ipointer */
            v.erase(ip2); /* remove the first item */
            assert(3 == (*ip1)); /* ip1 continues to point to the same item, not the same position */
            ip1--;
            assert(2 == (*ip1));
            for (mse::msevector<int>::cipointer cip = v.cibegin(); v.ciend() != cip; cip++) {
                /* You might imagine what would happen if cip were a regular vector iterator. */
                v.insert(v.ibegin(), (*cip));
            }
        }
        v = v1;
        {
            /* This code block is equivalent to the previous code block, but uses ipointer's more "readable" interface
            that might make the code a little more clear to those less familiar with C++ syntax. */
            mse::msevector<int>::ipointer ip_vit1 = v.ibegin();
            ip_vit1.advance(2);
            assert(3 == ip_vit1.item());
            auto ip_vit2 = v.ibegin();
            v.erase(ip_vit2);
            assert(3 == ip_vit1.item());
            ip_vit1.set_to_previous();
            assert(2 == ip_vit1.item());
            mse::msevector<int>::cipointer cip(v);
            for (cip.set_to_beginning(); cip.points_to_an_item(); cip.set_to_next()) {
                v.insert_before(v.ibegin(), (*cip));
            }
        }
    
        /* Btw, ipointers are compatible with stl algorithms, like any other stl iterators. */
        std::sort(v.ibegin(), v.iend());
    
        /* And just to be clear, mse::msevector<> retains its original (high performance) stl vector iterators. */
        std::sort(v.begin(), v.end());
    
        /* mse::msevector<> also provides "safe" (bounds checked) versions of the original stl vector iterators. */
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

Important note: In general, you should probably avoid sharing mse::msevector<>s among asynchronous threads.  

The mechanism mse::msevector<> uses to track its "ipointer" iterators is not thread safe (for performance reasons). Technically there is no issue as long as you don't obtain, release, move or copy any associated "ipointer" iterators from asyncronous threads. But there's no way to enforce that, so it's generally better just to follow the SaferCPlusPlus rule of thumb: If you have to share data between asynchronous threads, prefer the simplest possible packaging of that data (or one specifically designed for asynchronous sharing). Ideally a POD ("plain old data") data type with no member functions and no mutable members. std::vector<>, while perhaps still not ideal, may be more appropriate for asyncronous sharing. And of course, remember to use SaferCPlusPlus [asyncronous sharing data types](#asynchronously-shared-objects) when appropriate.

### ivector

ivector is for cases when safety and correctness are higher priorities than compatibility and performance. ivector, like mstd::vector<>, is almost completely safe. ivector takes the further step of dropping support for the (problematic) standard vector iterator, and replacing it with [ipointer](#msevector).

usage example:

    #include "mseivector.h"
    
    int main(int argc, char* argv[]) {
    
        mse::ivector<int> iv = { 1, 2, 3, 4 };
        std::sort(iv.begin(), iv.end());
        mse::ivector<int>::ipointer ivip = iv.begin();
    }

Important note: As a general rule, avoid sharing mse::ivector<>s among asynchronous threads. See [mse::mstd::vector<>](#vector)

### Arrays

We provide two arrays - [mstd::array<>](#array) and [msearray<>](#msearray). mstd::array<> is simply an almost completely safe implementation of std::array<>. msearray<> is also quite safe. Not quite as safe as mstd::array<>, but it requires less overhead.

### array

mstd::array<> is an almost completely safe implementation of std::array<>. Note that the current implementation requires "mseregistered.h".  

usage example:

    #include "msemstdarray.h"
    #include "msemsearray.h"
    #include <array>
    
    int main(int argc, char* argv[]) {
    
        mse::mstd::array<int, 3> ma;
        std::array<int, 3> sa;
        /* These two arrays should be completely interchangeable. The difference being that ma should throw
        an exception on any attempt to access invalid memory. */
    
    
        /* mse::msearray is not quite as safe as mse::mstd::array in the following way: */
    
        std::array<int, 3>::iterator sa1_it;
        mse::msearray<int, 3>::ss_iterator_type msea1_it; // bounds checked iterator just like mse::mstd::array::iterator
        mse::mstd::array<int, 3>::iterator ma1_it;
        {
            std::array<int, 3> sa1 = { 1, 2, 3 };
            sa1_it = sa1.begin();
    
            mse::msearray<int, 3> msea1 = { 1, 2, 3 };
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

Important note: As a general rule, avoid sharing mse::mstd::array<>s among asynchronous threads.  

The mechanism mse::mstd::array<> uses to track its iterators is not thread safe (for performance reasons). Technically there is no issue as long as you don't obtain, release, move or copy any associated iterators from asyncronous threads. But there's no way to enforce that, so it's generally better just to follow the SaferCPlusPlus rule of thumb: If you have to share data between asynchronous threads, prefer the simplest possible packaging of that data (or one specifically designed for asynchronous sharing). Ideally a POD ("plain old data") data type with no member functions and no mutable members. mse::mstd::array<> doesn't really qualify. mse::msearray<> is more appropriate for asyncronous sharing as it does not track its iterators. And of course, remember to use SaferCPlusPlus [asyncronous sharing data types](#asynchronously-shared-objects) when appropriate.  

Also note for real time applications that restrict heap allocations: If the number of iterators exceeds the space reserved for tracking them, mse::mstd::array<> will resort to obtaining space from the heap. You can instead use mse::msearray<>, which does not track its iterators. (The same applies to registered objects in general. Use scope objects instead.)

### xscope_iterator

The implementation of mstd::array iterators uses [registered pointers](#registered-pointers) to ensure that iterators are not used to access array elements after the array has been deallocated. This incurs a slight run-time cost. So just as the library provides [scope pointers](#scope-pointers) without run-time cost, scope iterators for arrays are also provided. Scope iterators have usage restrictions similar to scope pointers. For example, they can only target arrays declared as scope objects, and may not be used as a member of any class or struct, and may not be used as a function return value.

usage example:

    #include "msemstdarray.h"
    
    int main(int argc, char* argv[]) {
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
            mse::mstd::array<int, 3> m_array = { 1, 2, 3 };
        };
        mse::TXScopeObj<CContainer1> container1_scpobj;
        auto container1_m_array_scpptr = mse::make_pointer_to_member(container1_scpobj.m_array, &container1_scpobj);
        auto scp_iter4 = mse::mstd::make_xscope_iterator(container1_m_array_scpptr);
        scp_iter4++;
        auto res3 = *scp_iter4;
    }

### msearray

msearray<>, like msevector<>, is a essentially a compromise between safety and performance. And like msevector<>, msearray<> provides a safer iterator, in addition to the (high performance) standard iterator. Like msevector<>, msearray<>'s safe iterator also supports the more "readable" interface. In cases where the msearray is declared as a scope object, you can also use a "scope" version of the safe iterator. The restrictions on when and how scope iterators can be used ensure that they won't be used to access the array after it's been deallocated.  

usage example:

    #include "msemsearray.h"
    #include <array>
    
    int main(int argc, char* argv[]) {
        mse::msearray<int, 3> a1 = { 1, 2, 3 };
        mse::msearray<int, 3> a2 = { 11, 12, 13 };
        
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
            
            mse::TXScopeObj<mse::msearray<int, 3>> array1_scpobj = mse::msearray<int, 3>{ 1, 2, 3 };
            
            auto scp_ss_iter1 = mse::make_xscope_ss_iterator_type(&array1_scpobj);
            scp_ss_iter1.set_to_beginning();
            auto scp_ss_iter2 = mse::make_xscope_ss_iterator_type(&array1_scpobj);
            scp_ss_iter2.set_to_end_marker();
            
            std::sort(scp_ss_iter1, scp_ss_iter2);
            
            auto scp_ss_citer3 = mse::make_xscope_ss_const_iterator_type(&array1_scpobj);
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
                
                mse::msearray<int, 3> m_array;
            };
            mse::TXScopeObj<CContainer1> container1_scpobj;
            auto container1_m_array_scpptr = mse::make_pointer_to_member(container1_scpobj.m_array, &container1_scpobj);
            auto scp_ss_citer4 = mse::make_xscope_ss_iterator_type(container1_m_array_scpptr);
            scp_ss_citer4++;
            auto res3 = *scp_ss_citer4;
        }
    }

Note that we've decided to implement msearray<> as an "aggregate" type. This means that it gets automatic compiler support for [aggregate initialization](http://en.cppreference.com/w/cpp/language/aggregate_initialization), but it comes with some compromises as well. One detail to be aware of is that when replacing an aggregate initialized std::array<> with an mse::msearray<>, you generally need to add an extra set of braces around the initializer list. Note that with mse::mstd::array<>, you do not need the extra braces because it is not an aggregate type and instead tries to emulate support for aggregate initialization.

### Compatibility considerations
People have asked why the primitive C++ types can't be used as base classes - http://stackoverflow.com/questions/2143020/why-cant-i-inherit-from-int-in-c. It turns out that really the only reason primitive types weren't made into full-fledged classes is that they inherit these "chaotic" conversion rules from C that can't be fully mimicked by C++ classes, and Bjarne thought it would be too ugly to try to make special case classes that followed different conversion rules.  

But while substitute classes cannot be 100% compatible substitutes for their corresponding primitives, they can still be mostly compatible. And if you're writing new code or maintaining existing code, it should be considered good coding practice to ensure that your code is compatible with C++'s conversion rules for classes and not dependent on the "chaotic" legacy conversion rules of primitive types.

If you are using legacy code or libraries where it's not practical to update the code, it shouldn't be a problem to continue using primitive types there and the safer substitute classes elsewhere in the code. The safer substitute classes generally have no problem interacting with primitive types, although in some cases you may need to do some explicit type casting. [Registered pointers](#registered-pointers) can be cast to raw pointers, and, for example, [CInt](#primitives) can participate in arithmetic operations with regular ints.

### On thread safety
The choice to not include thread safety mechanisms in most of the types in this library is a deliberate one. If the goal is code safety, then we strongly discourage the casual sharing of objects between asynchronous threads (i.e. without the proper safety mechanisms). The practice of sharing objects between asynchronous threads can be prone to severe and insidious bugs that are particularly adept at evading exposure during testing. In cases where it's not practical to avoid the practice, we suggest doing so only in the context of some kind of system that comprehensively ensures against inadvertent unsafe access. For most straight-forward cases you can use the [asynchronous sharing data types](#asynchronously-shared-objects) in this library. And as a rule of thumb, if you have to share data between asynchronous threads, prefer the simplest possible packaging of that data (or one specifically designed for asynchronous sharing), even when that means foregoing the use of some of the elements in this library. Ideally, prefer a POD ("plain old data") data type with no member functions and no mutable members.  
To be clear, we are not discouraging asynchronous programming, or even inter-thread communication in general. Just the "casual" sharing of objects between asynchronous threads.

### Practical limitations

The degree of memory safety that can be achieved is a function of the degree to which use of C++'s (memory) unsafe elements is avoided. Unfortunately, there is not yet a tool to automatically identify such uses. But if, in the future, there is significant demand for such a tool, it shouldn't be a particulary difficult thing to develop. Certainly trivial compared to some of the existing static analysis tools.

Note that one of C++'s more subtle unsafe elements is the implicit "this" pointer when accessing member variables from member functions. Consider this example:

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

The above example contains unchecked accesses to deallocated memory via an implicit and explicit "this" pointer. The "this" pointer (implicit or explicit) is a native pointer, and like any other native pointer, is unsafe and can/should be replaced with a safer substitute:

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

So, technically, achieving complete memory safety requires passing a safe "this" pointer parameter as an argument to every member function that accesses a member variable. (I.e. No non-static member functions.)

Another couple of potential pitfalls are the potential misuse of "scope" pointers, and the sharing of objects with unprotected mutable members between asynchronous threads, as explained in the corresponding documentation. The library data types do what they can to prevent such misuse, but are ultimately limited in their enforcement capabilities. These shortcomings could also be addressed in the future with a reasonably straightforward "code checker" tool to detect the potential problems.

And also, SaferCPlusPlus does not yet provide safer substitutes for all of the standard library containers, just the ones responsible for the most problems (vector and array). So be careful with your maps, sets, etc. In many cases lists can be replaced with one of the safe vectors (msevector or ivector) that support list-style iterators, often with a [performance benefit](#msevector).

### Questions and comments
If you have questions or comments you can create a post in the [issues section](https://github.com/duneroadrunner/SaferCPlusPlus/issues).

