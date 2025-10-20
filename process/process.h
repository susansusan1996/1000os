#pragma once

#define PROCS_MAX 8       // Maximum number of processes

#define PROC_UNUSED   0   // Unused process control structure
#define PROC_RUNNABLE 1   // Runnable process


//PCB
struct process {
    int pid;             // Process ID
    int state;           // Process state: PROC_UNUSED or PROC_RUNNABLE
    vaddr_t sp;          // Stack pointer
    uint8_t stack[8192]; // Kernel stack
};

struct process *create_process(uint32_t pc);

void proc_a_entry(void);

void proc_b_entry(void);

extern struct process *proc_a;
extern struct process *proc_b;

#define PROC_A proc_a
#define PROC_B proc_b

extern struct process *current_proc; // Currently running process
extern struct process *idle_proc;    // Idle process

#define CURRENT_PROC current_proc
#define IDLE_PROC idle_proc


void yield(void);
