#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this
struct proc * curproc = myproc();
int exists = 0;
int index = 0;
acquire(&(shm_table.lock));

for (int i = 0; i < 64; i++) {
  if (shm_table.shm_pages[i].id == id) {
    exists = 1;
    index = i;
    break;
  }
}

if (exists) { // CASE 1
 mappages(curproc->pgdir, (void *)PGROUNDUP(curproc->sz), PGSIZE, V2P(shm_table.shm_pages[index].frame), PTE_W | PTE_U);

 shm_table.shm_pages[index].refcnt++; 
 *pointer = (void *)PGROUNDUP(curproc->sz);
 curproc->sz = PGROUNDUP(curproc->sz) + PGSIZE;
}
else { // CASE 2
 for (int i = 0; i < 64; i++) {
  if (!shm_table.shm_pages[i].id && !shm_table.shm_pages[i].frame && !shm_table.shm_pages[i].refcnt) {
   shm_table.shm_pages[i].id = id;
   shm_table.shm_pages[i].frame = kalloc();
   memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
   shm_table.shm_pages[i].refcnt = 1;

   mappages(curproc->pgdir, (void *)PGROUNDUP(curproc->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W | PTE_U);
   
   *pointer = (void *)PGROUNDUP(curproc->sz);
   curproc->sz = PGROUNDUP(curproc->sz) + PGSIZE;
   break;
  }
 }
}

release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!

acquire(&(shm_table.lock));

for (int i = 0; i < 64; i++) {
 if (shm_table.shm_pages[i].id == id) {
  if(shm_table.shm_pages[i].refcnt > 0) {
   shm_table.shm_pages[i].refcnt--;
  }
 }
 if (!shm_table.shm_pages[i].refcnt) {
  shm_table.shm_pages[i].id = 0;
  shm_table.shm_pages[i].frame = 0;
  shm_table.shm_pages[i].refcnt = 0;
 }
}

release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}
