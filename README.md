# SaferCPlusPlus
Jan 2016

A collection of safe data types that are compatible with, and can substitute for, common unsafe native c++ types. Currently these include:

i) A safe replacement for native pointers that, unlike std::shared_ptr for example, does not take ownership of the target (and so can point to objects on the stack).

ii) A safe implementation of std::vector<> - bounds checked and "invalid iterator dereference" checked.

iii) A couple of other highly compatible vectors that address the issue of unnecessary iterator invalidation upon insert, erase or reallocation

iv) replacements for the native "int", "size_t" and "bool" types that have default initialization values and address the "signed-unsigned mismatch" issues.

Tested with msvc2013, msvc2010 and g++4.8 (as of Dec 2015).

See the file msetl_blurb.pdf for more info. Or just have a look at msetl_example.cpp to see the library in action.
