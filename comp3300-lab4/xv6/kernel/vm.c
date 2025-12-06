#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "pageinfo.h"

extern char data[];  // defined by kernel.ld
pde_t *kpgdir;  // for use in scheduler()

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
  lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint)va);
  last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// There is one page table per process, plus one that's used when
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
// page protection bits prevent user code from using the kernel's
// mappings.
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap {
  void *virt;
  uint phys_start;
  uint phys_end;
  int perm;
} kmap[] = {
 { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
 { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
 { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, // kern data+memory
 { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more devices
};

// Set up kernel part of a page table.
pde_t*
setupkvm(void)
{
  pde_t *pgdir;
  struct kmap *k;

  if((pgdir = (pde_t*)kalloc()) == 0)
    return 0;
  memset(pgdir, 0, PGSIZE);
  if (P2V(PHYSTOP) > (void*)DEVSPACE)
    panic("PHYSTOP too high");
  for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
    if(mappages(pgdir, k->virt, k->phys_end - k->phys_start,
                (uint)k->phys_start, k->perm) < 0) {
      freevm(pgdir);
      return 0;
    }
  return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void
kvmalloc(void)
{
  kpgdir = setupkvm();
  switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm(void)
{
  lcr3(V2P(kpgdir));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void
switchuvm(struct proc *p)
{
  if(p == 0)
    panic("switchuvm: no process");
  if(p->kstack == 0)
    panic("switchuvm: no kstack");
  if(p->pgdir == 0)
    panic("switchuvm: no pgdir");

  pushcli();
  mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
                                sizeof(mycpu()->ts)-1, 0);
  mycpu()->gdt[SEG_TSS].s = 0;
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  mycpu()->ts.iomb = (ushort) 0xFFFF;
  ltr(SEG_TSS << 3);
  lcr3(V2P(p->pgdir));  // switch to process's address space
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem;

  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
  memmove(mem, init, sz);
}


// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pte_t *pte;

  if((uint) addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi(ip, P2V(pa), offset+i, n) != n)
      return -1;
  }
  return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;

  if(newsz >= KERNBASE)
    return 0;
  if(newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for(; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
      cprintf("allocuvm out of memory (2)\n");
      deallocuvm(pgdir, newsz, oldsz);
      kfree(mem);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  pte_t *pte;
  uint a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for(; a  < oldsz; a += PGSIZE){
    pte = walkpgdir(pgdir, (char*)a, 0);
    if(!pte)
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if((*pte & PTE_P) != 0){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
  uint i;

  if(pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, KERNBASE, 0);
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P){
      char * v = P2V(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void
clearpteu(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if(pte == 0)
    panic("clearpteu");
  *pte &= ~PTE_U;
}


// Given a parent process's page table, create a copy
// of it for a child.


pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i, flags;
  char *mem;

  if((d = setupkvm()) == 0)
    return 0;
  
  uint start = 0;
  // Set mem lower bound to USERBASE, except for initcode
  if (sz > USERBASE){
    start = USERBASE;
  }

  for(i = start; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if(!(*pte & PTE_P))
      panic("copyuvm: page not present");
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto bad;
    memmove(mem, (char*)P2V(pa), PGSIZE);
    if(mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
      kfree(mem);
      goto bad;
    }
  }
  return d;

bad:
  freevm(d);
  return 0;
}


//PAGEBREAK!
// Map user virtual address to kernel address.
char*
uva2ka(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if((*pte & PTE_P) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  return (char*)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
  char *buf, *pa0;
  uint n, va0;

  buf = (char*)p;
  while(len > 0){
    va0 = (uint)PGROUNDDOWN(va);
    pa0 = uva2ka(pgdir, (char*)va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (va - va0);
    if(n > len)
      n = len;
    memmove(pa0 + (va - va0), buf, n);
    len -= n;
    buf += n;
    va = va0 + PGSIZE;
  }
  return 0;
}

// A3 --

int mprotect(void* addr, int len)
{
  // There are some failure cases to consider: if addr is not page aligned, 
  // or addr points to a region that is not currently a part of the address space, 
  // or len is less than or equal to zero, return -1 and do not change anything. Otherwise, return 0 upon success.
  if(len <= 0 || (uint)addr % PGSIZE != 0){
    return -1;
  }

  // only mprotect addr in [USERBASE, KERNBASE)
  uint addr_num = (uint) addr;
  if(addr_num >= KERNBASE || addr_num < USERBASE){
    return -1;
  }
  
  
  // Get the page directory of the current process
  pde_t *pgdir = myproc()->pgdir;


  // Check all address in the region is mapped
  for(uint a = (uint)addr; a < (uint)addr + len; a += PGSIZE){
    pte_t *pte = walkpgdir(pgdir, (void*)a, 0);
    if(pte == 0){
      return -1;
    }
    if(!(*pte & PTE_P)){
      return -1;
    }
  }

  // Set PTE_W bits in the region to 0 (read-only)
  for(uint a = (uint)addr; a < (uint)addr + len; a += PGSIZE){
    pte_t *pte = walkpgdir(pgdir, (void*)a, 0);
    // set PTE_W bit to 0
    *pte &= ~PTE_W;
  }

  
  // After changing a page-table entry, you need to update the CR3 register to let
  // the hardware know of the change.  This guarantees that your PTE updates will be
  // used upon subsequent accesses. You  can use lcr3() (see kernel/vm.c for an 
  // example use of this function) to update CR3 register.
  lcr3(V2P(pgdir));  
  return 0;

}

int
munprotect(void *addr, int len)
{
  // There are some failure cases to consider: if addr is not page aligned, 
  // or addr points to a region that is not currently a part of the address space, 
  // or len is less than or equal to zero, return -1 and do not change anything. Otherwise, return 0 upon success.

  if(len <= 0 || (uint)addr % PGSIZE != 0){
    return -1;
  }

  // only munprotect addr in [USERBASE, KERNBASE)
  uint addr_num = (uint) addr;
  if(addr_num >= KERNBASE || addr_num < USERBASE){
    return -1;
  }
  
  
  // Get the page directory of the current process
  pde_t *pgdir = myproc()->pgdir;


  // Check all address in the region is mapped
  for(uint a = (uint)addr; a < (uint)addr + len; a += PGSIZE){
    pte_t *pte = walkpgdir(pgdir, (void*)a, 0);
    if(pte == 0){
      return -1;
    }
    if(!(*pte & PTE_P)){
      return -1;
    }
  }

  // Set PTE_W bits in the region to 1(writebale)
  for(uint a = (uint)addr; a < (uint)addr + len; a += PGSIZE){
    pte_t *pte = walkpgdir(pgdir, (void*)a, 0);
    // set PTE_W bit to 1
    *pte |= PTE_W;
  }

  
  // After changing a page-table entry, you need to update the CR3 register to let
  // the hardware know of the change.  This guarantees that your PTE updates will be
  // used upon subsequent accesses. You  can use lcr3() (see kernel/vm.c for an 
  // example use of this function) to update CR3 register.
  lcr3(V2P(pgdir));  
  return 0;
}

void *mmap(void *addr, int len, int prot)
{
  // prot must be 0 or 1
  //cprintf("mmap start\n");
  if(prot != 0 && prot != 1)
    return (void*)-1;
  
  // set the permission bits for the new mapping
  uint perm = PTE_U | (prot == 1 ? PTE_W : 0);
  
  // len must be > 0
  if(len <= 0){
    //cprintf("3\n");
    return (void*)-1;
  }

  // if addr is not NULL, it must be in [MMAPBASE, KERNBASE)]
  if(addr != 0 && ((uint)addr < MMAPBASE || (uint)addr >= KERNBASE)){
    //cprintf("addr: %x\n", (uint)addr);
    //cprintf("4\n");
    return (void*)-1;
  }
  
  // get the page directory of the current process
  pde_t *pgdir = myproc()->pgdir;

  // how many pages are needed, round up
  int npages = (len + PGSIZE - 1) / PGSIZE;

  // find the start search position, PGDOWN := addr with lower 12 bits cleared
  uint start = addr == 0 ? MMAPBASE : PGROUNDUP((uint)addr);
  //cprintf("start: %d\n", start);

  // search [start, KERNBASE - npages * PGSIZE] to find a free region
  
  // whether we found a region
  int found_npages = 0;
  // stores the start address of the region
  uint va;
  for(va = start; va + npages * PGSIZE <= KERNBASE; va += PGSIZE){

    // Determin whether there is contiguous npages free pages from va onwards
    // search it by checking each page whether it is mapped
    int i;
    for(i = 0; i < npages; i++){
      // get the PTE for the page
      pte_t *pte = walkpgdir(pgdir, (void *)(va + i * PGSIZE), 0);
      if(pte && (*pte & PTE_P)){
        // this page is already mapped, break and go to next iteration of outer loop
        break;
      }
    }

    if(i == npages){
      // we found a region!
      found_npages = 1;
      break;
    }
  }
  // cprintf("va: %x\n", (void *)va);

  // return -1 if cannot find a region
  if(found_npages == 0){
    // we did not find a region
    //cprintf("1\n");
    return (void*)-1;
  }

  // By here, we found a region starting at 'va'
  int i;
  // allocate physical pages for all npages
  for(i = 0; i < npages; i++){
    // allocate a physical page with kalloc()
    char *mem = kalloc();

    // if any of the pages within the range fail to be 
    // allocated, mmap should fail (return -1), and all 
    // the pages allocated for this request so far must 
    // be freed
    if(mem == 0){
      for(int k = 0; k < i; k++){
        pte_t *pte = walkpgdir(pgdir, (void *)(va + k * PGSIZE), 0);
        if(pte && (*pte & PTE_P)){
          uint pa = PTE_ADDR(*pte);
          char *v = P2V(pa);
          kfree(v);
        }
      }
      //cprintf("2\n");
      return (void*)-1;
    }

    // link the allocated physical page to the virtual address
    memset(mem, 0, PGSIZE);

    // map the PA to VA TODO
    mappages(pgdir, (void *)(va + i * PGSIZE), PGSIZE, V2P(mem), perm);

  }

  return (void *)va;
}

// assume all pages in [addr, addr + len] are mapped???
int munmap(void *addr, int len)
{
  // len must be > 0
  if(len <= 0){
    cprintf("5\n");
    return -1;
  }

  // if addr is not NULL, it must be in [MMAPBASE, KERNBASE)]
  if((uint)addr >= KERNBASE){
    cprintf("4\n");
    return -1;
  }
  
  // get the page directory of the current process
  pde_t *pgdir = myproc()->pgdir;

  // address must be page aligned
  uint start = (uint)addr;
  if(start % PGSIZE != 0){
    cprintf("start: %d\n", start);
    cprintf("3\n");
    return -1;
  }
  
  uint end = PGROUNDUP(start + len);
  for(uint va = start; va < end; va += PGSIZE){
    pte_t *pte = walkpgdir(pgdir, (void *)va, 0);
    
    if(pte != 0 && (*pte & PTE_P)){
      // get physical page
      char *pa = P2V(PTE_ADDR(*pte));
      kfree(pa);

      // should we set *pte = 0???
      *pte = 0;
    }
  }

  // user lcr3???
  lcr3(V2P(pgdir));
  return 0;
  
}

// DO NOT MODIFY THIS FUNCTION -- it is used for various tests for the assignment
int pageinfo(void *va, char *s)
{
  struct pageinfo_t *pginfo = (struct pageinfo_t *)s;  
  pte_t *pte; 
  pde_t *pgdir = myproc()->pgdir; 

  pginfo->va = (uint)va;
  pte = walkpgdir(pgdir, va, 0); 
  if(!pte || !(*pte & PTE_P)) {
    pginfo->pa = 0;
    pginfo->mapped = 0; 
    return 0; 
  }
  pginfo->pa = (*pte & 0xfffff000) | ((uint)va & 0x00000fff); 
  pginfo->flags = (*pte & 0x00000fff); 
  pginfo->mapped = 1; 
  return 0; 
}

// -- A3 