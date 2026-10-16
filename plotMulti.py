import re
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# 1. Carregar dados brutos do histórico
df_retornos = pd.read_csv("historico_retornos.csv", header=None)
df_cdi = pd.read_csv("historico_cdi.csv", header=None)

# 2. Ler o arquivo de resultados do C++ para extrair a série diária do MPC-DE e CDI, além dos pesos iniciais
dias_mpc = []
pnl_mpc = []
cdi_log = []

pesos_iniciais = {}
lendo_pesos_iniciais = False

with open("results4-t0.txt", "r", encoding="utf-8") as f:
  for line in f:
    match = re.search(
        r"\[Dia (\d+)\]\s+CDI=([\d.-]+)%\s*\|\s+PnL=([\d.-]+)%",
        line,
    )
    if match:
      dia = int(match.group(1))
      cdi_val = float(match.group(2)) / 100.0
      pnl_val = float(match.group(3)) / 100.0
      dias_mpc.append(dia)
      pnl_mpc.append(pnl_val)
      cdi_log.append(cdi_val)

    if "--- CARTEIRA INICIAL (PESOS) ---" in line:
      lendo_pesos_iniciais = True
      continue
    if lendo_pesos_iniciais:
      if "---" in line:
        lendo_pesos_iniciais = False
        continue
      parts = line.split(":")
      if len(parts) == 2:
        ativo = parts[0].strip()
        val_str = parts[1].replace("%", "").strip()
        pesos_iniciais[ativo] = float(val_str) / 100.0

print(f"Dias simulados extraídos do log: {len(dias_mpc)}")
dia_inicio = dias_mpc[0] if dias_mpc else 63
dia_fim = dia_inicio + len(dias_mpc)

# 3. Alinhando os retornos diários dos ativos para o período do backtest
retornos_periodo = df_retornos.iloc[dia_inicio:dia_fim].values
cdi_periodo = df_cdi.iloc[dia_inicio:dia_fim, 0].values

num_ativos = retornos_periodo.shape[1]

# 4. Estratégia B&H com pesos iguais (1/n para cada ativo)
pesos_iguais = np.ones(num_ativos) / num_ativos
pnl_bh_iguais = []
w_bh_eq = pesos_iguais.copy()

for r_t in retornos_periodo:
  pnl_dia = np.sum(w_bh_eq * r_t)
  pnl_bh_iguais.append(pnl_dia)
  w_bh_eq = w_bh_eq * (1.0 + r_t)
  w_bh_eq /= np.sum(w_bh_eq)

# 5. Estratégia B&H com os pesos iniciais do DE
w_de_list = []
for k, v in pesos_iniciais.items():
  if k != "Caixa inicial":
    w_de_list.append(v)

if len(w_de_list) == num_ativos:
  pesos_de_inicial = np.array(w_de_list)
else:
  pesos_de_inicial = pesos_iguais

pnl_bh_de = []
w_bh_de = pesos_de_inicial.copy()

for idx, r_t in enumerate(retornos_periodo):
  c_hoje = cdi_periodo[idx]
  soma_w = np.sum(w_bh_de)
  pnl_dia = np.sum(w_bh_de * r_t) + (1.0 - soma_w) * c_hoje
  pnl_bh_de.append(pnl_dia)

  novos_w_brutos = w_bh_de * (1.0 + r_t)
  val_acoes = np.sum(novos_w_brutos)
  fator_caixa = (1.0 - soma_w) * (1.0 + c_hoje)
  patrimonio_total = val_acoes + fator_caixa
  if patrimonio_total > 0:
    w_bh_de = novos_w_brutos / patrimonio_total


# 6. Cálculo dos Retornos Acumulados
def calcular_acumulado(retornos):
  return np.cumprod(1.0 + np.array(retornos)) - 1.0


acum_mpc = calcular_acumulado(pnl_mpc)
acum_bh_de = calcular_acumulado(pnl_bh_de)
acum_bh_eq = calcular_acumulado(pnl_bh_iguais)
acum_cdi = calcular_acumulado(cdi_periodo)


