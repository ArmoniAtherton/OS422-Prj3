/*
TCSS - Operating Systems
Armoni Atherton, Joshua Atherton


Running instructions:
To run do tail -fn 10 /var/log/syslog 

To remove a previously installed the module:
sudo rmmod ./procReport.ko

To install a newly built module:
sudo insmod ./procReport.ko   

cd /proc to view file

Do "make clean" before pushing to git-hub
*/

#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
// #include <linux/init.h>    /* Needed for the macros */

#include <linux/sched/signal.h>
#include <asm/pgtable.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include<linux/list.h>

// #include <linux/mm_types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armoni Atherton, Josh Atherton");
MODULE_DESCRIPTION("Count the segmentation of the pages memorry.");
MODULE_VERSION("1.0");

/* ***** structs **** */ 
typedef struct _counter {
  unsigned long pid;
  char * name;
  unsigned long contig_pages;
  unsigned long noncontig_pages;
  unsigned long total_pages;
} stats_counter;

typedef struct _counter_list {
  stats_counter page;
  struct _counter_list * next;
} counter_list;

/* ***** Prototypes ******** */
static int proc_init (void);
static unsigned long virt2phys(struct mm_struct * mm, unsigned long vpage);
static void iterate_pages(void);
static void write_to_console(void);

//proc file functions
static int proc_report_show(struct seq_file *m, void *v);
static int proc_report_open(struct inode *inode, struct  file *file);
static void proc_cleanup(void);


/* ***** Global values **** */ 
static const struct file_operations proc_report_fops = {
  .owner = THIS_MODULE,
  .open = proc_report_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
// to count total pages stats
unsigned long total_contig_pgs = 0;
unsigned long total_noncontig_pgs = 0;
unsigned long total_pgs = 0;
// linked list of page stats
counter_list stats_list;

/* ***** Functions For the kernel ***************** */ 
/**
 * Inialize  and start the kernal task.
 */
static int proc_init (void) { 
  
  iterate_pages();
  write_to_console();
  write_to_proc_report();

  proc_create("proc_report", 0, NULL, &proc_report_fops);
  return 0;
}

static void iterate_pages(void) {
  struct task_struct *task;
  counter_list *last = &stats_list;
  for_each_process(task) {
    stats_counter one_process_counter =  {
      .pid = 0,
      .name = 0,
      .contig_pages = 0,
      .noncontig_pages = 0,
      .total_pages = 0
    };
    
    //Check vaild process.
    if (task->pid > 650) {
      struct vm_area_struct *vma = 0;
      unsigned long vpage;
      unsigned long prev_page_phys = 0;
      if (task->mm && task->mm->mmap) {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
          for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
            unsigned long phys = virt2phys(task->mm, vpage);
            one_process_counter.total_pages += 1;
            //
            if ( phys != 0) {
              if (prev_page_phys - phys == 0) { // contiguous
                one_process_counter.contig_pages += 1;
              } else {
                one_process_counter.noncontig_pages += 1;
              }
            }
            prev_page_phys = phys;
          }
        }
      }
      //update fields for a page
      one_process_counter.pid = task->pid;
      one_process_counter.name = task->comm;

      //add to total page count
      total_contig_pgs += one_process_counter.contig_pages;
      total_noncontig_pgs += one_process_counter.noncontig_pages;
      total_pgs += one_process_counter.total_pages;
    } // end if > 650
    counter_list list_node =  {
      .page = one_process_counter,
      .next = last
    };
    last = list_node.next;
  } // end for_each
}

/**
 * Get the mapping of virtual to physical memory addresses.
 */
static unsigned long virt2phys(struct mm_struct * mm, unsigned long vpage) { //todo: //?!?!?!?!?!?! NOT SURE IF IT SHOULD BE UNSINED LONG
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
   pmt = pmd_offset(pud, vpage);
  if (pmd_none(*pmt) || pmd_bad(*pmt))
    return 0;

  if (!(pte = pte_offset_map(pmt, vpage)))
    return 0;

  if (!(page = pte_page(*pte)))
    return 0;

  //unsigned long physical_page_addr = page_to_phys(page);
  pte_unmap(pte);
  return page_to_phys(page); //physical_page_addr;
}

static void write_to_console(void) {
  counter_list * item;
  item = &stats_list;
  while (item) {
    //proc_id,proc_name,contig_pages,noncontig_pages,total_pages
    printk("%lu,%s,%lu,%lu,%lu", item->page.pid, item->page.name,
        item->page.contig_pages, item->page.noncontig_pages, item->page.total_pages);
    item = item->next;
  }
  // TOTALS,,contig_pages,noncontig_pages,total_pages
  printk("TOTALS,,%lu,%lu,%lu",
    total_contig_pgs, total_noncontig_pgs, total_pgs); 
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
  seq_printf(m, "PROCESS REPORT: \nproc_id,proc_name,contig_pages,noncontig_pages,total_pages \n");
  
  counter_list * item;
  item = &stats_list;
  while (item) {
    //proc_id,proc_name,contig_pages,noncontig_pages,total_pages
    seq_printf(m, "%lu,%s,%lu,%lu,%lu\n", item->page.pid, item->page.name,
        item->page.contig_pages, item->page.noncontig_pages, item->page.total_pages);
    item = item->next;
  }
  // TOTALS,,contig_pages,noncontig_pages,total_pages
  seq_printf(m, "TOTALS,,%lu,%lu,%lu\n",
    total_contig_pgs, total_noncontig_pgs, total_pgs); 

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
