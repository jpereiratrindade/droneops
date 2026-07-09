# Release Notes v0.2.0

Evolução estrutural do DroneOps, introduzindo persistência local robusta via SQLite e aplicativo instalável PWA para operação offline em campo.

## Núcleo C++ e Persistência (SQLite)

Conforme estabelecido pela ADR-007, o núcleo C++ do DroneOps foi atualizado para suportar armazenamento em SQLite:

- Implementada a camada `SqliteStore` (`src/Sqlite.cpp` e `include/droneops/Sqlite.hpp`).
- Compilação estática do SQLite (amalgamation) através de `FetchContent` no CMake, mantendo a arquitetura leve sem exigir privilégios de root para dependências de sistema (`sqlite-devel`).
- Suporte à flag `--db` em toda a interface de linha de comando (`init-mission`, `validate`, `package`).
- Criação automática de schema de tabelas e serialização de checklists.
- 100% dos novos testes SQLite passando em ambiente isolado de memória (`:memory:`).

## Interface Web (PWA e Offline-First)

A interface em `web/static` evoluiu de um mockup estático para uma *Single Page Application* instalável:

- Adição do `manifest.json` com identidade visual.
- Implementação do Service Worker (`sw.js`) para roteamento `cache-first`, garantindo abertura e navegação mesmo em avião ou no campo.
- Armazenamento de rascunhos no `localStorage`, permitindo edição dinâmica dos campos da missão.
- Toasts de feedback (sucesso e erro) para as operações principais, incluindo geolocalização e captura de QR Code.
- O arquivo HTML não contém mais IDs codificados fixos (como `DO-001`), reagindo dinamicamente às modificações do operador.
