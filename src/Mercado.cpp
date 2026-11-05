#include "Mercado.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {
std::vector<std::vector<double>> construirCenariosPorPeriodo(
    const std::vector<std::vector<double>>& retornos,
    int inicio,
    int fim,
    int tamanho_periodo,
    int num_ativos) {

    if (tamanho_periodo <= 0)
        throw std::invalid_argument("tamanho_periodo deve ser positivo.");

    std::vector<std::vector<double>> cenarios;
    for (int inicio_periodo = inicio;
         inicio_periodo + tamanho_periodo <= fim;
         ++inicio_periodo) {
        std::vector<double> retorno_periodo(num_ativos, 0.0);
        for (int ativo = 0; ativo < num_ativos; ++ativo) {
            double fator = 1.0;
            for (int dia = inicio_periodo;
                 dia < inicio_periodo + tamanho_periodo;
                 ++dia) {
                fator *= 1.0 + retornos[dia][ativo];
            }
            retorno_periodo[ativo] = fator - 1.0;
        }
        cenarios.push_back(std::move(retorno_periodo));
    }

    return cenarios;
}
}

Mercado::Mercado(
    const std::vector<std::vector<double>>& historico_retornos_diarios,
    const std::vector<double>& historico_cdi,
    const std::vector<std::string> nomes_ativos)
    : historico_retornos_(historico_retornos_diarios),
      historico_cdi_(historico_cdi),
      nomes_ativos_(nomes_ativos) {

    num_dias_ = static_cast<int>(historico_retornos_.size());
    if (num_dias_ > 0)
        num_ativos_ = static_cast<int>(historico_retornos_[0].size());

    for (const auto& linha : historico_retornos_) {
        if (static_cast<int>(linha.size()) != num_ativos_)
            throw std::invalid_argument("Historico de retornos possui linhas com tamanhos diferentes.");
    }
}

int Mercado::tamanhoHistorico() const { return num_dias_; }
int Mercado::numAtivos() const { return num_ativos_; }
std::vector<std::string> Mercado::obterNomesAtivos() const {return nomes_ativos_;}

std::vector<double> Mercado::calcularRetornosEsperados(
    int dia_atual, int tamanho_janela, int tamanho_periodo) const {

    std::vector<double> mu(num_ativos_, 0.0);
    if (tamanho_periodo <= 0)
        throw std::invalid_argument("tamanho_periodo deve ser positivo.");
    if (tamanho_janela <= 0 || dia_atual <= 0) return mu;

    const int inicio = std::max(0, dia_atual - tamanho_janela);
    const int fim = std::min(dia_atual, num_dias_);
    const auto cenarios = construirCenariosPorPeriodo(
        historico_retornos_, inicio, fim, tamanho_periodo, num_ativos_);
    if (cenarios.empty()) return mu;

    for (int j = 0; j < num_ativos_; ++j) {
        for (const auto& cenario : cenarios)
            mu[j] += cenario[j];
        mu[j] /= static_cast<double>(cenarios.size());
    }

    return mu;
}

std::vector<std::vector<double>> Mercado::calcularMatrizCovariancia(
    int dia_atual, int tamanho_janela, int tamanho_periodo) const {

    std::vector<std::vector<double>> sigma(
        num_ativos_, std::vector<double>(num_ativos_, 0.0));

    if (tamanho_periodo <= 0)
        throw std::invalid_argument("tamanho_periodo deve ser positivo.");
    if (tamanho_janela <= 0 || dia_atual <= 0) return sigma;

    const int inicio = std::max(0, dia_atual - tamanho_janela);
    const int fim = std::min(dia_atual, num_dias_);
    const auto cenarios = construirCenariosPorPeriodo(
        historico_retornos_, inicio, fim, tamanho_periodo, num_ativos_);
    const int n = static_cast<int>(cenarios.size());
    if (n <= 1) return sigma;

    std::vector<double> medias(num_ativos_, 0.0);
    for (const auto& cenario : cenarios) {
        for (int j = 0; j < num_ativos_; ++j)
            medias[j] += cenario[j];
    }
    for (double& media : medias)
        media /= static_cast<double>(n);

    for (int r = 0; r < num_ativos_; ++r) {
        for (int c = r; c < num_ativos_; ++c) {
            double soma = 0.0;
            for (const auto& cenario : cenarios) {
                soma += (cenario[r] - medias[r]) *
                        (cenario[c] - medias[c]);
            }
            sigma[r][c] = soma / static_cast<double>(n - 1);
            sigma[c][r] = sigma[r][c];
        }
    }

    return sigma;
}

std::vector<std::vector<double>> Mercado::obterCenariosRetorno(
    int dia_atual, int tamanho_janela, int tamanho_periodo) const {

    if (tamanho_periodo <= 0)
        throw std::invalid_argument("tamanho_periodo deve ser positivo.");
    if (tamanho_janela <= 0 || dia_atual <= 0)
        return {};

    const int inicio = std::max(0, dia_atual - tamanho_janela);
    const int fim = std::min(dia_atual, num_dias_);

    return construirCenariosPorPeriodo(
        historico_retornos_, inicio, fim, tamanho_periodo, num_ativos_);
}

std::vector<double> Mercado::obterRetornosReais(int dia_atual) const {
    if (dia_atual < 0 || dia_atual >= num_dias_)
        throw std::out_of_range("Dia atual fora dos limites do historico.");
    return historico_retornos_[dia_atual];
}

double Mercado::obterCdiDia(int dia_atual) const {
    if (dia_atual < 0 || dia_atual >= static_cast<int>(historico_cdi_.size()))
        throw std::out_of_range("Dia atual fora dos limites do CDI.");
    return historico_cdi_[dia_atual];
}
