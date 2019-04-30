#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define BUFFER_SIZE 1024
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

MODULE_LICENSE("GPL");

/* Define device_buffer and other global data structures you will need here */
static char * device_buffer;

ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/* length is the length of the userspace buffer*/
	/* offset will be set to current position of the opened file after read*/
	/* copy_to_user function: source is device_buffer and destination is the userspace buffer *buffer */
    int max_bytes = BUFFER_SIZE - *offset;
    int read = 0;
    int remaining_bytes = 0;

    if(max_bytes > length) {
        remaining_bytes = length;
    }
    else { 
        remaining_bytes = max_bytes; 
    }
    if(!remaining_bytes) printk(KERN_ALERT "End of Buffer");

    read = remaining_bytes - copy_to_user(buffer, device_buffer + *offset, length);
    printk(KERN_ALERT "Read %d characters from kernel\n", read);
    *offset += read;
    return read;
}

ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/* length is the length of the userspace buffer*/
	/* current position of the opened file*/
	/* copy_from_user function: destination is device_buffer and source is the userspace buffer *buffer */
	int max_bytes = BUFFER_SIZE - *offset;
    int written = 0;
	int remaining_bytes = 0;

	if(max_bytes > length) {
        remaining_bytes = length;
    }
    else { 
        remaining_bytes = max_bytes; 
    }
	written = remaining_bytes - copy_from_user(device_buffer + *offset, buffer, remaining_bytes);
	printk(KERN_ALERT "Wrote %d characters to user\n", written);
	*offset += written;
	return written;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile) {
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	static int opened;
	opened++;
	printk(KERN_ALERT "Char Driver opened %d times\n", opened);
	return 0;
}

int simple_char_driver_close (struct inode *pinode, struct file *pfile) {
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	static int closed;
	closed++;
	printk(KERN_ALERT "Char Driver closed %d times\n", closed);
	return 0;
}

loff_t simple_char_driver_seek (struct file *pfile, loff_t offset, int whence) {
	/* Update open file position according to the values of offset and whence */
	loff_t new_position = 0;
	switch(whence) {
		case SEEK_SET:
			new_position = offset;
			break;
		case SEEK_CUR:
			new_position = pfile->f_pos + offset;
			break;
		case SEEK_END:
			new_position = BUFFER_SIZE - offset;
            break;
        default:
            break;
	}
    if (new_position > BUFFER_SIZE) new_position = BUFFER_SIZE;
    if (new_position < 0) new_position = 0;
    pfile->f_pos = new_position;
    return new_position;
}
/* add the function pointers to point to the corresponding file operations. look at the file fs.h in the linux souce code*/
struct file_operations simple_char_driver_file_operations = {
	.owner   = THIS_MODULE,
	.open    = simple_char_driver_open,
	.release = simple_char_driver_close,
	.read    = simple_char_driver_read,
	.write   = simple_char_driver_write,
	.llseek  = simple_char_driver_seek
};

static int simple_char_driver_init(void) {
	/* print to the log file that the init function is called.*/
	/* register the device */
	printk(KERN_ALERT "Char Driver opened, Welcome!\n");
	device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    register_chrdev(241, "simple_char_device", &simple_char_driver_file_operations);
	return 0;
}

static void simple_char_driver_exit(void) {
	/* print to the log file that the exit function is called.*/
	/* unregister  the device using the register_chrdev() function. */
	kfree(device_buffer);
    unregister_chrdev(241, "simple_char_device");
	printk(KERN_ALERT "Char Driver closed, Bye!\n");

}

/* add module_init and module_exit to point to the corresponding init and exit function*/
module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);