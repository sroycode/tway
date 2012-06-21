//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Project: tway
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef _READ_DIMACS_HPP_
#define _READ_DIMACS_HPP_
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include "Locals.hh"

namespace TestAstar {
class ReadDimacs {
public:
	typedef boost::function<void (U_INT,U_INT,S_INT)> Graph_ExtrT;
	typedef boost::function<void (U_INT,S_INT,S_INT)> Coords_ExtrT;
	typedef boost::function<void (U_INT,U_INT)> Probpp_ExtrT;
	enum data_type { GRAPH_DATA, COORDS_DATA, PROBPP_DATA, ERROR_DATA };
	ReadDimacs() : nodes(0),arcs(0),probpp(0) {}
	~ReadDimacs() {}
	void Process_Graph_File(const char* File, Graph_ExtrT fn) {
		ReadFile(File,fn,NULL,NULL);
	}
	void Process_Coords_File(const char* File, Coords_ExtrT fn) {
		ReadFile(File,NULL,fn,NULL);
	}
	void Process_P2P_Problem_File(const char* File, Probpp_ExtrT fn) {
		ReadFile(File,NULL,NULL,fn);
	}
	void Process_Multiple(const char* File, Graph_ExtrT graph_fn, Coords_ExtrT coords_fn, Probpp_ExtrT probpp_fn) {
		ReadFile(File,graph_fn, coords_fn, probpp_fn);
	}
	size_t GetNodes() const {
		return nodes;
	}
private:
	typedef std::vector<std::string> StrVecT;
	size_t nodes, arcs, probpp;

	inline StrVecT LineConv(std::string inS, const char* Sep=" ") {
		StrVecT t;
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		try {
			boost::char_separator<char> sepA(Sep);
			tokenizer tokensA(inS, sepA);
			BOOST_FOREACH(std::string tokA, tokensA) {
				t.insert(t.end(),StrVecT::value_type(tokA));
			}
		} catch (std::exception e) {
			throw local_exception("Cannot Convert Line");
		}
		return t;
	}
#define FNKCOMP_ONE(ONE)         (a[1]==ONE)
#define FNKCOMP_TWO(ONE,TWO)     (a[1]==ONE) && (a[2]==TWO)
#define FNKCOMP_THR(ONE,TWO,THR) (a[1]==ONE) && (a[2]==TWO) && (a[3]==THR)
	void ReadFile(const char* File, Graph_ExtrT graph_fn, Coords_ExtrT coords_fn, Probpp_ExtrT probpp_fn) {
		std::ifstream file(File);
		if (!file.is_open()) throw local_exception("Cannot Open GraphFile");
		std::string line;
		size_t count_arcs=0,count_nodes=0,count_probpp=0;
		try {
			data_type D = ERROR_DATA;
			while (std::getline(file,line)) {
				if (! line.length()) continue;
				if (line[0] == 'c') continue; // comment
				if (line[0] == 'p') {
					StrVecT a=LineConv(line);
					if (a.size()==4) {
						if (FNKCOMP_ONE("sp")) {
							D = GRAPH_DATA;
							nodes = atol(a[2].c_str());
							arcs = atol(a[3].c_str());
						}
					} else if (a.size()==5) {
						if (FNKCOMP_THR("aux","sp","co")) {
							if (nodes != (size_t)atol(a[4].c_str()) )
								throw local_exception("Coord != Node  Mismatch");
							D = COORDS_DATA;
						}
						if (FNKCOMP_THR("aux","sp","p2p")) {
							D = PROBPP_DATA;
							probpp = atol(a[4].c_str());
						}
					} else
						throw local_exception("Invalid Line p in GraphFile");
					continue;
				}
				if (line[0] == 'a' && D==GRAPH_DATA) {
					StrVecT a=LineConv(line);
					if (a.size() != 4) throw local_exception("Invalid Line a in Graph Data");
					graph_fn( (U_INT)atol(a[1].c_str()), (U_INT)atol(a[2].c_str()), (S_INT)atol(a[3].c_str()) );
					++count_arcs;
					continue;
				}
				if ((line[0] == 'v') && (D==COORDS_DATA)) {
					StrVecT a=LineConv(line);
					if (a.size() != 4) throw local_exception("Invalid Line v in CoordsFile");
					coords_fn( (U_INT)atol(a[1].c_str()), (S_INT)atol(a[2].c_str()), (S_INT)atol(a[3].c_str()) );
					++count_nodes;
					continue;
				}
				if ((line[0] == 'q') && (D==PROBPP_DATA)) {
					StrVecT a=LineConv(line);
					if (a.size() != 3) throw local_exception("Invalid Line a in P2P ProblemFile");
					probpp_fn( (U_INT)atol(a[1].c_str()), (S_INT)atol(a[2].c_str()) );
					++count_probpp;
					continue;
				}
				throw local_exception("Invalid Line unknown in GraphFile");
			}
		} catch (local_exception d) {
			if (file.is_open()) file.close();
			throw local_exception(d.what());
		} catch (...) {
			if (file.is_open()) file.close();
			throw local_exception("Unknown Exception thrown");
		}
		file.close();
		if (count_arcs && count_arcs!=arcs) throw local_exception("Arc count Mismatch");
		if (count_nodes && count_nodes!=nodes) throw local_exception("Coord count Mismatch");
		if (count_probpp && count_probpp!=probpp) throw local_exception("Problem p2p count Mismatch");
	}
};
} // namespace TestAstar
#endif
