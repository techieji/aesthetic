run:
	gcc aestheti.c logi.c graphi.c -o a.out
	./a.out

debug:
	gcc aestheti.c logi.c graphi.c -o a.out -O0 -D_FORTIFY_SOURCE=0 -v -Q -da
	./a.out
