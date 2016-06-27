Jun 2016

### Overview

A collection of safe data types that are compatible with, and can substitute for, common unsafe native C++ types. Currently these include:

- A [fast](#simple-benchmarks), [safe replacement for native pointers](#registered-pointers) that, unlike std::shared_ptr for example, does not take ownership of the target (and so can point to objects on the stack).

- A fast, safe [reference counting pointer](#reference-counting-pointers) for all those situations when you, just for a moment, contemplated using an std::shared_ptr for something other than an object shared between asynchronous threads. Including [safe parameter passing](#safely-passing-parameters-by-reference) by reference.

- A "[scope pointer](#scope-pointers)" for target objects allocated on the stack, or whose "owning" pointer is allocated on the stack. By default, not as safe as the other smart pointers in this library, but with zero runtime overhead.

- An almost completely [safe implementation](#vector) of std::vector<> - bounds checked, iterator checked and memory managed.

- A couple of [other](#vectors) highly compatible vectors that address the issue of unnecessary iterator invalidation upon insert, erase or reallocation.

- [Replacements](#primitives) for the native "int", "size_t" and "bool" types that have default initialization values and address the "signed-unsigned mismatch" issues.

- Data types for safe, simple [sharing](#asynchronously-shared-objects) of objects among asynchronous threads.

Tested with msvc2015 and g++5.3 (as of May 2016) and msvc2013 (as of Feb 2016). Support for versions of g++ prior to version 5 was dropped on Mar 21, 2016.

You can have a look at [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp) to see the library in action.


### Use cases

This library is appropriate for use by two groups of C++ developers - those for whom safety and security are critical, and also everybody else.  
This library can help eliminate a lot of the opportunities for inadvertently accessing invalid memory or using uninitialized values. It essentially gets you a lot of the safety that you might get from, say Java, while retaining all of the power and most of the performance of C++.  
While using the library may sometimes cost a modest performance penalty, because the library elements are [largely compatible](#compatibility-considerations) with their native counterparts they can be easily "disabled" (automatically replaced with their native counterparts) with a compile-time directive, allowing them to be used to help catch bugs in debug/test/beta modes while incurring no overhead in release mode.  
So there is really no excuse for not using the library in pretty much any situation.  
For more information on how the safe smart pointers in this library are intended to be used, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).

### Setup and dependencies

The beauty of the library is that it is so small and simple. Using the library generally involves copying the include files you want to use into your project, and that's it. Outside of the stl, there are no other dependencies.  
A couple of notes about compling: With g++, you'll need to link to the pthread library (-lpthread). You may want to use the -Wno-unused flag as well. With msvc you may get a "[fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj](https://msdn.microsoft.com/en-us/library/8578y171(v=vs.140).aspx)". Just [add](https://msdn.microsoft.com/en-us/library/ms173499.aspx) the "/bigobj" compile flag. For more help you can try the [questions and comments](#questions-and-comments) section.

### Registered pointers

"Registered" pointers are intended to behave just like native C++ pointers, except that their value is (automatically) set to nullptr when the target object is destroyed. And by default they will throw an exception upon any attempt to dereference a nullptr. Because they don't take ownership like some other smart pointers, they can point to objects allocated on the stack as well as the heap. In most cases, they can be used as a compatible, direct substitute for native pointers, making it straightforward to update legacy code (to be safer).

Registered pointers come in two flavors - [TRegisteredPointer](#tregisteredpointer) and [TRelaxedRegisteredPointer](#trelaxedregisteredpointer). They are both very similar. TRegisteredPointer emphasizes speed and safety a bit more, while TRelaxedRegisteredPointer emphasizes compatibility and flexibility a bit more. If you want to undertake the task of en masse replacement of native pointers in legacy code, or need to interact with legacy native pointer interfaces, TRelaxedRegisteredPointer may be more convenient.

Note that these registered pointers cannot target types that cannot act as base classes. The primitive types like int, bool, etc. [cannot act as base classes](#compatibility-considerations). Fortunately, the library provides safer [substitutes](#primitives) for int, bool and size_t that can act as base classes. Also note that pointers that can point to the stack are inherently not thread safe. While we [do not encourage](#on-thread-safety) the casual sharing of objects between asynchronous threads, if you need to do so consider using the [safe sharing data types](#asynchronously-shared-objects) in this library. For more information on how the safe smart pointers in this library are intended to be used, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).



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

###TSyncWeakFixedPointer
TSyncWeakFixedPointer is primarily intended to be used as a safe pointer to a member of a registered object in cases where for some reason you can't, or don't want to, make the member itself a registered object. TSyncWeakFixedPointer essentially acts as a pointer to the member (or whatever object you specify), while keeping a copy of a registered pointer to the object. It uses the registered pointer to ensure that it is safe to access the object. Use mse::make_syncweak() to construct a TSyncWeakFixedPointer.  
What's with the name? "SyncWeak" is short for "sychronous weak", as opposed to "asynchronous weak". "Non-owning" pointers that support objects shared between asynchronous threads, like std::weak_ptr, cannot be used to access the object directly. If, on the other hand, asynchronous sharing is not supported, then a non-owning pointer (with the appropriate safety mechanisms), like TRegisteredPointer, can be used to access the object directly. There is a corresponding [TStrongFixedPointer](#tstrongfixedpointer).  

usage example:

    #include "mseregistered.h"
    
    class H {
    public:
        template<class _TString1Pointer, class _TString2Pointer>
        static std::string foo6(_TString1Pointer i1ptr, _TString2Pointer i2ptr) {
            return (*i1ptr) + (*i2ptr);
        }

    protected:
        ~H() {}
    };
    
    int main(int argc, char* argv[]) {
        /* Obtaining safe pointers to members of registered objects: */
        class E {
        public:
            virtual ~E() {}
            mse::TRegisteredObj<std::string> reg_s = "some text ";
            std::string s2 = "some other text ";
        };
        
        mse::TRegisteredObj<E> registered_e;
        mse::TRegisteredPointer<E> E_registered_ptr1 = &registered_e;
        
        /* The easiest way is to obtain a safe pointer to a member of a registered object is to make the
        member itself a registered object. */
        mse::TRegisteredPointer<std::string> string_registered_ptr1 = &(E_registered_ptr1->reg_s);
        
        /* Another option is to make a TSyncWeakFixedPointer. syncweak_string_ptr1 here is essentially
        a pointer to "E.s2" (string member of class E) with a registered pointer to E to in its pocket.
        It uses the registered pointer to ensure that it is safe to access the object. */
        auto syncweak_string_ptr1 = mse::make_syncweak(E_registered_ptr1->s2, E_registered_ptr1);
        
        /* In practice, rather than declaring a specific mse::TSyncWeakFixedPointer parameter, we expect
        functions to be "templatized" so that they can accept any type of pointer. */
        std::string res1 = H::foo6(syncweak_string_ptr1, syncweak_string_ptr1);
    }

###TSyncWeakFixedConstPointer

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

Just some simple microbenchmarks. We show the results for msvc2015 and msvc2013 (run on the same machine), since there are some interesting differences. The source code for these benchmarks can be found in the file [msetl_example.cpp](https://github.com/duneroadrunner/SaferCPlusPlus/blob/master/msetl_example.cpp).

#### Allocation, deallocation, pointer copy and assignment:
##### platform: msvc2015/x64/Windows7/Haswell (Mar 2016):
Pointer Type | Time
------------ | ----
mse::TRegisteredPointer (stack): | 0.0317188 seconds.
native pointer (heap): | 0.0394826 seconds.
mse::TRefCountingPointer (heap): | 0.0493629 seconds.
mse::TRegisteredPointer (heap): | 0.0573699 seconds.
std::shared_ptr (heap): | 0.0692405 seconds.
mse::TRelaxedRegisteredPointer (heap): | 0.14475 seconds.

##### platform: msvc2013/x64/Windows7/Haswell (Jan 2016):
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
##### platform: msvc2015/x64/Windows7/Haswell (Mar 2016):
Pointer Type | Time
------------ | ----
native pointer: | 0.0105804 seconds.
mse::TRelaxedRegisteredPointer unchecked: | 0.0136354 seconds.
mse::TRefCountingPointer (checked): | 0.0258107 seconds.
mse::TRelaxedRegisteredPointer (checked): | 0.0308289 seconds.
std::weak_ptr: | 0.179833 seconds.

##### platform: msvc2013/x64/Windows7/Haswell (Jan 2016):
Pointer Type | Time
------------ | ----
native pointer: | 0.0100006 seconds.
mse::TRelaxedRegisteredPointer unchecked: | 0.0130008 seconds.
mse::TRelaxedRegisteredPointer (checked): | 0.016001 seconds.
std::weak_ptr: | 0.17701 seconds.

The interesting thing here is that checking for nullptr seems to have gotten a lot slower between msvc2013 and msvc2015. But anyway, my guess is that pointer dereferencing is such a fast operation (std::weak_ptr aside) that outside of critical inner loops, the overhead of checking for nullptr would generally be probably pretty modest.  
Also note that [mse::TRefCountingNotNullPointer](#trefcountingnotnullpointer) and [mse::TRefCountingFixedPointer](#trefcountingfixedpointer) always point to a validly allocated object, so their dereferences don't need to be checked. mse::TRegisteredPointer's safety mechanisms are not compatible with the techniques used by the benchmark to isolate dereferencing performance, but mse::TRegisteredPointer's dereferencing performance would be expected to be essentially identical to that of mse::TRelaxedRegisteredPointer. By default, scope pointers have identical performance to native pointers.

###Reference counting pointers

If you're going to use pointers, then to ensure they won't be used to access invalid memory you basically have two options - detect any attempt to do so and throw an exception, or, alternatively, ensure that the pointer targets a validly allocated object. Registered pointers rely on the former, and so-called "reference counting" pointers can be used to achieve the latter. The most famous reference counting pointer is std::shared_ptr, which is notable for its thread-safe reference counting that's rather handy when you're sharing an object among asynchronous threads, but unnecessarily costly when you aren't. So we provide fast reference counting pointers that [forego](#on-thread-safety) any thread safety mechanisms. In addition to being substantially faster (and smaller) than std::shared_ptr, they are a bit more safety oriented in that they they don't support construction from raw pointers. (Use mse::make_refcounting&lt;&gt;() instead.) "Const", "not null" and "fixed" (non-retargetable) flavors are also provided with proper conversions between them. For more information on how the safe smart pointers in this library are intended to be used, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).


###TRefCountingPointer

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

Same as TRefCountingPointer, but cannot be constructed to or assigned a null value. Because TRefCountingNotNullPointer controls the lifetime of it's target it, should be always safe to assume that it points to a validly allocated object.

### TRefCountingFixedPointer

Same as TRefCountingNotNullPointer, but cannot be retargeted after construction (basically a "const TRefCountingNotNullPointer"). It is a recommended type to be used for safe parameter passing by reference.

### TRefCountingConstPointer, TRefCountingNotNullConstPointer, TRefCountingFixedConstPointer

TRefCountingPointer&lt;X&gt; actually does implicitly convert to TRefCountingPointer&lt;const X&gt;. But some prefer to think of the pointer giving "const" access to the object rather than giving access to a "const object".


###TStrongFixedPointer

TStrongFixedPointer is primarily intended to be used as a safe pointer to a member of a class/struct owned by a reference counting pointer. TStrongFixedPointer essentially acts as a pointer to the member (or whatever object you specify), while keeping a copy of the reference counting pointer (or whatever owner you specify) to ensure that the object is not deallocated while you're still using it. Use mse::make_strong() to construct a TStrongFixedPointer.  

usage example:

    #include "mserefcounting.h"
    
    class H {
    public:
        static std::string foo2(mse::TStrongFixedPointer<std::string, mse::TRefCountingFixedPointer<A>> strong_string_ptr, CRCFPVector& rcfpvector_ref) {
            rcfpvector_ref.clear();
            std::string retval = (*strong_string_ptr);
            return retval;
        }
    
        template<class _TString1Pointer, class _TString2Pointer>
        static std::string foo6(_TString1Pointer i1ptr, _TString2Pointer i2ptr) {
            return (*i1ptr) + (*i2ptr);
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
            std::string s = "some text ";
        };
        typedef std::vector<mse::TRefCountingFixedPointer<A>> CRCFPVector;

        {
            CRCFPVector rcfpvector;
            {
                mse::TRefCountingFixedPointer<A> A_refcountingfixed_ptr1 = mse::make_refcounting<A>();
                rcfpvector.push_back(A_refcountingfixed_ptr1);
            }

            /* strong_string_ptr1 here is essentially a pointer to "A.s" (the string member of class A) welded
            to a refcounting pointer to A to make sure that the object is not deallocated while strong_string_ptr1
            is still around. */
            auto strong_string_ptr1 = mse::make_strong(rcfpvector.front()->s, rcfpvector.front());
            H::foo2(strong_string_ptr1, rcfpvector);

            /* In practice, rather than declaring a specific mse::TStrongFixedPointer parameter, we expect
            functions to be "templatized" so that they can accept any type of pointer. */
            std::string res1 = H::foo6(strong_string_ptr1, strong_string_ptr1);
        }
    }

###TStrongFixedConstPointer

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
Scope pointers are different from other smart pointers in the library in that, by default, they have no runtime safety enforcement mechanism, and the compile-time safety mechanisms aren't (yet) quite sufficient to ensure that they will be used in an intrinsically safe manner. Scope pointers point to scope objects. Scope objects are objects that are allocated on the stack, or whose "owning" pointer is allocated on the stack. So basically the object is destroyed when it, or it's owner, goes out of scope. The purpose of scope pointers and objects is to identify a class of situations that are simple and deterministic enough that no (runtime) safety mechanisms are necessary. In theory, a tool could be constructed to verify that scope pointers are used in a safe manner at compile-time. But in the mean time we provide the option of using a relaxed registered pointer as the scope pointer's base class for enhanced safety and to help catch misuse. Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause relaxed registered pointers to be used in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause them to be used in non-debug modes as well. And as with registered pointers, scope pointers cannot target types that cannot act as a base class. For int, bool and size_t use the safer [substitutes](#primitives) that can act as base classes.

There are two types of scope pointers, [TXScopeFixedPointer](#txscopefixedpointer) and [TXScopeOwnerPointer](#txscopeownerpointer). TXScopeOwnerPointer is similar to boost::scoped_ptr in functionality (but more limited in intended use). It creates an instance of a given class on the heap and destroys that instance in its destructor. (We use "scope" to mean "execution scope", where in boost it seems to refer to "declaration scope".) TXScopeFixedPointer is a "non-owning" (or "weak") pointer to a scope object. It is (intentionally) limited in it's functionality, and is intended pretty much for the sole purpose of passing scope objects by reference as function arguments. For more information on how the safe smart pointers in this library are intended to be used, see [this article](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu).


### TXScopeFixedPointer
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

###TXScopeWeakFixedPointer
Use TXScopeWeakFixedPointer to obtain an xscope pointer to a member of an xscope object. TXScopeWeakFixedPointer is basically the xscope equivalent of [TSyncWeakFixedPointer](#tsyncweakfixedpointer). Use mse::make_xscopeweak() to construct a TXScopeWeakFixedPointer.  

usage example:

    #include "msescope.h"
    
    class H {
    public:
        template<class _TString1Pointer, class _TString2Pointer>
        static std::string foo6(_TString1Pointer i1ptr, _TString2Pointer i2ptr) {
            return (*i1ptr) + (*i2ptr);
        }

    protected:
        ~H() {}
    };
    
    int main(int argc, char* argv[]) {
        /* Obtaining a "safe" pointer to a member of an xscope object: */
        class E {
        public:
            virtual ~E() {}
            std::string s = "some text ";
        };
        
        mse::TXScopeObj<E> xscope_e;
        mse::TXScopePointer<E> E_xscope_ptr1 = &xscope_e;
        
        auto xscopeweak_string_ptr1 = mse::make_xscopeweak(E_xscope_ptr1->s, E_xscope_ptr1);
        
        /* In practice, rather than declaring a specific mse::TXScopeWeakFixedPointer parameter, we expect
        functions to be "templatized" so that they can accept any type of pointer. */
        std::string res1 = H::foo6(xscopeweak_string_ptr1, xscopeweak_string_ptr1);
    }

###TXScopeWeakFixedConstPointer


### Poly pointers
Ideally you won't need  to use these. Poly pointers can act as either a strong/owning pointer or weak/non-owning pointer, as needed. Generally, poly pointers would be expected to be used as function parameters. For functions intended for general use that take reference parameters, it is recommended that you "[templatize](#safely-passing-parameters-by-reference)" the function so that it can take any type of (smart) pointer reference the caller chooses. But if for some reason you can't or don't want to templatize the function, but still want to give the caller some flexibility in terms of pointer reference parameters then you might consider using a poly pointer. While convenient, poly pointers may have a small run-time cost.  

There are currently three types of poly pointer available (and their "const" counterparts) - TRefCountingOrXScopeFixedPointer, TRefCountingOrXScopeOrRawFixedPointer and TSharedOrRawFixedPointer. When constructed from a strong/owning pointer (i.e. a refcounting pointer or an std::shared_ptr), the poly pointer will obtain and hold (shared) ownership of the target object.

### TRefCountingOrXScopeFixedPointer, TRefCountingOrXScopeFixedConstPointer
As you may have guessed, TRefCountingOrXScopeFixedPointer can be constructed from either a TRefCountingFixedPointer or a TXScopeFixedPointer.

### TRefCountingOrXScopeOrRawFixedPointer, TRefCountingOrXScopeOrRawFixedConstPointer
Same as TRefCountingOrXScopeFixedPointer, but also supports construction from "raw" pointers. While this adds flexibility, it means that there is no assurance that TRefCountingOrXScopeOrRawFixedPointer points to a validly allocated object, the way there is with TRefCountingOrXScopeFixedPointer.

### TSharedOrRawFixedPointer, TSharedOrRawFixedConstPointer
For those that are sticking with the standard library, this one supports construction from either an std::shared_ptr, or a "raw" pointer.  

usage example:

    #include "msepoly.h"
    
    int main(int argc, char* argv[]) {
        class A {
        public:
            A(int x) : b(x) {}
            virtual ~A() {}

            int b = 3;
        };
        class D : public A {
        public:
            D(int x) : A(x) {}
        };
        class B {
        public:
            static int foo1(mse::TRefCountingOrXScopeFixedPointer<A> ptr) {
                int retval = ptr->b;
                return retval;
            }
            static int foo2(mse::TRefCountingOrXScopeFixedConstPointer<A> ptr) {
                int retval = ptr->b;
                return retval;
            }
            static int foo3(mse::TRefCountingOrXScopeOrRawFixedPointer<A> ptr) {
                int retval = ptr->b;
                return retval;
            }
            static int foo5(mse::TSharedOrRawFixedPointer<A> ptr) {
                int retval = ptr->b;
                return retval;
            }
        protected:
            ~B() {}
        };
        
        auto A_refcfp = mse::make_refcounting<A>(5);
        mse::TXScopeObj<A> a_xscpobj(7);
        A a_obj(11);
        int res1 = B::foo1(A_refcfp);
        int res2 = B::foo1(&a_xscpobj);

        int res3 = B::foo2(A_refcfp);
        int res4 = B::foo2(&a_xscpobj);
        
        int res21 = B::foo3(A_refcfp);
        int res22 = B::foo3(&a_xscpobj);
        int res23 = B::foo3(&a_obj);
        
        auto A_shp = std::make_shared<A>(5);
        int res41 = B::foo5(A_shp);
        int res42 = B::foo5(&a_obj);
    }

### Safely passing parameters by reference
As has been shown, you can use [registered pointers](#registered-pointers), [reference counting pointers](#reference-counting-pointers) and [scope pointers](#scope-pointers) to safely pass parameters by reference. (Well, scope pointers aren't completely safe yet, but "safer" anyway.) If you're writing a function for general use, we recommend that you "templatize" the function so that it can accept any type of pointer. This is demonstrated in the [TRefCountingOfRegisteredPointer](#trefcountingofregisteredpointer) usage example. Or you can read an article about it [here](http://www.codeproject.com/Articles/1093894/How-To-Safely-Pass-Parameters-By-Reference-in-Cplu). If for some reason you can't or don't want to templatize the function, but still want to give the caller some flexibility in terms of pointer reference parameters then you can use a [poly pointer](#poly-pointers) like TRefCountingOrXScopeFixedPointer. 


### Asynchronously shared objects
One situation where safety mechanisms are particularly important is when sharing objects between asynchronous threads. In particular, when one party (thread) is modifying an object, you want to ensure that no other party accesses it. So we provide TAsyncSharedReadWriteAccessRequester that (like std::shared_ptr) possesses shared ownership of an object to be shared among asynchronous threads, and provides (const and non-const smart) pointers that can be used to safely access the object. For a more information and examples, see [this article](http://www.codeproject.com/Articles/1106491/Sharing-Objects-Between-Threads-in-Cplusplus-the-S). At the moment, these data types cannot target types that cannot act as a base class. If you want to share an int, bool or size_t, use the [safer substitutes](#primitives) that can act as base classes.

### TAsyncSharedReadWriteAccessRequester
Use the writelock_ptr() and readlock_ptr() member functions to obtain pointers to the shared object. Those functions will block until they can obtain the needed lock on the shared object. The obtained pointers will hold on to their lock for as long as they exist. Their locks are released when the pointers are destroyed (generally when they go out of scope).  

Use mse::make_asyncsharedreadwrite<>() to obtain a TAsyncSharedReadWriteAccessRequester. TAsyncSharedReadWriteAccessRequester can be copied and passed-by-value as a parameter (to another thread, generally).

Non-blocking try_writelock_ptr() and try_readlock_ptr() member functions are also available. As are the limited-blocking try_writelock_ptr_for(), try_readlock_ptr_for(), try_writelock_ptr_until() and try_readlock_ptr_until().

### TAsyncSharedReadOnlyAccessRequester
Same as TAsyncSharedReadWriteAccessRequester, but only supports readlock_ptr(), not writelock_ptr(). You can use mse::make_asyncsharedreadonly<>() to obtain a TAsyncSharedReadOnlyAccessRequester. TAsyncSharedReadOnlyAccessRequester can also be copy constructed from a TAsyncSharedReadWriteAccessRequester.

### TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadWriteAccessRequester, TAsyncSharedObjectThatYouAreSureHasNoUnprotectedMutablesReadOnlyAccessRequester
A peculiarity of C++ is that a "const" object is not necessarily guaranteed to be unmodifiable. Specifically in cases where the object has "mutable" members. So, out of an abundance of prudence TAsyncSharedReadWriteAccessRequester and TAsyncSharedReadOnlyAccessRequester do not allow for the simultaneous existence of multiple "readlock_ptr"s. But sometimes you really want to allow for multiple simultaneous readers. So we provide these versions with unwieldy names to remind you of the potential dangers of shared objects with mutable members. Ideally, at some point in the future, we'd be able to determine at compile-time whether or not a type has mutable members.

### TReadOnlyStdSharedFixedConstPointer
For "read-only" situations when you need, or want, the shared object to be managed by std::shared_ptrs we provide a slightly safety-enhanced wrapper for std::shared_ptr. The wrapper enforces "const"ness and tries to ensure that it points to a validly allocated object. Use mse::make_readonlystdshared<>() to construct an mse::TReadOnlyStdSharedFixedConstPointer. And again, beware of sharing objects with mutable members.  

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
			TReadOnlyStdSharedFixedConstPointer is a "safety enhanced" wrapper for std::shared_ptr. And again, beware of
			sharing objects with mutable members. */
			auto read_only_sh_ptr = mse::make_readonlystdshared<A>(5);
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
    }

### msevector

If you're willing to forego a little theoretical safety, msevector<> is still very safe without the overhead of memory management.  
In addition to the (high performance) standard vector iterator, msevector<> also supports a new kind of iterator, called "ipointer", that acts more like a list iterator in the sense that it points to an item rather than a position, and like a list iterator, it is not invalidated by insertions or deletions occurring elsewhere in the container, even if a "reallocation" occurs. In fact, standard vector iterators are so prone to being invalidated that for algorithms involving insertion or deletion, they can be generously considered not very useful, and more prudently considered dangerous. ipointers, aside from being safe, just make sense. Algorithms that work when applied to list iterators will work when applied to ipointers. And that's important as Bjarne famously [points out](https://www.youtube.com/watch?v=YQs6IC-vgmo), for cache coherency reasons, in most cases vectors should be used in place of lists, even when lists are conceptually more appropriate.  
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

### On thread safety
The choice to not include thread safety mechanisms in most of the types in this library is a deliberate one. If the goal is code safety, then we strongly discourage the casual sharing of objects between asynchronous threads (i.e. without the proper safety mechanisms). The practice of sharing objects between asynchronous threads can be prone to severe and insidious bugs that are particularly adept at evading exposure during testing. In cases where it's not practical to avoid the practice, we suggest doing so only in the context of some kind of system that comprehensively ensures against inadvertent unsafe access. For most straight-forward cases you can use the [asynchronous sharing data types](#asynchronously-shared-objects) in this library.
To be clear, we are not discouraging asynchronous programming, or even inter-thread communication in general. Just the "casual" sharing of objects between asynchronous threads.

### Questions and comments
If you have questions or comments you can create a post in the [issues section](https://github.com/duneroadrunner/SaferCPlusPlus/issues).

