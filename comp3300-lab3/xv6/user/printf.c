#include "types.h"
#include "stat.h"
#include "user.h"

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static int
int2str(char buf[], int xx, int base, int sgn)
{
  static char digits[] = "0123456789ABCDEF";
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';
  return i; 
}

static void
printint(int fd, int xx, int base, int sgn)
{
  char buf[16]; 
  int i; 

  i=int2str(buf, xx, base, sgn); 
  while(--i >= 0)
    putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
printf(int fd, const char *fmt, ...)
{
  char *s;
  int c, i, state;
  uint *ap;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        putc(fd, c);
      }
    } else if(state == '%'){
      if(c == 'd'){
        printint(fd, *ap, 10, 1);
        ap++;
      } else if(c == 'x' || c == 'p'){
        printint(fd, *ap, 16, 0);
        ap++;
      } else if(c == 's'){
        s = (char*)*ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          putc(fd, *s);
          s++;
        }
      } else if(c == 'c'){
        putc(fd, *ap);
        ap++;
      } else if(c == '%'){
        putc(fd, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
      }
      state = 0;
    }
  }
}

// sprintf -- a simple clone of printf

char * 
sputc(char *s, int c)
{
  *s = c;
  return ++s; 
}

char *
sprintint(char *s, int xx, int base, int sgn)
{
  char buf[16]; 
  int i; 
  i=int2str(buf, xx, base, sgn);
  while(--i >= 0 && i < 16)
    s = sputc(s, buf[i]);
  return s; 
}


void
sprintf(char *dest, const char *fmt, ...)
{
  char *s;
  int c, i, state;
  uint *ap;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        //putc(fd, c);
        dest = sputc(dest, c); 
      }
    } else if(state == '%'){
      if(c == 'd'){
        dest = sprintint(dest, *ap, 10, 1); 
        ap++;
      } else if(c == 'x' || c == 'p'){
        dest = sprintint(dest, *ap, 16, 0);
        ap++;
      } else if(c == 's'){
        s = (char*)*ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          //putc(fd, *s);
          dest = sputc(dest, *s); 
          s++;
        }
      } else if(c == 'c'){
        dest = sputc(dest, *ap);
        ap++;
      } else if(c == '%'){
        dest = sputc(dest, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        dest = sputc(dest, '%');
        dest = sputc(dest, c);
      }
      state = 0;
    }
  }
}
