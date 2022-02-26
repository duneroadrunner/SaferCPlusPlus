
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESUPPRESSCHECKS_H_
#define MSESUPPRESSCHECKS_H_

#ifndef MSE_SUPPRESS_CHECK_IN_XSCOPE

namespace mse {
#define MSE_IMPL_MACRO_TEXT_CONCAT_(a,b)  a##b

	namespace rsv {
		inline void suppress_check_directive() {}
#define MSE_SUPPRESS_CHECK_IN_XSCOPE \
		mse::rsv::suppress_check_directive();

#define MSE_IMPL_SUPPRESS_CHECK_IN_DECLSCOPE_LABEL_(a) MSE_IMPL_MACRO_TEXT_CONCAT_(mse_suppress_check_directive_, a)
#define MSE_IMPL_SUPPRESS_CHECK_IN_DECLSCOPE_UNIQUE_NAME MSE_IMPL_SUPPRESS_CHECK_IN_DECLSCOPE_LABEL_(__LINE__)

#define MSE_SUPPRESS_CHECK_IN_DECLSCOPE \
		static void MSE_IMPL_SUPPRESS_CHECK_IN_DECLSCOPE_UNIQUE_NAME() {}
	}
}

#endif // !MSE_SUPPRESS_CHECK_IN_XSCOPE


#ifndef MSE_LH_SUPPRESS_CHECK_IN_XSCOPE

#ifndef MSE_LEGACYHELPERS_DISABLED

#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE

#else // !MSE_LEGACYHELPERS_DISABLED

#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE MSE_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE MSE_SUPPRESS_CHECK_IN_DECLSCOPE

#endif // !MSE_LEGACYHELPERS_DISABLED

#endif // !MSE_LH_SUPPRESS_CHECK_IN_XSCOPE


#endif // MSESUPPRESSCHECKS_H_
