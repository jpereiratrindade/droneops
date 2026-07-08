# Integração DroneOps -> SISTER-Observa

## Decisão operacional

O DroneOps será integrado ao SisTer pelo módulo `SISTER-Observa`, não por um app Django novo.

Razão: o Observa já possui o domínio institucional de drones:

- `Drone`
- `Sensor`
- `PilotoOperador`
- `AreaMonitoramento`
- `MissaoVoo`
- `AvaliacaoRiscoOperacional`

O SisTer-Campo continua sendo a base teórica, arquitetural e de governança. O DroneOps continua sendo o módulo local offline-first de campo.

## Fluxo

```text
CampoNode / DroneOps
  -> pacote CampoSync
  -> SISTER-Observa
  -> revisão humana
  -> MissaoVoo / ARO / evidências / indicadores
```

## Papel de cada parte

### DroneOps

- Planejar missão em campo.
- Registrar responsável, equipe, aeronave, sensor e área.
- Executar checklists.
- Registrar evidências, logs e ocorrências.
- Gerar pacote CampoSync.

### CampoSync

- Gerar `manifest.json`.
- Listar arquivos, tamanhos e hashes.
- Declarar status de validação e sincronização.
- Preservar rastreabilidade do pacote.
- Preparar importação manual ou assistida.

### SISTER-Observa

- Receber pacote.
- Validar manifesto.
- Criar ou associar registros institucionais.
- Exigir revisão humana antes de status definitivo.
- Vincular missão a ARO, áreas, drones, sensores e responsáveis.

## Mapeamento inicial

| DroneOps / CampoSync | SISTER-Observa |
| --- | --- |
| `mission_id` | `MissaoVoo.codigo_missao` |
| `project_id` | `MissaoVoo.projeto_validacao_ref` |
| `responsible` | `MissaoVoo.responsavel_tecnico` ou pendência de associação |
| `aircraft_id` | `Drone.numero_serie`, `Drone.nome` ou pendência de associação |
| `sensor_id` | `Sensor.numero_serie`, `Sensor.nome` ou pendência de associação |
| `area_id` | `AreaMonitoramento.nome` ou pendência de associação |
| `planned_date` | `MissaoVoo.data_planejada` |
| `objective` | `MissaoVoo.objetivo` ou `outro` com observação |
| `flight_log_paths` | `MissaoVoo.relatorio_processamento_link` ou evidência anexa |
| `evidence_paths` | links/caminhos de evidências do pacote |
| `occurrences` | `MissaoVoo.incidentes` |
| `status` | `MissaoVoo.status` com tradução controlada |

## Tradução de status

| DroneOps | Observa |
| --- | --- |
| `rascunho` | `planejada` |
| `planejada` | `planejada` |
| `apta` | `planejada` |
| `executada` | `executada` |
| `revisada` | `executada` |
| `bloqueada` | `cancelada` ou pendência de revisão |
| `empacotada` | `executada` com pacote importado |
| `sincronizada` | `executada` com pacote aceito |

Observação: `autorizada` no Observa não deve ser definido automaticamente pelo DroneOps. Esse status exige revisão humana e documentação aplicável.

## Condições mínimas para importação

- Manifesto com versão de contrato CampoSync.
- `origin_module = "droneops"`.
- `mission_id`, `project_id`, `campaign_id`, `area_id` e `human_responsible` preenchidos.
- Hash para cada arquivo listado.
- JSON da missão presente.
- Nenhuma ocorrência aberta para importação automática como revisada.

## Endpoint sugerido no Observa

Importação assistida:

```text
POST /observa/api/camposync/importar-droneops/
```

Corpo inicial:

```json
{
  "manifest": {},
  "mission": {},
  "mode": "dry_run"
}
```

Modos:

- `dry_run`: valida o pacote e retorna pendências, sem gravar.
- `staged`: registra pacote como importação pendente.
- `commit`: cria/atualiza registros após revisão autorizada.

## Estratégia recomendada

1. Implementar primeiro `dry_run` no Observa.
2. Criar modelo de importação CampoSync para guardar pacote, manifesto, status e pendências.
3. Só depois permitir `commit` com revisão humana.
4. Manter importação manual por arquivo enquanto a API amadurece.

