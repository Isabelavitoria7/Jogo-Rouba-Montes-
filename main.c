#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//--------------structd basicas
// 1- Copas, 2- espadas, 3- ouro, 4- paus

typedef struct carta{
  int valor;
  int naipe;
}carta;

typedef struct {
    carta cartas[52];
} baralho;


//---------- inicializa, embaralha, pilha de compra

typedef struct pilhacompra {
    carta *cartas;
    int topo;
} pilhacompra;

baralho *inicializabaralho(baralho *b){
    int count = 0;
    for (int naipe = 1; naipe <= 4; naipe++) {
        for (int valor = 1; valor <= 13; valor++) {
            b->cartas[count].valor = valor;
            b->cartas[count].naipe = naipe;
            count++;
        }
    }
    return b;
}

void embaralhabaralho(baralho *b) {
    srand(time(NULL));
    for (int j = 51; j > 0; j--) {
        int k = rand() % (j + 1);
        carta aux = b->cartas[j];
        b->cartas[j] = b->cartas[k];
        b->cartas[k] = aux;
    }
}

pilhacompra *inicializapilhacompra(pilhacompra *p, baralho *b, int quantbaralho) {
    p->topo = -1;
    p->cartas = (carta *)malloc(52 * quantbaralho * sizeof(carta));

    if (p->cartas == NULL) {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < quantbaralho; i++) {
        inicializabaralho(&b[i]);
        embaralhabaralho(&b[i]);
        for (int j = 0; j < 52; j++) {
            p->topo++;
            p->cartas[p->topo] = b[i].cartas[j];
        }
    }
    return p;
}


//-------- Função para aguardar a tecla Enter
    void aguardaEnter() {
    printf("Pressione Enter para comprar uma carta...");
    while (getchar() != '\n');
    printf("\n");
}

carta comprarCarta(pilhacompra *p) {
    carta c;
    if (p->topo >= 0) {
        c = p->cartas[p->topo];
        p->topo--;
    } else {
        // Tratar erro de pilha vazia
        c.valor = -1;
        c.naipe = -1;
    }
    return c;
}

//-------------------Pilha para cada jogador, nome

typedef struct pilhajog {
    carta carta;
    char nomeJogador[50]; // Adiciona o nome do jogador ao nó da pilha
    struct pilhajog *prox;
} pilhajog;

typedef struct pilhaJogador {
    char nomeJogador[50];
    pilhajog *topo;
    int posicao;
    int quantcartas;
} pilhaJogador;



pilhaJogador *inicializaPilhaJogador(char *nomeJogador) {
    pilhaJogador *p = (pilhaJogador *)malloc(sizeof(pilhaJogador));
    if (p == NULL) {
        exit(EXIT_FAILURE);
    }
    strncpy(p->nomeJogador, nomeJogador, sizeof(p->nomeJogador) - 1);
    p->nomeJogador[sizeof(p->nomeJogador) - 1] = '\0';
    p->topo = NULL;
    return p;
}

void empilhaCartaJogador(pilhaJogador *p, carta c) {
    pilhajog *novo = (pilhajog *)malloc(sizeof(pilhajog));
    if (novo == NULL) {
        exit(EXIT_FAILURE);
    }
    novo->carta = c;
    novo->prox = p->topo;
    p->topo = novo;
}


void liberaPilhaJogador(pilhaJogador *p) {
    while (p->topo != NULL) {
        pilhajog *removido = p->topo;
        p->topo = removido->prox;
        free(removido);
    }
    free(p);
}


//----------lista encadeada representar a mesa e suas operacoes

typedef struct cartanamesa {
    carta carta;
    struct cartanamesa *prox;
} cartanamesa;

typedef struct mesa {
    cartanamesa *ini;
} mesa;

mesa *inicializamesa(){
    mesa *m = (mesa *)malloc(sizeof(mesa));
    if (m == NULL) {
        exit(EXIT_FAILURE);
    }
    m->ini = NULL;
    return m;
}

void adicionaCartaMesa(mesa *m, carta c) {
    cartanamesa *novo = (cartanamesa *)malloc(sizeof(cartanamesa));
    if (novo == NULL) {
        exit(EXIT_FAILURE);
    }
    novo->carta = c;
    novo->prox = m->ini;
    m->ini = novo;
}

