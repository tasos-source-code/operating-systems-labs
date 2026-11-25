/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "privatestructs.h"

#define NEWTASKSLICE (NS_TO_JIFFIES(100000000))

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

#ifdef metrics
static long long total_waiting = 0;
static long long total_wait_events = 0;
static long long total_response = 0;
static long long total_started = 0;
static long long total_context_switches = 0;
static long long max_wait_time = 0;
#endif
/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *           There are HZ jiffies in a second, (HZ is 
 *           declared in macros.h), and is usually
 *           1 or 10 milliseconds.
 */
extern long long jiffies;
extern struct task_struct *idle;

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
 /* initscheduler
  * Sets up and allocates memory for the scheduler, as well
  * as sets initial values. This function should also
  * set the initial effective priority for the "seed" task 
  * and enqueu it in the scheduler.
  * INPUT:
  * newrq - A pointer to an allocated rq to assign to your
  *         local rq.
  * seedTask - A pointer to a task to seed the scheduler and start
  * the simulation.
  */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask)
{
    seedTask->next = seedTask->prev = seedTask;
    newrq->head = seedTask;
    newrq->nr_running++;
    newrq->head->exp_burst = 0;
    newrq->head->last_burst = 0;
    newrq->head->runtime=0;
	newrq->head->time_slice = 100;
    seedTask->ready_to_run = sched_clock();
    seedTask->execution_start = sched_clock();
#ifdef metrics
    
    seedTask->metrics_info.arrival_time = sched_clock();
    seedTask->metrics_info.first_run_time = -1;
    seedTask->metrics_info.has_started = 0;
#endif
}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule()
{
#ifdef metrics
        printf("\n===== Scheduler statistics =====\n");
    if (total_wait_events > 0)
        printf("Responsiveness        : %.2f\n", (double)total_waiting / total_wait_events);
    printf("Total waiting time    : %lld\n", total_waiting);
    printf("Maximum waiting time  : %lld\n", max_wait_time);
    printf("Context switches      : %lld\n", total_context_switches);
#endif
    return;
}


void print_rq () {
    struct task_struct *curr;
    
    printf("Rq: \n");
    curr = rq->head;
    // if (curr)
    //  printf("%s, exp_burst: %u, ready_to_run: %u\n", curr->thread_info->processName, curr->exp_burst, curr->ready_to_run);
    while(curr->next != rq->head) {
        curr = curr->next;
        //printf(", %s, goodness: %f, exp_burst: %f, ready_to_run: %f, execution_start: %f\n", curr->thread_info->processName, curr->goodness,curr->exp_burst, curr->ready_to_run, curr->execution_start    );
        printf("\t %s, goodness=%f\n", curr->thread_info->processName, curr->goodness);
    };
    printf("\n");
}


//returns the candidate with minimum exp burst and updates max_waitRQ and min_exp_burst
struct task_struct* find_min_exp_burst(double temp_exp_burst, long int *max_waitRQ, double *min_exp_burst) {
    struct task_struct *curr;
    *min_exp_burst = rq->head->next->exp_burst;
     struct task_struct *candidate = rq->head->next;
    list_for_each(curr, rq->head){
        if(curr!=rq->head){
            if(curr!=current){
                if((sched_clock() - curr->ready_to_run) > *max_waitRQ){*max_waitRQ = (sched_clock() - curr->ready_to_run);} //maximum waiting time in rq
                if(curr->exp_burst < *min_exp_burst){*min_exp_burst = curr->exp_burst; candidate = curr;}       //minimum exp burst
                
            }
            else{
                if(temp_exp_burst < *min_exp_burst){*min_exp_burst = temp_exp_burst; candidate = curr;}     //minimum exp burst
            }
        }
    }
    return candidate;
}

void update_goodness(double temp_exp_burst, long int max_waitRQ, double min_exp_burst) {
    struct task_struct *curr;
    list_for_each(curr, rq->head){
        if(curr!=rq->head){
            if(curr!=current){
                curr->goodness = ((1*10000000+curr->exp_burst) * (1*10000000+max_waitRQ)) / ((1*10000000+min_exp_burst) * (1*10000000 + (sched_clock() - curr->ready_to_run)));
                //printf("name=%s, goodness=%lf, expburst=%lf, maxWaitRq=%ld, minexpburst=%lf, waitinrq=%ld\n", curr->thread_info->processName ,curr->goodness, curr->exp_burst, max_waitRQ, min_exp_burst,(long int)(sched_clock() - curr->ready_to_run));
            }
            else{
                curr->goodness = ((1*10000000+temp_exp_burst) * (1*10000000+max_waitRQ)) / ((1*10000000+min_exp_burst)*10000000);
                //printf("name=%s, current goodness=%lf, temp_exp=%lf, maxWaitRq=%ld, minexpburst=%lf, waitinrq=0\n", curr->thread_info->processName, curr->goodness, curr->exp_burst, max_waitRQ, min_exp_burst);
            }
        }
    }
}

