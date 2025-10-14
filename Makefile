c:
	gcc main.c -ggdb3 -Og -o c.out
cxx:
	g++ main.cpp -std=c++23 -ggdb3 -Og -o cxx.out
bear:
	bear -- make -j c cxx
	make clean
clean:
	rm c.out || :
	rm cxx.out || :
