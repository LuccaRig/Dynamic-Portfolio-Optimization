#include "Backtest.hpp"

#include <cmath>
#include <iostream>

#include "DifferentialEvolution.hpp"
#include "Mercado.hpp"

ResultadoBacktest executarBacktest(
    Mercado& mercado,
    DifferentialEvolution& de,
    const ConfigBacktest& config,
    int num_ativos)
{
    ResultadoBacktest resultado;

    const int total_dias = mercado.tamanhoHistorico();
    const int dia_inicio = config.janela_estatistica;
    const int dia_fim = total_dias - config.horizonte_mpc;

    if (dia_inicio >= dia_fim)
        return resultado;

    const int quantidade_dias = dia_fim - dia_inicio;

    resultado.retornos_portfolio.reserve(quantidade_dias);
    resultado.retornos_cdi.reserve(quantidade_dias);
    resultado.turnovers.reserve(quantidade_dias);

    std::vector<double> peso_atual(
        num_ativos,
        0.0
    );

    std::cout
        << "\nIniciando simulacao de rebalanceamento via MPC..."
        << std::endl;

    for (int t = dia_inicio; t < dia_fim; ++t) {

        auto retornos_esperados =
            mercado.calcularRetornosEsperados(
                t,
                config.janela_estatistica
            );

        auto matriz_cov =
            mercado.calcularMatrizCovariancia(
                t,
                config.janela_estatistica
            );

        // Sem look-ahead: somente o CDI conhecido em t.
        const double cdi_hoje =
            mercado.obterCdiDia(t);

        std::vector<double> cdi_horizonte(
            config.horizonte_mpc,
            cdi_hoje
        );

        auto retornos_cenarios =
            mercado.obterCenariosRetorno(
                t,
                config.janela_estatistica
            );

        auto melhor_trajetoria_deltas =
            de.otimizar(
                peso_atual,
                matriz_cov,
                retornos_esperados,
                cdi_horizonte,
                retornos_cenarios,
                config.max_turnover,
                config.horizonte_mpc,
                num_ativos
            );

        if (melhor_trajetoria_deltas.empty()) {
            std::cerr
                << "Erro: DE retornou uma trajetoria vazia."
                << std::endl;
            return resultado;
        }

        const auto& delta =
            melhor_trajetoria_deltas[0];

        double turnover_dia = 0.0;

        for (int i = 0; i < num_ativos; ++i) {
            turnover_dia += std::abs(delta[i]);
            peso_atual[i] += delta[i];
        }

        auto retornos_reais =
            mercado.obterRetornosReais(t);

        double pnl_dia = 0.0;
        double soma_pesos = 0.0;

        for (int i = 0; i < num_ativos; ++i) {
            pnl_dia +=
                peso_atual[i] *
                retornos_reais[i];

            soma_pesos += peso_atual[i];
        }

        const double peso_caixa =
            1.0 - soma_pesos;

        pnl_dia +=
            peso_caixa * cdi_hoje;

        resultado.retornos_portfolio.push_back(pnl_dia);
        resultado.retornos_cdi.push_back(cdi_hoje);
        resultado.turnovers.push_back(turnover_dia);

        std::cout
            << "[Dia " << t << "] "
            << "CDI=" << cdi_hoje * 100.0
            << "% | PnL=" << pnl_dia * 100.0
            << "% | Turnover=" << turnover_dia * 100.0
            << "%"
            << std::endl;
    }

    resultado.metricas = calcularMetricas(
        resultado.retornos_portfolio,
        resultado.retornos_cdi,
        resultado.turnovers,
        config.dias_ano
    );

    return resultado;
}
