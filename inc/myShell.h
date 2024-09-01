/**
 * @file myShell.h
 * @author Robledo, Valentín
 * @brief Encabezado de myShell.c
 * @version 1.0
 * @date Septiembre de 2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYSHELL_H__
#define __MYSHELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include "jobControl.h"

#define clear() printf("\033[H\033[J")
#define READ_OK 1                           //valor de retorno lectura de input correcta
#define READ_FINISH 2                       //valor de retorno input determina que se debe finalizar
#define READ_OMIT 3                         //valor de retorno input determina se debe omitir la lectura

char path_buf[PATH_MAX];
char hostname[HOST_NAME_MAX];

/**
 * @brief Implementa la lógica de la shell.
 * 
 * 
 * @param file Archivo sobre el cual se leen las palabras.
 */
void shellLogic(FILE* file);

/**
 * @brief Lee la entrada de la shell y almaneca la línea de texto.
 * 
 * Genera un array unidimensional donde almacena la línea de texto ingresada.
 * 
 * @param file Archivo sobre el cual se leen las palabras.
 * @param line Dirección de memoria donde se almacena la linea de texto.
 * @return int* Dirección de memoria donde se almacena el código de confirmación.
 */
int readInput(FILE* file, char* line);

/**
 * @brief Separa las palabras de una linea de texto.
 * 
 * Genera un array bidimensional con las palabras contenidas en una línea de texto,
 * separandolas por cada espacio.
 * 
 * @param line Array que contiene la linea de texto ingresada en la shell.
 * @param n Puntero donde se va a almacenar la cantidad de palabras obtenidas.
 * @return char** dirección de memoria del array bidimensional donde se encuentran las palabras.
 */
char** getArguments(char* line, u_int8_t* n);

/**
 * @brief Libera el espacio de memoria donde se almacenan las palabras.
 * 
 * @param argv_list dirección de memoria donde se encuenta el array bidimensional de palabras.
 * @param n número de palabras almacenadas en el array.
 *
 */
void freeArguments(char** argv_list, u_int8_t w_count);

/**
 * @brief Imprime la "Prompt" de la shell.
 * 
 */
void printPrompt();

/**
 * @brief Imprime variable de entorno.
 * 
 * Imprime la variable de entorno que se ingresa luego del comando "echo $" en la shell.
 * 
 * @param line Array que contiene la linea de texto ingresada en la shell.
 *
 */
void printEchoCommand(char **argv_list, u_int8_t w_count);

/**
 * @brief Realiza un cambio de directorio.
 * 
 * Realiza un cambio de directorio e imprime por pantalla el nuevo directorio ingresado 
 * luego del comando "cd". En caso de ingresar "cd -" vuelve al directorio anterior.
 * 
 * @param line Array que contiene la linea de texto ingresada en la shell.
 *
 */
void changeDirectory(char **argv_list);

/**
 * @brief Imprime estado de los trabajos.
 *
 */
void printJobs();

/**
 * @brief Realiza un cambio de directorio.
 * 
 * Realiza un cambio de directorio e imprime por pantalla el nuevo directorio ingresado 
 * luego del comando "cd". En caso de ingresar "cd -" vuelve al directorio anterior.
 * 
 * @param line Array que contiene la linea de texto ingresada en la shell.
 *
 */
void programInvocation(char **argv_list, u_int8_t w_count);

#endif //__MYSHELL_H__