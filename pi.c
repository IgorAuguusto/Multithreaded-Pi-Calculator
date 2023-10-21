#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <time.h>
#include <sys/wait.h>
#include "pi.h"

/* Cria o relatório do programa escrevendo na tela as informações da estrutura Report.
 * Retorna TRUE se o relatório foi escrito com sucesso ou FALSE se os dados da estrutura Report são vazios ou nulos.
*/
int createReport(const Report *report) {
    if (report == NULL) {
        return FALSE;
    }

    if (report->processReport1.identification[0] == '\0' || report->processReport2.identification[0] == '\0') {
        return FALSE;
    }

    printf("\n\n%s\n", report->programName);
    printf("\n%s\n", report->message1);
    printf("%s\n", report->message2);

    printf("\n%s\n\n", report->processReport1.identification);
    printf("\t%s\n\n", report->processReport1.numberOfThreads);
    printf("\t%s", report->processReport1.start);
    printf("\t%s", report->processReport1.end);
    printf("\t%s", report->processReport1.duration);
    printf("\n\t%s\n", report->processReport1.pi);

    printf("\n%s\n\n", report->processReport2.identification);
    printf("\t%s\n\n", report->processReport2.numberOfThreads);
    printf("\t%s", report->processReport2.start);
    printf("\t%s", report->processReport2.end);
    printf("\t%s", report->processReport2.duration);
    printf("\n\t%s\n", report->processReport2.pi);

    return TRUE;
}//createReport();

/* Cria o arquivo texto no diretório atual usando o nome do arquivo, a descrição e os dados do vetor do tipo Threads.
 * Retorna TRUE se o arquivo foi criado com sucesso ou FALSE se ocorreu algum erro.
 */
int createFile(const FileName fileName, String description, const Threads *threads){
    return 0;
}//createFile();

/* Cria uma thread para fazer a soma parcial de n termos da série de Leibniz. 
   Esta função deve usar a função sumPartial para definir qual a função a ser executada por cada uma das x threads 
   do programa, onde x é igual a NUMBER_OF_THREADS. 
   Retorna a identificação da thread.
*/
pthread_t createThread(unsigned int *terms) {
    pthread_t threadID; // Identificação da thread.
    // Cria a thread filha para executar a função sumPartial.
    ThreadArgs *threadArgs = (ThreadArgs *)malloc(sizeof(ThreadArgs));
    threadArgs->termValue = *terms * PARTIAL_NUMBER_OF_TERMS; 
    printf("\nTERMS: %d\n", threadArgs->termValue);
    pthread_create(&threadID, NULL, sumPartial, threadArgs);
    free(threadArgs);
    return threadID;
}

/* Realiza a soma parcial de n (n é definido por PARTIAL_NUMBER_OF_TERMS) termos da série de Leibniz
   começando em x, por exemplo, como PARTIAL_NUMBER_OF_TERMS é 125.000.000, então se x é:

             0 -> calcula a soma parcial de 0 até 124.999.999;
   125.000.000 -> calcula a soma parcial de 125.000.000 até 249.999.999;
   250.000.000 -> calcula a soma parcial de 250.000.000 até 374.999.999; 
       
   e assim por diante. 

   O resultado dessa soma parcial deve ser um valor do tipo double a ser retornado 
   por esta função para o processo que criou a thread.
*/
void* sumPartial(void *terms) {
    double* sum = (double*)malloc(sizeof(double));
    *sum = 0.0;
    ThreadArgs *threadArgs = (ThreadArgs *)terms;
    int current = threadArgs->termValue;
    for (unsigned int i = current; i < current + PARTIAL_NUMBER_OF_TERMS; i++) {
        double term = 1.0 / (2.0 * i + 1);
        if (i % 2 == 0) {
            *sum += term;
        } else {
            *sum -= term;
        }
    }
    pthread_exit(sum);
}

