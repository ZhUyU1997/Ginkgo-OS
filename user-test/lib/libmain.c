#include "syscall.h"
extern int main(int argc, char *argv[], char *envp[]);

void _start_c(long *p)
{
	int argc = p[0];
	char **argv = p[1];
	char **envp = p[2];

	int ret = main(argc, argv, envp);
	usys_process_exit(ret);
	return;
}
