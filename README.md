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
- Operar nativamente como **PWA** (Progressive Web App) offline em campo.
- Armazenar dados em banco **SQLite** embutido ou planilhas estaticas.

## Uso rapido

Configure, compile e teste:

```bash
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Crie uma missao (com SQLite):

```bash
./build/droneops init-mission \
  --dir out/demo_DO001 \
  --project SISTER-CAMPO \
  --campaign CAMP-2026-001 \
  --area AREA-001 \
  --mission DO-001 \
  --responsible Operador \
  --aircraft DRONE-001 \
  --sensor RGB-001 \
  --db out/demo_DO001/droneops.db
```

Valide:

```bash
./build/droneops validate \
  --db out/demo_DO001/droneops.db \
  --out out/demo_DO001/validacao
```

Gere pacote CampoSync:

```bash
./build/droneops package \
  --dir out/demo_DO001 \
  --mission DO-001 \
  --db out/demo_DO001/droneops.db \
  --out out/demo_DO001/pacote
```

> **Nota**: Para uso legado com planilhas `.csv`, basta omitir o argumento `--db` e passar `--input` no `validate`.

## Interface web PWA

A interface foi evoluida para um **Progressive Web App (PWA)** totalmente dinamico em `web/static/`. Ela desenha o fluxo de missao, checklist, ocorrencias e permite uso completamente offline no celular em campo, salvando rascunhos no dispositivo ate que a conexao retorne.

Servidor local:

```bash
cd web
python3 -m http.server 8011
```

Abra:

```text
http://127.0.0.1:8011/
```

## Fora de escopo

- Substituir sistemas oficiais de autorizacao, regulacao ou controle de operacao.
- Declarar conformidade legal automatica.
- Controlar fisicamente aeronave ou executar voo.
- Depender de internet para registrar a missao em campo.
