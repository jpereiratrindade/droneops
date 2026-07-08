# ADR-005: Integrar DroneOps ao SISTER-Observa

## Status

Aceita.

## Contexto

O SISTER-Resiliência já possui o módulo `observa`, com domínio de drones, sensores, pilotos, áreas de monitoramento, missões de voo e avaliação de risco operacional. Criar um novo app Django para receber DroneOps duplicaria conceitos já existentes e aumentaria o risco de divergência institucional.

O SisTer-Campo permanece como base teórica e governança de campo. O DroneOps permanece como módulo local offline-first executado no CampoNode.

## Decisão

Integrar o DroneOps ao SisTer por meio do `SISTER-Observa`.

O fluxo inicial será:

1. DroneOps registra missão localmente no CampoNode.
2. CampoSync gera pacote com manifesto, JSON da missão, evidências, logs e hashes.
3. SISTER-Observa recebe ou importa o pacote.
4. Observa materializa os dados em seus modelos institucionais.
5. A revisão humana confirma vínculos, conformidade, ARO, status e destino dos dados.

## Consequências

- Não criar app Django novo para DroneOps neste momento.
- Não criar um app Django separado chamado SisTer-Campo apenas para missão de drone.
- CampoSync vira o contrato entre CampoNode/DroneOps e SISTER-Observa.
- O Observa precisa receber uma camada de importação/validação de pacote CampoSync.
- O DroneOps deve exportar dados compatíveis com os modelos do Observa.
- A integração não deve declarar autorização automática de voo.

