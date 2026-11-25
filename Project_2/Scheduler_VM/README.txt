noninteractives:
    ROUND ROBIN:
        ===== Scheduler statistics =====
        Responsiveness        : 19462365.59
        Total waiting time    : 1810000000
        Maximum waiting time  : 30000000
        Context switches      : 93

    SJF (only exp_burst):
        ===== Scheduler statistics =====
        Responsiveness        : 600000000.00
        Total waiting time    : 1800000000
        Maximum waiting time  : 900000000
        Context switches      : 3

    SJF (with goodness):
        ===== Scheduler statistics =====
        Responsiveness        : 19354838.71
        Total waiting time    : 1800000000
        Maximum waiting time  : 40000000
        Context switches      : 93

4interactives:
    ROUND ROBIN:
        ===== Scheduler statistics =====
        Responsiveness        : 14358452.14
        Total waiting time    : 7050000000
        Maximum waiting time  : 30000000
        Context switches      : 491

    SJF (only exp_burst):
        ===== Scheduler statistics =====
        Responsiveness        : 58571428.57
        Total waiting time    : 3280000000
        Maximum waiting time  : 270000000
        Context switches      : 56

    SJF (with goodness):
        ===== Scheduler statistics =====
        Responsiveness        : 15745967.74
        Total waiting time    : 7810000000
        Maximum waiting time  : 150000000
        Context switches      : 496

combined:
    ROUND ROBIN:
        ===== Scheduler statistics =====
        Responsiveness        : 26113789.78
        Total waiting time    : 27080000000
        Maximum waiting time  : 40000000
        Context switches      : 1037

    SJF (only exp_burst):
        ===== Scheduler statistics =====
        Responsiveness        : 107162162.16
        Total waiting time    : 23790000000
        Maximum waiting time  : 8900000000
        Context switches      : 222

    SJF (with goodness):
        ===== Scheduler statistics =====
        Responsiveness        : 25958549.22
        Total waiting time    : 25050000000
        Maximum waiting time  : 250000000
        Context switches      : 965

spawner:
    ROUND ROBIN:
        ===== Scheduler statistics =====
        Responsiveness        : 26055662.19
        Total waiting time    : 27150000000
        Maximum waiting time  : 40000000
        Context switches      : 1042

    SJF (only exp_burst):
        ===== Scheduler statistics =====
        Responsiveness        : 224069767.44
        Total waiting time    : 19270000000
        Maximum waiting time  : 10730000000
        Context switches      : 86

    SJF (with goodness):
        ===== Scheduler statistics =====
        Responsiveness        : 27144278.61
        Total waiting time    : 27280000000
        Maximum waiting time  : 280000000
        Context switches      : 1005

complex:
    ROUND ROBIN:
        ===== Scheduler statistics =====
        Responsiveness        : 36673003.80
        Total waiting time    : 38580000000
        Maximum waiting time  : 60000000
        Context switches      : 1052

    SJF (only exp_burst):
        ===== Scheduler statistics =====
        Responsiveness        : 155248226.95
        Total waiting time    : 21890000000
        Maximum waiting time  : 5870000000
        Context switches      : 141

    SJF (with goodness):
        ===== Scheduler statistics =====
        Responsiveness        : 35777351.25
        Total waiting time    : 37280000000
        Maximum waiting time  : 240000000
        Context switches      : 1042



Note: 
    the times are measured in nanoseconds

Comments:
        Round Robin is only fair due to the fact that every process gets the same time on the 
    cpu. However, its disadvantage is that interactive processes often get stuck behind big 
    non-interactive ones, leading to longer completion time.
        SJF is much more effective for I/O proccesses, but due to their bigger expected burst, 
    computational ones will suffer from starvation.
        SJF with goodness is the better all around choice as it both serves interactive proccesses
    fast enough while it avoids to neglect non_interactive ones. Thus, we have the best of
    both worlds.

    Our metrics:
        Responsiveness      -> how fast a procces takes the proccesor
        Total waiting time  -> the total time of procceses waiting in rq
        Maximum waiting time-> the max waiting time amongst all proccesses. (indicates starvation)
        Context switches    -> the number of context switches 


    tests:
        spawner and complex tests are more general and are used to simulate real environments
        combined contains 2 non-interactive and 3 interactives to test starvation
        4interactives contains 4 interactive proccesses 
        noninteractives contains 3 non-interactive proccesses 

        From the results above we can easily conclude that:
            - for real-like scenarios, SJF with goodness is visibly more effective at both balancing 
              workloads and responsiveness
            - for non-interactive procceses RR works the best as expected because we dont care about
              responsiveness and the execution time drops. However SJF (with goodness) has very similar 
              results. SJF just sorts the procceses based on the expected burst (finish time) but this
              increases average completion time and so is not prefered.
            - for the interactive ones SJF with and without goodness have similar performance but RR works
              the best as all the proccess are of the same type.

Concution:
        In conclution SJF with goodness proves itself the most effective in real-like scenarios. 
        Though RR may be better in theory (procceses of same type) because of its fairness, SJF 
        with goodness performs slightly worse.
        In general SJF with goodness is all around the best.

            