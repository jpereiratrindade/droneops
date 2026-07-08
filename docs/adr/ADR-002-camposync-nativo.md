# ADR-002: Incluir CampoSync desde o inicio

## Status

Aceita.

## Contexto

Missoes com drones geram documentos, evidencias, mapas, logs e metadados que precisam chegar ao SisTer com integridade e rastreabilidade. Se a sincronizacao for tratada apenas no final, o modelo de dados pode nascer incompleto.

## Decisao

CampoSync fara parte do desenho inicial do DroneOps. Toda missao deve poder gerar pacote exportavel com manifesto, arquivos, hashes, status de validacao e destino previsto.

## Consequencias

- O modelo de dominio deve carregar identificadores de projeto, campanha, area e missao.
- Evidencias e anexos precisam ser referenciaveis no manifesto.
- Validacoes de pacote entram cedo no roadmap.
- A interface deve expor status de pacote e pendencias de sincronizacao desde as primeiras versoes funcionais.

