# DroneOps Deploy

## Desenvolvimento no PC

Execute a partir da raiz do repositório:

```bash
deploy/run_dev.sh
```

O script compila o núcleo C++, roda os testes e sobe a interface em:

```text
https://127.0.0.1:8780/static/
```

Use HTTPS porque GPS, câmera e QR Code no navegador dependem de contexto seguro em muitos celulares/navegadores.

## Raspberry Pi 5

Premissas:

- Raspberry Pi OS/Debian.
- Repositório clonado no RPi.
- NetworkManager disponível.
- Uso dedicado ou imagem preparada para DroneOps.

Instalação:

```bash
sudo deploy/install_droneops_rpi5.sh
```

Padrões:

| Variável | Padrão |
| --- | --- |
| `DRONEOPS_WIFI_SSID` | `DRONEOPS` |
| `DRONEOPS_WIFI_PASSWORD` | `droneops5` |
| `DRONEOPS_HOSTNAME` | `droneops` |
| `DRONEOPS_PORT` | `8021` |
| `DRONEOPS_QR_URL` | `https://droneops.local:8021/static/` |
| `DRONEOPS_UPDATE_REPO` | `git@github.com:jpereiratrindade/droneops.git` |

Depois da instalação:

```bash
sudo systemctl status droneops
sudo journalctl -u droneops -f
```

Acesso no celular:

```text
Wi-Fi: DRONEOPS
URL: https://droneops.local:8021/static/
```

## Upgrade

Atualização manual:

```bash
sudo droneops-update
```

Atualização automática diária:

```bash
sudoedit /etc/droneops/droneops.env
```

Defina:

```bash
DRONEOPS_UPDATE_ENABLED=1
```

Depois:

```bash
sudo systemctl daemon-reload
sudo systemctl restart droneops-update.timer
```

O atualizador:

- busca tags `v*` no GitHub;
- compila em staging;
- roda CTest;
- faz backup do código anterior;
- troca o código;
- reinicia o serviço;
- restaura o código anterior se a atualização falhar.

## GitHub

Repositório remoto previsto:

```text
git@github.com:jpereiratrindade/droneops.git
```

Fluxo recomendado para liberar atualização no RPi:

```bash
git tag v0.1.1
git push origin main
git push origin v0.1.1
```

