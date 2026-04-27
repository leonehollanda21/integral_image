# Integral Image (Summed Area Table)

Equipe:
- Leone Barbosa Hollanda
- Joao Roberto Fernandes Magalhaes

## Visao Geral

Este projeto implementa em C o algoritmo **Integral Image**, tambem conhecido como
**Summed Area Table**. A ideia principal e fazer um pre-processamento em uma matriz
bidimensional para que a soma de qualquer sub-regiao retangular possa ser obtida em
**tempo constante O(1)**.

No pre-processamento, cada posicao da matriz integral armazena a soma acumulada de
todos os elementos entre `(0,0)` e `(r,c)`. Depois disso, uma consulta de soma em
um retangulo e resolvida com apenas quatro acessos a essa matriz.

Referencia:
- https://en.wikipedia.org/wiki/Summed-area_table

## Objetivo do Projeto

O foco do trabalho e mostrar uma implementacao:
- iterativa
- sem recursividade
- sem alocacao dinamica
- com matrizes estaticas
- adequada para contexto embarcado

Ao mesmo tempo, o projeto inclui uma camada de testes no PC para validar se o
algoritmo em C esta correto.

## Ideia do Algoritmo

Se `img[r][c]` representa a imagem original, a matriz integral guarda:

```text
integral[r][c] = soma de todos os pixels de (0,0) ate (r,c)
```

Uma forma equivalente da recorrencia e:

```text
integral[r][c] = img[r][c]
               + integral[r-1][c]
               + integral[r][c-1]
               - integral[r-1][c-1]
```

No codigo em C, a construcao foi implementada com soma acumulada por linha:

```text
integral[r][c] = soma_da_linha_ate_c + integral[r-1][c]
```

As duas formas sao equivalentes.

Depois que a matriz integral esta pronta, a soma de qualquer regiao
`(r1,c1) -> (r2,c2)` e dada por:

```text
soma = A - B - C + D
```

onde:
- `A = integral[r2][c2]`
- `B = integral[r1-1][c2]`
- `C = integral[r2][c1-1]`
- `D = integral[r1-1][c1-1]`

## Complexidade

- Construcao da matriz integral: `O(N x M)`
- Consulta de soma em sub-regiao: `O(1)`

No pior caso e no melhor caso, a construcao continua sendo `O(N x M)`, porque a
matriz inteira precisa ser preenchida uma vez.

## Estrutura do Projeto

```text
src/
  integral_image.h
  integral_image.c
  main.c

python/
  generate_files.py
  validate.py

requirements.txt
```

## Papel de Cada Arquivo

### `src/integral_image.h`

Define:
- as dimensoes fixas da imagem: `H = 64` e `W = 64`
- a interface publica do modulo

Funcoes expostas:
- `build_integral(int img[H][W], long integral[H][W])`
- `sum_region(long integral[H][W], int r1, int c1, int r2, int c2)`

### `src/integral_image.c`

Implementa o modulo principal do algoritmo.

Contem:
- `build_integral()`: monta a Integral Image
- `sum_region()`: responde consultas de soma em `O(1)`

Esse e o codigo central do trabalho.

### `src/main.c`

Este arquivo nao faz parte do modulo embarcado em si. Ele existe para testes e
validacao no PC.

Funcoes principais:
- gera uma imagem sintetica deterministica
- chama `build_integral()`
- exporta a matriz integral para `integral_c.txt`
- executa consultas fixas
- executa consultas pseudo-aleatorias
- compara `sum_region()` com uma implementacao por forca bruta
- exporta as consultas e resultados para `queries.txt`

### `python/generate_files.py`

Script auxiliar que:
- compila o programa C
- executa o binario de teste

Ele nao calcula a Integral Image em Python. Seu papel e apenas disparar o fluxo
do C para gerar os arquivos de saida.

### `python/validate.py`

Script de validacao externa.

Ele:
- compila e executa o programa C
- carrega `integral_c.txt`
- recria a mesma imagem em Python
- calcula a integral de referencia usando **OpenCV**
- compara a matriz do C com a matriz do OpenCV
- le `queries.txt`
- verifica se cada resultado salvo pelo C bate com a consulta calculada a partir
  da matriz de referencia

Esse script fortalece a prova de corretude do algoritmo.

## Fluxo Atual do Projeto

### 1. Geracao da imagem de teste

Em `src/main.c`, a imagem usada nos testes e sintetica e deterministica:

```c
img[r][c] = (r + c) & 0xFF;
```

Isso garante que:
- o C sempre gera a mesma imagem
- o Python consegue reconstruir exatamente a mesma entrada
- nao e necessario depender de arquivo externo de imagem

### 2. Construcao da Integral Image em C

O programa chama:

```c
build_integral(img, integral);
```

e produz a matriz integral em C.

### 3. Exportacao da matriz do C

O proprio programa C cria:

- `integral_c.txt`

Esse arquivo contem a matriz integral completa gerada pelo algoritmo em C.

Importante:
- o Python nao cria `integral_c.txt`
- quem abre e escreve esse arquivo e o `src/main.c`

### 4. Testes intermediarios com queries

O `src/main.c` tambem cria:

