#!/bin/sh
if [ $# -ne 2 ] ; then
	echo "Usage $0 NODES COUNT"
	echo "Notes: This programs prints a p2p problem file in dimacs format "
	echo "NODES - Number of nodes present in the graph"
	echo "COUNT - Number of problem lines to generate"
	exit 1
fi
awk -v NODES=$1 -v COUNT=$2 '
BEGIN{
if ((COUNT==0) || (NODES==0)) {
	print "c Error COUNT "COUNT
	print "c Error NODES "NODES
	exit(1)
}
print "c 9th DIMACS Implementation Challenge: Shortest Paths"
print "c http://www.dis.uniroma1.it/~challenge9"
print "c Sample point-to-point problem specification file"
print "c"
print "p aux sp p2p "COUNT
print "c contains "COUNT" query pairs"
print "c"
srand();
for (i=0;i<COUNT;++i) {
print "q",int(rand()*(NODES*2))%NODES, int(rand()*(NODES*2))%NODES;
}}'
