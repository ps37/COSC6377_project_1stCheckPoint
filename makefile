cmd:
	g++ serverdr.cpp -o serverdr.out
	g++ clientdr.cpp -o clientdr.out

clear:
	rm -rf *~ serverdr.out clientdr.out