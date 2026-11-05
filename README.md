# MPC para Otimização Dinâmica de Portfólio

Projeto em C++ para **otimização dinâmica de portfólio** utilizando **Model Predictive Control (MPC)** e **Differential Evolution (DE)** paralelizada com OpenMP como solver numérico.

O objetivo é determinar como os pesos dos ativos devem ser ajustados ao longo de um horizonte de previsão, considerando retorno, risco, CVaR e custo de movimentação do portfólio.

---

## 1. Formulação do problema

Considere um portfólio com `n` ativos.

O vetor de estados é definido pelos pesos atuais:

$$
x_t = w_t =
\begin{bmatrix}
w_{1,t} & w_{2,t} & \cdots & w_{n,t}
\end{bmatrix}^T
$$

O controle representa a variação dos pesos:

$$
u_t = \Delta w_t
$$

Assim, a dinâmica nominal do sistema é:

$$
x_{t+1} = x_t + u_t
$$

Na implementação, após aplicar o controle, os pesos são corrigidos para respeitar as restrições:

$$
w_{t+1} = \mathcal{R}(w_t+\Delta w_t)
$$

onde $\mathcal{R}$ representa a função de reparação dos pesos.

---

## 2. Restrições do portfólio

O modelo considera apenas posições compradas:

$$
w_{i,t} \geq 0
$$

A soma dos pesos não pode ultrapassar 100%:

$$
\sum_{i=1}^{n} w_{i,t} \leq 1
$$

O restante do capital permanece aplicado em CDI.

A movimentação do portfólio é limitada por um turnover máximo em cada
rebalanceamento:

$$
\tau_t = \|\Delta w_t\|_1
$$

$$
\tau_t \leq \tau_{\max}
$$

---

## 3. Retorno esperado

O backtest usa blocos de 21 pregões como períodos mensais. Para o ativo $i$,
o retorno composto do bloco $s$ é:

$$
R_{i,s}^{(21)} = \prod_{d=0}^{20}(1+r_{i,s+d})-1
$$

Os retornos esperados são a média dos blocos deslizantes completamente
contidos na janela histórica:

$$
\mu_i^{(21)} = \frac{1}{M}\sum_{s=1}^{M}R_{i,s}^{(21)}
$$

onde:

- $L$ é o tamanho da janela histórica em pregões (padrão: 63);
- $M$ é o número de blocos deslizantes de 21 pregões dentro da janela;
- $r_{i,s+d}$ é o retorno diário do ativo $i$;
- $\mu_i^{(21)}$ é o retorno esperado do ativo por período de rebalanceamento.

A covariância e os cenários do CVaR também são calculados sobre esses retornos
compostos de 21 pregões. O horizonte `horizonte_mpc` conta períodos de
rebalanceamento. A projeção do CDI considera constante, em cada bloco, a taxa
diária conhecida na data da otimização.

O vetor de retornos esperados por período de rebalanceamento é:

$$
\mu^{(21)} =
\begin{bmatrix}
\mu_1^{(21)} & \mu_2^{(21)} & \cdots & \mu_n^{(21)}
\end{bmatrix}^T
$$

---

## 4. Covariância e variância

A matriz de covariância é estimada entre os $M$ retornos compostos:

$$
\Sigma^{(21)}_{ij} =
\frac{1}{M-1}\sum_{s=1}^{M}
(R^{(21)}_{i,s}-\mu^{(21)}_i)
(R^{(21)}_{j,s}-\mu^{(21)}_j)
$$

Para um portfólio com pesos $w$, a variância é:

$$
\sigma_p^2 = w^T\Sigma w
$$

Quanto maior $\sigma_p^2$, maior a dispersão esperada dos retornos do portfólio.

---

## 5. Retorno do portfólio

Como parte do capital pode permanecer em CDI, o retorno esperado do portfólio é:

$$ R_p^{(21)} = w^T\mu^{(21)} + (1-\mathbf{1}^Tw)c^{(21)} $$

onde $c^{(21)}$ representa o CDI composto para os 21 pregões.

Para cenários históricos, o retorno do portfólio é calculado de forma equivalente:

$$ R_p^{(s,21)} = w^T R^{(s,21)} + (1-\mathbf{1}^Tw)c^{(21)} $$

---

## 6. CVaR

O CVaR mede a perda média nos piores cenários.

Definindo a perda como:

$$
\ell^{(s)}=-R_p^{(s)}
$$

e considerando nível de confiança $\alpha$, o VaR corresponde ao quantil:

$$
VaR_\alpha = Q_\alpha(\ell)
$$

O CVaR é a média das perdas que estão além desse quantil:

$$
CVaR_\alpha =
E[\ell \mid \ell \geq VaR_\alpha]
$$

Na implementação é utilizado:

$$
\alpha = 0.95
$$

portanto, o CVaR considera aproximadamente os **5% piores cenários históricos**.

---

## 7. Função de fitness

O MPC otimiza uma função de custo que combina retorno, risco, CVaR e movimentação:

$$
J =
-\alpha_r \bar{R}
+
\gamma \overline{\sigma^2}
+
\beta \overline{CVaR}
+
\eta \bar{\tau}
$$

Como o problema é de minimização:

