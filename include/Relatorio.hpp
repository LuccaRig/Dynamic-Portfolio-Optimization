#pragma once

#include "Backtest.hpp"

void imprimirRelatorio(
    const ResultadoBacktest& resultado,
    const ConfigBacktest& config,
    double cvar_alpha
);
