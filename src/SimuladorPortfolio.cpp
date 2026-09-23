#include "SimuladorPortfolio.hpp"
#include <cmath>
#include <algorithm>

double SimuladorPortfolio::avaliarTrajetoria(
    const std::vector<double>& peso_inicial,
    const std::vector<std::vector<double>>& trajetoria_deltas,
    const std::vector<std::vector<double>>& matriz_covariancia,
    const std::vector<double>& retornos_esperados,
    const ParametrosCusto& params,
    double max_turnover
) {
    int N = trajetoria_deltas.size();
    int M = peso_inicial.size();
    
    std::vector<double> peso_atual = peso_inicial;
    double custo_total = 0.0;
    
    std::vector<double> historico_retornos_simulados;

    for (int k = 0; k < N; ++k) {
        const auto& delta_w = trajetoria_deltas[k];
        
        // 1. Validação de Turnover no passo
        double turnover_passo = 0.0;
        std::vector<double> proximo_peso(M);
        for (int i = 0; i < M; ++i) {
            proximo_peso[i] = peso_atual[i] + delta_w[i];
            turnover_passo += std::abs(delta_w[i]);
        }

        if (turnover_passo > max_turnover) {
            return 1e9; // Penalidade por violação de restrição
        }

        // 2. Limites físicos (0 <= w_i <= 1 e soma <= 1)
        double soma_pesos = 0.0;
        for (int i = 0; i < M; ++i) {
            if (proximo_peso[i] < 0.0 || proximo_peso[i] > 1.0) return 1e9;
            soma_pesos += proximo_peso[i];
        }
        if (soma_pesos > 1.0) return 1e9;

        // 3. Retorno do Portfólio no passo k (incluindo caixa/CDI)
        double retorno_port = 0.0;
        for (int i = 0; i < M; ++i) {
            retorno_port += proximo_peso[i] * retornos_esperados[i];
        }
        double peso_caixa = 1.0 - soma_pesos;
        retorno_port += peso_caixa * params.rf;

        historico_retornos_simulados.push_back(retorno_port);

        // 4. Variância global (w^T * Sigma * w)
        double varianca_portfolio = 0.0;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < M; ++j) {
                varianca_portfolio += proximo_peso[i] * matriz_covariancia[i][j] * proximo_peso[j];
            }
        }

        // 5. Custo instantâneo da função scalarizada
        double retorno_excedente = retorno_port - params.rf;
        double custo_passo = -params.alpha * retorno_excedente 
                             + params.gamma * varianca_portfolio 
                             + params.eta * turnover_passo;
        
        custo_total += custo_passo;
        peso_atual = proximo_peso;
    }

    // 6. CVaR (Risco de Cauda) no horizonte simulado
    if (!historico_retornos_simulados.empty()) {
        auto retornos_copia = historico_retornos_simulados;
        std::sort(retornos_copia.begin(), retornos_copia.end());
        int idx_cvar = std::max(0, (int)(0.1 * retornos_copia.size()));
        double cvar = 0.0;
        for (int i = 0; i <= idx_cvar; ++i) {
            cvar += retornos_copia[i];
        }
        cvar /= (idx_cvar + 1);
        
        custo_total += params.beta * std::abs(cvar);
    }

    return custo_total;
}