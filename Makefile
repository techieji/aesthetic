# https://web.archive.org/web/20100924220817/http://utilitybase.com/article/show/2007/04/09/225/Size+does+matter:+Optimizing+with+size+in+mind+with+GCC
# https://interrupt.memfault.com/blog/code-size-optimization-gcc-flags
# http://www.catb.org/esr/structure-packing/

# https://stackoverflow.com/a/68339111/11555240    (executable shared objects)
dev:
	gcc -fPIC -shared -o aes -DDL_LOADER="\"/lib64/ld-linux-x86-64.so.2\"" aestheti.c -Wl,-e,run -Wall -rdynamic -g

simple:
	gcc aestheti.c -o aes -g -Wall -rdynamic

lib:
	gcc -c -fPIC lib/lib.c -o lib/lib.o -g -Wall -rdynamic
	gcc -shared -o lib/libstd.so lib/lib.o -Wall -rdynamic

release:
	gcc aestheti.c -o aes -Oz -fconserve-stack -ffunction-sections -fdata-sections -Wl,--gc-sections -fconserve-stack -flto=auto -march=native
	strip aes
	strip -R.comment aes
