#!/usr/bin/env python3
"""
Compila e executa o programa C, que gera `integral_c.txt` e `queries.txt`.
Usar: python3 generate_files.py
"""
import subprocess
import sys
import os


def run(cmd):
    """
    Executa um comando externo e encerra o script se houver erro.

    Usado para manter o fluxo simples: se a compilacao ou a execucao do C
    falhar, o erro e exibido e nenhum arquivo gerado e assumido como valido.
    """
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        print(f"Erro ao executar: {' '.join(cmd)}")
        if res.stdout:
            print(res.stdout)
        if res.stderr:
            print(res.stderr)
        sys.exit(res.returncode)
    return res


exe_name = "teste.exe" if os.name == "nt" else "teste"
run_cmd = [exe_name] if os.name == "nt" else ["./" + exe_name]

compile_cmd = ["gcc", "-Wall", "-Wextra", "-o", exe_name, "src/main.c", "src/integral_image.c"]

print("Compilando programa C...")
run(compile_cmd)

print("Executando programa C para gerar integral_c.txt e queries.txt...")
res = run(run_cmd)

if res.stdout:
    print(res.stdout, end="")
