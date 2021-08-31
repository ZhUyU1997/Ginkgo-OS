#include <syscall.h>
#include <vfs.h>
#include <stdio.h>

extern int main(int argc, char *argv[], char *envp[]);

void _start_c(long *p)
{
	int argc = (int)p[0];
	char **argv = (char **)p[1];
	char **envp = (char **)p[2];
	extern void do_init_mem();
	do_init_mem();
	do_vfs_init();

	int ret = main(argc, argv, envp);
	usys_process_exit(ret);
	return;
}
