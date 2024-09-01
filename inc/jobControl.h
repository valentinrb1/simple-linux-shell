/**
 * @file jobControl.h
 * @author Robledo, Valentín
 * @brief Encabezado de jobControl.c
 * @version 1.0
 * @date Septiembre de 2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __JOBCONTROL_H__
#define __JOBCONTROL_H__

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum STATUS
{
    PROCESS_NEW,
    PROCESS_RUNNING,
    PROCESS_DONE,
    PROCESS_SUSPEND,
    PROCESS_TERMINATED

}STATUS;

typedef enum EXEMODE
{
    FOREGROUND,
    BACKGROUND

}EXEMODE;

typedef struct process
{
    pid_t pid;
    int argc;
    char** argv;
    STATUS status;

    struct process *next;
}process;

typedef struct job
{   
    process* process_head;
    pid_t pgid;

    u_int8_t jid;
    EXEMODE exemode;
    int io_pipe[2];
    int e_pipe[2];

    struct job* next;
}job;

void addJob(job *new_job);
void removeJob(job* job);
job* findParentJob(job* job);
job* findLastJob();
void freeJob(job* job);
void launchJob(job* job);
void launchProcess(process* process, job* job);
void jobWait(job* job);
process* getProcessInJob(job* job, pid_t pid);
int jobIsComplete(job* job);
u_int8_t getProcessCount(job* job);
void initControlShell();
void handlerJob();
job* getJobOwner(pid_t pid);
void printJobStatus(job* job);
void printPipe(job* job);

extern job* head;
extern const char* string_status[];

#endif //__JOBCONTROL_H__