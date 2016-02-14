cl_pp.h:
	gcc -nostdinc -E -Iinclude/ -Iinclude/fake_libc_include/ -D'__attribute__(x)=' -D'__extension__=' -D'extern=' include/CL/cl.h > cl_pp.h

gen_func: cl_pp.h
	python func_defs.py cl_pp.h

