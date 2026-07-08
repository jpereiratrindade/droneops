#!/usr/bin/env bash
set -euo pipefail

if [[ "${EUID}" -ne 0 ]]; then
  echo "Execute como root: sudo deploy/install_droneops_rpi5.sh" >&2
  exit 1
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INSTALL_DIR="${DRONEOPS_INSTALL_DIR:-/opt/droneops}"
STATE_DIR="${DRONEOPS_STATE_DIR:-/var/lib/droneops}"
CONFIG_DIR="${DRONEOPS_CONFIG_DIR:-/etc/droneops}"
SERVICE_USER="droneops"
SERVICE_FILE="/etc/systemd/system/droneops.service"
SSID="${DRONEOPS_WIFI_SSID:-DRONEOPS}"
WIFI_PASSWORD="${DRONEOPS_WIFI_PASSWORD:-droneops5}"
WIFI_IFACE="${DRONEOPS_WIFI_IFACE:-}"
HOSTNAME_TARGET="${DRONEOPS_HOSTNAME:-droneops}"
PORT="${DRONEOPS_PORT:-8021}"
QR_URL="${DRONEOPS_QR_URL:-https://droneops.local:${PORT}/static/}"
LOG_FILE="${STATE_DIR}/install.log"

mkdir -p "${STATE_DIR}"
touch "${LOG_FILE}"
exec > >(tee -a "${LOG_FILE}") 2>&1

fail() {
  echo "ERRO: $*" >&2
  exit 1
}

detect_wifi_iface() {
  if [[ -n "${WIFI_IFACE}" ]]; then
    echo "${WIFI_IFACE}"
    return
  fi
  nmcli -t -f DEVICE,TYPE device status 2>/dev/null | awk -F: '$2 == "wifi" {print $1; exit}'
}

