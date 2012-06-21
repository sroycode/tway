//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Project: tway
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef _TESTASTAR_LOCALS_HPP_
#define _TESTASTAR_LOCALS_HPP_
#include <exception>

struct local_exception : std::exception {
	local_exception(const char* line) : line_(line) {}
	char const* what() const throw() {
		return line_;
	}
	const char* line_;
};
#define U_INT unsigned int
#define S_INT int

#endif
