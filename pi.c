#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <syscall.h>
#include <sys/wait.h>
#include <locale.h>
#include <sys/time.h>
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
    printf("\t%s\n", report->processReport1.start);
    printf("\t%s\n", report->processReport1.end);
    printf("\t%s", report->processReport1.duration);
    printf("\n\n\t%s\n", report->processReport1.pi);

    printf("\n%s\n\n", report->processReport2.identification);
    printf("\t%s\n\n", report->processReport2.numberOfThreads);
    printf("\t%s\n", report->processReport2.start);
    printf("\t%s\n", report->processReport2.end);
    printf("\t%s", report->processReport2.duration);
    printf("\n\n\t%s\n\n", report->processReport2.pi);

    return TRUE;
}//createReport();

/* Cria o arquivo texto no diretório atual usando o nome do arquivo, a descrição e os dados do vetor do tipo Threads.
 * Retorna TRUE se o arquivo foi criado com sucesso ou FALSE se ocorreu algum erro.
 */
int createFile(const FileName fileName, String description, const Threads *threads){
    
    FILE *arquivo;
    arquivo = fopen(fileName, "w");

    if (arquivo == NULL) {
        perror(ERROR_FILE);
        return FALSE;
    }

    fprintf(arquivo, SHOW_FILE_NAME, fileName);
    fprintf(arquivo, SHOW_FILE_DESCRIPTION, description);

    double totalTimeOfThreads = 0.0;
    for (int i = 0; i < NUMBER_OF_THREADS; i++){
        fprintf(arquivo, SHOW_TID, threads[0][i].tid, threads[0][i].time);
        totalTimeOfThreads += threads[0][i].time;
    }
    fprintf(arquivo, SHOW_TOTAL_TIME_THREAD, totalTimeOfThreads);    
    fclose(arquivo);

    return TRUE;
}//createFile();

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
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    ThreadResult* threadResult = (ThreadResult*)malloc(sizeof(ThreadResult));
    threadResult->sumPartional = 0.0;
    unsigned int *current = (unsigned int *)terms;
    for (unsigned int i = *current; i < *current + PARTIAL_NUMBER_OF_TERMS; i++) {
        double term = 1.0 / (2.0 * i + 1);
        if (i % 2 == 0) {
            threadResult->sumPartional += term;
        } else {
            threadResult->sumPartional -= term;
        }
    }
    
    gettimeofday(&endTime, NULL);
    time_t startTimeSeconds = startTime.tv_sec;
    time_t endTimeSeconds = endTime.tv_sec;
    double seconds = (double)(endTime.tv_sec - startTime.tv_sec);
    double milliseconds = (double)(endTime.tv_usec - startTime.tv_usec) / 1000.0;
    threadResult->thread.tid = syscall(SYS_gettid); 
    threadResult->thread.time = seconds + (milliseconds / 1000.0);
    
    // Liberando a região de memoria do argumento.
    free(terms);
   
    pthread_exit(threadResult);
}//sumPartial()

/* Cria uma thread para fazer a soma parcial de n termos da série de Leibniz. 
   Esta função deve usar a função sumPartial para definir qual a função a ser executada por cada uma das x threads 
   do programa, onde x é igual a NUMBER_OF_THREADS. 
   Retorna a identificação da thread.
*/
pthread_t createThread(unsigned int *terms) {
    pthread_t threadID; 
    unsigned int* sequenceNumber = (unsigned int *) malloc (sizeof(unsigned int));
    if (sequenceNumber == NULL) {
        perror(ERROR_MALLOC);
        exit(EXIT_FAILURE); 
    }
    *sequenceNumber = (*terms) * PARTIAL_NUMBER_OF_TERMS;
    pthread_create(&threadID, NULL, sumPartial, sequenceNumber);
    return threadID;
}//createThread()

/* A função 'fillThreadTidAndTime' é responsável por preencher uma estrutura de thread 'Thread' com o ID da thread (TID) e o tempo de execução,
   obtidos a partir da estrutura 'ThreadResult'.

   Parâmetros:
   - threadResult: A estrutura 'ThreadResult' que contém o TID e o tempo de execução.
   - thread: Ponteiro para a estrutura 'Thread' a ser preenchida.
*/
void fillThreadTidAndTime(ThreadResult threadResult, Thread* thread){
    thread->tid = threadResult.thread.tid;
    thread->time = threadResult.thread.time;
}//fillThreadTidAndTime()


