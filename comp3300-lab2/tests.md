# Sample outputs from tests

Here are some sample outputs of tests for Task 2 and Task 4. Each test is launched right after the OS finishes booting and the shell is launched. After each test is done, the OS is restarted before the next test.

# tracetest

There are six tests for testing the trace syscall. The binary `tracetest`, when not given any argument, will run all six tests, as exemplified below. You also have the option of running individual test, e.g., `tracetest 1` will run only the first test. See the source code (`./xv6/user/tracetest.c`) for further details of each test. 


```
$ tracetest
Running all tests...

test 1
==================
Description: 
opening and closing an existing file ('echo')
and a non-existing file ('doesnotexist') 
3: open("echo",0) = 3
3: close(3) = 0
3: open("doesnotexist",0) = -1
==================

test 2
==================
Description: 
creating 10 new files, named a0 to a11, with mode=514 (O_CREATE|O_RDWR)
3: open("a0",514) = 3
3: open("a1",514) = 4
3: open("a2",514) = 5
3: open("a3",514) = 6
3: open("a4",514) = 7
3: open("a5",514) = 8
3: open("a6",514) = 9
3: open("a7",514) = 10
3: open("a8",514) = 11
3: open("a9",514) = 12
3: close(3) = 0
3: close(4) = 0
3: close(5) = 0
3: close(6) = 0
3: close(7) = 0
3: close(8) = 0
3: close(9) = 0
3: close(10) = 0
3: close(11) = 0
3: close(12) = 0
==================

test 3
=======================
Description:
Writing two strings (str1, str2) to a file, and read the file to a buffer (buf).
str1 address: 0x11E0
str2 address: 0x11FF
buf  address: 0x3F90
3: open("small",514) = 3
3: write(3, "aaaaaaaaaa", 10) = 10
3: write(3, "bbbbbbbbbb", 10) = 10
3: close(3) = 0
3: open("small",0) = 3
3: read(3, "aaaaaaaaaabbbbbbbbbb", 20) = 20
3: close(3) = 0
=======================

test 4
=======================
Description:
exec an echo command ('echo one two three') in a child process.
3: fork() = 4
4: exec("echo", ...) = 0
=======================

test 5
=======================
Description:
exec a command ('wc README') in a child process,
and tracing open/close/read/write3: fork() = 5
5: close(1) = 0
5: exec("wc", ...) = 0
5: open("README",0) = 1
5: read(1, "NOTE: we have stopped maintaining the x86 version of xv6, and sw"..., 512) = 512
5: read(1, ",\n2000)). See also https://pdos.csail.mit.edu/6.828/, which\npr"..., 512) = 512
5: read(1, "orts and patches contributed by Silas\nBoyd-Wickizer, Anton Burt"..., 512) = 512
5: read(1, "y, tyfkda, Rafael Ubal, Warren\nToomey, Stephen Tu, Pablo Ventur"..., 512) = 512
5: read(1, "n on x86), you\nwill need to install a cross-compiler gcc suite "..., 512) = 238
5: read(1, "n on x86), you\nwill need to install a cross-compiler gcc suite "..., 512) = 0
5: write(1, "5", 1) = -1
5: write(1, "0", 1) = -1
5: write(1, " ", 1) = -1
5: write(1, "3", 1) = -1
5: write(1, "2", 1) = -1
5: write(1, "9", 1) = -1
5: write(1, " ", 1) = -1
5: write(1, "2", 1) = -1
5: write(1, "2", 1) = -1
5: write(1, "8", 1) = -1
5: write(1, "6", 1) = -1
5: write(1, " ", 1) = -1
5: write(1, "R", 1) = -1
5: write(1, "E", 1) = -1
5: write(1, "A", 1) = -1
5: write(1, "D", 1) = -1
5: write(1, "M", 1) = -1
5: write(1, "E", 1) = -1
5: write(1, "\n", 1) = -1
5: close(1) = 0
=======================

test 6
=======================
Description:
exec two commands ('echo first' and 'badexec second') in child processes
3: fork() = 6
3: fork() = 7
exec: fail
6: exec("badexec", ...) = -1
7: exec("echo", ...) = 0
=======================
```

# proctest 

There are six tests for testing the procinfo syscall. Each test should be run separately to see the following output. 

For each test N (N=1..6), run `proctest N` first. This will produce a log of all the fork() calls. Then run the `ps` command to see if you have copied and displayed the relevant data correctly. The following sample outputs show the kind of expected output (keeping in mind that some details such as the process ids, dates and elapsed time may differ in your runs). To ensure each test is independent of others, make sure you power-cycle the VM after each test (using the `poweroff` command you have implemented). 

You can combine these tests with the `settime` command you have implemented, to check whether the date/time formats are correctly implemented.


## Test 1

```
$ proctest 1
Testing procinfo syscall
$ fork:  4 ->  5
fork:  4 ->  6
fork:  4 ->  7

$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:40:48     00:00:10
2       S       sh                      2025-08-05 13:40:48     00:00:10
5       S       cat                     2025-08-05 13:40:51     00:00:07
4       S       proctest                2025-08-05 13:40:51     00:00:07
6       S       wc                      2025-08-05 13:40:51     00:00:07
7       S       grep                    2025-08-05 13:40:51     00:00:07
9       RN      ps                      2025-08-05 13:40:58     00:00:00
```

