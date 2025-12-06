# Sample test runs for the hybrid scheduler

The sample test runs shown below are meant to illustrate how to test the following scenarios. 

1. Tickets are initialised and updated correctly. Updates to tickets should only apply to runnable processes. 

2. Fairness of the scheduler. With a fixed number of processes, we should see the runtime to converge to roughly equal proportion. 

3. The response times (i.e., the `delay` column) are within a fixed bound (e.g., +/- 5 ticks). This can be tested by adding batches of long running processes, one batch at a time and then observing the response time. A correct implementation should show only small variations of response times. 

4. Short running jobs are not starved and can finish quickly. This will be tested by first adding a large number long running processes, then add shorter jobs, and observer whether the shorter jobs get more runtime in comparison. 


## Test 1: tickets intialisation and updates

This test consists of fist creating 5 worker processes using `schedtest` and running `pstat` at fixed intervals to observe the changes of the tickets assigned. 

The ticket updates should affect only runnable processes, so processes with a 'sleep' (S) status should not have its ticket number reduced. 


```
Booting from Hard Disk..xv6...
cpu0: starting 0
This kernel is running the hybrid scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedtest 5 1000
$ PIDs created:  5  6  7  8  9 

$ pstat
PID     State   Tickets Runtime Delay
1       S       100     29      2
2       S       100     17      0
5       RE      100     79      1
4       S       100     7       0
6       RE      100     86      1
7       RE      100     71      1
8       RE      100     84      1
9       RE      100     68      1
11      RN      100     9       0
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     29      2
2       S       100     19      0
5       RE      90      226     1
4       S       100     7       0
6       RE      90      236     1
7       RE      90      215     1
8       RE      90      234     1
9       RE      90      217     1
12      RN      100     10      0
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     29      2
2       S       100     21      0
5       RE      85      392     1
4       S       100     7       0
6       RE      80      412     1
7       RE      80      403     1
8       RE      80      413     1
9       RE      85      371     1
13      RN      100     11      0
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     29      2
2       S       100     23      0
5       RE      75      555     1
4       S       100     7       0
6       RE      75      541     1
7       RE      75      541     1
8       RE      75      555     1
9       RE      75      529     1
14      RN      100     11      0
```


# Test 2: fairness test

This test consists of running a slightly larger number of workers (compared to the previous test) with a much larger workload, and observe the accumulated runtime. As the tickets approach MIN_TICKETS, we should see the runtime among the runnable processes converge to similar numbers. 

```
Booting from Hard Disk..xv6...
cpu0: starting 0
This kernel is running the hybrid scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedtest 10 10000
$ PIDs created:  5  6  7  8  9  10  11  12  13  14 
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      1
2       S       100     13      0
7       RE      100     73      1
4       S       100     12      1
5       RE      100     82      1
6       RE      100     70      1
8       RE      100     79      1
9       RE      100     64      1
10      RE      100     61      1
11      RE      100     62      1
12      RE      100     65      1
13      RE      100     59      1
14      RE      100     60      1
15      RN      100     10      0
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      1
2       S       100     17      0
7       RE      80      488     1
4       S       100     12      1
5       RE      80      492     1
6       RE      80      485     1
8       RE      75      517     1
9       RE      80      431     1
10      RE      80      472     1
11      RE      75      514     1
12      RE      80      473     1
13      RE      80      463     1
14      RE      80      478     1
17      RN      100     12      0
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      1
2       S       100     21      0
7       RE      50      1082    1
4       S       100     12      1
5       RE      50      1057    1
6       RE      50      1010    1
8       RE      45      1120    1
9       RE      50      1019    1
10      RE      50      1047    1
11      RE      50      1097    1
12      RE      50      1088    1
13      RE      50      1052    1
14      RE      50      1053    1
19      RN      100     11      0
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      1
2       S       100     27      0
7       RE      10      1942    1
4       S       100     12      1
5       RE      10      1931    1
6       RE      10      1908    1
8       RE      10      1936    1
9       RE      10      1902    1
10      RE      10      1936    1
11      RE      10      1941    1
12      RE      10      1896    1
13      RE      10      1916    1
14      RE      10      1929    1
22      RN      100     11      0
```


## Test 3: response times

This will test whether newly created processes are given the first priority to run. A correct implementation should show very similar response times for new processes regardless of how many processes are in the queue  (assuming we have not maxed out the process table `ptable`). 

- First batch:

```
Booting from Hard Disk..xv6...
cpu0: starting 0
This kernel is running the hybrid scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedtest 5 10000
PIDs created:  5  6  7  8  9 
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     12      0
10      RN      100     7       0
4       S       100     6       1
5       RE      100     75      1
6       RE      100     79      1
7       RE      100     62      1
8       RE      100     65      1
9       RE      100     58      1
```

- Second batch (after waiting for a while for the first batch tickets to degrade)

```
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     13      0
11      RN      100     9       0
4       S       100     6       1
5       RE      65      735     1
6       RE      65      701     1
7       RE      65      701     1
8       RE      65      752     1
9       RE      65      740     1
$ schedtest 5 10000
$ PIDs created:  14  15  16  17  18 

$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     18      0
20      RN      100     10      0
4       S       100     6       1
5       RE      55      917     1
6       RE      60      859     1
7       RE      60      860     1
8       RE      55      951     1
9       RE      55      935     1
13      S       100     6       1
14      RE      100     73      1
15      RE      100     73      2
16      RE      100     82      1
17      RE      100     85      1
18      RE      100     82      2
```

- Third batch: arriving soon after the second batch. If your implementation is correct, the new batch should still outcompete the existing batches, even the ones that have not been degraded much.  

