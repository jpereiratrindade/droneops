# ADR-006: Iniciar DroneOps como certificador de protocolo

## Status

Proposta aceita para o MVP.

## Contexto

O SISTER-Observa ja possui o dominio institucional de operacoes com drones, incluindo drones, sensores, pilotos, areas, missoes de voo e ARO.

O DroneOps precisa gerar valor no campo sem duplicar o Observa. Os formularios atuais de ARO e checklist mostram que a necessidade imediata e comprovar que o protocolo foi aplicado corretamente antes e durante a operacao.

## Decisao

O MVP do DroneOps sera orientado a certificacao de aplicacao de protocolo:

- identificacao de operador;
- identificacao de drone por QR Code ou entrada manual;
- registro de localizacao GPS do celular;
- aplicacao do checklist pre-voo;
- registro de decisao final;
- anexacao de fotos e documentos comprobatórios;
- geracao de pacote CampoSync para o SISTER-Observa.

## Consequencias

- O DroneOps nao tentara substituir todo o backoffice do Observa.
- O primeiro fluxo sera checklist/ARO/evidencia, nao planejamento completo de missao.
- O Observa continuara sendo o registro institucional e ponto de revisao humana.
- A interface web do DroneOps deve priorizar uso em celular no local da operacao.
- O CampoNode precisara servir a interface em HTTPS para recursos de GPS/camera em celulares.

