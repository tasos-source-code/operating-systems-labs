# Project 2: Shortest Job First Scheduler

This project adds a **Shortest Job First (SJF)** process scheduler to a user-space
virtual machine that simulates a single-CPU Linux system. The VM offers the same
scheduler interface as the Linux kernel (`schedule()`, `scheduler_tick()`,
`activate_task()`, `deactivate_task()`, `sched_fork()`, `wake_up_new_task()`,
`context_switch()`, …), so the scheduling logic can be developed and debugged without
rebuilding a kernel.

## Scheduling policies

All the work is in `schedule.c` and `schedule.h`. Compile-time switches in
`schedule.h` choose the policy:

| `#define` | Effect |
|-----------|--------|
| *(none)* | **Round Robin**: the baseline |
| `shortest_job_first` | **SJF**: picks the task with the smallest expected burst |
| `shortest_job_first` + `goodness_en` | **SJF with goodness**: SJF with protection against starvation |
| `metrics` | Collects statistics and prints them when the run ends |

### Burst estimation

The expected burst of each task is an exponential average of its past CPU bursts:

```
exp_burst = (last_burst + a · exp_burst) / (1 + a),     a = 0.5
```

### Goodness

Plain SJF starves long CPU-bound tasks. To prevent this, each task gets a *goodness*
value that balances a short expected burst against time spent waiting in the run queue:

```
goodness = (C + exp_burst) · (C + max_wait_in_rq)
           ───────────────────────────────────────
           (C + min_exp_burst) · (C + own_wait_in_rq)        C = 10⁷ ns (smoothing constant)
```

The scheduler runs the task with the **lowest** goodness. The longer a task waits,
the lower its goodness becomes, so every task is eventually scheduled.

## Metrics

With `metrics` enabled, the scheduler reports these values in nanoseconds:

- **Responsiveness**: average time a task waits before it gets the CPU
- **Total waiting time**: sum of all time spent waiting in the run queue
- **Maximum waiting time**: the longest single wait, which shows starvation
- **Context switches**

## Test workloads

| Config | Description |
|--------|-------------|
| `simple.conf` | 1 non-interactive + 1 interactive task |
| `noninteractives.conf` | 3 CPU-bound tasks |
| `4interactives.conf` | 4 I/O-bound tasks |
| `combined.conf` | 2 CPU-bound + 3 I/O-bound tasks (tests starvation) |
| `spawner.conf`, `complex.conf` | Mixed workloads that resemble real use |

## Results (summary)

| Workload | Policy | Responsiveness | Max wait | Ctx switches |
|----------|--------|---------------:|---------:|-------------:|
| combined | Round Robin | 26.1 ms | 40 ms | 1037 |
| combined | SJF | 107.2 ms | 8.9 s | 222 |
| combined | SJF + goodness | 26.0 ms | 250 ms | 965 |
| complex | Round Robin | 36.7 ms | 60 ms | 1052 |
| complex | SJF | 155.2 ms | 5.87 s | 141 |
| complex | SJF + goodness | 35.8 ms | 240 ms | 1042 |

Plain SJF makes few context switches, but CPU-bound tasks starve: their maximum wait
reaches several seconds. **SJF with goodness** keeps responsiveness at the Round Robin
level and keeps the maximum wait bounded, so it gives the best overall balance on
realistic workloads. The full results and discussion are in
[`Scheduler_VM/README.txt`](Scheduler_VM/README.txt).

## Build & run

```bash
cd Scheduler_VM
# choose the policy in schedule.h, then:
make
./sjf_sched combined.conf
```

## Files

```
Project_2/
├── Scheduler_VM/
│   ├── schedule.c, schedule.h    # scheduler implementation (our code)
│   ├── cpu.c, cpuinit.c          # virtual machine (provided)
│   ├── list.h, macros.h, privatestructs.h
│   ├── *.conf                    # workload profiles
│   ├── Makefile
│   └── README.txt                # full results & discussion
└── *.pdf                         # assignment & VM instructions (Greek)
```
