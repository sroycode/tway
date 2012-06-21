//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// color property map for tway algos

#ifndef BOOST_GRAPH_TWAY_COLOR_HPP
#define BOOST_GRAPH_TWAY_COLOR_HPP

#include <boost/graph/properties.hpp>
#include <stdint.h>

#define TWAY_COLNAM_WHITE    000
#define TWAY_COLNAM_YELLOW   001
#define TWAY_COLNAM_GREEN    002
#define TWAY_COLNAM_GRAY     003
#define TWAY_COLNAM_BLUE     004
#define TWAY_COLNAM_RED      005
#define TWAY_COLNAM_BLACK    006
#define TWAY_COLNAM_MASKONE  007
#define TWAY_COLNAM_COPPER   010
#define TWAY_COLNAM_STEEL    020
#define TWAY_COLNAM_BRONZE   030
#define TWAY_COLNAM_BRASS    040
#define TWAY_COLNAM_SILVER   050
#define TWAY_COLNAM_GOLD     060
#define TWAY_COLNAM_MASKTWO  070


namespace boost {

/**
 *  tway_color_type: using the first 3*8 bits for regular colors,
 *                   the next 3*8 for special types
 */
struct tway_color_type {
	tway_color_type() : col_(TWAY_COLNAM_WHITE) {}
	tway_color_type(uint8_t col) : col_(col) {}
	inline void operator=(uint8_t& col) {
		col_=col;
	}
	inline void operator+(tway_color_type& c) {
		col_=col_|c.col_;
	}
	inline bool operator==(tway_color_type c) const {
		return (c.col_ & TWAY_COLNAM_MASKTWO ) ?
		       ((c.col_ & TWAY_COLNAM_MASKTWO ) == (col_& TWAY_COLNAM_MASKTWO ))
		       : ((c.col_ & TWAY_COLNAM_MASKONE ) == (col_ & TWAY_COLNAM_MASKONE ));
	}
private:
	uint8_t col_;
};

template <>
struct color_traits<tway_color_type> {
#define COL_TRAITS_STUB(Func,Eve) static tway_color_type Func() { return tway_color_type(TWAY_COLNAM_##Eve); }
	/** Standard Colors */
	COL_TRAITS_STUB(white,WHITE)
//	COL_TRAITS_STUB(yellow,YELLOW)
//	COL_TRAITS_STUB(green,GREEN)
	COL_TRAITS_STUB(gray,GRAY)
//	COL_TRAITS_STUB(red,RED)
//	COL_TRAITS_STUB(blue,BLUE)
	COL_TRAITS_STUB(black,BLACK)
	/** Metallic Colors */
//	COL_TRAITS_STUB(copper,COPPER)
//	COL_TRAITS_STUB(steel,STEEL)
//	COL_TRAITS_STUB(bronze,BRONZE)
//	COL_TRAITS_STUB(brass,BRASS)
	COL_TRAITS_STUB(silver,SILVER)
	COL_TRAITS_STUB(gold,GOLD)
#undef COL_TRAITS_STUB
#define COL_TRAITS_STUB(FuncEve,Func,Eve) static tway_color_type FuncEve() { return tway_color_type(TWAY_COLNAM_##Func | TWAY_COLNAM_##Eve); }
	COL_TRAITS_STUB(silver_gray,SILVER,GRAY)
	COL_TRAITS_STUB(silver_black,SILVER,BLACK)
	COL_TRAITS_STUB(gold_gray,GOLD,GRAY)
	COL_TRAITS_STUB(gold_black,GOLD,BLACK)
#undef COL_TRAITS_STUB

};


} // end namespace boost

#endif // BOOST_GRAPH_TWAY_COLOR_HPP
