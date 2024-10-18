test:
	gcc test.c mlcmpack.[ch] mpack.[ch] && ./a.out
	gcc parser.c mlcmpack.[ch] mpack.[ch] && ./a.out

clean:
	rm -f a.out *.gch
