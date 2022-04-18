/// Univeridade Federal do ParanÃ¡
// TE355 - Sistemas Operacionais Embarcados
// Base para o Trabalho 1 - 2022
// Prof. Pedroso

#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h> 

#define PROTOPORT 5193 /* default protocol port number */
#define QLEN 6         /* size of request queue        */

int visits = 0; /* counts client connections    */
char msg[1000][1000];
int ditados = 0;
int alteracoes = 0;

// Initialize semaphores
sem_t sem1;
sem_t sem2; 
sem_t sem3;
sem_t sem4;

void LeDitado() {
  FILE *arq;

  if ((arq = fopen("Ditados.txt", "r")) == NULL) {
    printf("\n Erro lendo arquivo ...\n\n");
    exit(0);
  }
  while (!feof(arq)) {

    fgets(msg[ditados], 999, arq);
    // para debug
    // printf("%d %s",ditados,msg[ditados]);
    ditados = (ditados + 1) % 1000;
  }
  printf("\n\nCarregou %d ditados", ditados);
}

void gravaDitado(){
  FILE *arq;
  int result;

  if ((arq = fopen("Ditados.txt", "wt")) == NULL){
    printf("\nErro lendo o arquivo\n");
    exit(0);
  }
  for (int i = 0; i < ditados; i++)
  {
    result = fputs(msg[i], arq);
    if (result == EOF)
    {
      printf("Erro na gravacao\n");
      fclose(arq);
    }    
  }
  fclose(arq);
  
}

char uppercase(char *input) {
  char i = 0;
  for (i = 0; (i < strlen(input)) && (i < 1000); i++) {
    input[i] = toupper(input[i]);
  }
  return i;
}

void encontrarSimilar(int* sd, char* palavra){
  // função que encontra palavra similar no banco de dados
  char* pos;
  palavra[strcspn(palavra, "\r\n")] = 0;
  fprintf(stdout, "%s", palavra);
  for (int i = 0; i < 1000; i++)
  {

    pos = strstr(msg[i], palavra);
    
    if(pos){
      send(*sd, msg[i], strlen(msg[i]), 0);
    }     
  }  
}

void palavrasD(int* sd, const int idx_ditado, char* str){
  if (idx_ditado >= ditados)  
  {
    char const* msg_erro_idx = "Indice nao existe!\n"; 
    send(*sd, msg_erro_idx, strlen(msg_erro_idx), 0);
  }else{
    int counter = 0;
    for (int i = 0; i < strlen(msg[idx_ditado]); i++)
    {
      if (msg[idx_ditado][i] == ' ') counter++;   
      else if(msg[idx_ditado][i] == '.' || msg[idx_ditado][i] == '!' || msg[idx_ditado][i] == '?' || msg[idx_ditado][i] == ',' || msg[idx_ditado][i] == ';') continue;   
    }
    sprintf(str, "Ditado %s\nTotal de palavras:%d\n", msg[idx_ditado], counter + 2);
    send(*sd, str, strlen(str), 0);
  }  
}

void palavrasT(int* sd, char* str){
  int counter = 0;
  for (int j = 0; j < ditados; j++)
  {
    for (int i = 0; i < strlen(msg[j]); i++)
    {
      if (msg[j][i] == ' ') counter++;   
      else if(msg[j][i] == '.' || msg[j][i] == '!' || msg[j][i] == '?' || msg[j][i] == ',' || msg[j][i] == ';') continue;   
    }
  }
    sprintf(str, "Total de palavras: %d\n", counter + ditados*2);
    send(*sd, str, strlen(str), 0);
}

