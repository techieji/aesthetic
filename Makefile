# https://web.archive.org/web/20100924220817/http://utilitybase.com/article/show/2007/04/09/225/Size+does+matter:+Optimizing+with+size+in+mind+with+GCC
# https://interrupt.memfault.com/blog/code-size-optimization-gcc-flags
# http://www.catb.org/esr/structure-packing/

run:
	gcc aestheti.c stdlib.c -o a.out -g
	./a.out

small:
	gcc aestheti.c stdlib.c -o aes -Os -fconserve-stack -ffunction-sections -fdata-sections -Wl,--gc-sections -fconserve-stack -flto=auto
	strip aes
	strip -R.comment aes
