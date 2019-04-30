#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_cs3753_add(int number1, int number2, int * pointer) {
	int kern;
	printk(KERN_ALERT "First value: %d, Second value %d\n", number1, number2); 
	kern = number1 + number2;

	if(access_ok(VERIFY_WRITE, pointer, 8)) {
		put_user(kern, pointer);
		printk(KERN_ALERT "Final value: %d\n", kern);
		return 0;
	}
	else {
		printk(KERN_ALERT "User space is not valid\n");
		return -1;
	}
}
