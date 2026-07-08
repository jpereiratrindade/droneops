#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${DRONEOPS_PORT:-8780}"
HOST="${DRONEOPS_HOST:-127.0.0.1}"
CERT_DIR="${REPO_ROOT}/web/dev-certs"

cmake -S "${REPO_ROOT}" -B "${REPO_ROOT}/build" -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${REPO_ROOT}/build"
ctest --test-dir "${REPO_ROOT}/build" --output-on-failure

mkdir -p "${CERT_DIR}"
if [[ ! -f "${CERT_DIR}/cert.pem" || ! -f "${CERT_DIR}/key.pem" ]]; then
  openssl req -x509 -newkey rsa:2048 \
    -keyout "${CERT_DIR}/key.pem" \
    -out "${CERT_DIR}/cert.pem" \
    -days 365 -nodes \
    -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
fi

echo "DroneOps dev: https://${HOST}:${PORT}/"
exec python3 "${REPO_ROOT}/web/server.py" \
  --directory "${REPO_ROOT}/web" \
  --host "${HOST}" \
  --port "${PORT}" \
  --ssl-keyfile "${CERT_DIR}/key.pem" \
  --ssl-certfile "${CERT_DIR}/cert.pem"
