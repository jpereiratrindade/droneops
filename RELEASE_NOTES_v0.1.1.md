# Release Notes v0.1.1

Evolução do DroneOps para MVP de certificação de protocolo e integração com SISTER-Observa.

## Incluído

- Decisão arquitetural para integrar DroneOps pelo SISTER-Observa.
- Decisão de MVP como certificador de aplicação de ARO/checklist.
- Decisão de manter SQLite/arquivos estruturados no MVP e PostgreSQL como opção futura.
- Campos de certificação no núcleo DroneOps:
  - operador;
  - QR/identificador do drone;
  - referência ARO;
  - GPS;
  - fotos comprobatórias;
  - decisão final.
- Pacote CampoSync agora inclui `certificado_protocolo_<missao>.json`.
- Interface web inicial com:
  - leitura de QR Code quando o navegador suportar `BarcodeDetector`;
  - coleta de GPS via `navigator.geolocation`;
  - anexação de fotos pelo celular;
  - decisão final do protocolo.

## Integração Observa

Foi criado no SISTER-Observa:

- modelo `CampoSyncDroneOpsImport`;
- migração `observa.0008_camposyncdroneopsimport`;
- endpoint `POST /observa/api/camposync/importar-droneops/`;
- modos `dry_run`, `staged` e `commit`;
- testes de validação, staging, commit e bloqueio por decisão final.

## Validação

DroneOps:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Resultado local: 6 testes passando.

SISTER-Observa:

```bash
.venv/bin/python manage.py test observa
.venv/bin/python manage.py check
```

Resultado local: 18 testes passando e system check sem issues.

