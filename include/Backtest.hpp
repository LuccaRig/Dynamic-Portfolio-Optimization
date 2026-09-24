#pragma once

#include <vector>

#include "Estatisticas.hpp"
#include "Parametros.hpp"

class Mercado;
class DifferentialEvolution;

struct ConfigBacktest {
    int janela_estatistica = 63;
    int horizonte_mpc = 5;
    double max_turnover = 0.30;
    int dias_ano = 252;
};

struct ResultadoBacktest {
    std::vector<double> retornos_portfolio;
    std::vector<double> retornos_cdi;
    std::vector<double> turnovers;

    MetricasBacktest metricas;
};

ResultadoBacktest executarBacktest(
    Mercado& mercado,
    DifferentialEvolution& de,
    const ConfigBacktest& config,
    int num_ativos
);