validate_environment() {
  [[ -f "${REPO_ROOT}/CMakeLists.txt" && -d "${REPO_ROOT}/web" ]] || fail "execute a partir da raiz do repositório DroneOps"
  [[ ${#WIFI_PASSWORD} -ge 8 ]] || fail "DRONEOPS_WIFI_PASSWORD precisa ter pelo menos 8 caracteres"
}

install_dependencies() {
  export DEBIAN_FRONTEND=noninteractive
  apt-get update
  apt-get install -y \
    avahi-daemon \
    build-essential \
    ca-certificates \
    cmake \
    git \
    iw \
    network-manager \
    ninja-build \
    openssl \
    python3 \
    rsync
}

install_code() {
  mkdir -p "${INSTALL_DIR}"
  rsync -a --delete \
    --exclude ".git" \
    --exclude "build" \
    --exclude "out" \
    --exclude "web/dev-certs" \
    --exclude "__pycache__" \
    "${REPO_ROOT}/" "${INSTALL_DIR}/"
}

build_app() {
  cmake -S "${INSTALL_DIR}" -B "${INSTALL_DIR}/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build "${INSTALL_DIR}/build"
  ctest --test-dir "${INSTALL_DIR}/build" --output-on-failure
}

create_user_and_dirs() {
  if ! id "${SERVICE_USER}" >/dev/null 2>&1; then
    useradd --system --home "${STATE_DIR}" --shell /usr/sbin/nologin "${SERVICE_USER}"
  fi
  mkdir -p "${STATE_DIR}/certs" "${STATE_DIR}/packages" "${STATE_DIR}/evidencias" "${CONFIG_DIR}"
}

generate_cert() {
  if [[ ! -f "${STATE_DIR}/certs/cert.pem" || ! -f "${STATE_DIR}/certs/key.pem" ]]; then
    openssl req -x509 -newkey rsa:2048 \
      -keyout "${STATE_DIR}/certs/key.pem" \
      -out "${STATE_DIR}/certs/cert.pem" \
      -days 3650 -nodes \
      -subj "/CN=droneops.local" \
      -addext "subjectAltName=DNS:droneops.local,DNS:localhost,IP:127.0.0.1"
  fi
}

install_config() {
  install -m 0644 "${INSTALL_DIR}/deploy/droneops.env.example" "${CONFIG_DIR}/droneops.env"
  grep -vE '^(DRONEOPS_PORT|DRONEOPS_QR_URL)=' "${CONFIG_DIR}/droneops.env" > "${CONFIG_DIR}/droneops.env.tmp"
  {
    printf 'DRONEOPS_PORT=%s\n' "${PORT}"
    printf 'DRONEOPS_QR_URL=%s\n' "${QR_URL}"
  } >> "${CONFIG_DIR}/droneops.env.tmp"
  mv "${CONFIG_DIR}/droneops.env.tmp" "${CONFIG_DIR}/droneops.env"

  install -m 0644 "${INSTALL_DIR}/deploy/droneops.service" "${SERVICE_FILE}"
  install -m 0644 "${INSTALL_DIR}/deploy/droneops-update.service" /etc/systemd/system/droneops-update.service
  install -m 0644 "${INSTALL_DIR}/deploy/droneops-update.timer" /etc/systemd/system/droneops-update.timer
  install -m 0755 "${INSTALL_DIR}/deploy/update_droneops.sh" /usr/local/bin/droneops-update

  chown -R "${SERVICE_USER}:${SERVICE_USER}" "${STATE_DIR}"
  chown -R root:root "${INSTALL_DIR}" "${CONFIG_DIR}"
  chmod 0640 "${CONFIG_DIR}/droneops.env"
}

record_installed_version() {
  {
    printf 'tag=%s\n' "$(git -C "${REPO_ROOT}" describe --tags --exact-match 2>/dev/null || true)"
    printf 'commit=%s\n' "$(git -C "${REPO_ROOT}" rev-parse HEAD 2>/dev/null || true)"
    printf 'installed_at=%s\n' "$(date -Iseconds)"
    printf 'repo=%s\n' "$(git -C "${REPO_ROOT}" config --get remote.origin.url 2>/dev/null || echo "git@github.com:jpereiratrindade/droneops.git")"
  } > "${STATE_DIR}/installed-version.txt"
}

configure_hostname_and_hotspot() {
  hostnamectl set-hostname "${HOSTNAME_TARGET}"
  systemctl enable --now avahi-daemon NetworkManager

  local iface
  iface="$(detect_wifi_iface)"
  [[ -n "${iface}" ]] || fail "nenhuma interface Wi-Fi detectada; defina DRONEOPS_WIFI_IFACE"

  nmcli radio wifi on || true
  nmcli connection delete "${SSID}" >/dev/null 2>&1 || true
  nmcli connection add type wifi ifname "${iface}" con-name "${SSID}" autoconnect yes ssid "${SSID}"
  nmcli connection modify "${SSID}" \
    802-11-wireless.mode ap \
    802-11-wireless.band bg \
    ipv4.method shared \
    ipv6.method ignore \
    wifi-sec.key-mgmt wpa-psk \
    wifi-sec.psk "${WIFI_PASSWORD}" \
    connection.autoconnect yes
  nmcli connection up "${SSID}" || fail "falha ao ativar hotspot ${SSID}"
}

enable_service() {
  systemctl daemon-reload
  systemctl enable --now droneops
  systemctl enable --now droneops-update.timer
}

write_summary() {
  local ip_addr
  ip_addr="$(hostname -I 2>/dev/null | awk '{print $1}')"
  {
    echo "DroneOps instalado"
    echo "Wi-Fi: ${SSID}"
    echo "Senha Wi-Fi: ${WIFI_PASSWORD}"
    echo "URL local: ${QR_URL}"
    echo "URL por IP: https://${ip_addr}:${PORT}/static/"
    echo "Serviço: sudo systemctl status droneops"
    echo "Atualização: sudo droneops-update"
  } | tee "${STATE_DIR}/droneops-info.txt"
}

main() {
  validate_environment
  install_dependencies
  create_user_and_dirs
  install_code
  build_app
  generate_cert
  install_config
  record_installed_version
  configure_hostname_and_hotspot
  enable_service
  write_summary
}

main "$@"

