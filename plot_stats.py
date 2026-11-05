import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

cenarios = [
    ("stats_boavista.csv", "Boa Vista (80 ha, 500 m)"),
    ("stats_saopedro.csv", "Sao Pedro (320 ha, 3000 m)"),
    ("stats_cerradogrande.csv", "Cerrado Grande (1500 ha, 6000 m)"),
]
for arquivo, rotulo in cenarios:
    df = pd.read_csv(arquivo)
    plt.errorbar(df.nDevices, df.pdr_medio, yerr=df.desvio,
                 marker="o", capsize=4, label=rotulo)

plt.xlabel("Numero de agentes (end devices)")
plt.ylabel("PDR medio +/- desvio (10 execucoes)")
plt.title("PDR vs. escala por porte de fazenda")
plt.ylim(0, 1.05)
plt.grid(True)
plt.legend()
plt.savefig("pdr_stats.png", dpi=150)
print("Gerado: pdr_stats.png")