/* Calcula o número pi com n (n é definido por DECIMAL_PLACES) casas decimais usando o número máximo de
   termos da série de Leibniz, que é definido por MAXIMUM_NUMBER_OF_TERMS. Esta função deve criar x threads
   usando a função createThread, onde x é igual a NUMBER_OF_THREADS. 
*/
double calculationOfNumberPi(unsigned int terms){
    Threads threads;
    void *result;
    ThreadResult threadResult;
    double pi = 0.0;

    for (unsigned int sequenceNumber = 0; sequenceNumber < NUMBER_OF_THREADS; sequenceNumber++) {
        threads[sequenceNumber].threadID = createThread(&sequenceNumber);
    }
    for (unsigned int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(threads[i].threadID, &result);
        threadResult = *(ThreadResult*)result;
        pi += threadResult.sumPartional;
        fillThreadTidAndTime(threadResult, &threads[i]);
        free(result);
    }
    
    FileName fileName;
    snprintf(fileName, STRING_DEFAULT_SIZE, FILE_NAME_PROCESS, terms);
    String description;
    snprintf(description, STRING_DEFAULT_SIZE, FILE_DESCRIPTION, terms); 
    createFile(fileName, description, &threads);
    return pi * 4;
}//calculationOfNumberPi();

/* A função 'fillProcessReportSun' é responsável por preencher uma estrutura 'ProcessReport' com informações específicas
   sobre a execução de um processo filho, incluindo seu número, tempo de início, tempo de término, duração e valor de π calculado.

   Parâmetros:
   - processReport: Ponteiro para a estrutura 'ProcessReport' a ser preenchida.
   - numberProcess: Número do processo (PROCESS_ONE ou PROCESS_TWO).
   - startTimeStr: String formatada representando o tempo de início.
   - endTimeStr: String formatada representando o tempo de término.
   - duration: Tempo de execução em segundos.
   - pi: Valor de π.
*/
void fillProcessReportSun(ProcessReport* processReport, int numberProcess, char* startTimeStr, char* endTimeStr, double duration, double pi){
    snprintf(processReport->identification, STRING_DEFAULT_SIZE, PROCESS_REPORT_IDENTIFICATION, numberProcess, getpid());
    sprintf(processReport->numberOfThreads, PROCESS_REPORT_NUMBER_OF_THREADS, NUMBER_OF_THREADS);
    snprintf(processReport->start, STRING_DEFAULT_SIZE, PROCESS_REPORT_START, startTimeStr);
    snprintf(processReport->end, STRING_DEFAULT_SIZE, PROCESS_REPORT_END, endTimeStr);
    snprintf(processReport->duration, STRING_DEFAULT_SIZE, PROCESS_REPORT_DURATION, duration);
    snprintf(processReport->pi, STRING_DEFAULT_SIZE, PROCESS_REPORT_PI, pi);
}//fillProcessReportSun()

/* A função 'processChild' é responsável por realizar tarefas específicas em um processo filho, identificado pelo parâmetro 'numberProcess'.
   Ela realiza o cálculo do valor de π e preenche um relatório de processo.

   Parâmetros:
   - numberProcess: Um valor que identifica o processo (PROCESS_ONE ou PROCESS_TWO).
   - pipe_fd: Um array de inteiros que representa o pipe para comunicação.
   - report: Ponteiro para a estrutura 'Report' que armazena informações do processo.

   A função calcula o valor de π usando 'calculationOfNumberPi' e mede o tempo de execução.
   Em seguida, preenche um relatório de processo 'ProcessReport' com os resultados e as informações de tempo.

*/
void processChild(int numberProcess, int pipe_fd[2], Report* report) {
    ProcessReport processReport;
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    double pi = calculationOfNumberPi(numberProcess);

    gettimeofday(&endTime, NULL);

    double seconds = (double)(endTime.tv_sec - startTime.tv_sec);
    double milliseconds = (double)(endTime.tv_usec - startTime.tv_usec) / 1000.0;
    double duration = seconds + (milliseconds / 1000.0);

    char startTimeStr[9];
    char endTimeStr[9];
    struct tm startTm, endTm;
    time_t startTimeSeconds = startTime.tv_sec;
    time_t endTimeSeconds = endTime.tv_sec;
    gmtime_r(&startTimeSeconds, &startTm);
    gmtime_r(&endTimeSeconds, &endTm);

    strftime(startTimeStr,STRING_DEFAULT_SIZE, TIME_FORMAT, &startTm);
    strftime(endTimeStr, STRING_DEFAULT_SIZE, TIME_FORMAT, &endTm);

    fillProcessReportSun(&processReport, numberProcess, startTimeStr, endTimeStr, duration, pi);

    if (numberProcess == PROCESS_ONE) {
        // Processo filho 1 (pi1)
        close(pipe_fd[PIPE_READ]); 
        write(pipe_fd[PIPE_WRITER], &processReport, sizeof(ProcessReport));
        close(pipe_fd[PIPE_WRITER]);
        exit(EXIT_SUCCESS);

    } else if (numberProcess == PROCESS_TWO) {
        // Processo filho 2 (pi2)
        close(pipe_fd[PIPE_WRITER]); 
        read(pipe_fd[PIPE_READ], &report->processReport1, sizeof(ProcessReport));
        close(pipe_fd[PIPE_READ]);
        report->processReport2 = processReport;
        createReport(report);
        exit(EXIT_SUCCESS);
    }
}//processChild()

