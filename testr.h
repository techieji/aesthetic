#define P(...) on?printf(__VA_ARGS__):0
#define TESTR(...) int __testr_tests(__VA_ARGS__)
#define __TB P("====================== \x1b[1mTestr Start\x1b[0m ======================\n");s=f=0
#define TEST(t) P("%sTest %d in file %s, line %d %s (%s).\n",t?"\x1b[92m":"\x1b[91m",t?++s+f:++f+s,__FILE__,__LINE__,t?"succeeded":"failed",t?"omitted":#t)
#define __TE char*c=!f?"\x1b[92m":"\x1b[91m";P("%s================= \x1b[92m%d succeeded, %s%d failed%s =================\x1b[0m\n",c,s,c,f,c)
#define RUN_TESTS(...) do { __TB; __testr_tests(__VA_ARGS__); __TE; } while (0)
char s=0,f=0,on=1;
