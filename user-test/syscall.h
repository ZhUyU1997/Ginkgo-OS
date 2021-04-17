#pragma once

#define	__NR_putstring	1
#define	__NR_create 	2
#define	__NR_present	3

unsigned long putstring(char *string);
void *create();
void present();