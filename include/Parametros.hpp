#pragma once
#include <vector>

struct ParametrosCusto {
    double alpha = 1.0;      // retorno
    double beta  = 0.5;      // CVaR
    double gamma = 0.8;      // variância
    double eta   = 0.1;      // custo de transação
    double cvar_alpha = 0.95;
};

struct ParametrosDE {
    int tamanho_populacao = 60;
    int max_geracoes = 40;
    double F = 0.7;
    double CR = 0.9;
};
