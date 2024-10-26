cpp:
	g++ mlcmpack.cpp mpack.[ch] mlcmpack.[ch]

test:
	bash test.sh

pylib:
	gcc -shared -o mlcmpack.so -fPIC mlcmpack.[ch] mpack.[ch]

rlib:
	R CMD SHLIB -o mlcmpack_r.so mlcmpack_r.c mlcmpack.[ch] mpack.[ch]

clean:
	rm -f a.out *.gch *.so
