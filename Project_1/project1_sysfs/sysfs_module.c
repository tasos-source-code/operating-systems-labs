#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/sched.h>

static struct kobject *find_roots;
volatile int roots = 0;

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    struct task_struct *curr = current;
	printk("find_roots sysfs opened by process %d\n", current->pid);
	
	while(curr != NULL ){
		printk("id: %d,	name: %s\n", curr->pid, curr->comm);
		if(curr->pid == 1) { break; }
		curr = curr->parent;
    }

    return sprintf(buf, "%d\n", current->pid);
}

static struct kobj_attribute foo_attribute = __ATTR(find_roots, 0660, foo_show, NULL);

static int __init mymodule_init(void){
    int error = 0;

    find_roots = kobject_create_and_add("team_385838473988", kernel_kobj);
    if (!find_roots)
        return -ENOMEM;

    error = sysfs_create_file(find_roots, &foo_attribute.attr);
    if (error) {
        printk("failed to create the foo file in /sys/kernel/team_385838473988\n");
    }

    return error;
}

static void __exit mymodule_exit(void){
    printk("Module uninitialized successfully\n");
    kobject_put(find_roots);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");