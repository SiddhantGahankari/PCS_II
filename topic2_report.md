# Project Report: CPU Scheduling in xv6

## Part A: Understanding the default scheduler

The default xv6 scheduler is implemented in `proc.c` within the `scheduler()` function. It operates as a simple round-robin scheduler. The scheduler continuously loops through the process table (`ptable.proc`). For each process that is in the `RUNNABLE` state, it acquires the `ptable.lock`, switches to the process using `swtch`, and allows it to run for a time slice (until it yields the CPU, typically due to a timer interrupt). Once the process yields, control returns to the scheduler, which then proceeds to the next process in the table, thus guaranteeing fairness across all runnable processes.

We wrote a test program (`schedtest.c`) to observe this behavior. It created multiple processes that each run a CPU-intensive loop, which demonstrated that their execution interleaved evenly as expected with round-robin.

## Part B & C: Priority-based Scheduler and System Call

We implemented a priority-based scheduler where each process has an assigned priority level. The scheduler always selects the `RUNNABLE` process with the highest priority. If multiple processes share the highest priority, the scheduler uses round-robin among them. The default priority assigned to a new process is `10`. Higher numerical values represent a higher priority.

To allow users to modify a process's priority, we introduced a new system call `setpriority(int pid, int priority)`.

### Implementation Details

| File       | Changes |
|------------|---------|
| `proc.h`   | Added `int priority;` to `struct proc` |
| `proc.c`   | Initialized priority in `allocproc()`. Modified `scheduler()` to always pick the highest priority RUNNABLE process. Implemented `setpriority()` function to update a process's priority. |
| `defs.h`   | Added `setpriority(int, int);` function declaration |
| `syscall.h`| Added `#define SYS_setpriority 25` |
| `syscall.c`| Registered the new system call in the syscall array |
| `sysproc.c`| Added `sys_setpriority(void)` wrapper to fetch arguments and call `setpriority` |
| `user.h`   | Added user-space prototype `int setpriority(int, int);` |
| `usys.S`   | Added `SYSCALL(setpriority)` assembly stub |
| `Makefile` | Added `_schedtest` and `_prioritytest` to `UPROGS` |

#### 1. Modifying the Scheduler
In `proc.c`, the `allocproc()` function was updated to assign `p->priority = 10;` to newly created processes.
The `scheduler()` function was rewritten to iterate over the `ptable` and, for each `RUNNABLE` process `p`, check if there is any other `RUNNABLE` process `p1` with a priority strictly greater than `p->priority`. If not, `p` is run. This guarantees we always run processes with the maximum priority, and naturally falls back to round-robin for processes with the same highest priority.

#### 2. Adding `setpriority`
We added `int setpriority(int pid, int priority)` in `proc.c` which iterates through the process table to find the given `pid` and updates its `priority`. The `sys_setpriority()` function in `sysproc.c` fetches the `pid` and `priority` arguments from the user stack and passes them to the kernel `setpriority()` function.

## Part D: Testing the Scheduler

We added two user programs to test the scheduler behavior:

1. **`schedtest.c` (Part A):** Forks 3 children with the default priority (10) and performs busy-waiting loops. It demonstrates round-robin scheduling because all processes have the same priority.
2. **`prioritytest.c` (Part D):** Forks 3 children and uses `setpriority()` to assign them different priorities (`5`, `10`, and `20`). Each child performs the same busy-wait loop.

### Output of `schedtest`
```output
$ schedtest
Starting schedtest with 3 processes
Process 0 (PID 4) is running (iteration 0)
Process 0 (PID 4) is running (iteration 1)
Process 1 (PID 5) is running (iteration 0)
Process 1 (PID 5) is running (iteration 1)
Process 0 (PID 4) is running (iteration 2)
Process 1 (PID 5) is running (iteration 2)
Process 2 (PID 6) is running (iteration 0)
Process 2 (PID 6) is running (iteration 1)
Process 0 (PID 4) finished
Process 1 (PID 5) finished
Process 2 (PID 6) is running (iteration 2)
Process 2 (PID 6) finished
schedtest finished
```
*(The execution neatly interleaves because the processes share the same priority.)*

### Output of `prioritytest`
```output
$ prioritytest
Starting prioritytest with 3 processes
Process 0 (PID 8, Prio 5) is running (iteration 0)
Process 1 (PID 9, Prio 10) is running (iteration 0)
Process 2 (PID 10, Prio 20) is running (iteration 0)
Process 2 (PID 10, Prio 20) is running (iteration 1)
Process 2 (PID 10, Prio 20) is running (iteration 2)
Process 2 (PID 10, Prio 20) finished
Process 1 (PID 9, Prio 10) is running (iteration 1)
Process 1 (PID 9, Prio 10) is running (iteration 2)
Process 1 (PID 9, Prio 10) finished
Process 0 (PID 8, Prio 5) is running (iteration 1)
Process 0 (PID 8, Prio 5) is running (iteration 2)
Process 0 (PID 8, Prio 5) finished
prioritytest finished
```
*(As soon as Process 2 with Priority 20 is scheduled, it monopolizes the CPU until completion. Then Process 1 with Priority 10 runs to completion, followed lastly by Process 0 with Priority 5. This clearly verifies the priority-based scheduling logic.)*
