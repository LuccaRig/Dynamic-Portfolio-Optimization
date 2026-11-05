cmake_minimum_required(VERSION 3.12)
project(PortifolioMPC VERSION 1.0 LANGUAGES CXX)

# Define o padrão C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontra e configura o suporte ao OpenMP
find_package(OpenMP REQUIRED)

# Inclui os diretórios de cabeçalhos
include_directories(include)

# Arquivos fonte do projeto
set(SOURCES
    src/main.cpp
    src/Mercado.cpp
    src/SimuladorPortfolio.cpp
    src/DifferentialEvolution.cpp
    src/Relatorio.cpp
    src/Estatisticas.cpp
    src/Dados.cpp
    src/Backtest.cpp
)

# Cria o executável
add_executable(PortifolioMPC ${SOURCES})

# Vincula o OpenMP ao executável compilado
target_link_libraries(PortifolioMPC PRIVATE OpenMP::OpenMP_CXX)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_mercado_mensal
        tests/test_mercado_mensal.cpp
        src/Mercado.cpp
    )
    add_test(NAME mercado_mensal COMMAND test_mercado_mensal)

    add_executable(test_backtest_mensal
        tests/test_backtest_mensal.cpp
        src/Backtest.cpp
        src/Mercado.cpp
        src/DifferentialEvolution.cpp
        src/SimuladorPortfolio.cpp
        src/Estatisticas.cpp
    )
    target_link_libraries(test_backtest_mensal PRIVATE OpenMP::OpenMP_CXX)
    add_test(NAME backtest_mensal COMMAND test_backtest_mensal)
endif()