#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // copy_from_user(), copy_to_user()
#include <linux/slab.h> //kmalloc


#define TABLE_SIZE 37
#define BIT_COUNT 16

int seg_for_c[TABLE_SIZE] = {
	0b1111001100010001, // A
	0b1111110001010100, // B
	0b1100111100000000, // C
	0b1111110001000100, // D
	0b1000011100000001, // E
	0b1000001100000001, // F
	0b1101111100010000, // G
	0b0011001100010001, // H
	0b1100110001000100, // I
	0b1100010001000100, // J
	0b0000001100101001, // K
	0b0000111100000000, // L
	0b0011001110100000, // M
	0b0011001110001000, // N
	0b1111111100000000, // O
	0b1000001101000001, // P
	0b0111000001010000, // q
	0b1110001100011001, // R
	0b1101110100010001, // S
	0b1100000001000100, // T
	0b0011111100000000, // U
	0b0000001100100010, // V
	0b0011001100001010, // W
	0b0000000010101010, // X
	0b0000000010100100, // Y
	0b1100110000100010, // Z
	0b0111100001000100, // 0
	0b0011000000000000, // 1
	0b0110100000010100, // 2
	0b0111100000010000, // 3
	0b0011000001010000, // 4
	0b0101100001010000, // 5 
	0b0101100001010100, // 6
	0b0111000000000000, // 7
	0b0111100001010100, // 8
	0b0111100001010000, // 9
	0b0000000000000000  // <space>
};

int ind;

// File Operations
// user to driver
static ssize_t vir_16seg_write(struct file *fp,const char *buf, size_t count, loff_t *fpos) {
	char *c = kmalloc(sizeof(char),GFP_KERNEL); //
	int ret = 0;
	printk("[vir_16seg driver] call write\n");
	if( (ret = copy_from_user(c, buf, sizeof(c))) != 0){
		printk("[vir_16seg driver] ERROR: %d bytes have NOT been copied from user\n",ret);
		return -EFAULT; //Bad address
	}
	printk("[vir_16seg driver] write: %c\n", *c);
	// char to seg_for_c's index
	if (*c >= '0' && *c <= '9'){
		ind = 26 + *c - '0';
	}else if((*c >= 'A' && *c <= 'Z') || (*c > 'a' && *c <= 'z')){
		*c = *c & ~('a'-'A'); // toUpper: lower-32 == upper
		ind = *c-'A';
	}else{ind = TABLE_SIZE-1;} // <space> 
	
	printk("[vir_16seg driver] Save idx to: %d\n", ind);
	kfree(c);
	return count;
}

// driver to user
static ssize_t vir_16seg_read(struct file *fp, char *buf, size_t count, loff_t *fpos) { 
	char output_buf[BIT_COUNT] = {};
	int data = seg_for_c[ind];
	int ret = 0;
	printk("[vir_16seg driver] call read\n");
	// binary to char list
    for(int i = 0; i < BIT_COUNT ; i++) {
        output_buf[BIT_COUNT-1 - i] = data % 2?'1':'0';
        data >>= 1;
    }

	if( (ret = copy_to_user(buf, output_buf, sizeof(output_buf))) != 0){
		printk("[vir_16seg driver] ERROR: %d bytes have NOT been copied to user\n",ret);
		return -EFAULT; // Bad address
	}
	printk("[vir_16seg driver] To user: %s\n", output_buf);
	ind = TABLE_SIZE-1; // <space>
	printk("[vir_16seg driver] Clear idx to: %d\n", ind);
	return sizeof(output_buf); 
} 

static int vir_16seg_open(struct inode *inode, struct file *fp) { 
	printk("[vir_16seg driver] call open\n"); 
	return 0; 
}

struct file_operations vir_16seg_fops = { 
	read: vir_16seg_read, 
	write: vir_16seg_write, 
	open: vir_16seg_open 
};
 
#define MAJOR_NUM 233 //244
#define DEVICE_NAME "vir_16seg"

static int vir_16seg_init(void) { 
	printk("[vir_16seg driver] call init\n"); 
	if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &vir_16seg_fops) < 0) { 
		printk("[vir_16seg driver] Can NOT get major %d\n", MAJOR_NUM);
		return (-EBUSY); 
	} 
	printk("[vir_16seg driver] Started and the major is %d\n", MAJOR_NUM);
	ind = TABLE_SIZE-1; //index of seg_for_c 
	return 0; 
} 

static void vir_16seg_exit(void) { 
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME); 
	printk("[vir_16seg driver] call exit\n"); 
} 

module_init(vir_16seg_init); 
module_exit(vir_16seg_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kerry(YuKai Lu) (KerryYK.Lu@gmail.com)");
MODULE_DESCRIPTION("[NYCU EOS 2024] A kernel module for virtual 16-segment display driver.");