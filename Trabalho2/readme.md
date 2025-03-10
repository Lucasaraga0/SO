# Trabalho 2 
Criar um mini sistema com gerenciamento de memória e sistema de arquivos próprios.

A. O sistema de arquivos:

Pode ter apenas o diretório raiz e arquivos com nomes de tamanho limitado (ganha ponto extra quem implementar uma árvore de diretórios). Ele deve ser implementado dentro de um arquivo de 1 GB criado no sistema hospedeiro.

B. O sistema deve aceitar os seguintes comandos:
1. criar nome tam

Cria um arquivo com nome "nome" (pode ser limitado o tamanho do nome) com uma lista aleatória de números inteiros positivos de 32 bits. O argumento "tam" indica a quantidade de números. A lista pode ser guardada em formato binário ou como string (lista de números legíveis separados por algum separador, como vírgula ou espaço).


2. apagar nome
Apaga o arquivo com o nome passado no argumento.

3. listar

Lista os arquivos no diretório. Deve mostrar, ao lado de cada arquivo, o seu tamanho em bytes. Ao final, deve mostrar também o espaço total do "disco" e o espaço disponível.


4. ordenar nome

Ordena a lista no arquivo com o nome passado no argumento. O algoritmo de ordenação a ser utilizado é livre, podendo inclusive ser utilizado alguma implementação de biblioteca existente. Ao terminar a ordenação, deve ser exibido o tempo gasto em ms.


5. ler nome inicio fim

Exibe a sublista de um arquivo com o nome passado com o argumento. O intervalo da lista é dado pelos argumentos inicio e fim.


6. concatenar nome1 nome2

Concatena dois arquivos com os nomes dados de argumento. O arquivo concatenado pode ter um novo nome predeterminado ou simplesmente pode assumir o nome do primeiro arquivo. Os arquivos originais devem deixar de existir.

C. Gerenciamento de memória

Deve ser alocado uma "Huge Page" (no Linux, ou equivalente em outro S.O.) de 2 MBytes para ser realizado o trabalho de ordenação. Nada mais de memória pode ser utilizado para isso, ou seja, deve ser implementada uma paginação com o disco. Para tal, você pode "particionar" o seu disco virtual, mantendo uma área fora do sistemas de arquivos para utilizar na troca de páginas, ou criar um arquivo para paginação dentro do seu sistema de arquivos.

A memória paginada que você deve implementar só é necessária para o trabalho da ordenação, isto é, para manter a lista de números. Qualquer outra memória de trabalho necessária, como aquelas para variáveis, para a pilha, ou para carregar a estrutura do seu sistema de arquivos, pode ser memória comum do sistema hospedeiro.