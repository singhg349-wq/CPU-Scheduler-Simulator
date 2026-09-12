/*
 * Author: Gurshmeer Singh
 * File Name: sched.c
 * Description: CPU scheduling simulator implementation.
 */

#include "sched.h"

int main(int argc, char *argv[])
{
    arguments_t args;
    process_t *processes;
    int *timeline;
    int num_processes;
    int timeline_length;

    /* Parse and validate command-line arguments before doing any simulation. */
    if (parsing(argc, argv, &args) == -1) {
        return EXIT_FAILURE;
    }

    /* Read the workload file into an array of process structures. */
    processes = read_processes(args.input_file, &num_processes);
    if (processes == NULL) {
        return EXIT_FAILURE;
    }

    /* Sort processes so scheduling is based on arrival time and PID order. */
    sort_processes(processes, num_processes);

    timeline = NULL;
    timeline_length = 0;

    /* Choose the correct scheduling algorithm based on the policy argument. */
    if (strcmp(args.policy, "FCFS") == 0) {
        timeline = simulate_fcfs(processes, num_processes, &timeline_length);
    
    } else if (strcmp(args.policy, "RR") == 0) {
        timeline = simulate_rr(processes, num_processes, atoi(args.quantum),
                           &timeline_length);
    }

    /* If the simulator failed to allocate or create a timeline, exit safely. */
    if (timeline == NULL) {
        free(processes);
        return EXIT_FAILURE;
    }

    /* Print the required CPU timeline and scheduling metrics. */
    print_timeline(timeline, timeline_length);
    print_metrics(processes, num_processes, timeline, timeline_length);

    /* Free dynamically allocated memory before exiting. */
    free(timeline);
    free(processes);

    return EXIT_SUCCESS;
}

int parsing(int argc, char *argv[], arguments_t *args)
{
    int i;

    /* Initialize all argument fields before parsing. */
    args->policy = NULL;
    args->quantum = NULL;
    args->input_file = NULL;

    /*
     * Parse each command-line option.
     * The pointer is moved past the option prefix to store only the value.
     */
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--policy=", 9) == 0) {
            args->policy = argv[i] + 9;
        }
        else if (strncmp(argv[i], "--quantum=", 10) == 0) {
            args->quantum = argv[i] + 10;
        }
        else if (strncmp(argv[i], "--in=", 5) == 0) {
            args->input_file = argv[i] + 5;
        }
        else {
            print_usage();
            return -1;
        }
    }

    /* Policy and input file are required for both scheduling algorithms. */
    if (args->policy == NULL || args->input_file == NULL) {
        print_usage();
        return -1;
    }

    /* Only FCFS and RR are accepted scheduling policies. */
    if (strcmp(args->policy, "FCFS") != 0 &&
        strcmp(args->policy, "RR") != 0) {
        print_usage();
        return -1;
    }

    /* Round Robin requires a quantum value. */
    if (strcmp(args->policy, "RR") == 0 && args->quantum == NULL) {
        print_usage();
        return -1;
    }

    return 0;
}

void print_usage(void)
{
    fprintf(stderr, "Usage: ./sched --policy=FCFS|RR [--quantum=N] --in=FILE\n");
}

process_t *read_processes(const char *filename, int *num_processes)
{
    FILE *file;
    process_t *processes;
    int pid;
    int arrival;
    int cpu_time;
    int i;

    *num_processes = 0;

    /* First pass: open the file and count how many process lines exist. */
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    while (fscanf(file, "%d %d %d", &pid, &arrival, &cpu_time) == 3) {
        (*num_processes)++;
    }

    fclose(file);

    /* Allocate exactly enough space for the number of processes counted. */
    processes = malloc((*num_processes) * sizeof(process_t));
    if (processes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    /* Second pass: reopen the file and store each process. */
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        free(processes);
        return NULL;
    }

    i = 0;

    while (fscanf(file, "%d %d %d", &pid, &arrival, &cpu_time) == 3) {
        processes[i].pid = pid;
        processes[i].arrival = arrival;
        processes[i].cpu_time = cpu_time;
        processes[i].remaining_time = cpu_time;
        processes[i].first_run = -1;
        processes[i].completion = -1;

        i++;
    }

    fclose(file);

    return processes;
}

