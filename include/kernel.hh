#pragma once

extern "C" {
#include "kern_c.h"
}

#include "atomic.hh"
#include <stdarg.h>

#define KBASE   0xFFFFFF0000000000ull
#define KCODE   0xFFFFFFFFC0000000ull
#define KSHARED 0xFFFFF00000000000ull
#define USERTOP 0x0000800000000000ull

#define KCSEG (2<<3)  /* kernel code segment */
#define KDSEG (3<<3)  /* kernel data segment */

static inline uptr v2p(void *a) {
  uptr ua = (uptr) a;
  if (ua >= KCODE)
    return ua - KCODE;
  else
    return ua - KBASE;
}

static inline void *p2v(uptr a) {
  uptr ac = a + KCODE;
  if (ac >= KCODE)
    return (void*) ac;
  else
    return (u8 *) a + KBASE;
}

struct trapframe;
struct cilkframe;
struct spinlock;
struct condvar;
struct context;
struct vmnode;
struct inode;
struct node;
struct file;
struct stat;
struct proc;
struct vmap;
struct pipe;
struct work;

// bio.c
void            binit(void);
struct buf*     bread(u32, u64, int writer);
void            brelse(struct buf*, int writer);
void            bwrite(struct buf*);

// cga.c
void            cgaputc(int c);

// condvar.c
extern u64 ticks;
extern struct spinlock tickslock;
extern struct condvar cv_ticks;
void            initcondvar(struct condvar *, const char *);
void            destroycondvar(struct condvar *);
void            cv_sleep(struct condvar *cv, struct spinlock*);
void            cv_sleepto(struct condvar *cv, struct spinlock*, u64);
void            cv_wakeup(struct condvar *cv);
void            cv_tick(void);

// console.c
void            cprintf(const char*, ...) __attribute__((format(printf, 1, 2)));
void            vcprintf(const char *fmt, va_list ap);
void            panic(const char*, ...) 
                  __noret__ __attribute__((format(printf, 1, 2)));
void            kerneltrap(struct trapframe *tf) __noret__;
void            snprintf(char *buf, u32 n, const char *fmt, ...);

#define assert(c)   if (!(c)) { cprintf("%s:%d: ", __FILE__, __LINE__); panic("assertion failure"); }

// e1000.c
extern int e1000irq;
extern int e1000init;
void            e1000intr(void);
int             e1000tx(void *buf, u32 len);
void            e1000hwaddr(u8 *hwaddr);

// exec.c
int             exec(const char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);

// fs.c
int             namecmp(const char*, const char*);
struct inode*   dirlookup(struct inode*, char*);
struct inode*   ialloc(u32, short);
struct inode*   namei(const char*);
void            iput(struct inode*);
struct inode*   iget(u32 dev, u32 inum);
void            ilock(struct inode*, int writer);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
void            iunlock(struct inode*);
int             readi(struct inode*, char*, u32, u32);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, u32, u32);
struct inode*   idup(struct inode*);
struct inode*   nameiparent(const char*, char*);
int             dirlink(struct inode*, const char*, u32);
void            dir_init(struct inode *dp);
void	        dir_flush(struct inode *dp);

// hz.c
void            microdelay(u64);
u64             nsectime(void);
void            inithz(void);

// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// ioapic.c
void            ioapicenable(int irq, int cpu);

// kalloc.c
char*           kalloc(void);
void            kfree(void*);
void*           ksalloc(int slabtype);
void            ksfree(int slabtype, void*);
void*           kmalloc(u64 nbytes);
void            kmfree(void*, u64 nbytes);
int             kmalign(void **p, int align, u64 size);
void            kmalignfree(void *, int align, u64 size);
void            verifyfree(char *ptr, u64 nbytes);
void            kminit(void);
void            kmemprint(void);

// kbd.c
void            kbdintr(void);

// lapic.c
int             cpunum(void);
void            lapicstartap(u8, u32 addr);
void            lapiceoi(void);
void            lapic_tlbflush(u32);
void            lapic_sampconf(u32);
void            lapicpc(char mask);

// mp.c
extern int      ncpu;
int             mpbcpu(void);