void *atendeConexao(void *sd2) {
  int *temp = sd2;
  int sd = *temp;

  sem_post(&sem3);

  char str[2000], *endptr;

  int i = 0, b, val;

  while (1) {
    visits++;
    sprintf(str, "%d", visits);
    send(sd, str, strlen(str), 0);
    b = recv(sd, str, 999, 0);
    str[b] = 0;
    printf("Comando recebido: %s", str);

    uppercase(str);

    if (!strncmp(str, "GETR", 4)) {
      int r = rand() % ditados;
      sprintf(str, "\nDitado %d: %s", r % ditados, msg[r % ditados]);
      send(sd, str, strlen(str), 0);
    }else if (!strncmp(str, "GETN", 4)) {
      b = recv(sd, str, 999, 0);
      str[b] = 0;
      val = strtol(str, &endptr, 10);
      fprintf(stdout, "%d", val);
      if (endptr == str) {
        sprintf(str, "\nFALHA");
        continue;
      } else
        send(sd, msg[val], strlen(msg[val]), 0);
    } else if (!strncmp(str, "REPLACE", 7)) {
      sem_wait(&sem4);      
      b = recv(sd, str, 999, 0);
      str[b] = 0;
      val = strtol(str, &endptr, 10);
      printf("valor de endptr %c", *(endptr+1));
      if (endptr == str) {
        sprintf(str, "\nFALHA\n");
        continue;

      } else
        sprintf(str, "\nOK\n");
      send(sd, str, strlen(str), 0);
      b = recv(sd, str, 999, 0);
      str[b] = 0;
      strcpy(msg[val], str);
      sprintf(str, "\nOK\n");
      send(sd, str, strlen(str), 0);
      printf("\nNovo ditado %d: %s", val, msg[val]);
      sem_post(&sem4);
      sem_wait(&sem2);
      alteracoes++;
      sem_post(&sem2);
    } else if (!strncmp(str, "FIM", 3)) {
      sprintf(str, "\nAte Logo\n");
      send(sd, str, strlen(str), 0);
      break;
    } else if (!strncmp(str, "VER", 3)) {
      sprintf(
          str,
          "\nServidor de Ditados 2.0 Beta.\nTE355 2022 Primeiro Trabalho\n");
      send(sd, str, strlen(str), 0);
    } else if (!strncmp(str, "DEL", 3)) {
      b = recv(sd, str, 999, 0);
      str[b] = 0;
      val = strtol(str, &endptr, 10);
      send(sd, str, strlen(str), 0);
      if (endptr == str) {
        sprintf(str, "\nFalha\n");
        continue;
      }
      strcpy(msg[val], "Ditado deletado!\n");
      sprintf(str, "Ditado n:%d - deletado\n", val);
      sem_wait(&sem2);
      alteracoes++;
      sem_post(&sem2);
      send(sd, str, strlen(str), 0);
      printf("Ditado: %d - deletado\n", val); // informa uma mensagem no servidor.
    }else if (!strncmp(str, "ROTATE", 6))
    {
      char str_aux[2000]; // cria um string auxiliar para copiar o texto
      int val2 = 0;
      // escolhe o primeiro ditado que será trocado
      sem_wait(&sem4);
      sprintf(str, "Trocar ditado\n");
      send(sd, str, strlen(str), 0);
      b = recv(sd, str, strlen(str), 0);
      str[b] = 0;
      val = strtol(str, &endptr, 10);
      if (endptr == str) {
        sprintf(str, "\nFalha\n");
        continue;
      }
      sprintf(str, "%d - %s", val, msg[val]);
      send(sd, str, strlen(str), 0);
      strcpy(str_aux, msg[val]);
      val2 = val;
      // escolhendo segundo ditado que será trocado
      sprintf(str, "com ditado\n");
      send(sd, str, strlen(str), 0);
      b = recv(sd, str, strlen(str), 0);
      str[b] = 0;
      val = strtol(str, &endptr, 10);
      if (endptr == str) {
        sprintf(str, "\nFalha\n");
        continue;
      }
      sprintf(str, "%d - %s", val, msg[val]);
      send(sd, str, strlen(str), 0);

      // troca de ditados
      strcpy(msg[val2], msg[val]);
      strcpy(msg[val], str_aux);

      sprintf(str, "Ditado %d trocado com ditado %d\n", val, val2);
      send(sd, str, strlen(str), 0);
      sem_post(&sem4);
      sem_wait(&sem2);
      alteracoes++; // aumenta um quando o banco de dados é alterado.
      sem_post(&sem2);
    }else if(!strncmp(str, "ALTERACOES", 10)){
      sprintf(str, "Número de alterações: %d\n", alteracoes);
      send(sd, str, strlen(str), 0);
    }else if(!strncmp(str, "SEARCH", 6)){
      sprintf(str, "Entre com a palavra: ");
      send(sd, str, strlen(str), 0);
      b = recv(sd, str, strlen(str), 0);
      str[b] = 0;
      encontrarSimilar(&sd, str);
    }else if(!strncmp(str, "PALAVRAS-D", 10)){
      // retorna o total de palavras de um ditado
      sprintf(str, "Entre com o numero do ditado: ");
      send(sd, str, strlen(str), 0);
      b = recv(sd, str, strlen(str), 0);
      str[b] = 0;
      val = strtol(str, &endptr, 10);

      palavrasD(&sd, val, str);
    }else if (!strncmp(str, "PALAVRAS-T", 10))
    {
      palavrasT(&sd, str);
    }else if(!strncmp(str, "GRAVA", 5)){
      sem_wait(&sem1);
      gravaDitado();
      sem_post(&sem1);
    }else if(!strncmp(str, "LE", 2)){
      sem_wait(&sem1);
      LeDitado();
      sem_post(&sem1);
    }
    else {
      sprintf(str, "\nErro de Protocolo\n");
      send(sd, str, strlen(str), 0);
    }
  }
  close(sd);
}

