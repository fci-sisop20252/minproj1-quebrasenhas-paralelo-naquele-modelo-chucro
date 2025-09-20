# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Caio Ariel (10439611), Gabriel Mires (10436741), Isabela Hissa (10441873), Kaique Paiva (10441787)
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

A estratégia de paralelização utilizada foi a divisão do espaço total de combinações de senhas igualmente entre os workers, de forma que cada worker recebe um intervalo contínuo de senhas para testar. Se o total de combinações não for divisível pelo número de workers, os primeiros remaining workers recebem uma senha a mais para cobrir todo o espaço.

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
// Calcular quantas senhas cada worker deve verificar
long long password_per_worker = total_space / num_workers;
long long remaining = total_space % num_workers;

// Loop para criar workers
for (int i = 0; i < num_workers; i++) {
    long long start_index, end_index;
    long long offset = i * password_per_worker;
    if (i < remaining) {
        offset += i;  // workers iniciais recebem 1 extra
    } else {
        offset += remaining;
    }
    long long count = password_per_worker;
    if (i < remaining) {
        count += 1;
    }
    start_index = offset;
    end_index = offset + count - 1;

    char start_password[password_len + 1];
    char end_password[password_len + 1];
    index_to_password(start_index, charset, charset_len, password_len, start_password);
    index_to_password(end_index, charset, charset_len, password_len, end_password);

    printf("Worker %d → intervalo: %s até %s\n", i, start_password, end_password);
}
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

No coordinator, cada worker foi criado usando fork(). No processo filho, utilizamos execl() para executar o programa do worker, passando como argumentos o intervalo de senhas (start_password e end_password) e outros parâmetros necessários. O processo pai mantém o PID de cada filho e, após criar todos os workers, utiliza wait() para aguardar cada worker terminar, garantindo que todos os processos filhos concluam antes de encerrar o coordinator.

**Código do fork/exec:**
```c
pid_t pid = fork(); // é tipo um ID (ent cada criação um fork é gerado um ID)

    // TODO 7:
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // ID foi criado ent começou o processo de virar work
        char password_len_str[16], worker_id_str[16]; // o execl só aceita string ent converti os bgl
        snprintf(password_len_str, sizeof(password_len_str), "%d", password_len);
        snprintf(worker_id_str, sizeof(worker_id_str), "%d", i);


        // TODO 6:
        execl(
            "./worker", "worker",
            target_hash,
            start_password,
            end_password,
            charset,
            password_len_str,
            worker_id_str,
            (char *)NULL
        );
        perror("execl ./worker");
        _exit(1);
    }
    // TODO 5:
    workers[i] = pid; // armazena o pid
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

Para garantir que apenas um worker escrevesse o resultado e evitar uma condição de corrida, utilizamos as flags O_CREAT | O_EXCL na chamada open(). Essa combinação torna a criação do arquivo atômica. O primeiro worker a encontrar a senha e executar a chamada open() com essas flags cria o arquivo com sucesso. Para qualquer outro worker que tentar criar o arquivo depois, o open() falhará com um erro (EEXIST), informando ao processo que a senha já foi encontrada e que ele pode parar sua busca.

**Como o coordinator consegue ler o resultado?**

O coordinator lê o resultado ao final da execução. Ele tenta abrir o arquivo password_found.txt no modo de leitura (O_RDONLY). Se o arquivo existir, ele usa a chamada read() para copiar o conteúdo para um buffer. O resultado é esperado no formato "worker_id:password". O coordinator então utiliza a função strchr() para localizar o caractere de dois pontos (:), separando a string em duas partes: o ID do worker e a senha encontrada. Por fim, ele converte o ID para um número inteiro e exibe a senha, verificando seu hash para confirmar a correção da informação.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0.008s | 0.006s | 0.004s | 2x |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | 4.684s | 4.641s | 0.784s | 5.97x |

**O speedup foi linear? Por quê?**

O motivo para a não linearidade é a Lei de Amdahl. Ela explica que a aceleração do programa é limitada pela parte do código que não pode ser paralelizada, ou seja, que deve ser executada de forma sequencial. Essa porção sequencial impede que a performance escale proporcionalmente com o aumento de workers.

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**

Tivemos dificuldade com a implementação da função de incrementar a senha. Para resolver foi necessário percorrer a senha de trás para frente, encontrando cada caractere no conjunto e substituindo pelo próximo na sequência. Se o caractere já era o último do conjunto, ele voltava para o primeiro e o incremento passava para o caractere anterior, repetindo até que a senha fosse completamente incrementada.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [X] Código compila sem erros
- [X] Todos os TODOs foram implementados
- [X] Testes passam no `./tests/simple_test.sh`
- [X] Relatório preenchido
