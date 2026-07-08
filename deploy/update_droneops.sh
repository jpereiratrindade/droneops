#!/usr/bin/env bash
set -euo pipefail

CONFIG_FILE="${DRONEOPS_CONFIG_FILE:-/etc/droneops/droneops.env}"
if [[ -f "${CONFIG_FILE}" ]]; then
  # shellcheck disable=SC1090
  source "${CONFIG_FILE}"
fi

INSTALL_DIR="${DRONEOPS_INSTALL_DIR:-/opt/droneops}"
STATE_DIR="${DRONEOPS_STATE_DIR:-/var/lib/droneops}"
SERVICE_NAME="${DRONEOPS_SERVICE_NAME:-droneops}"
REPO_URL="${DRONEOPS_UPDATE_REPO:-git@github.com:jpereiratrindade/droneops.git}"
REQUIRE_CI="${DRONEOPS_UPDATE_REQUIRE_CI:-0}"
RUN_TESTS="${DRONEOPS_UPDATE_RUN_TESTS:-1}"
UPDATE_ENABLED="${DRONEOPS_UPDATE_ENABLED:-0}"
GITHUB_TOKEN="${DRONEOPS_GITHUB_TOKEN:-${GITHUB_TOKEN:-}}"

CACHE_DIR="${STATE_DIR}/update-cache/repo"
STAGING_DIR="${STATE_DIR}/update-staging"
CODE_BACKUP_DIR="${STATE_DIR}/code-backups"
VERSION_FILE="${STATE_DIR}/installed-version.txt"
LOG_FILE="${STATE_DIR}/update.log"
LOCK_FILE="/run/lock/droneops-update.lock"

AUTO_MODE=0
if [[ "${1:-}" == "--auto" ]]; then
  AUTO_MODE=1
fi

mkdir -p "${STATE_DIR}"
touch "${LOG_FILE}"
exec >>"${LOG_FILE}" 2>&1

log() {
  printf '[%s] %s\n' "$(date -Iseconds)" "$*"
}

fail() {
  log "ERRO: $*"
  exit 1
}

require_root() {
  [[ "${EUID}" -eq 0 ]] || fail "execute como root"
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || fail "comando obrigatório não encontrado: $1"
}

internet_available() {
  python3 - <<'PY'
import sys
import urllib.request

try:
    urllib.request.urlopen("https://github.com", timeout=8).close()
except Exception:
    sys.exit(1)
PY
}

repo_slug() {
  python3 - "${REPO_URL}" <<'PY'
import re
import sys

url = sys.argv[1]
for pattern in [
    r"github\.com[:/](?P<owner>[^/]+)/(?P<repo>[^/.]+)(?:\.git)?$",
    r"github\.com/(?P<owner>[^/]+)/(?P<repo>[^/.]+)(?:\.git)?$",
]:
    match = re.search(pattern, url)
    if match:
        print(f"{match.group('owner')}/{match.group('repo')}")
        sys.exit(0)
sys.exit(1)
PY
}

prepare_cache() {
  mkdir -p "$(dirname "${CACHE_DIR}")"
  if [[ -d "${CACHE_DIR}/.git" ]]; then
    git -C "${CACHE_DIR}" remote set-url origin "${REPO_URL}"
    git -C "${CACHE_DIR}" fetch --force --prune --tags origin \
      '+refs/heads/*:refs/remotes/origin/*' \
      '+refs/tags/*:refs/tags/*'
  else
    rm -rf "${CACHE_DIR}"
    git clone --filter=blob:none --no-checkout "${REPO_URL}" "${CACHE_DIR}"
    git -C "${CACHE_DIR}" fetch --force --prune --tags origin \
      '+refs/heads/*:refs/remotes/origin/*' \
      '+refs/tags/*:refs/tags/*'
  fi
}

latest_tag() {
  git -C "${CACHE_DIR}" tag -l 'v*' --sort=-v:refname | head -n 1
}

current_version() {
  if [[ -f "${VERSION_FILE}" ]]; then
    awk -F= '$1 == "tag" {print $2}' "${VERSION_FILE}" | tail -n 1
  fi
}

tag_commit() {
  git -C "${CACHE_DIR}" rev-list -n 1 "$1"
}

