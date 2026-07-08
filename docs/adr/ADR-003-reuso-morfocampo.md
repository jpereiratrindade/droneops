# ADR-003: Reusar base tecnica e grafica do MorfoCampo

## Status

Aceita.

## Contexto

O MorfoCampo ja possui nucleo C++, validacoes, interface web local, scripts de deploy e identidade visual funcional em CampoNode. O DroneOps deve ganhar velocidade reaproveitando esse trabalho.

## Decisao

Usar o MorfoCampo como base de desenvolvimento do DroneOps, com adaptacao progressiva de dominio: de coleta morfologica para missao, checklist, evidencia, ocorrencia e pacote de sincronizacao.

## Consequencias

- O codigo inicial podera ser quase uma copia adaptada do MorfoCampo.
- Nomes, comandos, textos e modelos devem ser renomeados para DroneOps antes de uso operacional.
- Componentes comuns poderao ser extraidos no futuro, mas isso nao bloqueia o inicio.