int removeCartaMesa(mesa *m, pilhaJogador *jogador, carta c) {
    if (m->ini == NULL) {
        return 0;
    }

    // Verifica se existe uma carta na mesa com o mesmo valor da carta escolhida
    cartanamesa *atual = m->ini;

    while (atual != NULL && atual->carta.valor != c.valor) {
        atual = atual->prox;
    }
    // Se encontrou a carta, remove-a da mesa
    if (atual != NULL) {
        if (atual == m->ini) {
            m->ini = atual->prox;
        } else {
            cartanamesa *ant = m->ini;
            while (ant->prox != atual) {
                ant = ant->prox;
            }
            ant->prox = atual->prox;
        }
        // Adiciona a carta ao topo da pilha do jogador
        empilhaCartaJogador(jogador, atual->carta);
        empilhaCartaJogador(jogador, c);

        // Libera a memória da carta
        free(atual);
        return 1;
    }
  return 0;
}


void liberaMesa(mesa *m) {
    if (m != NULL) {
        cartanamesa *atual = m->ini;
        while (atual != NULL) {
            cartanamesa *prox = atual->prox;
            free(atual);
            atual = prox;
        }
        free(m);
    }
}


//------------------ Funcoes para capturar o monte
int cartasSaoIguais(int c1, int c2) {
    return (c1 == c2);
}

void capturarMonte(pilhaJogador *jogadorAtual, pilhaJogador *jogadorCapturado, carta cartaComprada) {
    if (cartasSaoIguais(cartaComprada.valor, jogadorCapturado->topo->carta.valor)){

        // Move todas as cartas da pilha do jogadorCapturado para a pilha do jogador atual
        while (jogadorCapturado->topo != NULL) {

            carta cartaCapturada = jogadorCapturado->topo->carta;

            empilhaCartaJogador(jogadorAtual,cartaCapturada);

            // Remove carta da pilha do jogadorCapturado
            pilhajog *aux = jogadorCapturado->topo;
            jogadorCapturado->topo = aux->prox;
            free(aux);

        }

        empilhaCartaJogador(jogadorAtual,cartaComprada);

        printf("Monte de %s Capturado por %s!\n", jogadorCapturado->nomeJogador,jogadorAtual->nomeJogador);
    } else {
        printf("Nao foi possivel capturar o monte do jogador.\n");
    }
}


//---------- Funcao deixar no proprio monte

void deixarProprioMonte(pilhaJogador *pilhajogador, carta cartaComprada) {
    if(pilhajogador->topo == NULL || pilhajogador->topo->carta.valor == cartaComprada.valor){
        pilhajog *novo = (pilhajog *)malloc(sizeof(pilhajog));
        if (novo == NULL) {
            exit(EXIT_FAILURE);
        }

        novo->carta = cartaComprada;
        strcpy(novo->nomeJogador, pilhajogador->topo->nomeJogador); // Copia o nome do jogador para o nó
        novo->prox = pilhajogador->topo;
        pilhajogador->topo = novo;
    }
}


