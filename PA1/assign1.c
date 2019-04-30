#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

int main(void){
	int pointer;
	syscall(333);
	syscall(334, 32, 40, &pointer);
	printf("User Space Value: %d\n", pointer);
	return 0;
}