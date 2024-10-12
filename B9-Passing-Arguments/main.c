#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/moduleparam.h>
#include<linux/init.h> 

int valueJerry, arr_valueJerry[4];
char *nameJerry;
int cb_valueJerry = 0;

module_param(valueJerry, int, S_IRUSR | S_IWUSR);
module_param(nameJerry, charp, S_IRUSR | S_IWUSR);
module_param_array(arr_valueJerry, int, NULL, S_IRUSR | S_IWUSR);

/*module_param_cd()*/
int notify_param(const char *val, const struct kernel_param *kp)
{
    int res = param_set_int(val, kp);
    if(res==0) {
        printk(KERN_INFO "Call back function called...\n");
        printk(KERN_INFO "New valur of cb_valueJerry = %d\n",cb_valueJerry);
        return 0;
    }
    return -1;
}

const struct kernel_param_ops my_param_ops = 
{
    .set = &notify_param,//Use our setter...
    .get = &param_get_int,//... and standard getter
};

module_param_cb(cb_valueJerry, &my_param_ops, &cb_valueJerry, S_IRUGO|S_IWUSR);

/*Constructor*/
static int __init hello_world_init(void)
{
    int i;
    printk(KERN_INFO "ValueJerry = %d  \n", valueJerry);
    printk(KERN_INFO "cb_valueJerry = %d  \n", cb_valueJerry);
    printk(KERN_INFO "NameJerry = %s \n", nameJerry);
    for (i = 0; i < (sizeof arr_valueJerry / sizeof (int)); i++) {
        printk(KERN_INFO "Arr_value[%d] = %d\n", i, arr_valueJerry[i]);
    }
    printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
    return 0;
}

/*Destructor*/
static void __exit hello_world_exit(void)
{
    printk(KERN_INFO "Kernel Module Removed Sucessfully...\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry <Jerry@gmail.com>");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("1.0");