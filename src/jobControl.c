/**
 * @file jobControl.c
 * @author Robledo, Valentín
 * @brief 
 * @version 1.0
 * @date Septiembre de 2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../inc/jobControl.h"

job* head = NULL;
const char* string_status[] = {"new","running","done","suspend","terminated"};

void addJob(job* new_job)
{
    new_job->next = NULL;                   //Asignamos siguiente como NULL para que el nodo por agregar sea el último.
    job* last = findLastJob();              //Buscamos el último nodo.

    if(!last)                               //Verificamos si no hay un último, es porque la lista está vacía.
    {
        new_job->jid = 1;                   //Le asignamos ID=1, es decir, el primer nodo.
        head = new_job;                     //Lo asignamos como HEAD.
    }
    else                                    //Si no está vacía, lo agregamos al siguiente.
    {
        new_job->jid = last->jid + 1;       //Asignamos ID + 1.
        last->next = new_job;               //Lo asignamos como NEXT.
    }
}

void removeJob(job* c_job)
{   
    if(head == NULL)
        return;

    job* parent_job = findParentJob(c_job); //Buscamos al nodo padre.

    if(c_job == head)                       //Verificamos si es el primer nodo.
        head = c_job->next;                 //Asignamos al NEXT del nodo como nueva HEAD.
    else if(c_job->next == NULL)            //verificamos si es el último nodo.
        parent_job->next = NULL;            //Asignamos al NEXT del nodo padre que apunte a NULL.
    else                                    //Si no es el primero, ni el último.
        parent_job->next = c_job->next;     //Asignamos al NEXT del nodo padre que apunte al NEXT del nodo actual.

    freeJob(c_job);                         //Liberamos memoria.
}

job* findLastJob()
{
    job* last = head;                       //Asignamos al último la HEAD.

    if(!last)                               //Verificamos si la lista está vacía.
        return NULL;

    while(last->next)                       //Mientras haya un siguiente sigo el bucle.
        last = last->next;                  //El último va a ser igual al siguiente.
    
    return last;                            //Retorno el último.
}

job* findParentJob(job* c_job)
{
    job* i_job;

    if(!head)                               //Verificamos si la lista está vacía.
        return NULL;    
    else if(head == c_job)                  //Verificamos si la cabeza de la lista es el nodo recibido como parámetro.
        return NULL;
    else
    {
        for(i_job = head; i_job; i_job = i_job->next)   //Recorremos la lista.
            if(i_job->next == c_job)        //Verificamos si el NEXT apunta a nuestro nodo.
                return i_job;               //De ser así, podemos decir que es su padre.  
    }

    return NULL;
}

void freeJob(job* job)
{
    process* aux;
    process* i = job->process_head;

    while(i)
    {
        for(u_int8_t j = 0; j < i->argc; j++)
            free(i->argv[j]);
        
        free(i->argv);
        
        aux = i;
        i = i->next;
    
        free(aux);
    }

    free(job);
}

void launchJob(job* job)
{
    addJob(job);

    if(pipe(job->io_pipe) < 0 || pipe(job->e_pipe) < 0)     //Creo las pipes.
        exit(EXIT_FAILURE);
    
    for(process* p = job->process_head; p; p = p->next)     //Lanzo procesos.
        launchProcess(p, job);
    
    if(job->exemode == FOREGROUND)                          //Si llego acá, terminaron los procesos.
        removeJob(job); 
    else                                                    //Segundo plano.
        printJobStatus(job);
}

void launchProcess(process* process, job* job)
{
    process->status = PROCESS_RUNNING;

    pid_t child_pid = fork();                               //Clona proceso actual, generando una copia.

    if(child_pid < 0)                                       //verifica si es menor que cero, para errores.
    {
        fprintf(stderr, "Fork failed"); 
        exit(EXIT_FAILURE);
    }
    else if(child_pid == 0)                                 //Nuevo proceso.
    {
        signal(SIGQUIT, SIG_DFL);                           //Atiende SIGUIT y la atiende por defecto(SIG_DFL), el defecto de SIGQUIT es matar el proceso.
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        close(job->io_pipe[0]);
        close(job->e_pipe[0]);                              //Ciero salidas de la pipe.

        dup2(job->io_pipe[1], STDOUT_FILENO);               //STDOUT_FILENO sale por io_pipe[1].
        dup2(job->e_pipe[1], STDERR_FILENO);

        close(job->io_pipe[1]);                             //Cierro entradas.
        close(job->e_pipe[1]);

        if(execvp(process->argv[0], process->argv) < 0)
        {
            fprintf(stderr, "Program not found");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
    else
    {
        process->pid = child_pid;

        if(job->pgid < 0)
            job->pgid = process->pid;

        setpgid(process->pid, job->pgid);

        if(job->exemode == FOREGROUND)
        {   
            tcsetpgrp(0,job->pgid); //Paso control de la consola al job
            jobWait(job);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(0,getpid());
            signal(SIGTTOU, SIG_DFL);
            printPipe(job);
        }
    }
}

void jobWait(job* job)
{
    u_int8_t process_done = 0;                                  //Cantidad de procesos finalizados.
    u_int8_t process_wait = getProcessCount(job);               //Cantidad de procesos a esperar.

    do
    {
        int status;
        pid_t pid = waitpid(-job->pgid, &status, WUNTRACED);    //Espera proceso del grupo PGID, retorna el estado del proceso matcheado.

        if(pid < 0)                                             //Verifica si el proceso es menor que cero, es decir, que no terminó.
            break;                                              //De ser así, rompe.
        
        process_done++;                                         //Caso contrario, aumenta el contador.

        if(WIFEXITED(status))                                   //Verifica si el proceso finalizó correctamente.
        {
            process* process = getProcessInJob(job, pid);       //Obtengo el proceso.
            process->status = PROCESS_DONE;                     //Estado: finalizado.
        }
    }
    while(process_done < process_wait);                         //Bucle, mientras que los procesos finalizados sean menores a los esperados.
}

process* getProcessInJob(job* job, pid_t pid)
{
    for(process* p = job->process_head; p; p = p->next)         //Recorre procesos.
        if(p->pid == pid)                                       //Verifica si el PID coincide.
            return p;                                           //En caso de ser así, retorna el proceso.

    return NULL;                                                //Retorna null.
}

int jobIsComplete(job* job)
{
    for(process* p = job->process_head; p; p = p->next)         //Recorre procesos.
        if(p->status != PROCESS_DONE)                           //Verifica si el proceso NO terminó.
            return 0;                                           //Retorna 0.
    
    return 1;                                                   //Retorna 1.
}

u_int8_t getProcessCount(job* job)
{
    u_int8_t n = 0;

    for(process* p = job->process_head; p; p = p->next)         //Recorre procesos.         
        if(p->status != PROCESS_DONE)                           //Verifica si el proceso NO terminó.
            n++;                                                //Suma un contador.

    return n;                                                   //Retorna el contador.
}

void initControlShell()
{
    struct sigaction action = {.sa_handler = handlerJob, .sa_flags = 0}; //Estructura que le dice quien va a controlar la interrupción dada por la señal.
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, NULL);                          //Que señal voy a controlar, y con que acción.

    pid_t pid = getpid();                                       //Obtengo pid de proceso actual
    setpgid(pid, pid);                                          //pgid = pid
    tcsetpgrp(STDOUT_FILENO, pid);                              //Le damos control a la consola de la terminal.
}

void handlerJob()
{
    int status;
    pid_t pid;

    while((pid = waitpid(WAIT_ANY, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
        job* job = getJobOwner(pid);

        if(WIFEXITED(status))
        {
            process* dprocess = getProcessInJob(job, pid);
            dprocess->status = PROCESS_DONE;
        }
        
        if(jobIsComplete(job))
        {
            printJobStatus(job);
            printPipe(job);
            removeJob(job);
        }
    }
}

job* getJobOwner(pid_t pid)
{
    for(job* j = head; j; j = j->next)                          //Recorre los trabajos.
        for(process* p = j->process_head; p; p = p->next)       //Recorre procesos en cada trabajo.
            if(p->pid == pid)                                   //Verifica si el PID coincide.
                return j;                                       //En caso de ser así, retorna el trabajo.

    return NULL;                                                //Retorna NULL.
}

void printJobStatus(job* job)
{
    printf("\n[%d]", job->jid);

    for(process* p = job->process_head; p; p = p->next)
    {
        printf(" %d %s %s", p->pid, string_status[p->status], p->argv[0]);

        if(p->next)
            printf(" |\n    ");
    }

    printf("\n");
}

void printPipe(job* job)
{
    char c;

    close(job->e_pipe[1]);
    close(job->io_pipe[1]);

    fcntl(job->e_pipe[0], F_SETFL, fcntl(job->e_pipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(job->io_pipe[0], F_SETFL, fcntl(job->io_pipe[0], F_GETFL) | O_NONBLOCK);

    while(read(job->e_pipe[0], &c, sizeof(c)) > 0)
        printf("%c", c);

    while(read(job->io_pipe[0], &c, sizeof(c)) > 0)
        printf("%c", c);

    printf("\n");

    close(job->e_pipe[0]);
    close(job->io_pipe[0]);
}