/* Calcula o número pi com n (n é definido por DECIMAL_PLACES) casas decimais usando o número máximo de
   termos da série de Leibniz, que é definido por MAXIMUM_NUMBER_OF_TERMS. Esta função deve criar x threads
   usando a função createThread, onde x é igual a NUMBER_OF_THREADS. 
*/
double calculationOfNumberPi(unsigned int terms){
    pthread_t threads[NUMBER_OF_THREADS];
    void *result;
    double pi = 0.0;

    for (unsigned int i = 0; i < NUMBER_OF_THREADS; i++) {
        threads[i] = createThread(&i);
    }
    for (unsigned int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(threads[i], &result);
        pi += *(double *) result; 
        free(result);
    }
    return pi;
}//calculationOfNumberPi();


void processChild(int numberProcess, int pipe_fd[2], Report* report) {
    ProcessReport processReport;
    time_t startTime;
    time(&startTime);

    snprintf(processReport.identification, STRING_DEFAULT_SIZE,
             "- Processo Filho: pi%d (PID %d)", numberProcess + 1, getpid());

    sprintf(processReport.numberOfThreads, "No de threads: 16");
    
    double pi = calculationOfNumberPi(PARTIAL_NUMBER_OF_TERMS);

    time_t endTime;
    time(&endTime);

    double duration = difftime(endTime, startTime);

    char startTimeStr[9];
    char endTimeStr[9];
    struct tm* startTm = localtime(&startTime);
    struct tm* endTm = localtime(&endTime);
    
    strftime(startTimeStr, sizeof(startTimeStr), "%H:%M:%S", startTm);
    strftime(endTimeStr, sizeof(endTimeStr), "%H:%M:%S", endTm);
    
    snprintf(processReport.start, STRING_DEFAULT_SIZE, "Início: %s\n", startTimeStr);
    snprintf(processReport.end, STRING_DEFAULT_SIZE, "Fim: %s\n", endTimeStr);
    snprintf(processReport.duration, STRING_DEFAULT_SIZE, "Duração: %.2lf s\n", duration);
    snprintf(processReport.pi, STRING_DEFAULT_SIZE, "Pi = %.9f", pi);

    if (numberProcess == PROCESS_ONE) {
        // Processo filho 1 (pi1)
        close(pipe_fd[0]); 
        write(pipe_fd[1], &processReport, sizeof(ProcessReport));
        close(pipe_fd[1]);
        exit(EXIT_SUCCESS);

    } else if (numberProcess == PROCESS_TWO) {
        // Processo filho 2 (pi2)
        close(pipe_fd[1]); 
        read(pipe_fd[0], &report->processReport1, sizeof(ProcessReport));
        close(pipe_fd[0]);
        report->processReport2 = processReport;
        createReport(report);
        exit(EXIT_SUCCESS);
    }
}




void process() {
    Report report;

    snprintf(report.programName, STRING_DEFAULT_SIZE, "Cálculo do Número π");
    snprintf(report.message1, STRING_DEFAULT_SIZE, "Criando os processos filhos pi1 e pi2...");
    snprintf(report.message2, STRING_DEFAULT_SIZE,"Processo pai (PID %d) finalizou sua execução.", getpid());

    int pipe_fd[2]; 

    if (pipe(pipe_fd) == -1) {
        perror("Erro ao criar o pipe");
        exit(EXIT_FAILURE);
    }

    pid_t process1 = createProcess();
    pid_t process2 = createProcess();

    if (process1 == 0 && process2 != 0) {
        // Processo filho 1
        close(pipe_fd[0]); // Fecha o descritor de leitura
        processChild(PROCESS_ONE, pipe_fd, &report); // Passe a estrutura Report como ponteiro
    }
    if (process2 == 0 && process1 != 0) {
        // Processo filho 2
        close(pipe_fd[1]); // Fecha o descritor de escrita
        processChild(PROCESS_TWO, pipe_fd, &report); // Passe a estrutura Report como ponteiro
    }
    else if (process1 != 0 && process2 != 0) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        exit(EXIT_SUCCESS);
    }
}

/*
 * Esta função inicia o programa.
 * Retorna EXIT_SUCCESS.
 */
int pi(){
    process();
    return  EXIT_SUCCESS;
};



pid_t createProcess() {
    // Cria o processo filho.
    pid_t pid = fork();
			
	// Verifica se ocorreu um erro na criação do processo filho.
	if (pid < 0) {
		printf("\nERRO: o processo filho não foi criado.\n\n");
		exit(EXIT_FAILURE);
	}
	return pid;
}



int main(){
    return pi();
}