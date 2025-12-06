// ANU COMP3300/COMP63300 Operating Systems Implementation
// Test programs for Coding Assignment 3
// (c) Alwen Tiu, 2024-2025

#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"
#include "pageinfo.h"

#define NULL 0
#define MMAPBASE 0x60000000
#define KERNBASE 0x80000000
#define RESERVED 0x7CAFE000
#define UNMAPPED 0x7B0BA000

char __attribute__ ((aligned (4096))) data[2*4096]; 

void test1()
{
    printf(1, "Test 1:\n"); 
    char * s = NULL;
    s = mmap((void*)MMAPBASE, 4096, 1); 
    strcpy(s, "123456"); 
    if(strcmp(s, "123456") == 0)
        printf(1, "OK\n"); 
    else printf(1, "**fail**\n"); 
}

void test2()
{
    printf(1, "Test 2:\n"); 
    char * s;
    s = mmap(NULL, 4096, 1); 
    if((int)s == -1) goto fail;
    if((int)s < MMAPBASE || (int)s >= KERNBASE) goto fail; 
    
    printf(1, "OK\n"); 
    return;
fail:
    printf(1, "**fail**\n"); 
}

void test3()
{
    printf(1, "Test 3:\n"); 
    char * s = NULL;
    
    s = mmap((void*)MMAPBASE, 4096, 1); 
    
    if((int)s == -1) goto fail; 

    s = mmap((void*)MMAPBASE, -4096, 1); 
    
    if((int)s != -1) goto fail; 
    
    printf(1, "OK\n"); 
    return;
fail:
    printf(1, "**fail**\n"); 

}

void test4()
{
    printf(1, "Test 4:\n"); 
    void * s = (void *) RESERVED;
    s = mmap(s, 4096, 1); 
    if((int)s != RESERVED)
    {
        printf(1, "**fail**\n");
        return; 
    }
    s = mmap((void *) KERNBASE, 4096, 1); 
    if((int)s != -1)
    {
        printf(1, "**fail**\n");
        return; 
    }
    printf(1, "mmap returns %p\n", s); 
    printf(1, "OK\n"); 
}


void test5()
{
    printf(1, "Test 5:\n"); 
    int i; 
    char* s;
    s = mmap(NULL, 4096, 1); 
    if((int)s == -1) 
        goto fail;
    for(i=0; i < 4096; ++i) 
        if(s[i] != 0) goto fail; 
    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");
}


void test6()
{
    printf(1, "Test 6:\n"); 
    char * s = NULL;
    s = mmap(NULL, 4096, 0); 
    struct pageinfo_t pi; 

    if((int) s == -1) goto fail; 

    if(pageinfo((void *)s, &pi) != 0)
    {
        printf(1, "pageinfo syscall failed!\n");
        exit(); 
    }
    
    if(pi.mapped && (pi.flags & PTE_W)) goto fail;
    
    printf(1, "OK\n"); 
    return; 

fail:
    printf(1, "**fail**\n");    
}

void test7()
{
    printf(1, "Test 7:\n"); 
    int * s = NULL;
    int i;

    s = mmap((void*)MMAPBASE, 10*4096, 1);
    if((int)s == -1) goto fail; 

    for(i=0; i < 1024; ++i)
    {
        s[i] = i+1; 
    }
    for(i=0; i < 1024; ++i)
    {
        if(s[i] != i+1) goto fail; 
    }
    printf(1, "OK\n");
    return; 
fail:
    printf(1, "**fail**\n"); 
}

void test8()
{
    int pid;

    printf(1, "Test 8:\n"); 
    pid=fork();
    if(pid == 0)
    {
        char *s1, *s2;

        s1 = (char *)mmap(NULL, 3*4096, 1);
        s2 = (char *)mmap(NULL, 4096, 1); 
        if(s2 == s1 + 3*4096) 
            printf(1, "OK\n");
        else printf(1, "**fail**\n");
        exit(); 
    }
    else if(pid < 0) {
        printf(1, "error\n");
        exit(); 
    }
    wait(); 
}


