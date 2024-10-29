#include <linux/module.h>

static int __init hello_world_init (void){
    pr_info("Hello, Module!\n");
    return 0;
}

static void __exit hello_world_exit (void){
    pr_info("Goodbye, Module!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dimitri Kokkonis (kokkonisd@gmail.com)");
MODULE_DESCRIPTION("A simple hello world kernel module");