- $-\alpha_r\bar{R}$ incentiva maior retorno;
- $\gamma\overline{\sigma^2}$ penaliza variância;
- $\beta\overline{CVaR}$ penaliza perdas extremas;
- $\eta\bar{\tau}$ penaliza grandes movimentações.

Os termos são calculados ao longo do horizonte do MPC.

---

## 8. Frequência mensal de rebalanceamento

O backtest reotimiza a cada `periodo_rebalanceamento` pregões (padrão 21). O
limite `max_turnover` é aplicado nessa data; nos demais pregões não há
negociação. As posições são mantidas e os pesos derivam naturalmente conforme
o retorno dos ativos e do CDI. O P&L e as métricas realizados continuam sendo
calculados diariamente. `horizonte_mpc` conta períodos de rebalanceamento: com
os valores padrão, o otimizador planeja 30 períodos mensais.

## 9. MPC

Em cada data de rebalanceamento, o controlador considera um horizonte de $N$
períodos mensais e procura uma sequência de controles:

$$
U_t =
\begin{bmatrix}
\Delta w_t &
\Delta w_{t+1} &
\cdots &
\Delta w_{t+N-1}
\end{bmatrix}
$$

O problema de otimização é:

$$ U_t^* = \arg\min_{U_t} J(U_t) $$

A sequência completa é otimizada, mas apenas o primeiro controle é aplicado:

$$
w_{t+P} = \mathcal{R}(w_t+\Delta w_t^*)
$$

onde $P$ é o número de pregões por período de rebalanceamento. Entre $t$ e
$t+P$, não há negociação e os pesos derivam com os retornos realizados. No
próximo rebalanceamento, o problema é resolvido novamente com os pesos e dados
de mercado atualizados.

Esse é o princípio do **receding horizon control**.

---

## 10. Differential Evolution

A Evolução Diferencial é utilizada como **solver do problema de otimização do MPC**.

Cada indivíduo da população representa uma trajetória completa:

$$
U =
[\Delta w_0,\Delta w_1,\ldots,\Delta w_{N-1}]
$$

Para gerar novos candidatos, a DE utiliza mutação:

$$
v =
x_{r_1}
+
F(x_{r_2}-x_{r_3})
$$

seguida de crossover e seleção.

Um candidato é mantido quando apresenta menor custo:

$$
J_{\text{novo}} < J_{\text{atual}}
$$

Assim, a DE procura numericamente a sequência de controles que minimiza a função de fitness do MPC.

---

## 10. Estrutura do projeto

```text
src/
├── main.cpp
├── Mercado.cpp
├── SimuladorPortfolio.cpp
├── DifferentialEvolution.cpp
├── Backtest.cpp
├── Estatisticas.cpp
├── Relatorio.cpp
└── Dados.cpp
```

### Principais responsabilidades

**Mercado**
- Retornos históricos
- Retornos esperados
- Matriz de covariância
- Cenários históricos

**SimuladorPortfolio**
- Simulação das trajetórias
- Variância
- CVaR
- Função de fitness
- Reparação dos pesos

**DifferentialEvolution**
- População
- Mutação
- Crossover
- Seleção
- Busca da melhor trajetória

**Backtest**
- Executa o MPC ao longo do histórico
- Aplica somente o primeiro controle da trajetória otimizada
- Atualiza os pesos e repete o processo

---

## 11. Fluxo do algoritmo

```text
Dados históricos
      ↓
Cálculo de μ, Σ e cenários
      ↓
Estado atual w(t)
      ↓
MPC
      ↓
Differential Evolution
      ↓
Sequência Δw(t:t+N-1)
      ↓
Avaliação da fitness
      ↓
Melhor sequência
      ↓
Aplica somente Δw(t)
      ↓
Atualiza w(t+1)
      ↓
Novo período
```

---

## 12. Backtest

O retorno realizado do portfólio é calculado por:

$$ R_t^{real} = w_t^T r_t + (1-\mathbf{1}^Tw_t)c_t $$

O sistema também compara o resultado do MPC com uma estratégia **Buy & Hold**.

As métricas de avaliação incluem retorno, volatilidade, drawdown e métricas de risco.

---

## 13. Parâmetros principais

Atualmente, a configuração utiliza:

| Parâmetro | Valor |
|---|---:|
| $\alpha_r$ | 1.0 |
| $\beta$ | 0.5 |
| $\gamma$ | 0.8 |
| $\eta$ | 0.1 |
| $\alpha_{CVaR}$ | 0.95 |
| População DE | 60 |
| Gerações DE | 40 |
| $F$ | 0.7 |
| $CR$ | 0.9 |

---

## 14. Resumo matemático

O problema pode ser resumido como:

$$
\boxed{
\begin{aligned}
\min_{\Delta w_0,\ldots,\Delta w_{N-1}}
\quad &
-\alpha_r\bar{R}
+\gamma\overline{\sigma^2}
+\beta\overline{CVaR}
+\eta\bar{\tau}
\\
\text{sujeito a}
\quad &
w_{k+1}=\mathcal{R}(w_k+\Delta w_k)
\\
&
w_{i,k}\geq0
\\
&
\sum_i w_{i,k}\leq1
\\
&
\|\Delta w_k\|_1\leq\tau_{\max}
\end{aligned}
}
$$

O **MPC** define o problema de controle, enquanto a **Differential Evolution** realiza a busca numérica pela sequência de controles que minimiza essa função de custo.