# Release Notes v0.1.0

Primeira versao executavel do DroneOps.

## Incluido

- Base documental de governanca do modulo.
- ADRs para modulo separado, CampoSync nativo, reuso do MorfoCampo e limite operacional.
- Modelo de dominio inicial para missao, equipe, aeronave, sensor, checklist, evidencia, ocorrencia e pacote.
- Nucleo C++ com status operacional de missao.
- Leitura e escrita de CSV/JSONL de missoes.
- Validacao inicial de campos obrigatorios, data ISO, duplicidade e ocorrencias abertas.
- Geracao de pacote CampoSync com `metadata.json`, JSON da missao, log e `manifest.json`.
- Hash deterministico FNV-1a 64-bit para manifesto inicial.
- CLI com `init`, `init-mission`, `validate` e `package`.
- Interface web estatica inicial para fluxo de missao, checklist e manifesto preliminar.
- Testes unitarios e testes de CLI via CTest.

## Validacao

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Resultado local: 6 testes passando.

## Limites conhecidos

- A interface web ainda e estatica e nao chama a CLI/API local.
- O hash do manifesto e deterministico, mas ainda nao usa SHA-256.
- Checklists editaveis ainda vivem como arquivos Markdown e estado visual local.
- CampoSync ainda gera pacote em diretorio, sem ZIP final.

