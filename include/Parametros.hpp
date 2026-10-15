#pragma once
#include <vector>

struct ParametrosCusto {
    double alpha = 3.0;      // retorno
    double beta  = 0.5;      // CVaR
    double gamma = 0.1;      // variância
    double eta   = 0.1;      // custo de transação
    double cvar_alpha = 0.95;
};

struct ParametrosDE {
    int tamanho_populacao = 120;
    int max_geracoes = 40;
    double F = 0.7;
    double CR = 0.9;
};