```
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     19      0
21      RN      100     10      0
4       S       100     6       1
5       RE      50      1081    1
6       RE      50      1027    1
7       RE      50      1019    1
8       RE      50      1096    1
9       RE      50      1095    1
13      S       100     6       1
14      RE      85      330     1
15      RE      85      352     2
16      RE      85      347     1
17      RE      85      390     1
18      RE      85      356     2
$ schedtest 5 10000
$ PIDs created:  24  25  26  27  28 

$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     25      0
26      RE      95      118     1
4       S       100     6       1
5       RE      40      1212    1
6       RE      45      1183    1
7       RE      45      1166    1
8       RE      40      1241    1
9       RE      40      1237    1
13      S       100     6       1
14      RE      70      602     1
15      RE      75      575     2
16      RE      75      595     1
17      RE      70      618     1
18      RE      75      585     2
23      S       100     7       0
24      RE      100     80      1
25      RE      95      108     1
27      RE      95      104     1
28      RE      100     87      1
30      RN      100     10      0
```

- Fourth batch: basically a repeat of the third batch, but we are now close to maxing out the process table. Still the response time should be very fast. 

```
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     27      0
26      RE      90      289     1
4       S       100     6       1
5       RE      40      1295    1
6       RE      40      1269    1
7       RE      40      1235    1
8       RE      35      1318    1
9       RE      35      1330    1
13      S       100     6       1
14      RE      65      751     1
15      RE      70      696     2
16      RE      65      752     1
17      RE      65      758     1
18      RE      65      728     2
23      S       100     7       0
24      RE      90      276     1
25      RE      90      283     1
27      RE      90      283     1
28      RE      90      277     1
31      RN      100     10      0
$ schedtest 5 10000
$ PIDs created:  34  35  36  37  38 

$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     32      0
26      RE      80      427     1
4       S       100     6       1
5       RE      35      1355    1
6       RE      35      1321    1
7       RE      35      1305    1
8       RE      35      1386    1
9       RE      35      1389    1
13      S       100     6       1
14      RE      60      849     1
15      RE      60      802     2
16      RE      60      855     1
17      RE      60      875     1
18      RE      60      841     2
23      S       100     7       0
24      RE      80      423     1
25      RE      80      435     1
27      RE      80      422     1
28      RE      80      447     1
40      RN      100     10      0
33      S       100     6       1
34      RE      100     41      1
35      RE      100     62      1
36      RE      100     57      2
37      RE      100     50      1
38      RE      100     49      1
```

# Test 4: prioritising shorter jobs

This will test the preference of the scheduler towards shorter jobs. We load the system with 10 large workloads initially. Then we add shorter jobs sparingly. Jobs with very small workloads should finish quickly (before we even have a chance to run `pstat`). Slightly heavier jobs, but still significantly lighter than the initial 10 jobs, should see their runtime accumulate faster than the long running jobs.

- Initial 10 large jobs: 

```
Booting from Hard Disk...
xv6...
cpu0: starting 0
This kernel is running the hybrid scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bma8
init: starting sh
$ schedtest 10 10000
$ PIDs created:  5  6  7  8  9  10  11  12  13  14 

$ pstat
PID     State   Tickets Runtime Delay
1       S       100     27      2
2       S       100     14      1
7       RE      100     77      1
4       S       100     12      1
5       RE      100     100     1
6       RE      100     65      1
8       RE      100     69      1
9       RE      100     74      1
10      RE      100     76      1
11      RE      100     68      1
12      RE      100     88      1
13      RE      100     72      1
14      RE      100     71      1
16      RN      100     8       0
```

- After the initial 10 jobs decrease in priority, we add some short running jobs. We should see these shorter jobs complete quickly. 

```
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     27      2
2       S       100     15      1
7       RE      70      634     1
4       S       100     12      1
5       RE      70      629     1
6       RE      70      635     1
8       RE      70      643     1
9       RE      70      621     1
10      RE      70      635     1
11      RE      70      625     1
12      RE      70      626     1
13      RE      70      630     1
14      RE      70      628     1
17      RN      100     10      0
$ schedtest 1 200
$ PIDs created:  20 

Process 20 completed
zombie!

$ schedtest 1 300
PIDs created:  24 
$ 
Process 24 completed
zombie!
```

- Now add a medium job: 

```
$ schedtest 1 1000
PIDs created:  28 
$ 
$ pstat
PID     State   Tickets Runtime Delay
1       S       100     31      2
2       S       100     29      1
7       RE      50      1057    1
4       S       100     12      1
5       RE      50      1055    1
6       RE      50      1074    1
8       RE      50      1050    1
9       RE      50      1046    1
10      RE      50      1053    1
11      RE      50      1047    1
12      RE      50      1052    1
13      RE      50      1075    1
14      RE      50      1043    1
30      RN      100     10      0
27      S       100     3       0
28      RE      100     64      1
```

- Run a few pstat's and observe the relative speed of runtime accumulation. You will see that the lighter job (process id 28) accumulates more runtime than the larger jobs, e.g., process id 14. (Irrelevant entries removed here)

```
$ pstat
PID     State   Tickets Runtime Delay
14      RE      45      1113    1
28      RE      95      188     1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      45      1182    1
28      RE      85      350     1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      40      1284    1
28      RE      75      546     1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      35      1353    1
28      RE      65      710     1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      30      1421    1
28      RE      60      859     1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      30      1474    1
28      RE      55      952     1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      25      1511    1
28      RE      50      1020    1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      25      1587    1
28      RE      45      1180    1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      20      1640    1
28      RE      40      1286    1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      20      1680    1
28      RE      35      1390    1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      15      1756    1
28      RE      25      1517    1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      10      1806    1
28      RE      25      1600    1

$ pstat
PID     State   Tickets Runtime Delay
14      RE      10      1857    1
28      RE      15      1716    1
$ 
```

