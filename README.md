# DroneOps

Modulo operacional do SisTer-Campo para planejamento, checklist, registro documental, evidencias e rastreabilidade de missoes com drones.

O DroneOps sera desenvolvido como projeto proprio, reaproveitando a base tecnica do `morfocampo`: C++ para nucleo de validacao/processamento, interface web local, operacao em rede local e padroes de deploy em CampoNode.

## Relacao com SisTer-Campo

- `SisTer-Campo`: base teorica, arquitetura, governanca e contratos gerais da plataforma.
- `droneops`: implementacao concreta do modulo de operacao com drones.
- `morfocampo`: modulo existente mantido como esta, usado como referencia tecnica e visual.

## Diferenca estrutural

Ao contrario do MorfoCampo, o DroneOps ja nasce com `CampoSync` no escopo inicial. Toda missao deve ser pensada desde o comeco como pacote sincronizavel com o SisTer.

## Escopo inicial

- Cadastrar missao vinculada a projeto, campanha e area.
- Registrar equipe, aeronave, sensor, baterias e equipamentos auxiliares.
- Registrar planejamento, objetivo, data, horario e condicoes ambientais.
- Executar checklist pre-voo e pos-voo.
- Anexar documentos, autorizacoes, mapas, planos de voo, imagens, logs e relatorios.
- Registrar ocorrencias, abortagens, falhas, riscos e inconsistencias.
- Gerar pacote de missao compatvel com CampoSync.

## Uso rapido

Configure, compile e teste:

```bash
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Crie uma missao:

```bash
./build/droneops init-mission \
  --dir out/demo_DO001 \
  --project SISTER-CAMPO \
  --campaign CAMP-2026-001 \
  --area AREA-001 \
  --mission DO-001 \
  --responsible Operador \
  --aircraft DRONE-001 \
  --sensor RGB-001
```

Valide:

```bash
./build/droneops validate \
  --input out/demo_DO001/droneops/missoes.csv \
  --out out/demo_DO001/validacao
```

Gere pacote CampoSync:

```bash
./build/droneops package \
  --dir out/demo_DO001 \
  --mission DO-001 \
  --out out/demo_DO001/pacote
```

## Interface web inicial

Uma primeira tela estatica esta em `web/static/`. Ela desenha o fluxo de missao, checklist, ocorrencias e manifesto preliminar com a identidade visual derivada do MorfoCampo.

Servidor local:

```bash
cd web
python3 -m http.server 8780
```

Abra:

```text
http://127.0.0.1:8780/
```

## Fora de escopo

- Substituir sistemas oficiais de autorizacao, regulacao ou controle de operacao.
- Declarar conformidade legal automatica.
- Controlar fisicamente aeronave ou executar voo.
- Depender de internet para registrar a missao em campo.
