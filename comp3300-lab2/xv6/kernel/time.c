#include "types.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"


// A1 --

// 平年每月天数
static int month_days[12] = {
  31, 28, 31, 30, 31, 30,
  31, 31, 30, 31, 30, 31
};

// 判断是否闰年
static int is_leap(int year) {
  if((year % 400) == 0) return 1;
  if((year % 100) == 0) return 0;
  if((year % 4) == 0) return 1;
  return 0;
}

extern long xtime; // current time; defined in proc.c

// TODO: complete this function for assignment 1
void time_init(void)
{
    struct rtcdate r;
    cmostime(&r);

    long days_num = 0;
    // how many days from 1970 to r.year - 1
    for(int i = 1970; i < r.year; i++){
        if(is_leap(i)){
            days_num += 366;
        }else{
            days_num += 365;
        }
    }

    // how many days previous months in this year
    for(int m = 1; m < r.month; m++){
        days_num += month_days[m-1];
        
        // if this is leap year Feb, +1
        if(m == 2 && is_leap(r.year)){
            days_num++;
        }
    }


    // days in this month
    days_num += r.day - 1;

    // calculate how many seconds
    long seconds = days_num * 24L * 60L * 60L;
    seconds += r.hour * 3600L;
    seconds += r.minute * 60L;
    seconds += r.second;


    xtime = seconds;
}

// -- A1
