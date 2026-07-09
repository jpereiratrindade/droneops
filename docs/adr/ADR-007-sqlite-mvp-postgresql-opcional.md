# ADR-007: Usar SQLite no MVP e manter PostgreSQL como opção futura

## Status

Aceita e **Implementada** (na versão 0.2.0 via `SqliteStore`).

## Contexto

O DroneOps deve operar em CampoNode local, offline-first, em situações de campo com baixa conectividade e infraestrutura simples. PostgreSQL pode trazer robustez em instalações multiusuário maiores, mas adiciona operação, backup, serviço residente e recuperação mais complexos.

O SISTER-Observa é o destino institucional e pode evoluir para PostgreSQL como banco de produção do SisTer.

## Decisão

No MVP, o DroneOps não terá PostgreSQL como dependência obrigatória. O armazenamento local deve continuar simples e portátil, com SQLite ou arquivos estruturados no pacote CampoSync.

PostgreSQL fica como opção futura para:

- CampoNode pesado em notebook/workstation;
- várias equipes simultâneas;
- alto volume de anexos/metadados;
- sincronização concorrente;
- necessidade de PostGIS/local geoespacial.

## Consequências

- O DroneOps continua fácil de instalar em Raspberry Pi, mini-PC e notebook.
- O pacote CampoSync permanece a unidade principal de integração.
- O SISTER-Observa deve ser o lugar preferencial para PostgreSQL institucional.
- Qualquer adoção de PostgreSQL no DroneOps exigirá ADR própria de operação, backup e migração.

