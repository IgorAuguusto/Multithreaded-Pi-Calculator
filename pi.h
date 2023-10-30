#pragma once

#include <pthread.h> // Requerido pela API Pthreads (POSIX Threads).

// Constantes lógicas.
#define TRUE 1
#define FALSE 0

// Acesso ao pipe
#define PIPE_READ 0
#define PIPE_WRITER 1

// Identificadores de processos 
#define PROCESS_ONE 1
#define PROCESS_TWO 2

// Número de processos
#define NUMBER_OF_PROCESS 2

// Tamanho do nome do arquivo.
#define FILE_NAME_SIZE 10

// Número de threads do processo filho.
#define NUMBER_OF_THREADS 16

// Tamanho padrão de string.
#define STRING_DEFAULT_SIZE 128

// Número de casas decimais do número pi.
#define DECIMAL_PLACES 9

// Número máximo de termos da série de Leibniz.
#define MAXIMUM_NUMBER_OF_TERMS 2000000000

// Número parcial de termos da série de Leibniz.
#define PARTIAL_NUMBER_OF_TERMS 125000000 

// Região 
#define LOCALE "pt_BR.utf8"

// Formato do tempo
#define TIME_FORMAT "%H:%M:%S"

// Representa um caractere nulo.
#define NULL_CHAR '\0'

//Formatações
#define NEW_LINE '\n'
#define EMPTY_LINE "\n\n"
#define TAB '\t'
#define DOT '.'
#define COMMA ','

// Constantes definidas para preencher a estrutura report do processo pai. 
#define REPORT_PROGRAM_NAME "Cálculo do Número π"
#define REPORT_MESSAGE1 "Criando os processos filhos pi1 e pi2..."
#define REPORT_MESSAGE2 "Processo pai (PID %d) finalizou sua execução."

// Constantes definidas para preencher a estrutura PocessReport dos processos filhos. 
#define PROCESS_REPORT_IDENTIFICATION  "- Processo Filho: pi%d (PID %d)"
#define PROCESS_REPORT_NUMBER_OF_THREADS "Nº de threads: %d"
#define PROCESS_REPORT_START "Início: %s" 
#define PROCESS_REPORT_END "Fim: %s"
#define PROCESS_REPORT_DURATION "Duração: %.2lf s"
#define PROCESS_REPORT_PI "Pi = %.9lf"

// Mensagens de erros.
#define ERROR_PIPE "Erro ao criar o pipe"
#define ERROR_PROCESS "ERRO: o processo filho não foi criado."
#define ERROR_FILE "Não foi possível abrir o arquivo."
#define ERROR_MALLOC "Erro na alocação de memória"

// Opção de abertura do arquivo.
#define FILE_OPENING_OPTION "w"

// Formatações de escrita em arquivo para o createFile.
#define FILE_NAME_PROCESS "pi%d.txt"
#define FILE_DESCRIPTION "Tempo em segundos das 16 threads do processo filho pi%d."
#define SHOW_FILE_NAME "Arquivo: %s%c"
#define SHOW_FILE_DESCRIPTION "Descrição: %s%s"
#define SHOW_TID "TID %d: %.2lf%c"
#define SHOW_TOTAL_TIME_THREAD "%cTotal: %.2lf s%c"

// Define uma string de tamanho padrão T, onde T é igual STRING_DEFAULT_SIZE.
typedef char String[STRING_DEFAULT_SIZE];

// Nome do arquivo.
typedef char FileName[FILE_NAME_SIZE];

// Estrutura do relatório a ser gerado pelo processo.
typedef struct {
   // Os comentários abaixo são apenas exemplos de valores a serem armazenados nos campos desta estrutura.
   String 
      identification, // Processo Filho: pi1 (PID 6924)
      numberOfThreads, // Nº de threads: 16
      start, // Início: 10:45:12
      end, // Fim: 10:45:21
      duration, // Duração: 9,59 s
      pi; // Pi = 3,141592653   
} ProcessReport;

// Estrutura do relatório a ser gerado pelo programa.
typedef struct {
   // Os comentários abaixo são apenas exemplos de valores a serem armazenados nos campos desta estrutura.
   String 
      programName, // Cálculo do Número π
      message1, // Criando os processos filhos pi1 e pi2...
      message2; // Processo pai (PID 6923) finalizou sua execução.
      
   ProcessReport processReport1, processReport2;     
} Report;

// Representa a identificação da thread e o seu tempo de execução em segundos.
typedef struct  {
   pthread_t threadID; // Identificação da thread obtida com pthread_create.
   pid_t tid;          // Identificação da thread obtida com gettid.
   double time; 
} Thread;

// Representa o resultado obtido por sumPartional
typedef struct {
   Thread thread;
   double sumPartional;
} ThreadResult;

// Relação de threads do processo filho.
typedef Thread Threads[NUMBER_OF_THREADS]; 

/* Cria o relatório do programa escrevendo na tela as informações da estrutura Report.
 * Retorna TRUE se o relatório foi escrito com sucesso ou FALSE se os dados da estrutura Report são vazios ou nulos.
*/
int createReport(const Report *report);

/* Cria o arquivo texto no diretório atual usando o nome do arquivo, a descrição e os dados do vetor do tipo Threads.
 * Retorna TRUE se o arquivo foi criado com sucesso ou FALSE se ocorreu algum erro.
 */
int createFile(const FileName fileName, String description, const Threads threads);

/* Cria uma thread para fazer a soma parcial de n termos da série de Leibniz. 
   Esta função deve usar a função sumPartial para definir qual a função a ser executada por cada uma das x threads 
   do programa, onde x é igual a NUMBER_OF_THREADS. 
   Retorna a identificação da thread.
*/
pthread_t createThread(unsigned int *terms); 