#ifdef metrics
static void update_sched_metrics(struct task_struct *prev, struct task_struct *next)
{
    long long now;
    long long current_wait;

    if (next == NULL)
        return;

    now = sched_clock();

    if (prev != NULL && prev != next)
        total_context_switches++;

    if (!next->metrics_info.has_started) {
        next->metrics_info.first_run_time = now;
        next->metrics_info.has_started = 1;
        total_response += now - next->metrics_info.arrival_time;
        total_started++;
    }

    if (prev != next) {
        current_wait = now - next->ready_to_run;
        total_waiting += current_wait;
        total_wait_events++;

        if (current_wait > max_wait_time){
            printf("New max wait time: %lld of process %s\n", current_wait, next->thread_info->processName);
            max_wait_time = current_wait;}
    }
}
#endif

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task in the queue
 */
//
#ifdef shortest_job_first
void schedule()
{
    struct task_struct *curr, *candidate=NULL;
    float a = 0.5;
    long int max_waitRQ=0; 
    double temp_exp_burst, min_exp_burst, min_goodness;

    //printf("In schedule before calculations\n");
    //print_rq();

    current->runtime += sched_clock() - current->execution_start;
    temp_exp_burst = (current->runtime + a * current->exp_burst)/(1+a);
    candidate = find_min_exp_burst(temp_exp_burst, &max_waitRQ, &min_exp_burst);
    update_goodness(temp_exp_burst, max_waitRQ, min_exp_burst);

    if (rq->nr_running == 1) {
        current->need_reschedule = 0;
        context_switch(rq->head);
    }
    else{
#ifdef goodness_en
        min_goodness = rq->head->next->goodness;
        candidate = rq->head->next;
        list_for_each(curr, rq->head){
            if(curr!=rq->head){
                if(curr->goodness < min_goodness){
                    candidate = curr;
                    min_goodness = curr->goodness;
                }
            }
        }
#endif

        if(candidate != current){
            if(current->need_reschedule){                //the task leaves the cpu on its own       
                current->last_burst = current->runtime;
                current->exp_burst = (current->last_burst + a*current->exp_burst) / (1 + a);
                current->runtime = 0;
            }
            else 
                current->exp_burst = temp_exp_burst;
            
            current->ready_to_run = sched_clock();
            candidate->execution_start = sched_clock();

#ifdef metrics
            update_sched_metrics(current, candidate);
#endif
            current->need_reschedule = 0;
			context_switch(candidate);
        }
    }
}
#endif

//round robin
#ifndef shortest_job_first
void schedule()
{
    static struct task_struct *nxt = NULL;
    struct task_struct *curr;

    //printf("In schedule before calculations\n");
    //print_rq();

    current->need_reschedule = 0;

    if (rq->nr_running == 1) {
        context_switch(rq->head);
        nxt = rq->head->next;
    }
    else {  
        curr = nxt;
        nxt = nxt->next;
        if (nxt == rq->head)    /* Do this to always skip init at the head */
            nxt = nxt->next;    /* of the queue, whenever there are other  */
                                /* processes available                     */

#ifdef metrics
        if (current != curr)
            current->ready_to_run = sched_clock();

        curr->execution_start = sched_clock();
        update_sched_metrics(current, curr);
#endif

        context_switch(curr);
    }
}
#endif

/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void sched_fork(struct task_struct *p)
{
    p->time_slice = 100;
    p->last_burst = 0;
    p->exp_burst  = 0;
    p->ready_to_run   = sched_clock();     //maybe not neccessary or wrong
    p->goodness = 0;
    p->runtime=0;

#ifdef metrics
    p->metrics_info.arrival_time = sched_clock();
    p->metrics_info.first_run_time = -1;
    p->metrics_info.has_started = 0;
#endif
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{
    schedule();
}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 */
void wake_up_new_task(struct task_struct *p)
{   
    p->next = rq->head->next;
    p->prev = rq->head;
    p->next->prev = p;
    p->prev->next = p;
    p->ready_to_run = sched_clock();
    //p->execution_start = sched_clock();
    p->goodness = 0;
    // p->exp_burst = 0;
    rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{
    p->next = rq->head->next;
    p->prev = rq->head;
    p->next->prev = p;
    p->prev->next = p;
    
    rq->nr_running++;
    p->ready_to_run = sched_clock();
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{
    p->prev->next = p->next;
    p->next->prev = p->prev;
    p->next = p->prev = NULL; /* Make sure to set them to NULL *
                               * next is checked in cpu.c      */

    rq->nr_running--;

}
