#pragma once
#include <vector>
#include <random>
#include "Parametros.hpp"

class DifferentialEvolution {
public:
    DifferentialEvolution(const ParametrosDE& params_de,
                           const ParametrosCusto& params_custo);

    std::vector<std::vector<double>> otimizar(
        const std::vector<double>& peso_inicial,
        const std::vector<std::vector<double>>& matriz_covariancia,
        const std::vector<double>& retornos_esperados,
        const std::vector<double>& cdi_horizonte,
        const std::vector<std::vector<double>>& cenarios_retornos,
        double max_turnover,
        int horizonte_N,
        int num_ativos
    );

private:
    ParametrosDE config_de_;
    ParametrosCusto config_custo_;

    static double gerarAleatorio(double min, double max, std::mt19937& rng);
};
