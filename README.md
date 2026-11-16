# Algoritmo do Banqueiro
Eduardo Boxwell Simões - ebs2@cesar.school

Como compilar e rodar:
Precisa de gcc e make (testei no Ubuntu pelo WSL)
```
make
./banker 10 5 7
```
Cada numero passado é a quantidade de instancias de um recurso. O programa le o `customer.txt` e o
`commands.txt` da pasta e escreve a saida no `result.txt`. Pra apagar o executavel: `make clean`

Entradas:
`customer.txt` - cada linha é um cliente com o maximo que ele pode pedir de cada recurso, separado por virgula:

```
7,5,3
3,2,2
9,0,2
```

`commands.txt` - um comando por linha:
- `RQ cliente r0 r1 ...` pede recursos
- `RL cliente r0 r1 ...` libera recursos
- `*` imprime as matrizes (MAXIMUM, ALLOCATION, NEED) e o AVAILABLE

Erros:
O programa confere tudo antes de comecar. Se tiver algo errado ele mostra uma mensagem de erro
(com a linha do arquivo quando da) e fecha sem criar o result.txt. Alguns casos que ele pega:
- numero de argumentos diferente do numero de recursos dos arquivos
- argumento que nao é numero ou é negativo
- customer.txt ou commands.txt que nao existe ou nao abre
- arquivo vazio, letra no lugar de numero, campo vazio, linha vazia no meio
- comando que nao é RQ/RL/*, cliente que nao existe, * com coisa depois
Se o cliente tentar liberar mais do que tem, o pedido é negado e aparece no result.txt.
Evidencias:
O `evidencias.log` tem a saida dos testes que eu fiz: o caso do enunciado e varios casos de erro.
