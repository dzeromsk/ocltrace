CFLAGS+=-std=gnu89 -fPIC -MMD -g -Itools/include
LDLIBS+=-Wl,--no-as-needed -ldl
LDFLAGS+=-fPIC -pie -rdynamic

ocltrace: opencl.o

clean:
	rm -f *.o *.d *.gch ocltrace

format:
	clang-format -style=Google -i *.c *.h

-include $(wildcard *.d)
