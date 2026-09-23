#ifndef MERCADO_HPP
#define MERCADO_HPP

#include <vector>

class Mercado {
public:
    // Construtor recebe a matriz completa de histórico de retornos diários [dias x ativos]
    Mercado(const std::vector<std::vector<double>>& historico_retornos_diarios);

    // Retorna o número total de dias disponíveis no histórico
    int tamanhoHistorico() const;

    // Retorna o número de ativos monitorados
    int numAtivos() const;

    // Estima os retornos esperados (mu) usando janela histórica passada (sem look-ahead)
    std::vector<double> calcularRetornosEsperados(int dia_atual, int tamanho_janela) const;

    // Estima a matriz de covariância (Sigma) usando janela histórica passada (sem look-ahead)
    std::vector<std::vector<double>> calcularMatrizCovariancia(int dia_atual, int tamanho_janela) const;

    // Retorna os retornos reais realizados no dia_atual (para simular a execução e o P&L real)
    std::vector<double> obterRetornosReais(int dia_atual) const;

private:
    std::vector<std::vector<double>> historico_;
    int num_dias_;
    int num_ativos_;
};

#endif