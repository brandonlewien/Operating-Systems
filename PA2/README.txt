Makefile is pretty simple, just the two lines to load the helloworld and char driver to the kernel
Test file follows the guidelines stated in the assignment.
Simple char driver is the driver created for the assignment.
Steps to load: 
sudo rmmod simple_char_driver 				(Remove driver if installed)
make -C /lib/modules/$(uname -r)/build M=$PWD modules   (Compile the module)
sudo insmod simple_char_driver.ko                       (Load and install the driver)

Useful things:
echo [insert words here] >/dev/simple_char_driver
cat /dev/simple_char_driver

lseek = Repositions the file offset of the open file description depending on whence and the offset
seek doesn't work for some odd reason
while(getchar() != '\n') cleans the input stream for the next input
