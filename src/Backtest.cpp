#include "Backtest.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "DifferentialEvolution.hpp"
#include "Mercado.hpp"

ResultadoBacktest executarBacktest(
    Mercado& mercado,
    DifferentialEvolution& de,
    const ConfigBacktest& config,
    int num_ativos)
{
    ResultadoBacktest resultado;

    if (config.periodo_rebalanceamento <= 0)
        throw std::invalid_argument(
            "periodo_rebalanceamento deve ser positivo.");
    if (config.janela_estatistica <= 0)
        throw std::invalid_argument("janela_estatistica deve ser positiva.");
    if (config.janela_estatistica < config.periodo_rebalanceamento)
        throw std::invalid_argument(
            "janela_estatistica deve conter ao menos um periodo de rebalanceamento.");
    if (config.horizonte_mpc <= 0)
        throw std::invalid_argument("horizonte_mpc deve ser positivo.");

    const int total_dias = mercado.tamanhoHistorico();
    const int dia_inicio = config.janela_estatistica;
    const int dia_fim = total_dias;

    if (dia_inicio >= dia_fim)
        return resultado;

    const int quantidade_dias = dia_fim - dia_inicio;

    resultado.retornos_portfolio.reserve(quantidade_dias);
    resultado.retornos_cdi.reserve(quantidade_dias);
    resultado.retornos_bh.reserve(quantidade_dias);
    resultado.turnovers.reserve(quantidade_dias);

    std::vector<double> peso_atual(
        num_ativos,
        0.0
    );

    std::vector<double> pesos_bh; 
    bool bh_inicializado = false;

    std::cout
        << "\nIniciando simulacao de rebalanceamento mensal via MPC..."
        << std::endl;

    for (int t = dia_inicio; t < dia_fim; ++t) {

        const double cdi_hoje =
            mercado.obterCdiDia(t);

        double turnover_dia = 0.0;
        const bool dia_rebalanceamento =
            (t - dia_inicio) % config.periodo_rebalanceamento == 0;

        if (dia_rebalanceamento) {
            const auto retornos_esperados =
                mercado.calcularRetornosEsperados(
                    t,
                    config.janela_estatistica,
                    config.periodo_rebalanceamento
                );

            const auto matriz_cov =
                mercado.calcularMatrizCovariancia(
                    t,
                    config.janela_estatistica,
                    config.periodo_rebalanceamento
                );

            const double cdi_periodo =
                std::pow(1.0 + cdi_hoje, config.periodo_rebalanceamento) - 1.0;
            const std::vector<double> cdi_horizonte(
                config.horizonte_mpc,
                cdi_periodo
            );

            const auto retornos_cenarios =
                mercado.obterCenariosRetorno(
                    t,
                    config.janela_estatistica,
                    config.periodo_rebalanceamento
                );

            const auto melhor_trajetoria_deltas =
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

            const auto& delta = melhor_trajetoria_deltas[0];
            for (int i = 0; i < num_ativos; ++i) {
                turnover_dia += std::abs(delta[i]);
                peso_atual[i] += delta[i];
            }
        }

        if (!bh_inicializado) {
            pesos_bh = peso_atual; 
            resultado.pesos_iniciais = peso_atual;
            resultado.nomes_ativos = mercado.obterNomesAtivos();
            bh_inicializado = true;
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

        const double peso_caixa = 1.0 - soma_pesos;

        pnl_dia += peso_caixa * cdi_hoje;

        const double patrimonio_futuro = 1.0 + pnl_dia;
        if (patrimonio_futuro <= 0.0) {
            std::cerr
                << "Erro: patrimonio do portfolio tornou-se nao positivo."
                << std::endl;
            return resultado;
        }

        for (int i = 0; i < num_ativos; ++i) {
            peso_atual[i] =
                peso_atual[i] * (1.0 + retornos_reais[i]) /
                patrimonio_futuro;
        }

        double pnl_bh_dia = 0.0;
        double soma_pesos_bh = 0.0;
        std::vector<double> novos_pesos_bh_brutos(num_ativos, 0.0);
        double valor_total_bh_futuro = 0.0;

        for (int i = 0; i < num_ativos; ++i) {
            pnl_bh_dia += pesos_bh[i] * retornos_reais[i];
            soma_pesos_bh += pesos_bh[i];
            
            // Efeito preço orgânico para o próximo dia (drift passivo)
            novos_pesos_bh_brutos[i] = pesos_bh[i] * (1.0 + retornos_reais[i]);
            valor_total_bh_futuro += novos_pesos_bh_brutos[i];
        }
        pnl_bh_dia += (1.0 - soma_pesos_bh) * cdi_hoje;

        // Atualiza os pesos passivos do B&H para o próximo dia (sem custos de turnover)
        if (valor_total_bh_futuro > 0.0) {
            // Ajusta considerando o caixa que rende CDI organicamente ou renormaliza se 100% ações
            double fator_caixa = (1.0 - soma_pesos_bh) * (1.0 + cdi_hoje);
            double patrimonio_total = valor_total_bh_futuro + fator_caixa;
            
            if (patrimonio_total > 0.0) {
                for (int i = 0; i < num_ativos; ++i) {
                    pesos_bh[i] = novos_pesos_bh_brutos[i] / patrimonio_total;
                }
            }
        }

        resultado.retornos_portfolio.push_back(pnl_dia);
        resultado.retornos_cdi.push_back(cdi_hoje);
        resultado.retornos_bh.push_back(pnl_bh_dia);
        resultado.turnovers.push_back(turnover_dia);

        std::cout
            << "[Dia " << t << "] "
            << "CDI=" << cdi_hoje * 100.0
            << "% | PnL=" << pnl_dia * 100.0
            << "% | Turnover=" << turnover_dia * 100.0
            << "%"
            << std::endl;
    }

    resultado.pesos_finais = peso_atual;

    resultado.metricas_bh = calcularMetricas(
        resultado.retornos_bh,
        resultado.retornos_cdi,
        std::vector<double>(resultado.retornos_bh.size(), 0.0),
        config.dias_ano
    );

    resultado.metricas = calcularMetricas(
        resultado.retornos_portfolio,
        resultado.retornos_cdi,
        resultado.turnovers,
        config.dias_ano,
        config.periodo_rebalanceamento
    );

    return resultado;
}
