#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "processo.h"

int ler_processos(const char *nome_arquivo, Processo processos[]) {
    FILE *fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror(">> Falha ao acessar o arquivo");
        return -1;
    }

    char linha[512];
    int i = 0;
    fgets(linha, sizeof(linha), fp); // Ignora cabecalho

    while (fgets(linha, sizeof(linha), fp) && i < MAX_PROCESSOS) {
        Processo p;
        char *token = strtok(linha, ",");

        if (!token) continue;
        p.id = atoi(token);

        token = strtok(NULL, ",");
        strcpy(p.numero, token);

        token = strtok(NULL, ",");
        strcpy(p.data_ajuizamento, token);

        token = strtok(NULL, ",");
        p.id_classe = atoi(token + 1); // Ignora '{'

        token = strtok(NULL, ",");
        p.num_assuntos = 0;

        char *ptr = token;
        while (*ptr) {
            if (*ptr >= '0' && *ptr <= '9') {
                p.id_assunto[p.num_assuntos++] = atoi(ptr);
                while (*ptr >= '0' && *ptr <= '9') ptr++;
            } else {
                ptr++;
            }
        }

        token = strtok(NULL, ",");
        p.ano_eleicao = atoi(token);

        processos[i++] = p;
    }

    fclose(fp);
    return i;
}

void ordenar_por_id(Processo v[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (v[j].id > v[j+1].id) {
                Processo temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }
}

void ordenar_por_data(Processo v[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (strcmp(v[j].data_ajuizamento, v[j+1].data_ajuizamento) < 0) {
                Processo temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }
}

void salvar_csv(const char *nome_arquivo, Processo v[], int n) {
    FILE *fp = fopen(nome_arquivo, "w");
    if (!fp) {
        printf("[ERRO] Não foi possível gerar o arquivo: %s\n", nome_arquivo);
        return;
    }

    fprintf(fp, "id,numero,data_ajuizamento,id_classe,id_assunto,ano_eleicao\n");

    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,\"%s\",%s,{%d},", v[i].id, v[i].numero, v[i].data_ajuizamento, v[i].id_classe);
        for (int j = 0; j < v[i].num_assuntos; j++) {
            fprintf(fp, "%d", v[i].id_assunto[j]);
            if (j < v[i].num_assuntos - 1)
                fprintf(fp, "|");
        }
        fprintf(fp, ",%d\n", v[i].ano_eleicao);
    }

    fclose(fp);
    printf("[OK] Arquivo salvo com sucesso: %s\n", nome_arquivo);
}

int contar_por_classe(Processo v[], int n, int id_classe) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (v[i].id_classe == id_classe) count++;
    }
    printf("-> Total de processos da classe %d: %d\n", id_classe, count);
    return count;
}

int contar_assuntos_unicos(Processo v[], int n) {
    int unicos[10000] = {0};
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < v[i].num_assuntos; j++) {
            unicos[v[i].id_assunto[j]] = 1;
        }
    }
    int count = 0;
    for (int i = 0; i < 10000; i++) {
        if (unicos[i]) count++;
    }
    printf("-> Quantidade total de assuntos únicos: %d\n", count);
    return count;
}

void listar_multiplos_assuntos(Processo v[], int n) {
    printf("\n===== Processos com Múltiplos Assuntos =====\n");
    int encontrados = 0;
    for (int i = 0; i < n; i++) {
        if (v[i].num_assuntos > 1) {
            encontrados++;
            printf("[Processo: %s] Assuntos: ", v[i].numero);
            for (int j = 0; j < v[i].num_assuntos; j++) {
                printf("%d", v[i].id_assunto[j]);
                if (j < v[i].num_assuntos - 1) printf(" | ");
            }
            printf("\n");
        }
    }
    if (encontrados == 0) {
        printf("Nenhum processo com mais de um assunto foi encontrado.\n");
    }
}

int dias_em_tramitacao(const char *data) {
    struct tm tm_data = {0};
    sscanf(data, "%d-%d-%d %d:%d:%d",
           &tm_data.tm_year, &tm_data.tm_mon, &tm_data.tm_mday,
           &tm_data.tm_hour, &tm_data.tm_min, &tm_data.tm_sec);

    tm_data.tm_year -= 1900;
    tm_data.tm_mon -= 1;

    time_t t_data = mktime(&tm_data);
    time_t agora = time(NULL);

    double diff = difftime(agora, t_data);
    return (int)(diff / (60 * 60 * 24));
}

void mostrar_dias_em_tramitacao(Processo v[], int n) {
    printf("\n===== Dias em Tramitação de Cada Processo =====\n");
    for (int i = 0; i < n; i++) {
        int dias = dias_em_tramitacao(v[i].data_ajuizamento);
        printf("Numero do processo: %-20s | Dias em tramitacao: %4d\n", v[i].numero, dias);
    }
}