void sort_processes(process_t *processes, int num_processes)
{
    int i;
    int j;
    process_t temp;

    /*
     * Sort by arrival time first.
     * If two processes arrive at the same time, sort by smaller PID.
     */
    for (i = 0; i < num_processes - 1; i++) {
        for (j = i + 1; j < num_processes; j++) {
            if (processes[j].arrival < processes[i].arrival ||
                (processes[j].arrival == processes[i].arrival &&
                 processes[j].pid < processes[i].pid)) {

                temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }
}

int *simulate_fcfs(process_t *processes, int num_processes, int *timeline_length)
{
    int i;
    int t;
    int current_time;
    int total_cpu_time;
    int last_arrival;
    int max_timeline_length;
    int *timeline;

    total_cpu_time = 0;
    last_arrival = 0;

    /* Estimate enough timeline space using CPU time and latest arrival. */
    for (i = 0; i < num_processes; i++) {
        total_cpu_time += processes[i].cpu_time;

        if (processes[i].arrival > last_arrival) {
            last_arrival = processes[i].arrival;
        }
    }

    max_timeline_length = total_cpu_time + last_arrival + 1;

    timeline = malloc(max_timeline_length * sizeof(int));
    if (timeline == NULL) {
        return NULL;
    }

    current_time = 0;
    *timeline_length = 0;

    /*
     * FCFS runs each process fully before moving to the next one.
     * Idle time is recorded when no process has arrived yet.
     */
    for (i = 0; i < num_processes; i++) {
        while (current_time < processes[i].arrival) {
            timeline[*timeline_length] = -1;
            (*timeline_length)++;
            current_time++;
        }

        processes[i].first_run = current_time;

        for (t = 0; t < processes[i].cpu_time; t++) {
            timeline[*timeline_length] = processes[i].pid;
            (*timeline_length)++;
            current_time++;
            processes[i].remaining_time--;
        }

        processes[i].completion = current_time;
    }

    return timeline;
}
    
void print_timeline(const int *timeline, int timeline_length)
{
    int i;

    /* Print one time label for each slot in the timeline. */
    printf("time:");

    for (i = 0; i < timeline_length; i++) {
        printf(" %d", i);
    }

    printf("\n");

    /* Print the PID running in each slot, or '-' for idle. */
    printf("run :");

    for (i = 0; i < timeline_length; i++) {
        if (timeline[i] == -1) {
            printf(" -");
        } else {
            printf(" %d", timeline[i]);
        }
    }

    printf("\n");
}

int count_context_switches(const int *timeline, int timeline_length)
{
    int i;
    int previous_pid;
    int current_pid;
    int context_switches;

    previous_pid = -1;
    context_switches = 0;

    /*
     * Count only changes between two non-idle PIDs.
     * Idle slots do not count as context switches.
     */
    for (i = 0; i < timeline_length; i++) {
        current_pid = timeline[i];

        if (current_pid != -1) {
            if (previous_pid != -1 && current_pid != previous_pid) {
                context_switches++;
            }

            previous_pid = current_pid;
        }
    }

    return context_switches;
}

void print_metrics(const process_t *processes, int num_processes,
                   const int *timeline, int timeline_length)
{
    int i;
    int turnaround;
    int response;
    int total_turnaround;
    int total_response;
    int context_switches;
    double avg_turnaround;
    double avg_response;

    total_turnaround = 0;
    total_response = 0;

    /* Print per-process metrics and accumulate totals for averages. */
    for (i = 0; i < num_processes; i++) {
        turnaround = processes[i].completion - processes[i].arrival;
        response = processes[i].first_run - processes[i].arrival;

        total_turnaround += turnaround;
        total_response += response;

        printf("P%d: first run=%d completion=%d TAT=%d RESP=%d\n",
               processes[i].pid,
               processes[i].first_run,
               processes[i].completion,
               turnaround,
               response);
    }

    context_switches = count_context_switches(timeline, timeline_length);

    avg_turnaround = (double) total_turnaround / num_processes;
    avg_response = (double) total_response / num_processes;

    printf("System: ctx_switches=%d, avgTAT=%.3f, avgRESP=%.3f\n",
           context_switches,
           avg_turnaround,
           avg_response);
}

int *simulate_rr(process_t *processes, int num_processes, int quantum,
                 int *timeline_length)
{
    int total_cpu_time;
    int last_arrival;
    int max_timeline_length;
    int *timeline;
    int *queue;
    int front;
    int back;
    int current_time;
    int completed;
    int next_process;
    int process_index;
    int ticks_used;
    int i;

    total_cpu_time = 0;
    last_arrival = 0;

    /* Estimate enough space for the timeline and ready queue. */
    for (i = 0; i < num_processes; i++) {
        total_cpu_time += processes[i].cpu_time;

        if (processes[i].arrival > last_arrival) {
            last_arrival = processes[i].arrival;
        }
    }

    max_timeline_length = total_cpu_time + last_arrival + 1;

    timeline = malloc(max_timeline_length * sizeof(int));
    if (timeline == NULL) {
        return NULL;
    }

    queue = malloc(max_timeline_length * sizeof(int));
    if (queue == NULL) {
        free(timeline);
        return NULL;
    }

    front = 0;
    back = 0;
    current_time = 0;
    completed = 0;
    next_process = 0;
    *timeline_length = 0;

    /*
     * Continue scheduling until every process has completed.
     * The queue stores indexes into the processes array.
     */
    while (completed < num_processes) {
        while (next_process < num_processes &&
               processes[next_process].arrival <= current_time) {
            queue[back] = next_process;
            back++;
            next_process++;
        }

        /* If no process is ready, the CPU stays idle for one tick. */
        if (front == back) {
            timeline[*timeline_length] = -1;
            (*timeline_length)++;
            current_time++;
            continue;
        }

        /* Remove the next process from the front of the ready queue. */
        process_index = queue[front];
        front++;

        /* Record response time information the first time the process runs. */
        if (processes[process_index].first_run == -1) {
            processes[process_index].first_run = current_time;
        }

        ticks_used = 0;

        /*
         * Run the process for at most one quantum.
         * Stop early if the process finishes.
         */
        while (ticks_used < quantum &&
               processes[process_index].remaining_time > 0) {
            timeline[*timeline_length] = processes[process_index].pid;
            (*timeline_length)++;

            processes[process_index].remaining_time--;
            current_time++;
            ticks_used++;

            /* Add any new processes that arrived during this time slice. */
            while (next_process < num_processes &&
                   processes[next_process].arrival <= current_time) {
                queue[back] = next_process;
                back++;
                next_process++;
            }
        }

        /*
         * If the process finished, record completion time.
         * Otherwise, place it at the back of the ready queue.
         */
        if (processes[process_index].remaining_time == 0) {
            processes[process_index].completion = current_time;
            completed++;
        } else {
            queue[back] = process_index;
            back++;
        }
    }

    free(queue);

    return timeline;
}