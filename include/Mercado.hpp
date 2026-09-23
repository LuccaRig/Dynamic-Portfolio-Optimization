#ifndef MERCADO_HPP
#define MERCADO_HPP

#include <vector>

class Mercado {
public:
    Mercado(const std::vector<std::vector<double>>& historico_retornos_diarios,
            const std::vector<double>& historico_cdi);

    int tamanhoHistorico() const;
    int numAtivos() const;
    std::vector<double> calcularRetornosEsperados(int dia_atual, int tamanho_janela) const;
    std::vector<std::vector<double>> calcularMatrizCovariancia(int dia_atual, int tamanho_janela) const;
    std::vector<double> obterRetornosReais(int dia_atual) const;
    double obterCdiDia(int dia_atual) const;
    std::vector<double> obterCdiHorizonte(int dia_atual, int horizonte_N) const;

private:
    std::vector<std::vector<double>> historico_retornos_;
    std::vector<double> historico_cdi_;
    int num_dias_;
    int num_ativos_;
};

#endif