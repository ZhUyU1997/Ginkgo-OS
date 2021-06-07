#define ENTRY(_name)	\
	.global _name;		\
    .type   _name, %function;	\
_name:

#define ENDPROC(_name)		\
	.size _name, .-_name

ENTRY(_start)
mv a0, sp
call _start_c
ENDPROC(_start)