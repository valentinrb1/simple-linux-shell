/**
 * @file myShell.c
 * @author Robledo, Valentín
 * @brief Implementación de myShell
 * @version 1.0
 * @date Septiembre de 2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../inc/myShell.h"

int main(int argc, char* argv[])
{   
    FILE* file;
    initControlShell();

    if(argc == 2)
    {
        file = fopen(argv[1],"r");
        if(file == NULL)
        {
            perror("Error trying to open file."); 
            exit(EXIT_FAILURE);
        }

        shellLogic(file);
    }
    else
    {
        file = stdin;
        
        clear();
        shellLogic(file);
    }

    return 0;
}

void shellLogic(FILE* file)
{
    while(1)
    {   
        if(file == stdin)                       //Verifica si el archivo es stdin.
            printPrompt();                      //Imprime la "Prompt".
        
        char *line = malloc(100);               //Se estima un largo de comando no mayor a 100 caracteres.
        int result = readInput(file, line);     

        if(result == READ_FINISH)               //Verifica si terminó la lectura.
        {
            fclose(file);                       //Cierra el archivo.
            while(head);                        //Espera a que no queden trabajos en segundo plano.
            exit(EXIT_SUCCESS);
        }
        else if(result == READ_OMIT)            //Verifica si se debe omitir la lectura.
        {
            free(line);                         //Libera memoria.
            continue;
        }

        u_int8_t n;
        char** argv_list = getArguments(line, &n);

        if(!strcmp(line, "clr"))
            clear();
        else if(!strcmp(line, "quit"))
            exit(EXIT_SUCCESS);
        else if(!strcmp(line, "jobs"))
            printJobs();
        else if(!strcmp(argv_list[0], "cd"))
            changeDirectory(argv_list);
        else if(!strcmp(argv_list[0], "echo"))
            printEchoCommand(argv_list, n);
        else
            programInvocation(argv_list, n);

        free(line);
    }
}

void printPrompt()
{
    gethostname(hostname, sizeof(hostname));

    printf("%s@%s:~%s$ ", getenv("USER"), hostname, getenv("PWD"));
}

void printEchoCommand(char** argv_list, u_int8_t w_count)
{
    for(u_int8_t n = 1; n < w_count; n++)
    {
        if(strchr(argv_list[n], '$'))                    //Verifico si contiene '$'
        {
            char* environment_var = strtok(argv_list[n], "$");  //Trunca la palabra quitando '$', dejando sola a la variable de entorno.

            printf("%s ", getenv(environment_var));      //Imprime la variable de entorno.
        }
        else                                             //Si no posee '$'.
            printf("%s ", argv_list[n]);                 //Imprime el comentario.
        
    }

    printf("\n");
}

void changeDirectory(char** argv_list)
{
    if(!strcmp(argv_list[1], "-"))
    {
        chdir(getenv("OLDPWD"));

        char* new_path = getcwd(path_buf, sizeof(path_buf));
        setenv("PWD", new_path, 1);

        printf("%s \n", getenv("PWD"));
    }
    else
    {
        if(chdir(argv_list[1]) != 0)
            perror("Change Directory failed.");
        else
        {
            setenv("OLDPWD", getenv("PWD"), 1);
            chdir(argv_list[1]);

            char* new_path = getcwd(path_buf, sizeof(path_buf));
            setenv("PWD", new_path, 1);

            printf("New directory: %s \n", getenv("PWD"));
        }
    }
}

void programInvocation(char** argv_list, u_int8_t w_count)
{   
    process* p = malloc(sizeof(process));
    p->argc = w_count;
    p->argv = argv_list;
    p->next = NULL;
    p->pid = -1;
    p->status = PROCESS_NEW;

    job* j = malloc(sizeof(job));
    j->next = NULL;
    j->jid = -1;
    j->pgid = -1;
    j->process_head = p;

    if(!strcmp(p->argv[p->argc - 1], "&"))
    {
        j->exemode = BACKGROUND;
        
        free(p->argv[p->argc - 1]);
        p->argv[p->argc - 1] = NULL;
    }
    else
        j->exemode = FOREGROUND;

    launchJob(j);
}

char** getArguments(char* line, u_int8_t* n)
{
    u_int8_t w_count = 0;                                //Contador de palabras.

    char** argv_list = malloc(sizeof(char*));            //Reservo memoria para almacenar una palabra.
    argv_list[w_count] = malloc(sizeof(char));           //Reservo memoria para al menos un caracter correspondiente a la palabra.

    char* word = strtok(line, " ");                      //Trunco la linea de texto cada un espacio y guardo la palabra truncada.

    while(word != NULL)                                  //Recorro la linea de texto truncando palabras.
    {
        argv_list[w_count] = realloc(argv_list[w_count], strlen(word)*sizeof(char));  //Re asigno memoria para la primer palabra.

        strcpy(argv_list[w_count], word);                //Copio la palabra dentro de argv_list (lista de argumentos).

        w_count++;                                       //Sumo en 1 el contador de palabras.

        argv_list = realloc(argv_list, (w_count + 1)*sizeof(char*));  //Re asigno memoria para una nueva palabra.

        word = strtok(NULL, " ");
    }

    argv_list[w_count] = NULL;                           //La última palabra como NULL.

    *n = w_count;

    return argv_list;
}

void printJobs()
{
    for(job* j = head; j; j = j->next)
    {    
        printJobStatus(j);
        printf("\n");
    }
}

void freeArguments(char** argv_list, u_int8_t w_count)
{
    for(int i = 0; i < w_count; i++)                     //Recorre todas las palabras del array.
    {
        free(argv_list[i]);                              //Libera la memoria.
    }

    free(argv_list);                                     //Libera la memoria.
}

int readInput(FILE* file, char* line)
{
    char* result = fgets(line, 100, file);

    if(result == NULL && *line != '\n' && file != stdin) //Verifica si se llega a un EOF en la lectura de un archivo pasado por argumento.
        return READ_FINISH;                              //Se retorna un aviso para finalizar ejecución  .                                                                      
    
    if(result == NULL || *line == '\n')                  //Verifica si se obtiene solamente un caracter blanco.
        return READ_OMIT;                                //Se retorna un aviso para ignorar el comando.

    if(line[strlen(line) - 1] != '\n')                   //verifica si el último caracter es un salto de línea.
        line[strlen(line)] = '\0';                       //Se agrega caracter de finalización al final del string obtenido.
    else
        line[strlen(line) - 1] = '\0';                   //Caso contrario, finalización en el mismo caracter.

    for(int i = 0; i <= strlen(line) - 1; i++)           //Reemplazo de tabulaciones por espacios.
        if(line[i] == '\t') 
            line[i] = ' ';
 
    return READ_OK;                                      //Si todo sale bien, se retorna una confirmación
}