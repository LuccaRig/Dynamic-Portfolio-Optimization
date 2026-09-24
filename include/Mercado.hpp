#pragma once
#include <vector>
#include <string>

class Mercado {
public:
    Mercado(const std::vector<std::vector<double>>& historico_retornos_diarios,
            const std::vector<double>& historico_cdi,
            const std::vector<std::string> nomes_ativos);

    int tamanhoHistorico() const;
    int numAtivos() const;

    std::vector<double> calcularRetornosEsperados(int dia_atual, int tamanho_janela) const;
    std::vector<std::vector<double>> calcularMatrizCovariancia(int dia_atual, int tamanho_janela) const;

    std::vector<std::vector<double>> obterCenariosRetorno(int dia_atual, int tamanho_janela) const;
    std::vector<double> obterRetornosReais(int dia_atual) const;
    double obterCdiDia(int dia_atual) const;
    std::vector<std::string> obterNomesAtivos() const;

private:
    std::vector<std::vector<double>> historico_retornos_;
    std::vector<std::string> nomes_ativos_;
    std::vector<double> historico_cdi_;
    int num_dias_ = 0;
    int num_ativos_ = 0;
};
