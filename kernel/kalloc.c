// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

static struct run* steal_page(int cpu_id);
static void add2tail(struct run* head, struct run* content);

struct {
  struct spinlock lock;
  struct run *freelist;
  char lockName[8];
} kmem[NCPU];

void
kinit()
{
  int i;
  
  for(i = 0; i < NCPU; ++i) {
    snprintf(kmem[i].lockName, 8, "kmem_%d", i);
    initlock(&kmem[i].lock, kmem[i].lockName);
  }

  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int cpu_id;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // get current cpu id
  push_off();
  cpu_id = cpuid();

  acquire(&kmem[cpu_id].lock);
  r->next = kmem[cpu_id].freelist;
  kmem[cpu_id].freelist = r;
  release(&kmem[cpu_id].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int cpu_id;

  // get current cpu id
  push_off();
  cpu_id = cpuid();
  
  acquire(&kmem[cpu_id].lock);
  r = kmem[cpu_id].freelist;
  if(r)
    kmem[cpu_id].freelist = r->next;
  release(&kmem[cpu_id].lock);

  // steal page for empty free-list
  if (r == 0) {

    r = steal_page(cpu_id);
    
    if(r) {
      acquire(&kmem[cpu_id].lock);
      if (kmem[cpu_id].freelist == 0)
        kmem[cpu_id].freelist = r->next;
      else {
        // special case: current freelist become not empty
        add2tail(kmem[cpu_id].freelist, r);
        r = kmem[cpu_id].freelist;
        kmem[cpu_id].freelist = r->next;
      }
      release(&kmem[cpu_id].lock); 
    }
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk

  pop_off();
  return (void*)r;
}

static struct run*
steal_page(int cpu_id) {
  struct run *slow, *fast, *ret;
  int i;

  for (i = 0; i < NCPU; ++i) {
    if (cpu_id == i)
      continue;
    
    acquire(&kmem[i].lock);
    if (kmem[i].freelist) {
      slow = fast = kmem[i].freelist;

      while (fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
      }

      
      ret = kmem[i].freelist;
      kmem[i].freelist = slow->next;
      slow->next = 0;
      release(&kmem[i].lock);
      return ret;
      
    }
    release(&kmem[i].lock);
  }

  return 0;
}


static void 
add2tail(struct run* head, struct run* content) {
  while (head->next)
    head = head->next;
  head->next = content;
}