#Simulação de Crescimento Populacional Exponencial de Colônias de Bactérias com Threads e Prevenção de Impasse

Para executar os programas em C neste mesmo diretório, utilize o compilador de linguagem C de sua preferência, compilando-os com os comandos a seguir, que incluem a biblioteca de threads e de matemática:

###Versão Com Impasses
gcc -o v1ComImpasse v1ComImpasse.c -pthread -lm

###Versão Sem Impasses
gcc -o v2SemImpasse v2SemImpasse.c -pthread -lm


##Observação
Os códigos já possuem valores carregados para simulação.
Pode ser alterado o número de colônias e a quantidade de recursos de cada tipo (nutrientes e espaço) no começo do código, nas linhas 9 a 11.
Também, o código está com valores estáticos de população inicial, taxa de crescimento e tempo de simulação das colônias para facilitar a testagem, caso queira que o usuário insira os valores, basta comentar as linhas 49 a 51 e tirar o comentário das linhas 54 a 60.