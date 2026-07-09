# Release Notes v0.3.0

O DroneOps agora é oficialmente uma solução **Fullstack**, operando suas três camadas (Motor C++, Persistência Relacional e Interface Web PWA) de forma nativa e integrada em um único artefato compilado.

## Servidor Web C++ Integrado

Cumprindo a ADR-003, eliminamos a dependência de servidores web externos ou scripts Python (`server.py`) para operação. O próprio binário do DroneOps agora atua como servidor backend.

- Integração nativa do `yhirose/cpp-httplib` (Servidor HTTP multithreaded leve).
- Integração do `nlohmann/json` para manipulação robusta de JSONs.
- Novo comando CLI: `droneops serve --db <caminho.db> --web <caminho/para/web/static> --port 8011`.
- O comando serve simultaneamente a interface web PWA e a API.

## API REST Nativa

Foi introduzida uma API REST para comunicação direta entre o frontend e o banco de dados local SQLite:
- `GET /api/missions/:id`: Responde com o JSON da missão buscando os dados nativamente do banco.
- `POST /api/missions`: Recebe payloads em JSON do frontend e converte para registros `MissionRecord`, armazenando tudo no `.db`.

## Sincronização Dinâmica PWA

A interface `app.js` foi refatorada para atuar em conjunto com o novo servidor C++:
- Se estiver online (`navigator.onLine`), o PWA sincroniza as alterações da missão em tempo real (via requisições `fetch()`) para o backend C++, mantendo o banco de dados SQLite sempre atualizado.
- O fallback para `localStorage` foi mantido para casos onde o equipamento perde comunicação no campo.
- Ao abrir a página, a interface automaticamente tenta carregar o estado mais recente direto da API C++, sincronizando visualmente o progresso.