//-----------funcao que converte o numero para nomes das cartas
void converteParaNome(carta c) {
    char *nomes_valores[] = {"As", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    char *nomes_naipes[] = {"Copas", "Espadas", "Ouro", "Paus"};

    if (c.valor >= 1 && c.valor <= 13 && c.naipe >= 1 && c.naipe <= 4) {
        printf("%s de %s\n", nomes_valores[c.valor - 1], nomes_naipes[c.naipe - 1]);
    }
}

//------------------- funcao imprime topo de todos e mesa

void imprimeMesa(mesa *m) {
    if (m == NULL || m->ini == NULL) {
        printf("A mesa esta vazia ou não foi inicializada.\n");
        return;
    }
    printf("Cartas na mesa: \n");
    for (cartanamesa *atual = m->ini; atual != NULL; atual = atual->prox) {
        converteParaNome(atual->carta);
    }
}

void imprimeTopos(pilhaJogador *pilha) {
    if (pilha != NULL && pilha->topo != NULL) {
        converteParaNome(pilha->topo->carta);
        printf("\n");
    } else {
        printf("Monte vazio\n");
    }
}

void imprimeOpcoes(){
  int i = 1;
  printf("O que deseja fazer?\n");

  printf("%d - Roubar o monte de um jogador\n",i++);

  printf("%d - Retirar uma carta da mesa\n", i++);

  printf("%d - Deixar no proprio monte\n",i++);

  printf("%d - Deixar carta na mesa\n", i++);

}

//Conta quantas cartas tem cada jogador
int quantasCartas(pilhaJogador *p) {
  int count = 0;
  pilhajog *aux = p->topo;

  while (aux != NULL) {
    count++;
    aux = aux->prox;
  }

  return count;
}

//Metodo de ordenacao InsertionSort
// vet com a quant de cartas total de cada jogador

void insertionSort(int vet[], int t) {
    int i, j, aux;

    for (i = 1; i < t; i++) {
        aux = vet[i];
        j = i - 1;

        while (j >= 0 && vet[j] < aux) {
            vet[j + 1] = vet[j];
            j--;
        }
        vet[j + 1] = aux;
    }
}

// Ordena as cartas
void insertionCartas(carta vet[], int t) {
    int i, j;
    carta aux;

    for (i = 1; i < t; i++) {
        aux = vet[i];
        j = i - 1;

        while (j >= 0 && vet[j].valor > aux.valor) {
            vet[j + 1] = vet[j];
            j--;
        }
        vet[j + 1] = aux;
    }
}



int main(){
  printf("******************************************************\n");
  printf("** Bem-vindos ao jogo de cartas Rouba Monte! **\n");
  printf("******************************************************\n\n");

  // quantos jogadores vao jogar e quant baralhos
  int quantjogadores, quantbaralho;

  printf("Quantos jogadores vao jogar: ");
  scanf("%d", &quantjogadores);

  while(quantjogadores <= 0 || quantjogadores == 1){
    printf("Quantidade de jogadores invalidos.\n");
    printf("Quantos jogadores vao jogar: ");
    scanf("%d", &quantjogadores);
  }

  printf("\n");
  printf("Quantos baralhos vao ser usados: ");
  scanf("%d", &quantbaralho);

  while(quantbaralho <= 0){
    printf("Quantidade de baralho invalido.\n");
    printf("Quantos baralhos vao ser usados: ");
    scanf("%d", &quantbaralho);
  }

  printf("\n");


  //inicializar os baralhos e embaralha
  baralho meusbaralhos[quantbaralho];
  pilhacompra minhapilhacompra;

  // Inicializa a pilha de compra com os baralhos embaralhados
  inicializapilhacompra(&minhapilhacompra, meusbaralhos, quantbaralho);


  //inicializa os jogadores

  pilhaJogador jogadores[quantjogadores];

  //inicializa pilha para cada jogador

   pilhaJogador **pilhajogador = (pilhaJogador **)malloc(quantjogadores * sizeof(pilhaJogador*));
  if (pilhajogador == NULL) {
      exit(EXIT_FAILURE);
  }

  for (int i = 0; i < quantjogadores; i++) {
      printf("Nome do jogador(a) %d: ", i+1);
      setbuf(stdin, NULL);
      fgets(jogadores[i].nomeJogador, 50, stdin);
      int len = strlen(jogadores[i].nomeJogador);
      if (jogadores[i].nomeJogador[len - 1] == '\n') {
          jogadores[i].nomeJogador[len - 1] = '\0';
      }

      pilhajogador[i] = inicializaPilhaJogador(jogadores[i].nomeJogador);
      pilhajogador[i]->posicao = i+1;
  }


  // Inicializa a mesa
  mesa *mesadescarte = inicializamesa();


  // sorteia quem vai comecar
  srand(time(NULL));
  int quemcomeca = (rand() % quantjogadores);
  printf("%s vai comecar!\n\n", jogadores[quemcomeca].nomeJogador);

  int jogadoratual = quemcomeca;


    do {
        printf("%s, sua vez!\n", jogadores[jogadoratual].nomeJogador);
        aguardaEnter();

        printf("INFORMACOES___________________________\n");
        if (mesadescarte->ini != NULL) {
            imprimeMesa(mesadescarte);

        } else {
            printf("A mesa esta vazia.\n");
        }
        printf("\n");


        // Verifica se o topo dos jogadores tem cartas, depois imprimi

        for (int i = 0; i < quantjogadores; i++) {
            if (pilhajogador[i] != NULL && pilhajogador[i]->topo != NULL) {
                printf("Topo de %s (%d): ", pilhajogador[i]->nomeJogador,pilhajogador[i]->posicao);
                imprimeTopos(pilhajogador[i]);
            } else {
                printf("O monte de %s esta vazio.\n", jogadores[i].nomeJogador);
            }
        }
        printf("______________________________________");
        printf("\n\n");


        carta cartaComprada = comprarCarta(&minhapilhacompra);
        printf("Carta comprada: ");
        converteParaNome(cartaComprada);
        printf("\n");


//------ imprime as opcoes para o jogador, cases

  int opcao;
  int opcaovalida = 0;

  while(!opcaovalida){
    do {
      imprimeOpcoes();
      scanf("%d", &opcao);
        if (opcao < 1 || opcao > 4) {
           printf("Opcao invalida.\n");
        }
    } while (opcao < 1 || opcao > 4);


    switch (opcao) {
    int verifica;
    int posicaoCapturado;

    case 1:
          printf("Posicao do jogador que deseja roubar o monte: ");
          scanf("%d", &posicaoCapturado);
          posicaoCapturado--;

          if (posicaoCapturado >= 0 && posicaoCapturado < quantjogadores && pilhajogador[posicaoCapturado] != pilhajogador[jogadoratual] && pilhajogador[posicaoCapturado]->topo != NULL) {
                capturarMonte(pilhajogador[jogadoratual], pilhajogador[posicaoCapturado],cartaComprada);
            opcaovalida = 1;
          } else {
              printf("Nao e possivel capturar o monte do jogador.\n");
              opcaovalida = 0;
            }
        break;


    case(2):
       verifica = removeCartaMesa(mesadescarte, pilhajogador[jogadoratual], cartaComprada);
          if(verifica == 1){
        printf("Carta removida da mesa e adicionada a pilha do jogador %s.\n\n",pilhajogador[jogadoratual]->topo->nomeJogador);
            opcaovalida = 1;
          }
          else{
            printf("Carta nao existe na mesa ou mesa esta vazia. Tente novamente.\n");
            opcaovalida = 0;
          }
      break;


      case 3:
      if (pilhajogador[jogadoratual]->topo != NULL && pilhajogador[jogadoratual]->topo->carta.valor == cartaComprada.valor) {
          deixarProprioMonte(pilhajogador[jogadoratual], cartaComprada);
          printf("Carta adicionada ao proprio monte com sucesso.\n");
          opcaovalida = 1;
      } else {
          printf("Nao e possivel adicionar a carta ao proprio monte.\n");
          opcaovalida = 0;
      }
      break;


      case(4):
      adicionaCartaMesa(mesadescarte, cartaComprada);
      opcaovalida = 1;
      jogadoratual = (jogadoratual + 1) % quantjogadores;
      break;
     }

    printf("\n\n");
  }

} while (minhapilhacompra.topo >= 0);


  // Mostra o Ranking

  int *vetTotalcartas = (int *)malloc(quantjogadores * sizeof(int));

  for (int i = 0; i < quantjogadores; i++) {
      vetTotalcartas[i] = quantasCartas(pilhajogador[i]);
  }

  insertionSort(vetTotalcartas, quantjogadores);

  printf("Ranking:\n");
  for (int i = 0; i < quantjogadores; i++) {
      int indice = 0;

      while (quantasCartas(pilhajogador[indice]) != vetTotalcartas[i]) {
          indice++;
      }

      printf("%dº lugar: %s (%d cartas)\n", i + 1, pilhajogador[indice]->nomeJogador, vetTotalcartas[i]);
  }

  printf("\n\n");

  //Mostra o ganhador e se tiver empate

      int i = 0;
        while(i>=0){

          int indice2 = 0;
          while (quantasCartas(pilhajogador[indice2+i]) != vetTotalcartas[i]) {
              indice2++;
          }

          printf("Parabens %s voce ganhou (%d cartas):\n", pilhajogador[indice2+i]->nomeJogador, vetTotalcartas[i]);

          // Obtem as cartas do topo da pilha do jogador
          carta cartas[quantasCartas(pilhajogador[indice2+i])];

          pilhajog *aux = pilhajogador[indice2+i]->topo;

          for (int j = 0; j < quantasCartas(pilhajogador[indice2+i]); j++) {
              cartas[j] = aux->carta;
              aux = aux->prox;
          }


          insertionCartas(cartas, quantasCartas(pilhajogador[indice2+i]));

          // Mostrar as cartas ordenadas
          for (int j = 0; j < quantasCartas(pilhajogador[indice2+i]); j++) {
              converteParaNome(cartas[j]);
          }

          if(vetTotalcartas[i] == vetTotalcartas[i+1]){
            i += 1;
          }
          else{
            i = -1;
          }

          printf("\n");
        }

        printf("Fim do Jogo.");


  // Libera as pilhas dos jogadores e a lista mesa

    for(int i = 0; i<quantjogadores; i++){
      liberaPilhaJogador(pilhajogador[i]);
    }

    liberaMesa(mesadescarte);


  return 0;
}
