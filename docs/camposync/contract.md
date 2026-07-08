# Contrato CampoSync para DroneOps

## Objetivo

Definir como uma missao do DroneOps sera exportada para pacote de campanha sincronizavel com o SisTer.

## Estrutura inicial

```text
campanha_<id>/
|-- metadata.json
|-- manifest.json
|-- logs/
|-- evidencias/
|   |-- documentos/
|   |-- imagens/
|   `-- relatorios/
`-- droneops/
    |-- missao_<id>.json
    |-- checklists/
    |-- mapas/
    |-- planos_voo/
    |-- logs_voo/
    `-- anexos/
```

## Manifesto minimo

O arquivo `manifest.json` deve registrar:

- identificador do pacote;
- versao do DroneOps;
- versao do contrato CampoSync;
- identificador do CampoNode;
- projeto, campanha, area e missao;
- responsavel humano pelo pacote;
- modulo de origem: `droneops`;
- lista de arquivos com caminho, tamanho e hash;
- contagem de evidencias, logs e ocorrencias;
- status de validacao;
- status de sincronizacao;
- destino previsto no SisTer.

## Status do pacote

- `rascunho`
- `validado`
- `com_pendencias`
- `bloqueado`
- `exportado`
- `importado_no_sister`

## Regra inicial

Um pacote nao pode ser `validado` se houver evidencia referenciada sem arquivo correspondente, arquivo sem hash ou ocorrencia operacional aberta.

