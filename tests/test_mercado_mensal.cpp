#include "Mercado.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void exigir(bool condicao, const std::string& mensagem) {
    if (!condicao)
        throw std::runtime_error(mensagem);
}
}

int main() {
    const std::vector<std::vector<double>> retornos = {
        {0.1},
        {0.1},
        {0.0},
        {0.2}
    };
    const Mercado mercado(retornos, {0.0, 0.0, 0.0, 0.0}, {"ativo"});

    const auto cenarios = mercado.obterCenariosRetorno(4, 4, 2);
    exigir(cenarios.size() == 3,
           "Devem ser criados blocos deslizantes completos.");
    exigir(std::abs(cenarios[0][0] - 0.21) < 1e-12 &&
               std::abs(cenarios[1][0] - 0.10) < 1e-12 &&
               std::abs(cenarios[2][0] - 0.20) < 1e-12,
           "Os retornos dos blocos devem ser compostos.");

    const auto media = mercado.calcularRetornosEsperados(4, 4, 2);
    exigir(std::abs(media[0] - 0.17) < 1e-12,
           "O retorno esperado deve ser a media dos blocos mensais.");

    const auto covariancia =
        mercado.calcularMatrizCovariancia(4, 4, 2);
    exigir(std::abs(covariancia[0][0] - 0.0037) < 1e-12,
           "A covariancia deve usar retornos compostos dos blocos.");

    const auto media_sem_lookahead =
        mercado.calcularRetornosEsperados(3, 4, 2);
    exigir(std::abs(media_sem_lookahead[0] - 0.155) < 1e-12,
           "A estimativa nao deve incluir dados do dia atual ou futuros.");

    const auto retornos_diarios =
        mercado.calcularRetornosEsperados(4, 4);
    exigir(std::abs(retornos_diarios[0] - 0.1) < 1e-12,
           "O tamanho de periodo padrao deve preservar chamadas diarias.");

    bool rejeitou_periodo_invalido = false;
    try {
        mercado.obterCenariosRetorno(4, 4, 0);
    } catch (const std::invalid_argument&) {
        rejeitou_periodo_invalido = true;
    }
    exigir(rejeitou_periodo_invalido,
           "O tamanho de periodo zero deve ser rejeitado.");

    std::cout << "Testes de agregacao mensal aprovados." << std::endl;
    return 0;
}
