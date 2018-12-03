/*
TCSS - Operating Systems
Armoni Atherton, Joshua Atherton
*/

#include <linux/module.h>	/* Needed by all modules */
// #include <linux/kernel.h>	/* Needed for KERN_INFO */
// #include <linux/init.h>		/* Needed for the macros */

#include <linux/kernel.h>
#include <linux/sched/signal.h>
// #include <linux/mm_types.h>

long virt2phys(struct mm_struct * mm, unsigned long vpage) {
  printk("YES!!");
  return 0;

}
int proc_init (void) {
  // printk(KERN_INFO "helloModule: kernel module initialized\n");
  struct task_struct *task;
    for_each_process(task) {
      //Check vaild process.
      if (task->pid > 650) {
        printk("%s [%d]\n",task->comm , task->pid);

        struct vm_area_struct *vma = 0;
        unsigned long vpage;
        if (task->mm && task->mm->mmap) {
          for (vma = task->mm->mmap; vma; vma = vma->vm_next)
            for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
              unsigned long phys = virt2phys(task->mm, vpage);
            }
              // unsigned long phys = virt2phys(task->mm, vpage);
              // printk("YES!");


        }
          // if (task->mm && task->mm->mmap) 
          //     for (vma = task->mm->mmap; vma; vma = vma->vm_next)
          //         for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE)
          //             // unsigned long phys = virt2phys(task->mm, vpage);
          
        }
      // printk("%s [%d]\n",task->comm , task->pid);
    }
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

// struct vm_area_struct *vma = 0;
// unsigned long vpage;
  // if (task->mm && task->mm->mmap)
  //     for (vma = task->mm->mmap; vma; vma = vma->vm_next)
  //         for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE)
  //             unsigned long phys = virt2phys(task->mm, vpage);