/*
TCSS - Operating Systems
Armoni Atherton, Joshua Atherton
*/

#include <linux/module.h>	/* Needed by all modules */
// #include <linux/kernel.h>	/* Needed for KERN_INFO */
// #include <linux/init.h>		/* Needed for the macros */

int proc_init (void) {
  printk(KERN_INFO "helloModule: kernel module initialized\n");
  return 0;
}

void proc_cleanup(void) {
  printk(KERN_INFO "helloModule: performing cleanup of module\n");
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);

/*
To run do tail -fn 10 /var/log/syslog 

To remove a previously installed the module:
sudo rmmod ./procReport.ko

To install a newly built module:
sudo insmod ./procReport.ko  

Do "make clean" before pushing to git-hub

*/