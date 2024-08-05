# https://web.archive.org/web/20100924220817/http://utilitybase.com/article/show/2007/04/09/225/Size+does+matter:+Optimizing+with+size+in+mind+with+GCC
# https://interrupt.memfault.com/blog/code-size-optimization-gcc-flags
# http://www.catb.org/esr/structure-packing/

dev:
	gcc aestheti.c lib.c -o a.out -g -Wall

release:
	gcc aestheti.c lib.c -o aes -Oz -fconserve-stack -ffunction-sections -fdata-sections -Wl,--gc-sections -fconserve-stack -flto=auto
	strip aes
	strip -R.comment aes
