# ADR-001: Desenvolver DroneOps como modulo separado

## Status

Aceita.

## Contexto

O SisTer-Campo sera a base teorica e de governanca da plataforma. O MorfoCampo permanece como modulo funcional existente. O DroneOps precisa nascer como modulo real, com desenvolvimento pratico semelhante ao MorfoCampo.

## Decisao

Criar `droneops` como projeto separado em `/dev/cpp/droneops`, usando o MorfoCampo como referencia tecnica e visual, sem mover ou alterar o projeto `morfocampo`.

## Consequencias

- O SisTer-Campo documenta arquitetura, contratos e diretrizes.
- O DroneOps concentra codigo, interface, testes, deploy e releases do modulo.
- Reuso do MorfoCampo deve ser feito por copia adaptada, migracao seletiva ou extracao futura de componentes comuns.