- `queries.txt`

Esse arquivo registra consultas no formato:

```text
r1 c1 r2 c2 valor
```

Cada linha representa:
- a regiao consultada
- o valor calculado por `sum_region()`

Essas queries sao importantes porque testam o uso final da estrutura: somar
sub-regioes rapidamente.

### 5. Validacao interna no C

Para cada query, o programa em C compara:
- o resultado rapido de `sum_region()`
- o resultado de `naive_sum_region()`, que soma pixel por pixel

Ou seja, o C se valida internamente contra uma referencia simples e direta.

### 6. Validacao externa com Python + OpenCV

Depois disso, `python/validate.py`:
- executa o C
- le `integral_c.txt`
- calcula uma matriz integral de referencia com `cv2.integral()`
- remove o padding extra retornado pelo OpenCV
- compara as duas matrizes pixel a pixel

Depois, usando `queries.txt`, o Python recalcula cada consulta e verifica se o
valor salvo pelo C esta correto.

## Como os Testes Estao Organizados

Hoje o projeto tem tres camadas de validacao:

### 1. Validacao da construcao da matriz

Comparacao entre:
- matriz integral produzida pelo C
- matriz integral de referencia produzida com OpenCV

Isso mostra se `build_integral()` esta correto.

### 2. Validacao das queries

Comparacao entre:
- valor produzido por `sum_region()` no C
- valor recalculado a partir da integral de referencia no Python

Isso mostra se a consulta em `O(1)` esta correta.

### 3. Validacao por forca bruta no C

Comparacao entre:
- `sum_region()`
- `naive_sum_region()`

Isso funciona como uma segunda prova independente, dentro do proprio executavel C.

## Por Que os `queries` Sao um Bom Teste Intermediario

Usar `queries.txt` como teste intermediario faz bastante sentido porque:
- valida a etapa mais importante para uso pratico da Integral Image
- testa diretamente a funcao `sum_region()`
- cobre casos fixos e consultas pseudo-aleatorias
- ajuda a separar erro de construcao da matriz de erro na formula da consulta

Em outras palavras:
- se `integral_c.txt` estiver errado, o problema pode estar em `build_integral()`
- se a matriz estiver certa, mas as queries falharem, o problema tende a estar em
  `sum_region()`

## Requisitos Atendidos

O projeto atende aos requisitos propostos porque:
- usa lacos de repeticao aninhados sobre matriz bidimensional
- usa matrizes estaticas de tamanho fixo `64x64`
- nao usa recursividade
- nao usa alocacao dinamica
- e adequado para contexto embarcado
- separa modulo principal e codigo de validacao

Observacao importante sobre tipos:
- a imagem de entrada usa `int img[64][64]`
- a matriz integral usa `long integral[64][64]`

Isso foi feito para reduzir risco de overflow acumulado nas somas.

## Memoria Utilizada

Cada matriz `64x64` possui:

```text
64 x 64 = 4096 elementos
```

No projeto ha pelo menos:
- uma matriz de entrada `img`
- uma matriz integral `integral`

Entao o uso total de memoria estatica associado a essas estruturas e superior a
8 KB, dependendo do tamanho dos tipos no ambiente de compilacao.

## Aplicacoes do Algoritmo

Algumas aplicacoes classicas da Integral Image:
- calculo eficiente de medias em regioes de imagens
- filtros de imagem, como box blur
- visao computacional
- extracao de features tipo Haar-like
- processamento de sinais bidimensionais
- sistemas embarcados com necessidade de consultas rapidas em matriz

## Dependencias

O ambiente Python usa:
- `numpy`
- `opencv-python`

Instalacao:

```bash
pip install -r requirements.txt
```

## Como Compilar e Rodar

### Rodar apenas a geracao dos arquivos pelo C

```bash
python3 python/generate_files.py
```

Esse comando:
- compila `src/main.c` e `src/integral_image.c`
- executa o binario `./teste`
- gera `integral_c.txt` e `queries.txt`

### Rodar a validacao completa

```bash
python3 python/validate.py
```

Esse comando:
- compila o C
- executa o C
- carrega `integral_c.txt`
- compara a matriz do C com a referencia do OpenCV
- valida as queries

## Saidas Esperadas

Ao rodar a validacao, o esperado e algo nesta linha:

```text
Compilando programa C para gerar integral_c.txt...
Executando binario C para gerar arquivos...
Arquivo integral_c.txt gerado pelo programa C.
Matrizes iguais (pixel a pixel)
Validacao queries: OK
```

Se houver falha, o script indica:
- diferenca entre matrizes
- erro maximo encontrado
- query que falhou
- valor do C e valor de referencia

## Observacoes Finais

Este projeto foi organizado para separar claramente:
- o **algoritmo principal** em C
- os **testes internos** no executavel C
- a **validacao externa independente** em Python/OpenCV

Com isso, a prova de corretude nao depende de um unico teste. O algoritmo e
verificado:
- por comparacao com forca bruta
- por comparacao com OpenCV
- por consultas intermediarias registradas em `queries.txt`

Essa combinacao torna a demonstracao do funcionamento bem mais forte para fins
academicos e de validacao pratica.