void test9()
{
    int pid;

    printf(1, "Test 9:\n"); 
    pid=fork();
    if(pid == 0)
    {
        char *s1, *s2, *s3, *s4;
        //  [ x ][ x ][ x ][   ][   ][ x ][   ][   ][   ][ x ]
        // s1                       s2                  s3
        s1 = (char *)mmap((void*)MMAPBASE, 3*4096, 1);
        s2 = (char *)mmap(s1+5*4096, 4096, 1);
        s3 = (char *)mmap(s1+9*4096, 4096, 1); 
        s4 = (char *)mmap(NULL, 3*4096, 1); 

        printf(1, "s1 = %p, s2 = %p, s3 = %p, s4 = %p\n", s1, s2, s3, s4); 
        if(s4 == s2 + 4096) 
            printf(1, "OK\n");
        else printf(1, "**fail**\n");
        exit(); 
    }
    else if(pid < 0) {
        printf(1, "error\n");
        exit(); 
    }
    wait(); 
}

void test10()
{
    printf(1, "Test 10:\n"); 
    int * s = NULL;
    int i;

    s = mmap(NULL, 1024*4096, 1);
    if((int)s == -1) goto fail; 

    for(i=0; i < 1024*1024; ++i)
    {
        s[i] = i+1; 
    }
    for(i=0; i < 1024*1024; ++i)
    {
        if(s[i] != i+1) goto fail; 
    }
    printf(1, "OK\n");
    return; 
fail:
    printf(1, "**fail**\n"); 
}

void test11()
{
    printf(1, "Test 11:\n");
    char *s; 
    struct pageinfo_t pi; 

    s = mmap(NULL, 4096, 1); 
    if(pageinfo(s, &pi) == -1) 
    {
        printf(1, "pageinfo error\n");
        exit();
    }; 

    if(!pi.mapped){
        goto fail;
    }

    printf(1, "s: %x\n", s);
    if(munmap(s, 4096) == -1) {
        goto fail;
    }
    if(pageinfo(s, &pi) == -1) 
    {
        printf(1, "pageinfo error\n");
        exit();
    }; 

    if(pi.mapped) {
        goto fail; 
    }

    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");
}

void test12()
{
    printf(1, "Test 12:\n");
    char *s = mmap(NULL, 4096, 1);
    if(munmap(s, 4096) == -1) goto fail; 

    if(munmap((void*)0x80100000, 4096) != -1) goto fail; 

    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");
}

void test13()
{
    printf(1, "Test 13:\n");
    char *s1 = mmap(NULL, 4096, 1); 
    char *s2 = mmap(NULL, 4096, 1); 
    if(munmap(s1, 4096) == -1) goto fail; 
    if(munmap(s2+1, 4096) != -1) goto fail; 
    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");

}

void test14()
{
    printf(1, "Test 14:\n");
    char *s; 
    struct pageinfo_t pi; 

    s = mmap(NULL, 10*4096, 1); 
    if(pageinfo(s, &pi) == -1) 
    {
        printf(1, "pageinfo error\n");
        exit();
    }; 

    if(!pi.mapped) goto fail;

    if(munmap(s, 3*4096) == -1) goto fail;

    int i; 
    for(i=0; i < 3; ++i)
    {
        if(pageinfo(s, &pi) == -1) 
        {
            printf(1, "pageinfo error\n");
            exit();
        }; 
        if(pi.mapped) goto fail; 
        s = s + 4096;
    }
       
    for(i=3; i < 10; ++i)
    {
        if(pageinfo(s, &pi) == -1) 
        {
            printf(1, "pageinfo error\n");
            exit();
        }; 
        if(!pi.mapped) goto fail; 
        s = s + 4096;
    }
 
    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");
}

void test15()
{
    printf(1, "Test 15:\n");
    char *s; 
    struct pageinfo_t pi; 

    s = mmap(NULL, 1024*4096, 1); 
    if(pageinfo(s, &pi) == -1) 
    {
        printf(1, "pageinfo error\n");
        exit();
    }; 

    if(!pi.mapped) goto fail;

    if(munmap(s, 512*4096) == -1) goto fail;

    int i; 
    for(i=0; i < 512; ++i)
    {
        if(pageinfo(s, &pi) == -1) 
        {
            printf(1, "pageinfo error\n");
            exit();
        }; 
        if(pi.mapped) goto fail; 
        s = s + 4096;
    }
       
    for(i=512; i < 1024; ++i)
    {
        if(pageinfo(s, &pi) == -1) 
        {
            printf(1, "pageinfo error\n");
            exit();
        }; 
        if(!pi.mapped) goto fail; 
        s = s + 4096;
    }
 
    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");
}

