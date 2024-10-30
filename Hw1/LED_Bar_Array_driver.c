#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> // copy_from_user(), copy_to_user()
#include <linux/slab.h> //kmalloc

#define SIZE 8

static struct gpio leds_gpios[SIZE] = {
    { 21, GPIOF_OUT_INIT_LOW, "LED_1"}, 
    { 20, GPIOF_OUT_INIT_LOW, "LED_2"},
    { 16, GPIOF_OUT_INIT_LOW, "LED_3"},
    { 12, GPIOF_OUT_INIT_LOW, "LED_4"},
    {  1, GPIOF_OUT_INIT_LOW, "LED_5"}, 
    {  7, GPIOF_OUT_INIT_LOW, "LED_6"},
    {  8, GPIOF_OUT_INIT_LOW, "LED_7"},
    { 25, GPIOF_OUT_INIT_LOW, "LED_8"}
};

dev_t dev = 0; 					// device number (包含major&minor)
static struct cdev LED_cdev;	// character device
static struct class *dev_class;	// device class

// File Operations
// driver to user
static ssize_t LED_Bar_read(struct file *fp, char *buf, size_t count, loff_t *fpos) { 
	uint8_t gpio_state[SIZE];
	pr_info("%s: %s: call read\n", __FILE__, __func__);
	//reading GPIO value
	for(int i=0; i < SIZE; i++){
		gpio_state[i] = gpio_get_value(leds_gpios[i].gpio);
	}
	if( copy_to_user(buf, &gpio_state, SIZE) > 0) { 
		pr_err("%s: %s: Not all the bytes have been copied to user\n", __FILE__, __func__); 
	}
	pr_info("%s: %s: To user: %hhn\n", __FILE__, __func__, gpio_state);
	return 0; 
} 

// user to driver
static ssize_t LED_Bar_write(struct file *fp,const char *buf, size_t count, loff_t *fpos) {
	char *rec_buf = kzalloc(sizeof(char),GFP_KERNEL);
	int ret = 0, data = 0;
	pr_info("%s: %s: call write\n", __FILE__, __func__);
	if( (ret = copy_from_user(rec_buf, buf, sizeof(buf))) != 0){
		pr_err("%s: %s: %d bytes have NOT been copied from user\n", __FILE__, __func__, ret);
		return -EFAULT; //Bad address
	}
	// char 0~255
	data = *rec_buf;
    if (data > 255 || data < 0){
        pr_warn("%s: %s: Wrote data from user %d are overflow (!=0~255)\n", __FILE__, __func__, data);
    } 
	pr_info("%s: %s: write: %d\n", __FILE__, __func__, data);
    for(int i = 0; i < SIZE ; i++) {
        if(data % 2){
			gpio_set_value(leds_gpios[SIZE-1-i].gpio, 1);
		}else{
			gpio_set_value(leds_gpios[SIZE-1-i].gpio, 0);
		}
		pr_info("%s: %s: Set GPIO %d: %d\n", __FILE__, __func__, leds_gpios[SIZE-1-i].gpio, gpio_get_value(leds_gpios[SIZE-1-i].gpio));
        data >>= 1;
    }
	
	kfree(rec_buf);
	return count;
}

static int LED_Bar_open(struct inode *inode, struct file *fp) { 
	printk("[LED Bar driver] call open\n"); 
	return 0; 
}
static int LED_Bar_release(struct inode *inode, struct file *fp) { 
	printk("[LED Bar driver] call release\n");
	for(int i=0; i < SIZE; i++){
		gpio_set_value(leds_gpios[i].gpio, 0);
	}
	return 0; 
}

struct file_operations LED_Bar_fops = { 
	read:  LED_Bar_read, 
	write: LED_Bar_write, 
	open:  LED_Bar_open, 
	release: LED_Bar_release
};
 
#define DEVICE_NAME "LED_Bar_Arrary"

/* Module Init function */ 
static int LED_Bar_init(void) {

	pr_info("%s: %s: call init\n", __FILE__, __func__);
    
    /*Allocating Major number*/ 
	// 由kernel動態分配
	if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) <0){ 
		pr_err("%s: %s: Cannot allocate major number\n", __FILE__, __func__);
		goto r_unreg; 
	} 
    pr_info("%s: %s: Major = %d Minor = %d \n", __FILE__, __func__,MAJOR(dev), MINOR(dev));
    /*Creating cdev structure*/ 
	// 將cdev的ops指標指到fops
	cdev_init(&LED_cdev,&LED_Bar_fops);

    /*Adding character device to the system*/
	if((cdev_add(&LED_cdev,dev,1)) < 0){ 
		pr_err("%s: %s: Cannot add the device to the system\n", __FILE__, __func__);
		goto r_del; 
	}

    /*Creating struct class*/ 
	if((dev_class = class_create(THIS_MODULE,"LED_Bar_class")) == NULL){ 
		pr_err("%s: %s: Cannot create the struct class\n", __FILE__, __func__);
		goto r_class; 
	} 
    
	/*Creating device*/ 
	if((device_create(dev_class,NULL,dev,NULL,"LED_Bar_Array_device")) == NULL){ 
		pr_err("%s: %s: Cannot create the Device\n", __FILE__, __func__);
		goto r_device; 
	} 
    
	// Verify whether the GPIO is valid or not.
	/*
	for (int i = 0; i < SIZE; i++){
        if(!gpio_is_valid(leds_gpios[i].gpio)){
			pr_err("%s: %s: GPIO %d is not valid.\n", __FILE__, __func__, leds_gpios[i].gpio);
			goto r_device;
		}
    } 
	*/
    // Request the GPIO from the Kernel GPIO subsystem.
	if(gpio_request_array(leds_gpios, SIZE)){
		pr_err("%s: %s: GPIO can not request.\n", __FILE__, __func__);
		goto r_gpio;
	}

	pr_info("%s: %s: Started and the device number is (%d,%d)\n", __FILE__, __func__, MAJOR(dev), MINOR(dev));
	return 0; 

	// Error: 註銷流程
	r_gpio:
		gpio_free_array(leds_gpios, SIZE); 
	r_device: 
		device_destroy(dev_class,dev); 
	r_class: 
		class_destroy(dev_class); 
	r_del: 
		cdev_del(&LED_cdev); 
	r_unreg: 
		unregister_chrdev_region(dev, 1); 
	return -1;
} 

static void LED_Bar_exit(void) {
    pr_info("%s: %s: call exit\n", __FILE__, __func__);
	// Release the GPIO
	gpio_free_array(leds_gpios, SIZE); 
	device_destroy(dev_class,dev);
	class_destroy(dev_class); 
	cdev_del(&LED_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("%s: %s: Device Driver Remove...Done!!\n", __FILE__, __func__); 
} 

module_init(LED_Bar_init); 
module_exit(LED_Bar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kerry(YuKai Lu) (KerryYK.Lu@gmail.com)");
MODULE_DESCRIPTION("[NYCU EOS 2024] A kernel module for 4-LED Array driver.");