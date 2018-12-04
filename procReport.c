/*
TCSS - Operating Systems
Armoni Atherton, Joshua Atherton


Running instructions:
To run do tail -fn 10 /var/log/syslog 

To remove a previously installed the module:
sudo rmmod ./procReport.ko

To install a newly built module:
sudo insmod ./procReport.ko   

Do "make clean" before pushing to git-hub
*/

#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
// #include <linux/init.h>		/* Needed for the macros */

#include <linux/sched/signal.h>
#include <asm/pgtable.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

// #include <linux/mm_types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armoni Atherton, Josh Atherton");
MODULE_DESCRIPTION("Count the segmentation of the pages memorry.");
MODULE_VERSION("1.0");

/* ***** Prototypes ******** */
static int proc_init (void);
static unsigned long virt2phys(struct mm_struct * mm, unsigned long vpage);
static void proc_cleanup(void);
//proc file functions
static int proc_report_show(struct seq_file *m, void *v);
static int proc_report_open(struct inode *inode, struct  file *file);

/* ***** structs **** */ 
typedef struct _counter {
  int contig_pages;
  int noncontig_pages;
  int total_pages;
} stats_counter;

static const struct file_operations proc_report_fops = {
  .owner = THIS_MODULE,
  .open = proc_report_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};

//todo: delete comment below
// wanted fields example: proc_id,proc_name,contig_pages,noncontig_pages,total_pages 

/* ***** Global values **** */ 
 stats_counter pages_counter =  {
   .contig_pages = 0,
   .noncontig_pages = 0,
   .total_pages = 0
 };

/* ***** Functions For the kernel ***************** */ 
/**
 * Inialize  and start the kernal task.
 */
static int proc_init (void) {
  // printk(KERN_INFO "helloModule: kernel module initialized\n");
  struct task_struct *task;
  proc_create("proc_report", 0, NULL, &proc_report_fops);
  for_each_process(task) {
    //Check vaild process.
    if (task->pid > 650) {
      // this prints the name and PID of each task
      printk("%s [%d]\n",task->comm , task->pid);

      struct vm_area_struct *vma = 0;
      unsigned long vpage;
      if (task->mm && task->mm->mmap) {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
          for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
            unsigned long phys = virt2phys(task->mm, vpage);
            //printk("This is physical adress: %lu \n", phys);
          }
      }          
    }
    // printk("%s [%d]\n",task->comm , task->pid);
  }
  return 0;
}

/**
 * Get the mapping of virtual to physical memory addresses.
 */
static unsigned long virt2phys(struct mm_struct * mm, unsigned long vpage) {
  pgd_t *pgd;
  p4d_t *p4d;
  pud_t *pud;
  pmd_t *pmt;
  pte_t *pte;
  struct page *page;

  pgd = pgd_offset(mm, vpage);
  if (pgd_none(*pgd) || pgd_bad(*pgd))
    return 0;

  p4d = p4d_offset(pgd, vpage);
  if (p4d_none(*p4d) || p4d_bad(*p4d))
    return 0;

  pud = pud_offset(p4d, vpage);
  if (pud_none(*pud) || pud_bad(*pud))
    return 0;
  //?!?!?!?!?!?! NOT SURE IF IT SHOULD BE UNSINED LONG
   pmt = pmd_offset(pud, vpage);
  if (pmd_none(*pmt) || pmd_bad(*pmt))
    return 0;

  if (!(pte = pte_offset_map(pmt, vpage)))
    return 0;

  if (!(page = pte_page(*pte)))
    return 0;

  unsigned long physical_page_addr = page_to_phys(page);
  pte_unmap(pte);
  return physical_page_addr;
}

/**
 * Finish and end the kernal task.
 */
static void proc_cleanup(void) {
  remove_proc_entry("proc_report", NULL);
  printk(KERN_INFO "helloModule: performing cleanup of module\n");
}

/**
 * write 
 */
static int proc_report_show(struct seq_file *m, void *v) {
  seq_printf(m, "Hello proc!\n");
  return 0;
}

/**
 * Open the proc file
 */
static int proc_report_open(struct inode *inode, struct  file *file) {
  return single_open(file, proc_report_show, NULL);
}


/* ******** Kernal Run functions ************************ */
module_init(proc_init);
module_exit(proc_cleanup);


//todo: delete
// struct vm_area_struct *vma = 0;
// unsigned long vpage;
  // if (task->mm && task->mm->mmap)
  //     for (vma = task->mm->mmap; vma; vma = vma->vm_next)
  //         for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE)
  //             unsigned long phys = virt2phys(task->mm, vpage);