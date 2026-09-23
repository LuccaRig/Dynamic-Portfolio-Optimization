#include "Mercado.hpp"
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <algorithm>

Mercado::Mercado(const std::vector<std::vector<double>>& historico_retornos_diarios,
                const std::vector<double>& historico_cdi)
    : historico_retornos_(historico_retornos_diarios), historico_cdi_(historico_cdi) {
    num_dias_ = historico_retornos_.size();
    if (num_dias_ > 0) {
        num_ativos_ = historico_retornos_[0].size();
    } else {
        num_ativos_ = 0;
    }
}

int Mercado::tamanhoHistorico() const { return num_dias_; }
int Mercado::numAtivos() const { return num_ativos_; }

std::vector<double> Mercado::calcularRetornosEsperados(int dia_atual, int tamanho_janela) const {
    std::vector<double> mu(num_ativos_, 0.0);
    if (tamanho_janela <= 0 || dia_atual <= 0) return mu;
    int inicio = std::max(0, dia_atual - tamanho_janela);
    int fim = dia_atual;
    int n_amostras = fim - inicio;
    if (n_amostras <= 0) return mu;

    for (int j = 0; j < num_ativos_; ++j) {
        double soma = 0.0;
        for (int i = inicio; i < fim; ++i) {
            soma += historico_retornos_[i][j];
        }
        mu[j] = soma / n_amostras;
    }
    return mu;
}

std::vector<std::vector<double>> Mercado::calcularMatrizCovariancia(int dia_atual, int tamanho_janela) const {
    std::vector<std::vector<double>> sigma(num_ativos_, std::vector<double>(num_ativos_, 0.0));
    if (tamanho_janela <= 1 || dia_atual <= 1) return sigma;
    int inicio = std::max(0, dia_atual - tamanho_janela);
    int fim = dia_atual;
    int n_amostras = fim - inicio;
    if (n_amostras <= 1) return sigma;

    std::vector<double> medias(num_ativos_, 0.0);
    for (int j = 0; j < num_ativos_; ++j) {
        double soma = 0.0;
        for (int i = inicio; i < fim; ++i) {
            soma += historico_retornos_[i][j];
        }
        medias[j] = soma / n_amostras;
    }

    for (int r = 0; r < num_ativos_; ++r) {
        for (int c = r; c < num_ativos_; ++c) {
            double soma_cov = 0.0;
            for (int i = inicio; i < fim; ++i) {
                soma_cov += (historico_retornos_[i][r] - medias[r]) * (historico_retornos_[i][c] - medias[c]);
            }
            sigma[r][c] = soma_cov / (n_amostras - 1);
            sigma[c][r] = sigma[r][c];
        }
    }
    return sigma;
}

std::vector<double> Mercado::obterRetornosReais(int dia_atual) const {
    if (dia_atual < 0 || dia_atual >= num_dias_) {
        throw std::out_of_range("Dia atual fora dos limites do historico de retornos.");
    }
    return historico_retornos_[dia_atual];
}

double Mercado::obterCdiDia(int dia_atual) const {
    if (dia_atual < 0 || dia_atual >= (int)historico_cdi_.size()) {
        return 0.0003; // Fallback seguro caso estoure
    }
    return historico_cdi_[dia_atual];
}

std::vector<double> Mercado::obterCdiHorizonte(int dia_atual, int horizonte_N) const {
    std::vector<double> cdi_h(horizonte_N);
    for (int k = 0; k < horizonte_N; ++k) {
        int d = dia_atual + k;
        cdi_h[k] = obterCdiDia(d);
    }
    return cdi_h;
}