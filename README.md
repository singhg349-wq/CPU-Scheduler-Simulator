# CPU Scheduler Simulator

A command-line CPU scheduling simulator written in C. It reads a workload of processes from a file and simulates how they'd be scheduled under two classic scheduling policies: **First-Come, First-Served (FCFS)** and **Round Robin (RR)**. It then prints a tick-by-tick execution timeline along with standard scheduling metrics.

## Features

- Reads process workloads (PID, arrival time, CPU burst time) from a text file
- Simulates FCFS scheduling
- Simulates Round Robin scheduling with a configurable time quantum
- Prints a full CPU timeline showing which process ran at every time tick (or if the CPU was idle)
- Computes per-process turnaround time and response time
- Computes system-wide average turnaround time, average response time, and total context switches
- Verified leak-free with Valgrind

## Building

```
make
```

This compiles the simulator into an executable named `sched`.

## Running

```
./sched --policy=FCFS --in="Workload Samples/workload1.txt"
./sched --policy=RR --quantum=2 --in="Workload Samples/workload1.txt"
```

- `--policy` — `FCFS` or `RR`
- `--quantum` — required only for `RR`, sets the time slice length
- `--in` — path to the workload file

### Workload file format

Each line represents one process: `pid arrival_time cpu_burst_time`

```
0 0 7
1 2 4
2 4 1
```

## Example output

```
$ ./sched --policy=RR --quantum=2 --in="Workload Samples/workload1.txt"
time: 0 1 2 3 4 5 6 7 8 9 10 11
run : 0 0 1 1 0 0 2 1 1 0 0 0
P0: first run=0 completion=12 TAT=12 RESP=0
P1: first run=2 completion=9 TAT=7 RESP=0
P2: first run=6 completion=7 TAT=3 RESP=2
System: ctx_switches=5, avgTAT=7.333, avgRESP=0.667
```

## Metrics explained

- **Turnaround time (TAT)** — completion time minus arrival time; how long a process took from arriving to finishing.
- **Response time (RESP)** — first run time minus arrival time; how long a process waited before it first got the CPU.
- **Context switches** — counted whenever the CPU switches from one non-idle process to a different non-idle process. Idle ticks don't count.

## Testing for memory safety

```
make valgrind
```

Runs both policies through Valgrind's leak checker.

## Design Notes

- **Two-pass file reading**: `read_processes` scans the workload file once just to count 
  how many process lines exist, then allocates an array of exactly that size before 
  reading the file a second time to fill it in. This avoids over-allocating a fixed 
  buffer or using a dynamically-resizing array, at the cost of opening the file twice.

- **Array-based ready queue for Round Robin**: the ready queue is a plain array sized to 
  the theoretical maximum number of enqueues (`total_cpu_time + last_arrival + 1`), with 
  `front`/`back` indices that only move forward rather than wrapping. This keeps enqueue 
  and dequeue O(1) without needing per-node allocation the way a linked list would.

- **Mid-quantum arrivals**: Round Robin has to account for new processes arriving while 
  another process is mid-slice. The simulation checks for newly-arrived processes after 
  every single tick inside the quantum loop, not just when a process is first dequeued, 
  so a process that arrives partway through someone else's time slice is still queued at 
  the correct simulated time rather than only being picked up on the next full cycle.

- **Context switch counting**: only transitions between two different non-idle processes 
  count as a context switch — idle ticks are explicitly excluded so that a process 
  resuming after CPU idle time isn't miscounted as a switch.

  
## Author

Gurshmeer Singh