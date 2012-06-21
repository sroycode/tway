//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Project: tway
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef _TIMER_HPP_
#define _TIMER_HPP_
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

namespace TestAstar {
U_INT timer(U_INT Elapsed=0L)
{
	struct timeval x;
	gettimeofday(&x,NULL);
	return (U_INT)((x.tv_sec*1000L)+(x.tv_usec/1000L)) -Elapsed;
}
}
#endif
