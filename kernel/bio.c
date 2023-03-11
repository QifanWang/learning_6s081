// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
#define BIO_HASH(x) (x % NBUCKET)

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked lists of each bucket buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf buc_head[NBUCKET];

  struct spinlock buc_locks[NBUCKET];
} bcache;

static char bucket_names[NBUCKET][16];
int i = sizeof(bcache.buc_head);
static void removeFrom(struct buf* b);
static void insertTo(struct buf* b, int bucketID);

void
binit(void)
{
  int i;
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers for each bucket
  for (i = 0; i < NBUCKET; i++) {
    bcache.buc_head[i].prev = &bcache.buc_head[i];
    bcache.buc_head[i].next = &bcache.buc_head[i];

    snprintf(bucket_names[i], 16, "bucket_%d", i);
    initlock(&bcache.buc_locks[i], bucket_names[i]);

    for (b = &bcache.buf[i]; b < bcache.buf+NBUF; b += NBUCKET) {
      b->next = bcache.buc_head[i].next;
      b->prev = &bcache.buc_head[i];
      initsleeplock(&b->lock, "buffer");
      bcache.buc_head[i].next->prev = b;
      bcache.buc_head[i].next = b;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  int i, id, oldInd;
  uint lastvisit;
  struct buf *b;
  struct buf *pick;

  id = BIO_HASH(blockno);

  acquire(&bcache.buc_locks[id]);

  // Is the block already cached?
  for(b = bcache.buc_head[id].next; b != &bcache.buc_head[id]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buc_locks[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.buc_locks[id]);

  // eviction
  acquire(&bcache.lock);
  acquire(&bcache.buc_locks[id]);
  
  // must search again. There might be an eviction after last search.
  for(b = bcache.buc_head[id].next; b != &bcache.buc_head[id]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buc_locks[id]);
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.buc_locks[id]);

  lastvisit = 0xffffffff;
  pick = 0;
  oldInd = -1;
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer by time stamp
  for (i = 0; i < NBUCKET; i++) {
    acquire(&bcache.buc_locks[i]);
    
    // find lru unused buf in this bucket
    struct buf* curPick = 0;
    for (b = bcache.buc_head[i].next; b != &bcache.buc_head[i]; b = b->next) {
      if (b->refcnt == 0 && b->lasttick <= lastvisit) {
        lastvisit = b->lasttick;
        curPick = b;
      }
    }

    if(curPick) {

      if (pick) {
        
        removeFrom(curPick);
        release(&bcache.buc_locks[i]);

        // ensure most two locks acquired
        // restore
        acquire(&bcache.buc_locks[oldInd]);
        insertTo(pick, oldInd);
        release(&bcache.buc_locks[oldInd]);

        pick = curPick;
        oldInd = i;

      } else {
        removeFrom(curPick);
        release(&bcache.buc_locks[i]);

        pick = curPick;
        oldInd = i;
      }
    } else {
      release(&bcache.buc_locks[i]);
    }
    
  }
  
  if (pick) {
    pick->dev = dev;
    pick->blockno = blockno;
    pick->valid = 0;
    pick->refcnt = 1;

    // insert to 
    acquire(&bcache.buc_locks[id]);
    insertTo(pick, id);
    release(&bcache.buc_locks[id]);

    release(&bcache.lock);
    acquiresleep(&pick->lock);
    return pick;
  }


  release(&bcache.lock);

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  int id;

  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  id = BIO_HASH(b->blockno);

  acquire(&bcache.buc_locks[id]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->lasttick = ticks;
  }
  
  release(&bcache.buc_locks[id]);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}

static void 
removeFrom(struct buf* b) {
  b->next->prev = b->prev;
  b->prev->next = b->next;
}

static void 
insertTo(struct buf* b, int bucketID) {
  b->next = bcache.buc_head[bucketID].next;
  b->prev = &bcache.buc_head[bucketID];
  bcache.buc_head[bucketID].next->prev = b;
  bcache.buc_head[bucketID].next = b;
}