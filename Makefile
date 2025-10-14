c:
	gcc main.c -ggdb3 -Og -o c.out
	cp c.out a.out
cxx:
	g++ main.cpp -std=c++23 -ggdb3 -Og -o cxx.out
	cp cxx.out a.out
bear:
	bear -- make -j c cxx
	make clean
clean:
	rm a.out || :
	rm c.out || :
	rm cxx.out || :