void test16()
{
    printf(1, "Test 16:\n");

    struct pageinfo_t pi; 

    if(munmap(data, 2*4096) == -1) goto fail; 
    pageinfo(data, &pi);
    
    if(pi.mapped) goto fail; 

    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");

}

void test17()
{
    printf(1, "Test 17:\n");

    char *s = mmap(NULL, 5 * 4096, 1);
    char *t; 
    struct pageinfo_t pi; 

    if((int)s == -1) goto fail; 

    t = s + 4096; 
    munmap(t, 4096);

    pageinfo(t, &pi);
    if(pi.mapped) goto fail; 
    pageinfo(t+4096, &pi); 
    if(!pi.mapped) goto fail; 

    munmap(s, 4*4096); 
    pageinfo(t+4096, &pi); 
    if(pi.mapped) goto fail; 

    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");

}

void test18()
{
    printf(1, "Test 18:\n");

    char *s = mmap(NULL, 4096, 1); 
    struct pageinfo_t pi; 
    int pa; 

    pageinfo(s, &pi);
    if(!pi.mapped) goto fail; 

    if(pi.va != (int)s) goto fail; 

    printf(1, "va:%p -> pa:%p\n", pi.va, pi.pa); 
    pa = pi.pa; 

    if(munmap(s, 4096) == -1) goto fail;
    pageinfo(s, &pi);

    if(pi.mapped) goto fail; 

    s = mmap(s+10*4096, 4096, 1);

    pageinfo(s, &pi);

    if(!pi.mapped) goto fail; 
    printf(1, "va:%p -> pa:%p\n", pi.va, pi.pa); 
    if(pi.pa != pa) goto fail;

    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");
}

int member(int pa, struct pageinfo_t pi[], int n)
{
    int i;
    for(i=0; i < n; ++i)
    {
        if(pa == pi[i].pa) return 1;
    }
    return 0;
}

void test19()
{
    printf(1, "Test 19:\n");
    
    char *s = mmap(NULL, 10*4096, 1); 
    struct pageinfo_t pi1[10];
    struct pageinfo_t pi2[10]; 
    int i; 

    for(i=0; i < 10; ++i) {
        pageinfo(s+i*4096, &pi1[i]);
        if(!pi1[i].mapped) goto fail; 
    }

    if(munmap(s, 10*4096) == -1) goto fail;

    s = mmap(s+20*4096, 10*4096, 1);

    for(i=0; i < 10; ++i) {
        pageinfo(s+i*4096, &pi2[i]);
        if(!pi2[i].mapped) goto fail; 
        if(!member(pi2[i].pa, pi1, 10)) goto fail; 
    }

    printf(1, "OK\n"); 
    return; 
fail:
    printf(1, "**fail**\n");

}


void test20()
{
    char *s; 
    printf(1, "Test 20: \n"); 
    int i; 
    for(i=1; i <= 10000; ++i) {
        if(i%10 == 0) printf(1, ".");
        if(i%800 == 0) printf(1, "\n"); 
        s = mmap(NULL, 1024 * 4096, 1); 
        if((int)s == -1) {
            printf(1, "**fail**\n"); 
            return; 
        }
        if(munmap(s, 1024 * 4096) == -1)
        {
            printf(1, "**fail**\n");
            return;
        } 
    }
    printf(1, "\nOK\n");
}

static void (*tests[])(void) = {
0,
test1,
test2,
test3,
test4,
test5,
test6,
test7,
test8,
test9,
test10,
test11,
test12,
test13,
test14,
test15,
test16,
test17,
test18,
test19,
test20, 
};

int
main(int argc, char *argv[])
{
    int i,j; 

    if(argc < 2)
    {
        printf(1, "Running all tests...\n");
        for(i=1; i <= 20; ++i) 
            (*tests[i])();
        exit();
    }    

    for(i=1; i < argc; ++i)
    {
        j = atoi(argv[i]); 
        if(j >= 1 && j <= 20) {
            (*tests[j])(); 
        }
        else {
            printf(1, "invalid test\n"); 
            exit();
        }
    }
    exit(); 
}
