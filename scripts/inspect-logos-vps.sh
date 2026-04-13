#!/usr/bin/env bash
set -u

LOGOS_CONTAINER_NAME="${LOGOS_CONTAINER_NAME:-logos-node}"
WAKU_CONTAINER_NAME="${WAKU_CONTAINER_NAME:-nwaku}"
LOGOS_API_PORT="${LOGOS_API_PORT:-8080}"
USE_SUDO="${USE_SUDO:-false}"

if [[ "$USE_SUDO" == "true" ]]; then
  DOCKER_CMD="sudo -n docker"
else
  DOCKER_CMD="docker"
fi

section() {
  printf '__NODE_INSPECTOR_SECTION__:%s\n' "$1"
}

section_end() {
  printf '__NODE_INSPECTOR_END_SECTION__:%s\n' "$1"
}

section HOST_BASICS
hostname 2>/dev/null || true
uname -srmo 2>/dev/null || true
uptime -p 2>/dev/null || uptime 2>/dev/null || true
section_end HOST_BASICS

section ROOT_DF
df -B1 / 2>/dev/null || true
section_end ROOT_DF

section MEMORY
free -b 2>/dev/null || true
section_end MEMORY

section DOCKER_SYSTEM_DF
$DOCKER_CMD system df 2>/dev/null || true
section_end DOCKER_SYSTEM_DF

section DOCKER_PS
$DOCKER_CMD ps --format '{{.Names}}|{{.Image}}|{{.Status}}|{{.Ports}}' 2>/dev/null || true
section_end DOCKER_PS

section SERVICE_LOGOS_NODE
if $DOCKER_CMD inspect "$LOGOS_CONTAINER_NAME" >/dev/null 2>&1; then
  printf 'name=%s\n' "$LOGOS_CONTAINER_NAME"
  $DOCKER_CMD inspect --format 'running={{.State.Running}}
health={{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}
restart={{.HostConfig.RestartPolicy.Name}}
image={{.Config.Image}}
ports={{json .NetworkSettings.Ports}}
status={{.State.Status}}' "$LOGOS_CONTAINER_NAME"
else
  printf '__MISSING__\n'
fi
section_end SERVICE_LOGOS_NODE

section SERVICE_NWAKU
if $DOCKER_CMD inspect "$WAKU_CONTAINER_NAME" >/dev/null 2>&1; then
  printf 'name=%s\n' "$WAKU_CONTAINER_NAME"
  $DOCKER_CMD inspect --format 'running={{.State.Running}}
health={{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}
restart={{.HostConfig.RestartPolicy.Name}}
image={{.Config.Image}}
ports={{json .NetworkSettings.Ports}}
status={{.State.Status}}' "$WAKU_CONTAINER_NAME"
else
  printf '__MISSING__\n'
fi
section_end SERVICE_NWAKU

section NETWORK_INFO
curl -fsS --max-time 8 "http://127.0.0.1:${LOGOS_API_PORT}/network/info" 2>/dev/null || printf '__ERROR__:curl_failed\n'
section_end NETWORK_INFO

section CRYPTARCHIA_INFO
curl -fsS --max-time 8 "http://127.0.0.1:${LOGOS_API_PORT}/cryptarchia/info" 2>/dev/null || printf '__ERROR__:curl_failed\n'
section_end CRYPTARCHIA_INFO

section LOGOS_NODE_LOGS
$DOCKER_CMD logs --tail 120 "$LOGOS_CONTAINER_NAME" 2>&1 || printf '__ERROR__:docker_logs_failed\n'
section_end LOGOS_NODE_LOGS

section NWAKU_LOGS
$DOCKER_CMD logs --tail 120 "$WAKU_CONTAINER_NAME" 2>&1 || printf '__ERROR__:docker_logs_failed\n'
section_end NWAKU_LOGS
