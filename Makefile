# https://web.archive.org/web/20100924220817/http://utilitybase.com/article/show/2007/04/09/225/Size+does+matter:+Optimizing+with+size+in+mind+with+GCC

run:
	gcc aestheti.c logi.c graphi.c -o a.out -Os
	./a.out

debug:
	gcc aestheti.c logi.c graphi.c -o a.out -O0 -D_FORTIFY_SOURCE=0 -v -Q -da
	./a.out
