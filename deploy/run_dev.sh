#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${DRONEOPS_PORT:-8011}"
HOST="${DRONEOPS_HOST:-0.0.0.0}"
LAN_IP="${DRONEOPS_LAN_IP:-$(ip route get 1.1.1.1 2>/dev/null | awk '{for (i=1; i<=NF; i++) if ($i == "src") {print $(i+1); exit}}')}"
LAN_IP="${LAN_IP:-$(hostname -I 2>/dev/null | awk '{print $1}')}"
CERT_DIR="${REPO_ROOT}/web/dev-certs"
CERT_IP_FILE="${CERT_DIR}/lan-ip.txt"

cmake -S "${REPO_ROOT}" -B "${REPO_ROOT}/build" -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${REPO_ROOT}/build"
ctest --test-dir "${REPO_ROOT}/build" --output-on-failure

mkdir -p "${CERT_DIR}"
if [[ ! -f "${CERT_DIR}/cert.pem" || ! -f "${CERT_DIR}/key.pem" || ! -f "${CERT_IP_FILE}" || "$(cat "${CERT_IP_FILE}")" != "${LAN_IP}" ]]; then
  openssl req -x509 -newkey rsa:2048 \
    -keyout "${CERT_DIR}/key.pem" \
    -out "${CERT_DIR}/cert.pem" \
    -days 365 -nodes \
    -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:${LAN_IP}"
  printf '%s\n' "${LAN_IP}" > "${CERT_IP_FILE}"
fi

echo "DroneOps dev local: https://localhost:${PORT}/"
echo "DroneOps dev rede : https://${LAN_IP}:${PORT}/"
echo "Pressione Ctrl+C para parar."

# Roda o servidor C++ com suporte nativo a HTTPS (ADR-003: Removida dependência do Python)
exec "${REPO_ROOT}/build/droneops" serve \
  --db "${REPO_ROOT}/build/droneops_dev.db" \
  --web "${REPO_ROOT}/web/static" \
  --port "${PORT}" \
  --cert "${CERT_DIR}/cert.pem" \
  --key "${CERT_DIR}/key.pem"
