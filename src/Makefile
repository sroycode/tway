CC = g++
BOOST_TWAY_INCLUDE = ../include
CCFLAGS = -ansi -Wall -Wno-deprecated -O2 -DNDEBUG -I$(BOOST_TWAY_INCLUDE)
LDFLAGS = -rdynamic
DATADIR = ../data

EXECS = test_tway test_astar

all:	$(EXECS)

test_tway:	TestTway.o
	$(CC) $(CCFLAGS) $(LDFLAGS) -o test_tway TestTway.o

test_astar:	TestAstar.o
	$(CC) $(CCFLAGS) $(LDFLAGS) -o test_astar TestAstar.o 

TestTway.o:	TestProgram.cc
	$(CC) -c $(CCFLAGS) -DCOMPILE_WITH_TWAY TestProgram.cc -o TestTway.o

TestAstar.o:	TestProgram.cc
	$(CC) -c $(CCFLAGS) -DCOMPILE_WITH_ASTAR TestProgram.cc -o TestAstar.o

test:
	./test_tway $(DATADIR)/sample.gr $(DATADIR)/sample.co $(DATADIR)/sample.p2p
	./test_astar $(DATADIR)/sample.gr $(DATADIR)/sample.co $(DATADIR)/sample.p2p

strip:
	strip $(EXECS)

clean:
	rm -f $(EXECS) *.o
