#include "Relatorio.hpp"

#include <iomanip>
#include <iostream>

void imprimirRelatorio(
    const ResultadoBacktest& resultado,
    const ConfigBacktest& config,
    double cvar_alpha)
{
    const auto& m = resultado.metricas;

    if (resultado.retornos_portfolio.empty())
        return;

    std::cout
        << std::fixed
        << std::setprecision(4);

    std::cout
        << "\n\n====================================================\n"
        << "           ANALISE ESTATISTICA DO BACKTEST\n"
        << "====================================================\n";

    std::cout
        << "\n--- GERAL ---\n"
        << "Dias simulados             : "
        << resultado.retornos_portfolio.size()
        << std::endl;

    std::cout
        << "\n--- RETORNO ---\n"
        << "Retorno medio diario       : "
        << m.media_diaria * 100.0 << "%\n"
        << "Retorno acumulado          : "
        << m.retorno_acumulado * 100.0 << "%\n"
        << "Retorno anualizado         : "
        << m.retorno_anualizado * 100.0 << "%\n"
        << "Dias positivos             : "
        << m.percentual_dias_positivos << "%\n";

    std::cout
        << "\n--- RISCO ---\n"
        << "Volatilidade diaria        : "
        << m.volatilidade_diaria * 100.0 << "%\n"
        << "Volatilidade anualizada    : "
        << m.volatilidade_anualizada * 100.0 << "%\n"
        << "Sharpe                     : "
        << m.sharpe << "\n"
        << "Max Drawdown               : "
        << m.drawdown_maximo * 100.0 << "%\n"
        << "VaR 95% diario             : "
        << m.var_95 * 100.0 << "%\n"
        << "CVaR 95% diario            : "
        << m.cvar_95 * 100.0 << "%\n";

    std::cout
        << "\n--- CDI ---\n"
        << "CDI acumulado              : "
        << m.retorno_cdi_acumulado * 100.0 << "%\n"
        << "Excesso sobre CDI          : "
        << m.excesso_cdi * 100.0 << "%\n";

    std::cout
        << "\n--- TURNOVER ---\n"
        << "Turnover medio             : "
        << m.turnover_medio * 100.0 << "%\n"
        << "Turnover maximo            : "
        << m.turnover_maximo * 100.0 << "%\n";

    std::cout
        << "\n--- CONFIGURACAO ---\n"
        << "Janela estatistica         : "
        << config.janela_estatistica << " dias\n"
        << "Horizonte MPC              : "
        << config.horizonte_mpc << " dias\n"
        << "Turnover maximo permitido : "
        << config.max_turnover * 100.0 << "%\n"
        << "CVaR alpha                 : "
        << cvar_alpha << "\n"
        << "====================================================\n";
}
