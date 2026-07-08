# ADR-004: DroneOps nao substitui sistemas oficiais

## Status

Aceita.

## Contexto

Operacoes com drones envolvem normas, autorizacoes, responsabilidade tecnica e condicoes locais. O software pode apoiar o registro e a auditoria, mas nao deve declarar conformidade legal automatica.

## Decisao

DroneOps sera ferramenta de organizacao, checklist, evidencias e rastreabilidade. Ele nao substitui sistemas oficiais, revisao humana, autorizacao de voo, avaliacao de risco ou responsabilidade do operador.

## Consequencias

- A interface deve evitar linguagem de "autorizado automaticamente".
- Ocorrencias e pendencias bloqueiam conclusao automatica.
- Documentos oficiais podem ser anexados, mas sua validade deve ser revisada por humano responsavel.

