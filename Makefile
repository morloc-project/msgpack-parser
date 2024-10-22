test:
	bash test.sh

lib:
	gcc -shared -o mlcmpack.so -fPIC mlcmpack.[ch] mpack.[ch]

clean:
	rm -f a.out *.gch
