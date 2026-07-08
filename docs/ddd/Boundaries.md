# Boundaries

## Contextos

- `DroneOps`: missao, equipe, aeronave, sensor, checklist, evidencia, ocorrencia e status operacional.
- `CampoSync`: manifesto, pacote, hashes, validacao de exportacao e destino SisTer.
- `CampoNode`: execucao local, rede, armazenamento, banco, arquivos e interface web.
- `SisTer-Campo`: arquitetura, governanca e criterios gerais da plataforma.
- `SisTer`: destino territorial e institucional dos pacotes sincronizados.

## Regras

- DroneOps nao grava dados diretamente no SisTer sem passar por contrato CampoSync ou API aprovada.
- CampoSync nao altera evidencias; apenas empacota, valida, hasheia e registra status.
- Operacao com drone exige responsavel humano identificado.
- Ocorrencias operacionais bloqueiam status final ate revisao.
- Dados de localizacao, imagens, documentos e logs de voo sao potencialmente sensiveis.

