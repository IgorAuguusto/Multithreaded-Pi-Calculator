# Multithreaded-Pi-Calculator


## Programa de Cálculo do Número π
Este programa foi desenvolvido como parte de um trabalho acadêmico que utiliza os conceitos de sistemas operacionais para calcular o número π com nove casas decimais. Ele implementa a Fórmula de Leibniz para π e utiliza processos e threads para acelerar o cálculo.

## Descrição
O programa realiza o cálculo do número π com nove casas decimais usando a Fórmula de Leibniz. Ele calcula o valor de π com a precisão desejada usando um total de 2 bilhões de termos da série de Leibniz.

## O programa é dividido em três processos:

1. O processo pai, que cria os processos filhos e exibe uma mensagem.
2. O primeiro processo filho (pi1), que cria 16 threads para calcular uma parte dos termos da série.
3. O segundo processo filho (pi2), que também cria 16 threads para calcular outra parte dos termos da série.
<br>
Cada thread calcula a soma parcial de 125 milhões de termos da série de Leibniz e armazena o resultado em uma variável local do processo filho. Não são usadas variáveis globais para evitar problemas de concorrência.

Após a conclusão de cada thread, o processo filho calcula a soma total a partir das somas parciais geradas pelas threads, resultando nos 2 bilhões de termos mencionados. Além disso, o processo filho mede o tempo de execução, considerando todo o comportamento multithread do programa.

## Relatório
O programa gera um relatório que contém as seguintes informações para cada processo filho:

* Nome do processo.
* Número de threads.
* Hora de início e término.
* Duração da execução em segundos.
* Valor de π calculado com nove casas decimais.

![image](https://github.com/IgorAuguusto/Multithreaded-Pi-Calculator/assets/82172424/228e309b-aff8-4ff8-a35a-1695adec362f)



<br>
Além disso, cada processo filho cria um arquivo de texto no diretório atual (pi1.txt ou pi2.txt) que contém os tempos gastos por cada thread para calcular as somas parciais dos termos da série de Leibniz.

![image](https://github.com/IgorAuguusto/Multithreaded-Pi-Calculator/assets/82172424/38308542-82ee-438e-9a8a-bdee07beca75)

![image](https://github.com/IgorAuguusto/Multithreaded-Pi-Calculator/assets/82172424/a58c2ad6-1755-4739-a31c-0e73fef8e4fd)



## Como Executar
Para executar o programa, basta compilar o código-fonte fornecido e executar o binário resultante. O programa criará os processos e threads automaticamente e exibirá o relatório na saída padrão. Os arquivos de texto com os tempos das threads também serão criados no diretório atual.

## Requisitos
O programa foi desenvolvido em C e requer um ambiente de desenvolvimento C compatível, como GCC, para compilação. Certifique-se de ter as bibliotecas padrão de C instaladas em seu sistema.
