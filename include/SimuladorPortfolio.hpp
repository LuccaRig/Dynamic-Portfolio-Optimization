#ifndef SIMULADOR_PORTFOLIO_HPP
#define SIMULADOR_PORTFOLIO_HPP

#include <vector>

struct ParametrosCusto {
    double alpha; // Retorno Excedente
    double beta;  // CVaR (Risco de Cauda)
    double gamma; // Variância Global
    double eta;   // Turnover / Custos
    double rf;    // Taxa livre de risco (CDI vigente)
};

class SimuladorPortfolio {
public:
    static double avaliarTrajetoria(
        const std::vector<double>& peso_inicial,
        const std::vector<std::vector<double>>& trajetoria_deltas, // N passos de Δw
        const std::vector<std::vector<double>>& matriz_covariancia, // Σ
        const std::vector<double>& retornos_esperados, // μ̂
        const ParametrosCusto& params,
        double max_turnover
    );
};

#endif