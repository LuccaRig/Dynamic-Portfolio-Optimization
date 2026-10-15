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
    auto historico_retornos = carregarCSV2D("historico_retornos.csv");

    auto historico_cdi = carregarCSV1D("historico_cdi.csv");

    auto nome_ativos = carregarCSVTickers("ativos_nomes.csv");

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
        historico_cdi,
        nome_ativos
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

    // printar params_custo
    std::cout << "\n=== PARAMETROS DA FUNCAO OBJETIVO === \n"
              << "alpha: " << params_custo.alpha << "\n"
              << "beta: " << params_custo.beta << "\n"
              << "gama: " << params_custo.gamma << "\n"
              << "eta (custo da transacao): " << params_custo.eta << "\n"
              << "cvar_alpha: " << params_custo.cvar_alpha
              << std::endl;


    // 5. Evolucao Diferencial
    ParametrosDE params_de;

    std::cout << "\n=== PARAMETROS DA EVOLUCAO DIFERENCIAL === \n"
              << "tamanho_populacao: " << params_de.tamanho_populacao << "\n"
              << "max_geracoes: " << params_de.max_geracoes << "\n"
              << "F: " << params_de.F << "\n"
              << "CR: " << params_de.CR
              << std::endl;
    
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
