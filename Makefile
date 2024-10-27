all:
	# build morloc c mpack library and install
	gcc -shared -o ~/.morloc/lib/libmlcmpack.so -fPIC src/mlcmpack.[ch] src/mpack.[ch]
	cp src/mlcmpack.h ~/.morloc/include/
	# build R libraries
	mkdir -p rbuild
	cp lang/r/rmpack.c src/mlcmpack.[ch] src/mpack.[ch] rbuild
	cd rbuild && R CMD SHLIB -o librmpack.so *.[ch] && mv librmpack.so ~/.morloc/lib/libmpackr.so
	rm -rf rbuild

test:
	bash test/test.sh

clean:
	rm -fr a.out *.gch *.so *.o rbuild/
