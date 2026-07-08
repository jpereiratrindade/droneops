# Interface web DroneOps

Primeira tela estatica para desenhar o fluxo visual do modulo com a mesma identidade do MorfoCampo.

Uso local simples:

```bash
cd web
python3 -m http.server 8011
```

Abra:

```text
http://127.0.0.1:8011/
```

Esta tela ainda nao substitui a API local. Ela serve para estabilizar o fluxo visual de missao, checklists, ocorrencias e manifesto CampoSync antes de portar o servidor web do MorfoCampo.
