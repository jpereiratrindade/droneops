# Governanca do DroneOps

## Papel do repositorio

Este repositorio implementa o modulo DroneOps do SisTer-Campo. A governanca geral fica no projeto `SisTer-Campo`; este repositorio guarda as decisoes, contratos e politicas especificas da operacao com drones.

## Principios

- Operacao local e offline-first.
- Registro operacional auditavel.
- Separacao entre apoio documental e responsabilidade legal/operacional humana.
- CampoSync como contrato nativo desde o inicio.
- Reuso tecnico e visual do MorfoCampo sempre que fizer sentido.

## Artefatos obrigatorios

- ADR para decisoes arquiteturais ou mudancas de escopo.
- Politica quando houver risco operacional, dado sensivel ou dependencia externa.
- Contrato CampoSync quando houver mudanca em pacote, manifesto, exportacao ou importacao.
- Teste automatizado quando houver regra de status, checklist, validacao ou exportacao.

## Linha de base

DroneOps deve acompanhar a arquitetura do SisTer-Campo, mas evoluir como modulo independente. O MorfoCampo permanece inalterado e serve como referencia de implementacao.

