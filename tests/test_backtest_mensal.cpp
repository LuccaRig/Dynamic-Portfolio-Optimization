#include "Backtest.hpp"
#include "DifferentialEvolution.hpp"
#include "Mercado.hpp"

#include <cmath>
#include <iostream>
#include <omp.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void exigir(bool condicao, const std::string& mensagem) {
    if (!condicao)
        throw std::runtime_error(mensagem);
}
}

int main() {
    omp_set_num_threads(1);

    const std::vector<std::vector<double>> retornos_diarios(
        12, std::vector<double>{0.05});
    const std::vector<double> cdi(12, 0.0);
    Mercado mercado(retornos_diarios, cdi, {"ativo"});

    ConfigBacktest config;
    config.janela_estatistica = 3;
    config.horizonte_mpc = 1;
    config.periodo_rebalanceamento = 3;
    config.max_turnover = 0.5;

    ParametrosDE parametros_de;
    parametros_de.tamanho_populacao = 64;
    parametros_de.max_geracoes = 0;

    ParametrosCusto parametros_custo;
    parametros_custo.alpha = 10.0;
    parametros_custo.beta = 0.0;
    parametros_custo.gamma = 0.0;
    parametros_custo.eta = 0.0;
    DifferentialEvolution de(parametros_de, parametros_custo);

    const auto resultado =
        executarBacktest(mercado, de, config, mercado.numAtivos());

    exigir(resultado.retornos_portfolio.size() == 9,
           "O backtest deve simular todos os pregoes apos a janela.");
    exigir(resultado.turnovers[0] > 0.0 &&
               resultado.turnovers[0] <= config.max_turnover,
           "O primeiro rebalanceamento deve respeitar o limite de turnover.");
    exigir(resultado.turnovers[1] == 0.0 &&
               resultado.turnovers[2] == 0.0 &&
               resultado.turnovers[4] == 0.0 &&
               resultado.turnovers[5] == 0.0,
           "Nao deve haver turnover nos pregoes entre rebalanceamentos.");
    exigir(resultado.turnovers[3] <= config.max_turnover &&
               resultado.turnovers[6] <= config.max_turnover,
           "Cada rebalanceamento deve respeitar o limite de turnover.");
    const double turnover_total =
        resultado.turnovers[0] + resultado.turnovers[3] +
        resultado.turnovers[6];
    exigir(std::abs(resultado.metricas.turnover_medio - turnover_total / 3.0) <
               1e-12,
           "O turnover medio deve ser calculado por rebalanceamento.");

    const double peso_ativo_a = resultado.retornos_portfolio[0] / 0.05;
    const double pnl_esperado_no_dia_seguinte =
        0.05 * peso_ativo_a * 1.05 /
        (1.0 + resultado.retornos_portfolio[0]);
    exigir(std::abs(
               resultado.retornos_portfolio[1] -
               pnl_esperado_no_dia_seguinte) < 1e-10,
           "Os pesos devem derivar naturalmente entre rebalanceamentos.");
    exigir(resultado.retornos_portfolio[0] > 0.0,
           "A configuracao de teste deve investir no ativo de retorno positivo.");

    const double peso_ativo_a_dia_seguinte =
        resultado.retornos_portfolio[1] / 0.05;
    const double pnl_esperado_no_terceiro_dia =
        0.05 * peso_ativo_a_dia_seguinte * 1.05 /
        (1.0 + resultado.retornos_portfolio[1]);
    exigir(std::abs(
               resultado.retornos_portfolio[2] -
               pnl_esperado_no_terceiro_dia) < 1e-10,
           "O drift deve continuar sendo aplicado sem negociacao.");

    std::cout << "Testes de rebalanceamento mensal aprovados." << std::endl;
    return 0;
}
