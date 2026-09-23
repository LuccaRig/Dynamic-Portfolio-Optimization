#include "SimuladorPortfolio.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace {
double calcularVariancia(const std::vector<double>& w,
                         const std::vector<std::vector<double>>& sigma) {
    double v = 0.0;
    for (size_t i = 0; i < w.size(); ++i)
        for (size_t j = 0; j < w.size(); ++j)
            v += w[i] * sigma[i][j] * w[j];
    return std::max(0.0, v);
}

double calcularCVaR(
    const std::vector<double>& w,
    double cdi,
    const std::vector<std::vector<double>>& cenarios,
    double alpha) {

    if (cenarios.empty()) return 0.0;
    if (!(alpha > 0.0 && alpha < 1.0))
        throw std::invalid_argument("cvar_alpha deve estar entre 0 e 1.");

    std::vector<double> perdas;
    perdas.reserve(cenarios.size());

    const double caixa = 1.0 - std::accumulate(w.begin(), w.end(), 0.0);

    for (const auto& retorno : cenarios) {
        double rp = caixa * cdi;
        for (size_t i = 0; i < w.size(); ++i)
            rp += w[i] * retorno[i];

        perdas.push_back(-rp);
    }

    std::sort(perdas.begin(), perdas.end(), std::greater<double>());

    const double tail_real = (1.0 - alpha) * perdas.size();
    const int n_tail = std::max(1, static_cast<int>(std::ceil(tail_real)));

    double soma = 0.0;
    for (int i = 0; i < n_tail; ++i)
        soma += perdas[i];

    return soma / static_cast<double>(n_tail);
}
}

std::vector<double> SimuladorPortfolio::repararPesos(
    const std::vector<double>& peso_inicial,
    const std::vector<double>& delta,
    double max_turnover) {

    if (peso_inicial.size() != delta.size())
        throw std::invalid_argument("peso_inicial e delta possuem tamanhos diferentes.");

    std::vector<double> candidato(peso_inicial.size());

    // 1. Candidato bruto
    for (size_t i = 0; i < candidato.size(); ++i)
        candidato[i] = peso_inicial[i] + delta[i];

    // 2. Long-only
    for (double& w : candidato)
        w = std::max(0.0, w);

    // 3. Orçamento: soma dos ativos de risco <= 1
    double soma = std::accumulate(candidato.begin(), candidato.end(), 0.0);
    if (soma > 1.0) {
        for (double& w : candidato)
            w /= soma;
    }

    // 4. Restrição dura de turnover.
    std::vector<double> delta_reparado(candidato.size());
    double turnover = 0.0;

    for (size_t i = 0; i < candidato.size(); ++i) {
        delta_reparado[i] = candidato[i] - peso_inicial[i];
        turnover += std::abs(delta_reparado[i]);
    }

    if (turnover > max_turnover && turnover > 0.0) {
        const double escala = max_turnover / turnover;
        for (size_t i = 0; i < candidato.size(); ++i)
            candidato[i] = peso_inicial[i] + escala * delta_reparado[i];
    }

    return candidato;
}

double SimuladorPortfolio::avaliarTrajetoria(
    const std::vector<double>& peso_inicial,
    const std::vector<std::vector<double>>& trajetoria_deltas,
    const std::vector<std::vector<double>>& matriz_covariancia,
    const std::vector<double>& retornos_esperados,
    const std::vector<double>& cdi_horizonte,
    const std::vector<std::vector<double>>& cenarios_retornos,
    const ParametrosCusto& params,
    double max_turnover) {

    const int N = static_cast<int>(trajetoria_deltas.size());
    if (N == 0) return 1e12;

    std::vector<double> pesos_atuais = peso_inicial;

    double retorno_acumulado = 0.0;
    double variancia_acumulada = 0.0;
    double cvar_acumulado = 0.0;
    double custo_transacao = 0.0;

    for (int k = 0; k < N; ++k) {
        const auto pesos_novos = repararPesos(
            pesos_atuais, trajetoria_deltas[k], max_turnover);

        double turnover = 0.0;
        for (size_t i = 0; i < pesos_novos.size(); ++i)
            turnover += std::abs(pesos_novos[i] - pesos_atuais[i]);

        const double soma_risco =
            std::accumulate(pesos_novos.begin(), pesos_novos.end(), 0.0);

        const double cdi = (k < static_cast<int>(cdi_horizonte.size()))
                               ? cdi_horizonte[k]
                               : cdi_horizonte.back();

        double retorno_risco = 0.0;
        for (size_t i = 0; i < pesos_novos.size(); ++i)
            retorno_risco += pesos_novos[i] * retornos_esperados[i];

        const double retorno_caixa = (1.0 - soma_risco) * cdi;
        retorno_acumulado += retorno_risco + retorno_caixa;

        variancia_acumulada += calcularVariancia(
            pesos_novos, matriz_covariancia);

        cvar_acumulado += calcularCVaR(
            pesos_novos, cdi, cenarios_retornos, params.cvar_alpha);

        // Custo linear de negociação, além da restrição de turnover.
        custo_transacao += turnover;

        pesos_atuais = pesos_novos;
    }

    const double retorno_medio = retorno_acumulado / N;
    const double variancia_media = variancia_acumulada / N;
    const double cvar_medio = cvar_acumulado / N;
    const double turnover_medio = custo_transacao / N;

    return
        -params.alpha * retorno_medio +
         params.gamma * variancia_media +
         params.beta * cvar_medio +
         params.eta * turnover_medio;
}