## Test 2

```
$ proctest 2
Testing procinfo syscall
$ fork:  4 ->  5
fork:  5 ->  6

$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:46:31     00:00:12
2       S       sh                      2025-08-05 13:46:31     00:00:12
6       Z       echo                    2025-08-05 13:46:39     00:00:04
4       S       proctest                2025-08-05 13:46:39     00:00:04
5       S       proctest                2025-08-05 13:46:39     00:00:04
8       RN      ps                      2025-08-05 13:46:43     00:00:00
$ 
```


## Test 3

```
$ proctest 3
$ Testing procinfo syscall
fork:  4 ->  5
fork:  5 ->  6

$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:48:51     00:00:08
2       S       sh                      2025-08-05 13:48:51     00:00:08
5       RE      proctest                2025-08-05 13:48:56     00:00:03
4       S       proctest                2025-08-05 13:48:56     00:00:03
6       Z       echo                    2025-08-05 13:48:56     00:00:03
8       RN      ps                      2025-08-05 13:48:59     00:00:00
```

## Test 4

```
$ proctest 4
Testing procinfo syscall
$ fork:  4 ->  5
fork:  4 ->  6
fork:  4 ->  7
fork:  4 ->  8
fork:  4 ->  9
fork:  4 -> 10
fork:  4 -> 11
fork:  4 -> 12
fork:  4 -> 13
fork:  4 -> 14

$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:51:34     00:00:10
2       S       sh                      2025-08-05 13:51:34     00:00:10
6       Z       proctest                2025-08-05 13:51:37     00:00:07
4       S       proctest                2025-08-05 13:51:36     00:00:08
5       Z       proctest                2025-08-05 13:51:36     00:00:08
7       Z       proctest                2025-08-05 13:51:37     00:00:07
8       Z       proctest                2025-08-05 13:51:37     00:00:07
9       Z       proctest                2025-08-05 13:51:37     00:00:07
10      Z       proctest                2025-08-05 13:51:37     00:00:07
11      Z       proctest                2025-08-05 13:51:37     00:00:07
12      Z       proctest                2025-08-05 13:51:37     00:00:07
13      Z       proctest                2025-08-05 13:51:37     00:00:07
14      Z       proctest                2025-08-05 13:51:37     00:00:07
16      RN      ps                      2025-08-05 13:51:44     00:00:00
$ 
```


## Test 5

```
$ proctest 5
Testing procinfo syscall
$ fork:  4 ->  5
fork:  4 ->  6
fork:  5 ->  7
fork:  5 ->  8

$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:52:55     00:00:10
2       S       sh                      2025-08-05 13:52:55     00:00:10
6       S       grep                    2025-08-05 13:53:02     00:00:03
4       S       proctest                2025-08-05 13:53:02     00:00:03
5       S       proctest                2025-08-05 13:53:02     00:00:03
7       S       cat                     2025-08-05 13:53:02     00:00:03
8       S       wc                      2025-08-05 13:53:02     00:00:03
10      RN      ps                      2025-08-05 13:53:05     00:00:00
$ 
```


## Test 6

```
$ proctest 6
$ Testing procinfo syscall
fork:  4 ->  5
fork:  4 ->  5
fork:  5 ->  6
fork:  4 ->  7
fork:  7 ->  8
fork:  5 ->  9
fork:  4 -> 10
fork:  8 -> 12
fork:  6 -> 11
fork:  9 -> 13
fork:  9 -> 15
fork: 11 -> 16
fork: 13 -> 17
fork: 13 -> 19
fork:  6 -> 14
fork: 11 -> 18

$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:54:17     00:00:09
2       S       sh                      2025-08-05 13:54:17     00:00:09
5       S       proctest                2025-08-05 13:54:21     00:00:05
4       RE      proctest                2025-08-05 13:54:21     00:00:05
6       S       proctest                2025-08-05 13:54:21     00:00:05
7       S       proctest                2025-08-05 13:54:21     00:00:05
8       RE      proctest                2025-08-05 13:54:21     00:00:05
9       S       proctest                2025-08-05 13:54:21     00:00:05
10      RE      proctest                2025-08-05 13:54:21     00:00:05
11      S       proctest                2025-08-05 13:54:21     00:00:05
12      Z       echo                    2025-08-05 13:54:22     00:00:04
13      S       proctest                2025-08-05 13:54:22     00:00:04
14      S       grep                    2025-08-05 13:54:22     00:00:04
15      S       grep                    2025-08-05 13:54:22     00:00:04
16      S       cat                     2025-08-05 13:54:22     00:00:04
17      S       cat                     2025-08-05 13:54:22     00:00:04
18      S       wc                      2025-08-05 13:54:22     00:00:04
19      S       wc                      2025-08-05 13:54:22     00:00:04
21      RN      ps                      2025-08-05 13:54:25     00:00:01  
```