// net.c
void            netfree(void *va);
void*           netalloc(void);
void            netrx(void *va, u16 len);
int             nettx(void *va, u16 len);
void            nethwaddr(u8 *hwaddr);

// picirq.c
void            picenable(int);
void            piceoi(void);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, char*, int);
int             pipewrite(struct pipe*, char*, int);

// proc.c
void            addrun(struct proc *);
struct proc*    copyproc(struct proc*);
void            exit(void);
int             fork(int);
int             growproc(int);
int             kill(int);
void            pinit(void);
void            procdumpall(void);
void            scheduler(void) __noret__;
void            sched(void);
void            userinit(void);
int             wait(void);
void            yield(void);
struct proc*    threadalloc(void (*fn)(void*), void *arg);
void            threadpin(void (*fn)(void*), void *arg, const char *name, int cpu);

// prof.c
extern int profenable;
void            profreset(void);
void            profdump(void);

// sampler.c
void            sampstart(void);
int             sampintr(struct trapframe*);
void            sampdump(void);
void            sampconf(void);

// spinlock.c
void            acquire(struct spinlock*);
int             tryacquire(struct spinlock*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, const char*, int);
void            destroylock(struct spinlock *lk);
void            release(struct spinlock*);

// syscall.c
int             argcheckptr(void *argval, int);
int             argcheckstr(const char*);
int             fetchint64(uptr, u64*);
int             umemcpy(void*, void*, u64);
int             kmemcpy(void*, void*, u64);
u64             syscall(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 num);

// string.c
extern  "C" int  memcmp(const void*, const void*, u32);
void*           memmove(void*, const void*, u32);
extern "C" void* memset(void*, int, u32);
extern "C" void* memcpy(void*, const void *, u32);
char*           safestrcpy(char*, const char*, u32);
extern "C" unsigned int strlen(const char*);
int             strncmp(const char*, const char*, u32);
char*           strncpy(char*, const char*, u32);
int             strcmp(const char *p, const char *q);

// swtch.S
void            swtch(struct context**, struct context*);

// trap.c
extern struct segdesc bootgdt[NSEGS];
void            pushcli(void);
void            popcli(void);
void            getcallerpcs(void*, uptr*, int);

// uart.c
void            uartputc(char c);
void            uartintr(void);

// vm.c
void            switchuvm(struct proc*);
void            switchkvm(void);
int             pagefault(struct vmap *, uptr, u32);

// wq.c
int             wq_trywork(void);
int             wq_push(struct work *w);
void            wq_dump(void);
struct work *   allocwork(void);
void            freework(struct work *w);

// cilk.c
#if CILKENABLE
void            cilk_push(void *rip, u64 arg0, u64 arg1);
void            cilk_start(void);
void            cilk_end(void);
void            cilk_dump(void);
int             cilk_trywork(void);
void            initcilkframe(struct cilkframe *wq);
#else
#define cilk_push(rip, arg0, arg1) do { \
  void (*fn)(uptr, uptr) = rip; \
  fn(arg0, arg1); \
} while(0)
#define cilk_start() do { } while(0)
#define cilk_end() do { } while(0)
#define cilk_dump() do { } while(0)
#define cilk_trywork() 0
#define initcilkframe(x) do { } while (0)
#endif

// various init functions
void initpic(void);
void initioapic(void);
void inituart(void);
void initcga(void);
void initconsole(void);
void initpg(void);
void initmp(void);
void initlapic(void);
void inittls(void);
void initnmi(void);
void inittrap(void);
void initseg(void);
void initkalloc(u64 mbaddr);
void initrcu(void);
void initproc(void);
void initbio(void);
void initinode(void);
void initdisk(void);
void inituser(void);
void initcilk(void);
void initsamp(void);
void initpci(void);
void initnet(void);
void initsched(void);
void initlockstat(void);
void initwq(void);

// other exported/imported functions
void cmain(u64 mbmagic, u64 mbaddr);
void mpboot(void);
void trapret(void);
void threadstub(void);
void threadhelper(void (*fn)(void *), void *arg);
void trap(struct trapframe *tf);