/* A função 'createProcess' é responsável por criar um novo processo filho utilizando a função 'fork'.
   Ela cria um novo processo, que é uma cópia do processo pai, e retorna o PID (identificador de processo) do processo filho.

   A função também lida com erros na criação do processo, imprimindo uma mensagem de erro e encerrando o programa em caso de falha.
   Retorna:
   - O PID do processo filho criado.
*/
pid_t createProcess() {
    // Cria o processo filho.
    pid_t pid = fork();
			
	// Verifica se ocorreu um erro na criação do processo filho.
	if (pid < 0) {
		perror(ERROR_PROCESS);
		exit(EXIT_FAILURE);
	}
	return pid;
}//createProcess()

/* A função 'createPipe' é responsável por criar um pipe (tubo de comunicação) utilizando a função 'pipe'.
   O pipe permite a comunicação entre processos, fornecendo um meio de transferência de dados entre eles.

   Parâmetros:
   - pipe_fd: Array de inteiros de tamanho 2 para armazenar os descritores de arquivo do pipe.

   A função utiliza a função 'pipe' para criar o pipe e atribui os descritores de arquivo resultantes ao array 'pipe_fd'.
   Em caso de falha na criação do pipe, a função imprime uma mensagem de erro e encerra o programa com falha.
*/
int createPipe(int pipe_fd[2]) {
    if (pipe(pipe_fd) == -1) {
        perror(ERROR_PIPE);
        exit(EXIT_FAILURE);
    }
    return TRUE;
}//createPipe()

/* A função 'fillReportProcessFather' é responsável por preencher a estrutura 'Report' com informações específicas,
   como o nome do programa, mensagens e o PID do processo atual.

   Parâmetros:
   - report: Ponteiro para a estrutura 'Report' a ser preenchida.

   Essa função é utilizada para inicializar a estrutura 'Report' com informações relevantes antes de usá-la em outras partes do programa.
*/
void fillReportProcessFather(Report* report){
    snprintf(report->programName, STRING_DEFAULT_SIZE, REPORT_PROGRAM_NAME);
    snprintf(report->message1, STRING_DEFAULT_SIZE, REPORT_MESSAGE1);
    snprintf(report->message2, STRING_DEFAULT_SIZE, REPORT_MESSAGE2, getpid());
}//fillReportProcessFather()

/* A função 'process' é responsável por coordenar a execução de múltiplos processos e a criação de um pipe para comunicação entre eles.
   Ela segue a lógica de criação de dois processos filho.

   - Cria uma estrutura 'Report' para armazenar informações.
   - Chama 'fillReportProcessFather' para preencher a estrutura 'Report'.
   - Cria um pipe usando 'createPipe' para permitir a comunicação entre os processos filhos.
   - Cria dois processos filhos usando 'createProcess'.
   - No primeiro filho, fecha o descritor de leitura do pipe e executa 'processChild' com a identificação 'PROCESS_ONE' e a estrutura 'Report'.
   - No segundo filho, fecha o descritor de escrita do pipe e executa 'processChild' com a identificação 'PROCESS_TWO' e a estrutura 'Report'.
   - No processo pai, fecha ambos os descritores do pipe e sai com EXIT_SUCCESS.
*/
void process() {
    Report report;

    fillReportProcessFather(&report);

    int pipe_fd[2]; 
    createPipe(pipe_fd);

    pid_t process1 = createProcess();
    pid_t process2 = createProcess();

    if (process1 == 0 && process2 != 0) {
        close(pipe_fd[PIPE_READ]); 
        processChild(PROCESS_ONE, pipe_fd, &report); 
    }
    if (process2 == 0 && process1 != 0) {
        close(pipe_fd[PIPE_WRITER]); 
        processChild(PROCESS_TWO, pipe_fd, &report); 
    }
    else if (process1 != 0 && process2 != 0) {
        close(pipe_fd[PIPE_READ]);
        close(pipe_fd[PIPE_WRITER]);
        exit(EXIT_SUCCESS);
    }
}//process()

/*
 * Esta função inicia o programa.
 * Retorna EXIT_SUCCESS.
 */
int pi(){
    setlocale(LC_ALL, "pt_BR.utf8");
    process();
    return  EXIT_SUCCESS;
}//pi()

int main(){
    return pi();
}//main()