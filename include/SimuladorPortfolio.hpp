#pragma once
#include <vector>
#include "Parametros.hpp"

class SimuladorPortfolio {
public:
    static double avaliarTrajetoria(
        const std::vector<double>& peso_inicial,
        const std::vector<std::vector<double>>& trajetoria_deltas,
        const std::vector<std::vector<double>>& matriz_covariancia,
        const std::vector<double>& retornos_esperados,
        const std::vector<double>& cdi_horizonte,
        const std::vector<std::vector<double>>& cenarios_retornos,
        const ParametrosCusto& params,
        double max_turnover
    );

    static std::vector<double> repararPesos(
        const std::vector<double>& peso_inicial,
        const std::vector<double>& delta,
        double max_turnover
    );
};
