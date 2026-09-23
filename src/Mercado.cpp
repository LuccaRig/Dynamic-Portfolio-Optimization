#include "Mercado.hpp"
#include <numeric>
#include <cmath>
#include <stdexcept>

Mercado::Mercado(const std::vector<std::vector<double>>& historico_retornos_diarios)
    : historico_(historico_retornos_diarios) {
    num_dias_ = historico_.size();
    if (num_dias_ > 0) {
        num_ativos_ = historico_[0].size();
    } else {
        num_ativos_ = 0;
    }
}

int Mercado::tamanhoHistorico() const {
    return num_dias_;
}

int Mercado::numAtivos() const {
    return num_ativos_;
}

std::vector<double> Mercado::calcularRetornosEsperados(int dia_atual, int tamanho_janela) const {
    std::vector<double> mu(num_ativos_, 0.0);
    if (tamanho_janela <= 0 || dia_atual <= 0) return mu;

    // Garante causalidade: olha apenas para o passado (de dia_atual - tamanho_janela até dia_atual - 1)
    int inicio = std::max(0, dia_atual - tamanho_janela);
    int fim = dia_atual; // exclusivo
    int n_amostras = fim - inicio;

    if (n_amostras <= 0) return mu;

    for (int j = 0; j < num_ativos_; ++j) {
        double soma = 0.0;
        for (int i = inicio; i < fim; ++i) {
            soma += historico_[i][j];
        }
        mu[j] = soma / n_amostras;
    }

    return mu;
}

std::vector<std::vector<double>> Mercado::calcularMatrizCovariancia(int dia_atual, int tamanho_janela) const {
    // Inicializa matriz M x M com zeros
    std::vector<std::vector<double>> sigma(num_ativos_, std::vector<double>(num_ativos_, 0.0));
    
    if (tamanho_janela <= 1 || dia_atual <= 1) return sigma;

    int inicio = std::max(0, dia_atual - tamanho_janela);
    int fim = dia_atual;
    int n_amostras = fim - inicio;

    if (n_amostras <= 1) return sigma;

    // 1. Calcula a média dos retornos na janela para cada ativo
    std::vector<double> medias(num_ativos_, 0.0);
    for (int j = 0; j < num_ativos_; ++j) {
        double soma = 0.0;
        for (int i = inicio; i < fim; ++i) {
            soma += historico_[i][j];
        }
        medias[j] = soma / n_amostras;
    }

    // 2. Calcula a covariância amostral entre os ativos
    for (int r = 0; r < num_ativos_; ++r) {
        for (int c = r; c < num_ativos_; ++c) {
            double soma_cov = 0.0;
            for (int i = inicio; i < fim; ++i) {
                soma_cov += (historico_[i][r] - medias[r]) * (historico_[i][c] - medias[c]);
            }
            // Divisão por (n_amostras - 1) para covariância amostral não-viciada (Bessel)
            sigma[r][c] = soma_cov / (n_amostras - 1);
            // A matriz de covariância é simétrica
            sigma[c][r] = sigma[r][c];
        }
    }

    return sigma;
}

std::vector<double> Mercado::obterRetornosReais(int dia_atual) const {
    if (dia_atual < 0 || dia_atual >= num_dias_) {
        throw std::out_of_range("Dia atual fora dos limites do histórico de mercado.");
    }
    return historico_[dia_atual];
}