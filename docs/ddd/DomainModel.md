# Modelo de Dominio

## Entidades

- `Projeto`: referencia institucional ou tecnica do trabalho.
- `Campanha`: unidade de organizacao de campo.
- `AreaOperacao`: area onde a missao sera planejada ou executada.
- `MissaoDrone`: unidade principal do DroneOps.
- `Equipe`: pessoas e papeis envolvidos.
- `Aeronave`: drone usado na missao.
- `Sensor`: carga util ou instrumento associado.
- `Bateria`: itens de energia vinculados a aeronave ou sensor.
- `Checklist`: conjunto de itens pre-voo, execucao ou pos-voo.
- `Evidencia`: documento, imagem, mapa, plano, log ou relatorio.
- `Ocorrencia`: falha, abortagem, risco, inconsistencia ou observacao critica.
- `PacoteMissao`: unidade CampoSync exportavel.

## Estados da missao

- `rascunho`: missao iniciada, ainda incompleta.
- `planejada`: identidade minima e planejamento registrados.
- `apta`: checklist pre-voo obrigatorio completo e sem bloqueio conhecido.
- `executada`: ha evidencias ou logs de execucao.
- `revisada`: checklist pos-voo completo e ocorrencias tratadas.
- `bloqueada`: pendencia critica ou ocorrencia aberta.
- `empacotada`: pacote CampoSync gerado.
- `sincronizada`: pacote aceito no destino definido.

## Invariantes

- Toda missao deve ter `id`, `projeto`, `campanha` e `area`.
- Toda missao operacional deve ter ao menos um responsavel humano.
- Toda evidencia exportada deve entrar no manifesto CampoSync.
- Missao com ocorrencia aberta nao deve virar `revisada` automaticamente.

