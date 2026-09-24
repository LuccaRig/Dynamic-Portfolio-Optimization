#pragma once

#include <vector>
#include <string>

#include "Estatisticas.hpp"
#include "Parametros.hpp"

class Mercado;
class DifferentialEvolution;

struct ConfigBacktest {
    int janela_estatistica = 63;
    int horizonte_mpc = 5;
    double max_turnover = 1.0;
    int dias_ano = 252;
};

struct ResultadoBacktest {
    std::vector<double> retornos_portfolio;
    std::vector<double> retornos_cdi;
    std::vector<double> retornos_bh;     
    std::vector<double> turnovers;
    std::vector<double> pesos_iniciais;
    std::vector<double> pesos_finais;
    std::vector<std::string> nomes_ativos;
    MetricasBacktest metricas;
    MetricasBacktest metricas_bh;         
};

ResultadoBacktest executarBacktest(
    Mercado& mercado,
    DifferentialEvolution& de,
    const ConfigBacktest& config,
    int num_ativos
);
