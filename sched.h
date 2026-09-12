/*
 * Author: Gurshmeer Singh
 * File Name: sched.h
 * Description: CPU scheduling simulator header file containing function prototypes and structure definitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SCHED_H
#define SCHED_H

/*
 * Stores the command-line arguments after parsing.
 * policy is either "FCFS" or "RR".
 * quantum is used only for Round Robin.
 * input_file stores the workload filename.
 */
typedef struct {
    char *policy;
    char *quantum;
    char *input_file;
} arguments_t;

/*
 * Parses command-line arguments and stores them in args.
 * Returns 0 if parsing is successful, and -1 if input is invalid.
 */
int parsing(int argc, char *argv[], arguments_t *args);

/*
 * Prints the required usage message for invalid command-line input.
 */
void print_usage(void);

/*
 * Stores information for one process in the workload.
 * remaining_time is mainly used for Round Robin.
 * first_run and completion are filled in during the simulation.
 */
typedef struct {
    int pid;
    int arrival;
    int cpu_time;
    int remaining_time;
    int first_run;
    int completion;
} process_t;

/*
 * Reads the workload file and returns a dynamically allocated array of processes.
 * num_processes is updated with the number of processes read.
 */
process_t* read_processes(const char *filename, int *num_processes);

/*
 * Sorts processes by arrival time.
 * If arrival times are equal, PID is used as the tie-breaker.
 */
void sort_processes(process_t *processes, int num_processes);

/*
 * Simulates First-Come, First-Served scheduling.
 * Returns a dynamically allocated timeline array.
 */
int *simulate_fcfs(process_t *processes, int num_processes, int *timeline_length);

/*
 * Prints the timeline in the required time/run format.
 */
void print_timeline(const int *timeline, int timeline_length);

/*
 * Counts context switches by scanning the timeline.
 * Idle slots are ignored.
 */
int count_context_switches(const int *timeline, int timeline_length);

/*
 * Prints per-process metrics and system-level metrics.
 */
void print_metrics(const process_t *processes, int num_processes,
                   const int *timeline, int timeline_length);

/*
 * Simulates Round Robin scheduling using the given quantum.
 * Returns a dynamically allocated timeline array.
 */
int *simulate_rr(process_t *processes, int num_processes, int quantum,
                 int *timeline_length);

#endif
