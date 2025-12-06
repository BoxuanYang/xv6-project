// A1 -- 

#include "types.h"
#include "user.h"
#include "procinfo.h"
#include "date.h"

struct procinfo_t proc_info[64];

// 判断闰年
static int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// 每个月的天数（非闰年）
static int month_days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

// epoch -> rtcdate
void epoch_to_rtcdate(long epoch, struct rtcdate *r) {
    long days = epoch / (24L * 3600L);   // total days
    long rem  = epoch % (24L * 3600L);   // seconds in a day

    // determine year
    int year = 1970;
    while (1) {
        int days_in_year = is_leap(year) ? 366 : 365;
        if (days >= days_in_year) {
            days -= days_in_year;
            year++;
        } else {
            break;
        }
    }

    // calculate month
    int month = 1;
    for (int i = 0; i < 12; i++) {
        int dim = month_days[i];
        if (i == 1 && is_leap(year)) dim++; // 2月闰年加一天
        if (days >= dim) {
            days -= dim;
            month++;
        } else {
            break;
        }
    }

    // 剩余的就是 day
    int day = (int)days + 1;

    // 剩余秒数 → 时分秒
    int hour   = rem / 3600;
    rem       %= 3600;
    int minute = rem / 60;
    int second = rem % 60;

    // 填充 rtcdate
    r->year   = year;
    r->month  = month;
    r->day    = day;
    r->hour   = hour;
    r->minute = minute;
    r->second = second;
}

int main(int argc, char *argv[])
{
    // TODO: implement this for assignment 1

    //  Assume there are 200 processes at most
    int n = procinfo(64, proc_info);
    if(n < 0){
        printf(1, "ps: procinfo failed\n");
        exit();
    }

    printf(1, "n: %d\n", n);
    printf(1, "hello world\n");
    printf(1, "PID     State   Name    Started             Elapsed\n");

    for(int i=0; i < n; i++){
        struct procinfo_t *p = &proc_info[i];

        // start time(epoch) -> rtcdate
        struct rtcdate r;
        epoch_to_rtcdate(p->start_time, &r);

        printf(1, "%d\t%s\t%s\t",
            p->pid, p->st, p->name);
        
        

        // print year
        printf(1, "%d-", r.year);
        // print month
        if(r.month < 10){
            printf(1, "0%d-", r.month);
        }else{
            printf(1, "%d", r.month);
        }
        // print day
        if(r.day < 10){
            printf(1, "0%d ", r.day);
        }else{
            printf(1, "%d ", r.day);
        }

        // print hour
        if(r.hour < 10){
            printf(1, "0%d:", r.hour);
        }else{
            printf(1, "%d:", r.hour);
        }

        // print minutes
        if(r.minute < 10){
            printf(1, "0%d:", r.minute);
        }else{
            printf(1, "%d:", r.minute);
        }

        // print seconds
        if(r.second < 10){
            printf(1, "0%d ", r.second);
        }else{
            printf(1, "%d ", r.second);
        }




        long seconds_in_a_day = 24 * 60 * 60;
        struct rtcdate elapsed_r;
        epoch_to_rtcdate(p->elapsed_time, &elapsed_r);

        if(p->elapsed_time < seconds_in_a_day){
            // "hh:mm:ss" format

            // print hour
        if(elapsed_r.hour < 10)
            printf(1, "0%d:", elapsed_r.hour);
        else
            printf(1, "%d:", elapsed_r.hour);
        

        // print minutes
        if(elapsed_r.minute < 10)
            printf(1, "0%d:", elapsed_r.minute);
        else
            printf(1, "%d:", elapsed_r.minute);
        

        // print seconds
        if(elapsed_r.second < 10)
            printf(1, "0%d ", elapsed_r.second);
        else
            printf(1, "%d ", elapsed_r.second);
        

        }else{
            // "d-hh:mm:ss"
            printf(1, "%d-", elapsed_r.day);
            // print hour
        if(elapsed_r.hour < 10)
            printf(1, "0%d:", elapsed_r.hour);
        else
            printf(1, "%d:", elapsed_r.hour);
        

        // print minutes
        if(elapsed_r.minute < 10)
            printf(1, "0%d:", elapsed_r.minute);
        else
            printf(1, "%d:", elapsed_r.minute);
        

        // print seconds
        if(elapsed_r.second < 10)
            printf(1, "0%d ", elapsed_r.second);
        else
            printf(1, "%d ", elapsed_r.second);

        }

        printf(1, "\n");
    }

    exit(); 
}

// -- A1
