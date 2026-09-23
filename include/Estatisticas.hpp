#pragma once

#include <vector>

double media(const std::vector<double>& valores);

double desvioPadrao(const std::vector<double>& valores);

double retornoAcumulado(const std::vector<double>& retornos);

double maxDrawdown(const std::vector<double>& retornos);

double VaR95(const std::vector<double>& retornos);

double CVaR95(const std::vector<double>& retornos);

struct MetricasBacktest {
    double media_diaria = 0.0;
    double volatilidade_diaria = 0.0;
    double retorno_acumulado = 0.0;
    double retorno_anualizado = 0.0;
    double volatilidade_anualizada = 0.0;
    double sharpe = 0.0;
    double drawdown_maximo = 0.0;
    double var_95 = 0.0;
    double cvar_95 = 0.0;
    double retorno_cdi_acumulado = 0.0;
    double excesso_cdi = 0.0;
    double turnover_medio = 0.0;
    double turnover_maximo = 0.0;
    double percentual_dias_positivos = 0.0;
};

MetricasBacktest calcularMetricas(
    const std::vector<double>& retornos_portfolio,
    const std::vector<double>& retornos_cdi,
    const std::vector<double>& turnovers,
    int dias_ano = 252
);