# --- NOVAS FUNÇÕES DE RISCO E MÉTRICAS ---
def calcular_metricas(retornos, cdi_retornos):
  retornos = np.array(retornos)
  # Volatilidade Anualizada (considerando 252 dias úteis)
  vol_anual = np.std(retornos, ddof=1) * np.sqrt(252) * 100

  # Maximum Drawdown (DD Máximo)
  acumulado = np.cumprod(1.0 + retornos)
  pico = np.maximum.accumulate(acumulado)
  drawdown = (acumulado - pico) / pico
  max_dd = np.min(drawdown) * 100

  # Índice Sharpe (assumindo CDI médio diário convertido para anualizado ou taxa livre de risco)
  retorno_medio_anual = np.mean(retornos) * 252
  cdi_medio_anual = np.mean(cdi_retornos) * 252
  vol_decimal = np.std(retornos, ddof=1) * np.sqrt(252)
  sharpe = (
      (retorno_medio_anual - cdi_medio_anual) / vol_decimal
      if vol_decimal > 0
      else 0.0
  )

  return vol_anual, max_dd, sharpe


# 7. Plotagem dos Gráficos Comparativos
plt.figure(figsize=(12, 6))
plt.plot(
    dias_mpc,
    acum_mpc * 100,
    label="MPC-DE (Otimizado)",
    linewidth=2,
    color="blue",
)
plt.plot(
    dias_mpc,
    acum_bh_de * 100,
    label="Buy & Hold (Pesos Iniciais DE)",
    linewidth=1.5,
    linestyle="--",
    color="orange",
)
plt.plot(
    dias_mpc,
    acum_bh_eq * 100,
    label="Buy & Hold (Pesos Iguais 1/n)",
    linewidth=1.5,
    linestyle=":",
    color="green",
)
plt.plot(
    dias_mpc,
    acum_cdi * 100,
    label="Taxa Livre de Risco (CDI)",
    linewidth=1.5,
    linestyle="-.",
    color="gray",
)

plt.title(
    "Comparativo de Desempenho Acumulado: MPC-DE vs. Benchmarks", fontsize=14
)
plt.xlabel("Dias de Simulação (Índice Temporal)", fontsize=12)
plt.ylabel("Retorno Acumulado (%)", fontsize=12)
plt.legend(fontsize=11)
plt.grid(True, linestyle="--", alpha=0.6)
plt.tight_layout()

plt.savefig("comparativo_estrategias.png", dpi=300)
print("Gráfico salvo com sucesso como 'comparativo_estrategias.png'!")
plt.show()

# 8. Cálculo e Impressão de Retornos, Volatilidade, Drawdown e Sharpe
vol_mpc, dd_mpc, sharpe_mpc = calcular_metricas(pnl_mpc, cdi_periodo)
vol_bh_de, dd_bh_de, sharpe_bh_de = calcular_metricas(pnl_bh_de, cdi_periodo)
vol_bh_eq, dd_bh_eq, sharpe_bh_eq = calcular_metricas(
    pnl_bh_iguais, cdi_periodo
)
vol_cdi, dd_cdi, sharpe_cdi = calcular_metricas(cdi_periodo, cdi_periodo)

print("\n" + "=" * 65)
print(f"{'ESTRATÉGIA':<32} | {'RETORNO':<8} | {'VOL (a.a.)':<10} | {'MAX DD':<8} | {'SHARPE':<6}")
print("-" * 65)
print(
    f"{'MPC-DE (Otimizado)':<32} | {acum_mpc[-1]*100:>6.2f}% |"
    f" {vol_mpc:>8.2f}% | {dd_mpc:>6.2f}% | {sharpe_mpc:>6.2f}"
)
print(
    f"{'Buy & Hold (Pesos Iniciais DE)':<32} | {acum_bh_de[-1]*100:>6.2f}% |"
    f" {vol_bh_de:>8.2f}% | {dd_bh_de:>6.2f}% | {sharpe_bh_de:>6.2f}"
)
print(
    f"{'Buy & Hold (Pesos Iguais 1/n)':<32} | {acum_bh_eq[-1]*100:>6.2f}% |"
    f" {vol_bh_eq:>8.2f}% | {dd_bh_eq:>6.2f}% | {sharpe_bh_eq:>6.2f}"
)
print(
    f"{'Taxa Livre de Risco (CDI)':<32} | {acum_cdi[-1]*100:>6.2f}% |"
    f" {vol_cdi:>8.2f}% | {dd_cdi:>6.2f}% | {sharpe_cdi:>6.2f}"
)
print("=" * 65)