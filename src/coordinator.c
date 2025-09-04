#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/*
 * TODO  : Adicionar validações dos parâmetros ✅
 * TODO 1: Validar argumentos de entrada ✅
 * TODO 2: Dividir o espaço de busca entre os workers ✅
 * TODO 3: Criar os processos workers usando fork() ✅
 * TODO 4: Usar fork() para criar processo filho ✅
 * TODO 5: No processo pai: armazenar PID ✅
 * TODO 6: No processo filho: usar execl() para executar worker ✅
 * TODO 7: Tratar erros de fork() e execl() ✅
 * TODO 8: Aguardar todos os workers terminarem usando wait()
 * TODO 9: Verificar se algum worker encontrou a senha
 * TODO 10: Calcular e exibir estatísticas de performance
 */

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 *
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 *
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 *
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 *
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 *
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 *
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */

int main(int argc, char *argv[]) { 
    // TODO 1: 
    
    // Verificação de argc e mensagem de erro
    if (argc != 5)
    {
      printf("Erro: quantidade de argumentos inválidos!\n");
      exit(1);
    }

    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);

     
    // TODO: 
    
    // -password_len deve estar entre 1 e 10
    // -num_workers deve estar entre 1 e MAX_WORKERS
    // -charset não pode ser vazio

    if (password_len < 1 && password_len > 10)
    {
      printf("Erro: tamanho da senha inválido!");
      exit(1);
    }

    if (num_workers < 1 && num_workers > MAX_WORKERS)
    {
      printf("Erro: número de workers inválido!");
      exit(1);
    }

    if (charset_len == 0)
    {
      printf("Erro: charset nulo!");
      exit(1);
    }

    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);

    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);

    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);

    // Registrar tempo de início
    time_t start_time = time(NULL);
 
    // TODO 2:

    // Calcular quantas senhas cada worker deve verificar
    long long password_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;


    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];

    // TODO 3: 
    printf("Iniciando workers...\n");

    // Loop para criar workers
      for (int i = 0; i < num_workers; i++) {
        // Calcular intervalo de senhas para este worker
        long long start_index, end_index;
        long long offset = i * password_per_worker;
        if (i < remaining) {
          offset += i;  // worker faz mais 1
        }
        else {
          offset += remaining;
        }
        long long count = password_per_worker;
        if (i < remaining) {
          count += 1;
        }
        start_index = offset;
        end_index = offset + count - 1;


        // Converter indices para senhas de inicio e fim
        char start_password[password_len + 1];
        char end_password[password_len + 1];
        index_to_password(start_index, charset, charset_len, password_len, start_password);
        index_to_password(end_index, charset, charset_len, password_len, end_password);
        printf("Worker %d → intervalo: %s até %s\n", i, start_password, end_password);

        // TODO 4:
        pid_t pid = fork(); // é tipo um ID (ent cada criação um fork é gerado um ID)
        
        // TODO 7:
        if (pid < 0){ 
          printf("Error"); 
          exit(1);
        }

        if(pid == 0){ // ID foi criado ent começou o processo de virar work
          char password_len_str[4], worker_id_str[4]; // o execl só aceita string ent converti os bgl
          sprintf(password_len_str, "%d", password_len);
          sprintf(worker_id_str, "%d", i);
          exit(1);

          // TODO 6:
          execl(
            "./worker", "worker",
            target_hash,
            start_password,
            end_password,
            charset,
            password_len_str,
            worker_id_str,
            NULL
          );
          perror("Error");
          exit(1);
        }
        // TODO 5:
        workers[i] = pid; // armazena o pid 

        // Pode dar bomba pq eu nao quis usar if else, ent caso de estrago já sabe oq fazer
}   

    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");

    // TODO 8:
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis

    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram

    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);

    printf("\n=== Resultado ===\n");

    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir

    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado

    // Estatísticas finais (opcional)
    // TODO 10: Calcular e exibir estatísticas de performance

    return 0;
}
