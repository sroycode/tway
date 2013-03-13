//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Project: tway
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include "Locals.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <list>
#include <limits>
#include <boost/bind.hpp>
#include "Timer.hpp"
#include "ReadDimacs.hpp"

// #define USE_MANHATTAN_DISTANCE 1
#define USE_MANHATTAN_DISTANCE 1

#ifdef COMPILE_WITH_ASTAR
#include "AstarGraph.hpp"
#define TESTPROG TestAstar::AstarGraph
#else
#ifdef COMPILE_WITH_TWAY
#include "TwayAstarGraph.hpp"
#define TESTPROG TestAstar::TwayAstarGraph
#endif
#endif

// use io
void use_io(TestAstar::ReadDimacs& R, TESTPROG& S)
{
	do {
		U_INT src=0,trg=0, cost=0;
		while ((std::cout << "\n Enter src ") && !(std::cin >> src)) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Invalid Source " << std::endl;
		}
		while ((std::cout << "\n Enter trg ") && !(std::cin >> trg)) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Invalid Target " << std::endl;
		}
		if ( (src>=R.GetNodes()) || (trg>R.GetNodes()) ) {
			std::cout << "\n Please use input range " << R.GetNodes() << std::endl;
			continue;
		}
		std::list<U_INT> vlist;
		U_INT t=TestAstar::timer();
		bool bStat = S.Search(src,trg,vlist,cost);
		t=TestAstar::timer(t);
		std::cout << ((bStat)?" OK   ":" FAIL ") << " Time(ms) " << t;
		if (bStat) std::cout << "  Elems " << vlist.size() << " Cost " << cost;
		std::cout << std::endl;
	} while(true);
}
// use file
struct Cont {
	typedef std::vector<std::pair<U_INT,U_INT> > VecPairT;
	typedef std::list<U_INT> ListT;
	typedef std::vector<ListT> VecListT;
	VecPairT inlist;
	VecListT outlist;
	void addin(U_INT s,U_INT t) {
		inlist.push_back(VecPairT::value_type(s,t));
	}
	void addout(std::list<U_INT> v) {
		outlist.push_back(v);
	}
	size_t size() {
		return inlist.size();
	}
	void print() {
		if (inlist.size()!=outlist.size()) {
			std::cerr << " Somewhere this broke " << std::endl;
			return;
		}
		for (std::size_t i=0;i<inlist.size();++i) {
			std::cout << "[" << inlist[i].first << " -> " << inlist[i].second << "] ";
			for (ListT::const_iterator jt=outlist[i].begin(); jt!=outlist[i].end(); ++jt) {
				std::cout << " - " << *jt ;
			}
			std::cout << std::endl;
		}
	}
};

void use_file(TestAstar::ReadDimacs& R, TESTPROG& S,char *problem_file)
{
	Cont C;
	std::list<U_INT> blanklist;
	R.Process_P2P_Problem_File(problem_file,boost::bind(boost::mem_fn(&Cont::addin),&C,_1,_2));
	size_t county=0,countn=0;
	std::cout << "\nStarting  " << std::endl;
	U_INT t=TestAstar::timer();
	for (Cont::VecPairT::const_iterator it=C.inlist.begin(); it!=C.inlist.end(); ++it) {
		U_INT cost=0;
		std::list<U_INT> vlist;
		bool bStat = S.Search(it->first,it->second,vlist,cost);
		if (bStat) {
			C.addout(vlist);
			++county ;
		} else {
			C.addout(blanklist);
			++countn;
		}
	}
	t=TestAstar::timer(t);
	std::cout << " Time " << t <<  ", Ave " << (long int)(t/C.size());
	std::cout << ",Success " << county << ",Fail " << countn << std::endl;
	C.print();
}
void use_cmdline(TestAstar::ReadDimacs& R, TESTPROG& S,U_INT src, U_INT trg)
{
	if ( (src>=R.GetNodes()) || (trg>R.GetNodes()) ) {
		std::cout << "\n Please use input range " << R.GetNodes() << std::endl;
		exit(1);
	}
	U_INT cost=0;
	std::list<U_INT> vlist;
	U_INT t=TestAstar::timer();
	bool bStat = S.Search(src,trg,vlist,cost);
	t=TestAstar::timer(t);
	std::cout << ((bStat)?" Success ":" Fail ") << " Time(ms) " << t;
	if (bStat) {
		std::cout << "  Elems " << vlist.size() << " List " << cost << std::endl;
		for (std::list<U_INT>::const_iterator it=vlist.begin(); it!=vlist.end(); ++it) {
			std::cout << " - " << *it ;
		}
	}
	std::cout << std::endl;
}



int main(int argc, char **argv)
{
	if (argc<3 || argc>5) {
		std::cerr << "Usage: " << argv[0] << " GRAPHFILE COORDSFILE " << std::endl;
		std::cerr << "   Or  " << argv[0] << " GRAPHFILE COORDSFILE QUERYFILE" << std::endl;
		std::cerr << "   Or  " << argv[0] << " GRAPHFILE COORDSFILE SOURCE TARGET" << std::endl;
		exit(1);
	}
	try {
		U_INT t=TestAstar::timer();
		TestAstar::ReadDimacs R;
		TESTPROG S;
		R.Process_Graph_File(argv[1],boost::bind(boost::mem_fn(&TESTPROG::AddEdge),&S,_1,_2,_3));
		R.Process_Coords_File(argv[2],boost::bind(boost::mem_fn(&TESTPROG::ModifyNode),&S,_1,_2,_3));
		t=TestAstar::timer(t);
		std::cout << " Load Time(ms) " << t << ", Vertices " << R.GetNodes() << std::endl;
		switch (argc) {
		case 3:
			use_io(R,S);
			break;
		case 4:
			use_file(R,S,argv[3]);
			break;
		case 5:
			use_cmdline(R,S, atol(argv[3]),atol(argv[4]));
			break;
		default:
			std::cerr << "Illogical !! " << std::endl;
			break;
		}
	} catch (local_exception d) {
		std::cerr << "Error: " << d.what() << std::endl;
	} catch (std::exception d) {
		std::cerr << "std::exception Error: " << d.what() << std::endl;
	} catch (...) {
		std::cerr << "... Error: " << "Uncaught Exception" << std::endl;
	}
	return 0;
}



