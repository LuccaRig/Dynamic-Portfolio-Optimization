#include <iostream>

#include "Backtest.hpp"
#include "Dados.hpp"
#include "DifferentialEvolution.hpp"
#include "Mercado.hpp"
#include "Relatorio.hpp"

int main()
{
    std::cout
        << "=== INICIANDO SISTEMA MPC COM "
        << "EVOLUCAO DIFERENCIAL (OpenMP) ==="
        << std::endl;

    // 1. Dados
    auto historico_retornos =
        carregarCSV2D("historico_retornos.csv");

    auto historico_cdi =
        carregarCSV1D("historico_cdi.csv");

    if (historico_retornos.empty() ||
        historico_cdi.empty()) {

        std::cerr
            << "Erro: arquivos CSV ausentes ou vazios."
            << std::endl;

        return 1;
    }

    // 2. Mercado
    Mercado mercado(
        historico_retornos,
        historico_cdi
    );

    const int num_ativos =
        mercado.numAtivos();

    std::cout
        << "Dados carregados! Dias disponiveis: "
        << mercado.tamanhoHistorico()
        << " | Ativos: "
        << num_ativos
        << std::endl;

    // 3. Configuracao do backtest
    ConfigBacktest config;

    // 4. Custos
    ParametrosCusto params_custo;

    params_custo.alpha = 1.0;
    params_custo.beta = 0.5;
    params_custo.gamma = 0.8;
    params_custo.eta = 0.1;
    params_custo.cvar_alpha = 0.95;

    // 5. Evolucao Diferencial
    ParametrosDE params_de;

    params_de.tamanho_populacao = 60;
    params_de.max_geracoes = 40;
    params_de.F = 0.7;
    params_de.CR = 0.9;

    DifferentialEvolution de(
        params_de,
        params_custo
    );

    // 6. Executa backtest
    auto resultado =
        executarBacktest(
            mercado,
            de,
            config,
            num_ativos
        );

    if (resultado.retornos_portfolio.empty()) {
        std::cerr
            << "Erro: backtest nao produziu resultados."
            << std::endl;
        return 1;
    }

    // 7. Relatorio
    imprimirRelatorio(
        resultado,
        config,
        params_custo.cvar_alpha
    );

    std::cout
        << "\n=== SIMULACAO MPC CONCLUIDA ==="
        << std::endl;

    return 0;
}
