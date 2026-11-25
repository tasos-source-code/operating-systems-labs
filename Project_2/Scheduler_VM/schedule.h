#ifndef SCHEDULE_H
#define SCHEDULE_H
// #define shortest_job_first
#define goodness_en
#define metrics

#include "macros.h"
#include "list.h"

struct thread_info;

/* ---------------- Do NOT Touch -------------- */
/* Sleep Types */
enum sleep_type
{
	SLEEP_NORMAL,
	SLEEP_NONINTERACTIVE,
	SLEEP_INTERACTIVE,
	SLEEP_INTERRUPTED
};

/* task_struct */

struct task_metrics{
	long long arrival_time;
	long long first_run_time;
	int has_started;
};
struct task_struct
{
	struct thread_info *thread_info;		/* Information about the thread */
	unsigned int time_slice;				/* Timeslice values */
	enum sleep_type sleep_type;				/* What type of sleep task is in */
	int need_reschedule;					/* Flag, set if task needs to
											   have schedule called */
	struct task_struct *prev, *next;	/* Used to link the task struct in the 
										   runqueue. Make sure to set them to
										   NULL when the process is not in the
										   runqueue */
	unsigned long long last_burst;					//last busrt time
	double exp_burst;					// current estimation of burst time
	unsigned long long ready_to_run;					// last execution time OR last runque insertion time
	unsigned long long execution_start;				// total execution time
	double goodness;					// nice value
	unsigned long long runtime;

	#ifdef metrics
	struct task_metrics metrics_info;
	#endif
/* ---------------- Do NOT Touch END-------------- */
};



/* runqueue */
struct runqueue {
    unsigned long    nr_running;			/* number of runnable tasks */
	struct task_struct *head;
};

/*----------------------- System Calls ------------------------------*/
/* These calls are provided by the VM for your
 * convenience, and mimic system calls provided
 * normally by Linux
 */
void context_switch(struct task_struct *next); 
unsigned long long sched_clock();

/*------------------YOU MAY EDIT BELOW THIS LINE---------------------*/
/*------------------- User Defined Functions -------------------------*/
/*-------------These functions MUST be defined for the VM-------------*/
void initschedule(struct runqueue *newrq, struct task_struct *seedTask);
void killschedule();
void schedule();
void activate_task(struct task_struct *p);
void deactivate_task(struct task_struct *p);
void scheduler_tick(struct task_struct *p);
void sched_fork(struct task_struct *p);
void wake_up_new_task(struct task_struct *p);

#endif