int main(int argc, char **argv) {
  struct protoent *ptrp;  /* pointer to a protocol table entry   */
  struct sockaddr_in sad; /* structure to hold server's address  */
  struct sockaddr_in cad; /* structure to hold client's address  */
  int sd, sd2;            /* socket descriptors                  */
  int port;               /* protocol port number                */
  int alen;               /* length of address                   */
  pthread_t t;

  // SEMAPHORES
  //sem_init(&sem1, 0, 1);
  sem_init(&sem2, 0, 1);
  sem_init(&sem3, 0, 1);
  sem_init(&sem4, 0, 1);

  srandom(
      time(NULL)); /* inicializa a semente do gerador de nÃºmeros aleatÃ³rios */

  memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
  sad.sin_family = AF_INET;             /* set family to Internet     */
  sad.sin_addr.s_addr = INADDR_ANY;     /* set the local IP address   */

  /* Check command-line argument for protocol port and extract    */
  /* port number if one is specified.  Otherwise, use the default */
  /* port value given by constant PROTOPORT                       */

  if (argc > 1) {         /* if argument specified        */
    port = atoi(argv[1]); /* convert argument to binary   */
  } else {
    port = PROTOPORT; /* use default port number      */
  }
  if (port > 0) /* test for illegal value       */
    sad.sin_port = htons((u_short)port);
  else { /* print error message and exit */
    fprintf(stderr, "bad port number %s\n", argv[1]);
    exit(1);
  }

  LeDitado();

  /* Map TCP transport protocol name to protocol number */

  if (((ptrp = getprotobyname("tcp"))) == NULL) {
    fprintf(stderr, "cannot map \"tcp\" to protocol number");
    exit(1);
  }

  /* Create a socket */

  sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if (sd < 0) {
    fprintf(stderr, "socket creation failed\n");
    exit(1);
  }

  /* Bind a local address to the socket */

  if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
    fprintf(stderr, "bind failed\n");
    exit(1);
  }

  /* Specify size of request queue */

  if (listen(sd, QLEN) < 0) {
    fprintf(stderr, "listen failed\n");
    exit(1);
  }

  /* Main server loop - accept and handle requests */

  while (1) {
    alen = sizeof(cad);
    sem_wait(&sem3);
    if ((sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
      fprintf(stderr, "accept failed\n");
      exit(1);
    }
    printf("\nServidor atendendo conexao %d\n", visits);
    pthread_create(&t, NULL, atendeConexao, &sd2);
  }
}
