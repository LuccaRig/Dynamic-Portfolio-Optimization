#include "DifferentialEvolution.hpp"
#include "SimuladorPortfolio.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <omp.h>

DifferentialEvolution::DifferentialEvolution(
    const ParametrosDE& params_de,
    const ParametrosCusto& params_custo)
    : config_de_(params_de), config_custo_(params_custo) {}

double DifferentialEvolution::gerarAleatorio(
    double min, double max, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

std::vector<std::vector<double>> DifferentialEvolution::otimizar(
    const std::vector<double>& peso_inicial,
    const std::vector<std::vector<double>>& matriz_covariancia,
    const std::vector<double>& retornos_esperados,
    const std::vector<double>& cdi_horizonte,
    const std::vector<std::vector<double>>& cenarios_retornos,
    double max_turnover,
    int horizonte_N,
    int num_ativos) {

    const int NP = config_de_.tamanho_populacao;
    const int G_max = config_de_.max_geracoes;
    const double F = config_de_.F;
    const double CR = config_de_.CR;

    std::vector<std::vector<std::vector<double>>> populacao(
        NP,
        std::vector<std::vector<double>>(
            horizonte_N, std::vector<double>(num_ativos, 0.0)));

    // Inicialização + reparo de cada passo.
    #pragma omp parallel
    {
        std::mt19937 rng(42u + omp_get_thread_num());

        #pragma omp for
        for (int i = 0; i < NP; ++i) {
            std::vector<double> peso = peso_inicial;

            for (int k = 0; k < horizonte_N; ++k) {
                std::vector<double> delta(num_ativos);

                for (int j = 0; j < num_ativos; ++j)
                    delta[j] = gerarAleatorio(-max_turnover, max_turnover, rng);

                const auto peso_reparado =
                    SimuladorPortfolio::repararPesos(
                        peso, delta, max_turnover);

                for (int j = 0; j < num_ativos; ++j)
                    populacao[i][k][j] = peso_reparado[j] - peso[j];

                peso = peso_reparado;
            }
        }
    }

    std::vector<double> custos(NP, 1e100);

    #pragma omp parallel for
    for (int i = 0; i < NP; ++i) {
        custos[i] = SimuladorPortfolio::avaliarTrajetoria(
            peso_inicial,
            populacao[i],
            matriz_covariancia,
            retornos_esperados,
            cdi_horizonte,
            cenarios_retornos,
            config_custo_,
            max_turnover);
    }

    for (int g = 0; g < G_max; ++g) {
        std::vector<std::vector<std::vector<double>>> nova_populacao = populacao;
        std::vector<double> novos_custos = custos;

        #pragma omp parallel
        {
            std::mt19937 rng(
                1337u + 97u * g + omp_get_thread_num());

            std::uniform_int_distribution<int> dist_np(0, NP - 1);
            std::uniform_int_distribution<int> dist_k(0, horizonte_N - 1);
            std::uniform_int_distribution<int> dist_j(0, num_ativos - 1);
            std::uniform_real_distribution<double> dist_01(0.0, 1.0);

            #pragma omp for
            for (int i = 0; i < NP; ++i) {
                int r1, r2, r3;

                do { r1 = dist_np(rng); } while (r1 == i);
                do { r2 = dist_np(rng); } while (r2 == i || r2 == r1);
                do { r3 = dist_np(rng); }
                while (r3 == i || r3 == r1 || r3 == r2);

                const int j_rand_k = dist_k(rng);
                const int j_rand_j = dist_j(rng);

                std::vector<std::vector<double>> mutante(
                    horizonte_N,
                    std::vector<double>(num_ativos, 0.0));

                for (int k = 0; k < horizonte_N; ++k) {
                    for (int j = 0; j < num_ativos; ++j) {
                        if (dist_01(rng) < CR ||
                            (k == j_rand_k && j == j_rand_j)) {

                            mutante[k][j] =
                                populacao[r1][k][j] +
                                F * (populacao[r2][k][j] -
                                     populacao[r3][k][j]);
                        } else {
                            mutante[k][j] = populacao[i][k][j];
                        }
                    }
                }

                // Repara sequencialmente a trajetória.
                std::vector<std::vector<double>> mutante_reparado(
                    horizonte_N,
                    std::vector<double>(num_ativos, 0.0));

                std::vector<double> peso = peso_inicial;

                for (int k = 0; k < horizonte_N; ++k) {
                    const auto peso_reparado =
                        SimuladorPortfolio::repararPesos(
                            peso, mutante[k], max_turnover);

                    for (int j = 0; j < num_ativos; ++j)
                        mutante_reparado[k][j] =
                            peso_reparado[j] - peso[j];

                    peso = peso_reparado;
                }

                const double custo_mutante =
                    SimuladorPortfolio::avaliarTrajetoria(
                        peso_inicial,
                        mutante_reparado,
                        matriz_covariancia,
                        retornos_esperados,
                        cdi_horizonte,
                        cenarios_retornos,
                        config_custo_,
                        max_turnover);

                if (custo_mutante < custos[i]) {
                    nova_populacao[i] = std::move(mutante_reparado);
                    novos_custos[i] = custo_mutante;
                }
            }
        }

        populacao = std::move(nova_populacao);
        custos = std::move(novos_custos);
    }

    const int melhor_idx =
        static_cast<int>(std::distance(
            custos.begin(),
            std::min_element(custos.begin(), custos.end())));

    return populacao[melhor_idx];
}