ci_succeeded() {
  local slug="$1"
  local sha="$2"
  python3 - "${slug}" "${sha}" "${GITHUB_TOKEN}" <<'PY'
import json
import sys
import urllib.parse
import urllib.request

slug, sha, token = sys.argv[1], sys.argv[2], sys.argv[3]
query = urllib.parse.urlencode({"head_sha": sha, "status": "completed", "per_page": "30"})
headers = {"Accept": "application/vnd.github+json", "User-Agent": "droneops-updater"}
if token:
    headers["Authorization"] = f"Bearer {token}"
url = f"https://api.github.com/repos/{slug}/actions/runs?{query}"
try:
    with urllib.request.urlopen(urllib.request.Request(url, headers=headers), timeout=15) as response:
        payload = json.load(response)
except Exception as exc:
    print(exc, file=sys.stderr)
    sys.exit(2)
for run in payload.get("workflow_runs", []):
    if run.get("head_sha") == sha and run.get("conclusion") == "success":
        print(run.get("html_url", "CI success"))
        sys.exit(0)
sys.exit(1)
PY
}

build_candidate() {
  local tag="$1"
  rm -rf "${STAGING_DIR}"
  mkdir -p "${STAGING_DIR}/src"
  git -C "${CACHE_DIR}" archive "${tag}" | tar -C "${STAGING_DIR}/src" -xf -

  cmake -S "${STAGING_DIR}/src" -B "${STAGING_DIR}/src/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build "${STAGING_DIR}/src/build"
  if [[ "${RUN_TESTS}" == "1" ]]; then
    ctest --test-dir "${STAGING_DIR}/src/build" --output-on-failure
    python3 -m py_compile "${STAGING_DIR}/src/web/server.py"
  fi
}

backup_code() {
  mkdir -p "${CODE_BACKUP_DIR}"
  local archive="${CODE_BACKUP_DIR}/droneops_code_$(date +%Y%m%d_%H%M%S).tar.gz"
  tar -C "${INSTALL_DIR}" -czf "${archive}" .
  chmod 0600 "${archive}"
  printf '%s\n' "${archive}"
}

restore_code() {
  local archive="$1"
  log "restaurando código anterior: ${archive}"
  rm -rf "${INSTALL_DIR:?}/"*
  tar -C "${INSTALL_DIR}" -xzf "${archive}"
}

record_version() {
  local tag="$1"
  local sha="$2"
  {
    printf 'tag=%s\n' "${tag}"
    printf 'commit=%s\n' "${sha}"
    printf 'installed_at=%s\n' "$(date -Iseconds)"
    printf 'repo=%s\n' "${REPO_URL}"
  } > "${VERSION_FILE}"
  chmod 0644 "${VERSION_FILE}"
}

install_candidate() {
  local tag="$1"
  local sha="$2"
  local code_backup="$3"

  systemctl stop "${SERVICE_NAME}"
  if ! rsync -a --delete \
      --exclude ".git" \
      --exclude "build" \
      --exclude "out" \
      --exclude "web/dev-certs" \
      --exclude "__pycache__" \
      "${STAGING_DIR}/src/" "${INSTALL_DIR}/"; then
    restore_code "${code_backup}"
    systemctl start "${SERVICE_NAME}" || true
    fail "falha ao copiar nova versão"
  fi

  chown -R root:root "${INSTALL_DIR}"
  record_version "${tag}" "${sha}"

  if ! systemctl start "${SERVICE_NAME}"; then
    restore_code "${code_backup}"
    systemctl start "${SERVICE_NAME}" || true
    fail "serviço não iniciou após atualização"
  fi
}

main() {
  require_root
  require_command git
  require_command cmake
  require_command ctest
  require_command rsync
  require_command tar
  require_command python3

  exec 9>"${LOCK_FILE}"
  flock -n 9 || fail "outra atualização já está em execução"

  if [[ "${AUTO_MODE}" == "1" && "${UPDATE_ENABLED}" != "1" ]]; then
    log "atualização automática desabilitada"
    exit 0
  fi

  internet_available || fail "sem internet"
  prepare_cache
  local tag current sha slug backup
  tag="$(latest_tag)"
  [[ -n "${tag}" ]] || fail "nenhuma tag v* encontrada"
  current="$(current_version || true)"
  if [[ "${tag}" == "${current}" ]]; then
    log "já está na versão ${tag}"
    exit 0
  fi

  sha="$(tag_commit "${tag}")"
  if [[ "${REQUIRE_CI}" == "1" ]]; then
    slug="$(repo_slug)" || fail "não foi possível extrair owner/repo de ${REPO_URL}"
    ci_succeeded "${slug}" "${sha}" || fail "CI não aprovado para ${tag}"
  fi

  build_candidate "${tag}"
  backup="$(backup_code)"
  install_candidate "${tag}" "${sha}" "${backup}"
  log "atualização concluída: ${tag}"
}

main "$@"