/* Realiza a soma parcial de n (n é definido por PARTIAL_NUMBER_OF_TERMS) termos da série de Leibniz
   começando em x, por exemplo, como PARTIAL_NUMBER_OF_TERMS é 125.000.000, então se x é:

             0 -> calcula a soma parcial de 0 até 124.999.999;
   125.000.000 -> calcula a soma parcial de 125.000.000 até 249.999.999;
   250.000.000 -> calcula a soma parcial de 250.000.000 até 374.999.999; 
       
   e assim por diante. 

   O resultado dessa soma parcial deve ser um valor do tipo double a ser retornado 
   por esta função para o processo que criou a thread.
*/
void* sumPartial(void *terms);

/* Calcula o número pi com n (n é definido por DECIMAL_PLACES) casas decimais usando o número máximo de
   termos da série de Leibniz, que é definido por MAXIMUM_NUMBER_OF_TERMS. Esta função deve criar x threads
   usando a função createThread, onde x é igual a NUMBER_OF_THREADS. 
*/
double calculationOfNumberPi(unsigned int terms);

/*
 * Esta função inicia o programa.
 * Retorna EXIT_SUCCESS.
 */
int pi();

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
void process();

/* A função 'fillReportProcessFather' é responsável por preencher a estrutura 'Report' com informações específicas,
   como o nome do programa, mensagens e o PID do processo atual.

   Parâmetros:
   - report: Ponteiro para a estrutura 'Report' a ser preenchida.

   Essa função é utilizada para inicializar a estrutura 'Report' com informações relevantes antes de usá-la em outras partes do programa.
*/
void fillReportProcessFather(Report* report);


/* A função 'createProcess' é responsável por criar um novo processo filho utilizando a função 'fork'.
   Ela cria um novo processo, que é uma cópia do processo pai, e retorna o PID (identificador de processo) do processo filho.

   A função também lida com erros na criação do processo, imprimindo uma mensagem de erro e encerrando o programa em caso de falha.
   Retorna:
   - O PID do processo filho criado.
*/
pid_t createProcess();


/* A função 'createPipe' é responsável por criar um pipe (tubo de comunicação) utilizando a função 'pipe'.
   O pipe permite a comunicação entre processos, fornecendo um meio de transferência de dados entre eles.

   Parâmetros:
   - pipe_fd: Array de inteiros de tamanho 2 para armazenar os descritores de arquivo do pipe.

   A função utiliza a função 'pipe' para criar o pipe e atribui os descritores de arquivo resultantes ao array 'pipe_fd'.
   Em caso de falha na criação do pipe, a função imprime uma mensagem de erro e encerra o programa com falha.
*/
int createPipe(int pipe_fd[2]);

/* A função 'processChild' é responsável por realizar tarefas específicas em um processo filho, identificado pelo parâmetro 'numberProcess'.
   Ela realiza o cálculo do valor de π e preenche um relatório de processo.

   Parâmetros:
   - numberProcess: Um valor que identifica o processo (PROCESS_ONE ou PROCESS_TWO).
   - pipe_fd: Um array de inteiros que representa o pipe para comunicação.
   - report: Ponteiro para a estrutura 'Report' que armazena informações do processo.

   A função calcula o valor de π usando 'calculationOfNumberPi' e mede o tempo de execução.
   Em seguida, preenche um relatório de processo 'ProcessReport' com os resultados e as informações de tempo.

*/
void processChild(int numberProcess, int pipe_fd[2], Report* report);

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
void fillProcessReportSun(ProcessReport* processReport, int numberProcess, char* startTimeStr, char* endTimeStr, double duration, double pi);

/* A função 'fillThreadTidAndTime' é responsável por preencher uma estrutura de thread 'Thread' com o ID da thread (TID) e o tempo de execução,
   obtidos a partir da estrutura 'ThreadResult'.

   Parâmetros:
   - threadResult: A estrutura 'ThreadResult' que contém o TID e o tempo de execução.
   - thread: Ponteiro para a estrutura 'Thread' a ser preenchida.
*/
void fillThreadTidAndTime(ThreadResult threadResult, Thread* thread);

/* A função 'replace' substitui todas as ocorrências do caractere 'replaced' pelo caractere 'replacer' em uma string.

   Parâmetros:
   - string: A string na qual as substituições serão feitas.
   - replacer: O caractere que substituirá 'replaced'.
   - replaced: O caractere que será substituído por 'replacer'.

   A função itera através da string e, para cada caractere igual a 'replaced', substitui-o por 'replacer'.
*/
void replace(String string, char replacer, char replaced);

/* A função 'replaceDotForComma' substitui todas as ocorrências do caractere ponto '.' pelo caractere vírgula ',' 
   em várias partes da estrutura 'Report' que contém informações sobre processos.

   Parâmetros:
   - report: Um ponteiro para a estrutura 'Report' que contém informações a serem processadas.

   A função utiliza a função 'replace' para realizar a substituição do ponto '.' pelo caractere vírgula ',' 
   nas strings de duração e PI em duas 'ProcessReport' dentro da estrutura 'Report'.
*/
void replaceDotForComma(Report* report);

/* A função 'calculateDuration' calcula a diferença de tempo entre duas estruturas 'struct timeval' e retorna a duração em segundos, incluindo a parte decimal representando milissegundos.

   Parâmetros:
   - startTime: Estrutura 'struct timeval' representando o tempo de início.
   - endTime: Estrutura 'struct timeval' representando o tempo de término.
*/
double calculateDuration(struct timeval startTime, struct timeval endTime);