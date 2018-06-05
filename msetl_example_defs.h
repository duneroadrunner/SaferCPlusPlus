
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSETL_EXAMPLE_DEFS_H
#define MSETL_EXAMPLE_DEFS_H

/* Following are compile-time directives that control the behavior of the library. Generally, these symbols should be
consistently defined (or not) in all modules in the project. And generally, these symbols would be defined via build
options rather than "#define" directives in the code such as the ones here (for demonstration/documentation purposes). */

//#define MSE_SAFER_SUBSTITUTES_DISABLED /* This will replace all the classes with their native/standard counterparts. */

/* Each of the following will replace a subset of the classes with their native/standard counterparts. */
//#define MSE_MSTDVECTOR_DISABLED
//#define MSE_REGISTEREDPOINTER_DISABLED
//#define MSE_SAFERPTR_DISABLED /* MSE_SAFERPTR_DISABLED implies MSE_REGISTEREDPOINTER_DISABLED too. */
//#define MSE_PRIMITIVES_DISABLED
//#define MSE_REFCOUNTINGPOINTER_DISABLED
//#define MSE_SCOPEPOINTER_DISABLED

/* The following adds run-time checks to scope pointers in debug mode */
//#define MSE_SCOPEPOINTER_DEBUG_RUNTIME_CHECKS_ENABLED
//#define MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED // This adds them to non-debug modes too.

/* The following will result in program termination instead of exceptions being thrown. */
//#define MSE_CUSTOM_THROW_DEFINITION(x) std::cerr << std::endl << x.what(); exit(-11)

/* The following directs the vectors and arrays to use the safe substitutes for native primitives (like int
and size_t) in their interface and implementation. This adds a marginal increase in safety. (Mostly due to
the interface.) */
//#define MSE_MSEVECTOR_USE_MSE_PRIMITIVES 1
//#define MSE_MSEARRAY_USE_MSE_PRIMITIVES 1

/* msvc2015's incomplete support for "constexpr" means that range checks that should be done at compile time would
be done at run time, at significant cost. So they are disabled by default for that compiler. Here we're "forcing"
them to be enabled. */
#define MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED

#define MSE_SELF_TESTS

#endif /*ndef MSETL_EXAMPLE_DEFS_H*/
