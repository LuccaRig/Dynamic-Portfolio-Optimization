#include "Estatisticas.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

double media(const std::vector<double>& valores)
{
    if (valores.empty())
        return 0.0;

    return std::accumulate(
        valores.begin(),
        valores.end(),
        0.0
    ) / static_cast<double>(valores.size());
}

double desvioPadrao(const std::vector<double>& valores)
{
    if (valores.size() < 2)
        return 0.0;

    const double mu = media(valores);
    double soma = 0.0;

    for (double x : valores)
        soma += (x - mu) * (x - mu);

    return std::sqrt(
        soma / static_cast<double>(valores.size() - 1)
    );
}

double retornoAcumulado(const std::vector<double>& retornos)
{
    double riqueza = 1.0;

    for (double r : retornos)
        riqueza *= (1.0 + r);

    return riqueza - 1.0;
}

double maxDrawdown(const std::vector<double>& retornos)
{
    if (retornos.empty())
        return 0.0;

    double riqueza = 1.0;
    double pico = 1.0;
    double maior_drawdown = 0.0;

    for (double r : retornos) {
        riqueza *= (1.0 + r);

        if (riqueza > pico)
            pico = riqueza;

        const double drawdown = (riqueza - pico) / pico;

        if (drawdown < maior_drawdown)
            maior_drawdown = drawdown;
    }

    return maior_drawdown;
}

double VaR95(const std::vector<double>& retornos)
{
    if (retornos.empty())
        return 0.0;

    std::vector<double> perdas;
    perdas.reserve(retornos.size());

    for (double r : retornos)
        perdas.push_back(-r);

    std::sort(perdas.begin(), perdas.end());

    const size_t indice = static_cast<size_t>(
        std::floor(0.95 * static_cast<double>(perdas.size() - 1))
    );

    return perdas[indice];
}

double CVaR95(const std::vector<double>& retornos)
{
    if (retornos.empty())
        return 0.0;

    std::vector<double> perdas;
    perdas.reserve(retornos.size());

    for (double r : retornos)
        perdas.push_back(-r);

    std::sort(perdas.begin(), perdas.end());

    size_t indice = static_cast<size_t>(
        std::ceil(0.95 * static_cast<double>(perdas.size()))
    );

    if (indice >= perdas.size())
        indice = perdas.size() - 1;

    double soma = 0.0;
    size_t quantidade = 0;

    for (size_t i = indice; i < perdas.size(); ++i) {
        soma += perdas[i];
        ++quantidade;
    }

    if (quantidade == 0)
        return 0.0;

    return soma / static_cast<double>(quantidade);
}

MetricasBacktest calcularMetricas(
    const std::vector<double>& retornos_portfolio,
    const std::vector<double>& retornos_cdi,
    const std::vector<double>& turnovers,
    int dias_ano)
{
    MetricasBacktest m;

    if (retornos_portfolio.empty())
        return m;

    // 1. Calcular retornos líquidos descontando o turnover e as taxas da B3
    // Taxa B3 total: 0,0050% (negociação) + 0,0180% (liquidação) = 0,0230% (0.00023)
    // Taxa Total por Giro = 0,00023(B3) + 0,00030(Spread)
    const double taxa_b3 = 0.00053; 
    std::vector<double> retornos_liquidos = retornos_portfolio;

    for (size_t i = 0; i < retornos_liquidos.size(); ++i) {
        double t = (i < turnovers.size()) ? turnovers[i] : 0.0;
        // Subtrai o custo do turnover do retorno do período
        retornos_liquidos[i] -= (t * taxa_b3); 
    }

    // 2. Utilizar os retornos líquidos para todas as métricas de desempenho
    m.media_diaria = media(retornos_liquidos);
    m.volatilidade_diaria = desvioPadrao(retornos_liquidos);
    m.retorno_acumulado = retornoAcumulado(retornos_liquidos);
    m.retorno_cdi_acumulado = retornoAcumulado(retornos_cdi);

    m.retorno_anualizado =
        std::pow(
            1.0 + m.retorno_acumulado,
            static_cast<double>(dias_ano) /
            static_cast<double>(retornos_liquidos.size())
        ) - 1.0;

    m.volatilidade_anualizada =
        m.volatilidade_diaria *
        std::sqrt(static_cast<double>(dias_ano));

    double media_excesso_cdi = 0.0;

    const size_t n = std::min(
        retornos_liquidos.size(),
        retornos_cdi.size()
    );

    for (size_t i = 0; i < n; ++i)
        media_excesso_cdi += retornos_liquidos[i] - retornos_cdi[i];

    if (n > 0)
        media_excesso_cdi /= static_cast<double>(n);

    if (m.volatilidade_diaria > 0.0) {
        m.sharpe =
            (media_excesso_cdi / m.volatilidade_diaria) *
            std::sqrt(static_cast<double>(dias_ano));
    }

    m.drawdown_maximo = maxDrawdown(retornos_liquidos);
    m.var_95 = VaR95(retornos_liquidos);
    m.cvar_95 = CVaR95(retornos_liquidos);

    m.turnover_medio = media(turnovers);

    if (!turnovers.empty())
        m.turnover_maximo =
            *std::max_element(turnovers.begin(), turnovers.end());

    size_t dias_positivos = 0;

    for (double r : retornos_liquidos) {
        if (r > 0.0)
            ++dias_positivos;
    }

    m.percentual_dias_positivos =
        100.0 *
        static_cast<double>(dias_positivos) /
        static_cast<double>(retornos_liquidos.size());

    m.excesso_cdi =
        m.retorno_acumulado - m.retorno_cdi_acumulado;

    return m;
}
