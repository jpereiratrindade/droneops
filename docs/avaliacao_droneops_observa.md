# Avaliacao: DroneOps como certificador de protocolo

## Sintese

O SISTER-Observa ja atende bem ao registro institucional de drones, sensores, pilotos, areas, missoes de voo e ARO. Ele deve ser o destino SisTer do DroneOps.

O ponto de partida mais adequado para o DroneOps nao e tentar replicar todo o Observa em campo. O melhor inicio e fazer o DroneOps funcionar como certificador local de aplicacao de protocolo:

- identificacao do operador;
- identificacao do drone;
- leitura ou confirmacao da ARO aplicavel;
- aplicacao guiada do checklist pre-voo;
- coleta de localizacao;
- anexos fotograficos de comprovacao;
- registro de decisao final;
- pacote CampoSync para importacao no SISTER-Observa.

## O que o Observa ja cobre

- Cadastro institucional de drones.
- Cadastro de sensores.
- Cadastro de pilotos/operadores.
- Areas de monitoramento.
- Missoes de voo.
- Avaliacao de Risco Operacional.
- Relatorio ARO.
- API REST de drones, sensores, pilotos, areas, missoes e ARO.

## Lacuna atual

O Observa ainda nao modela explicitamente uma aplicacao de checklist em campo com evidencias verificaveis.

Essa lacuna e exatamente o espaco natural do DroneOps.

## Papel recomendado para o DroneOps no MVP

DroneOps deve começar como modulo de certificacao operacional de campo, nao como sistema completo de missao.

O aplicativo local deve responder:

1. Quem aplicou o protocolo?
2. Qual drone foi identificado?
3. Onde o protocolo foi aplicado?
4. Qual ARO/checklist foi usado?
5. Quais itens foram cumpridos, nao cumpridos ou nao aplicaveis?
6. Que evidencias comprovam o registro?
7. O voo foi liberado, bloqueado ou liberado com restricao?
8. O pacote CampoSync e importavel pelo SISTER-Observa?

## Recursos recomendados

### Leitura de QR Code

Usar QR Code para identificar:

- drone;
- sensor/payload;
- operador;
- ARO vigente;
- campanha ou missao.

No MVP, o QR Code pode carregar apenas um identificador simples. Em etapa posterior, pode carregar payload assinado.

### Localizacao GPS do celular

Registrar:

- latitude;
- longitude;
- acuracia;
- data/hora da coleta;
- permissao concedida ou negada.

Observacao: em celulares, geolocalizacao no navegador exige contexto seguro. Em rede local, deve-se prever HTTPS no CampoNode.

### Fotos de comprovacao

Permitir anexar fotos de:

- drone identificado;
- numero de serie ou etiqueta/QR;
- tela/controlador com autorizacao ou status relevante;
- area de decolagem/pouso;
- condicoes ambientais;
- payload instalado;
- checklist assinado quando fisico.

### Registro de decisao

Estados recomendados:

- `rascunho`;
- `em_aplicacao`;
- `apto_para_voo`;
- `apto_com_restricao`;
- `bloqueado`;
- `executado`;
- `empacotado`;
- `sincronizado`.

## Relacao com SISTER-Observa

O Observa deve receber o pacote e materializar:

- `MissaoVoo`;
- vinculo com `Drone`;
- vinculo com `Sensor`;
- vinculo com `PilotoOperador`;
- vinculo com `AreaMonitoramento`;
- vinculo com `AvaliacaoRiscoOperacional`;
- evidencias e pendencias do checklist.

## Recomendacao

Implementar primeiro o DroneOps como certificador de aplicacao de checklist e ARO. Depois expandir para planejamento completo, logs de voo e resultados.

Essa abordagem reduz escopo, aumenta utilidade imediata e cria uma ponte limpa com o SISTER-Observa.

