#!/usr/bin/env sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# contract-include.sh — prints the directory whose omxcontract/ holds the omx-contract C header
# this tree builds against (FreeMixer/omx-dsp#1, #9, #107). The header is never committed here.
#
#   1. $OMX_CONTRACT_INC, when set: a scratch render, used by the perturbation arms. Not
#      version-checked: it is a deliberate other contract.
#   2. the installed omx-contract-devel / libomx-contract-dev, found by pkg-config: the published package
#      this level builds against. No version is typed anywhere (owner ruling 2026-11-24, no hand-kept
#      floors): what is installed is what the tree builds against, and tools/no-floor-check.sh refuses
#      a floor put back in the packaging or data/.
#
# Nothing is downloaded. When no omx-contract is installed the script stops, naming the packages.
#
# Prints one line on stdout and nothing else; the reason for a refusal goes to stderr, exit 1.
set -eu
if [ -n "${OMX_CONTRACT_INC:-}" ]; then
  [ -f "$OMX_CONTRACT_INC/omxcontract/omx_contract.h" ] || { echo "contract-include: OMX_CONTRACT_INC=$OMX_CONTRACT_INC has no omxcontract/omx_contract.h" >&2; exit 1; }
  printf '%s\n' "$OMX_CONTRACT_INC"; exit 0
fi
need="omx-contract (omx-contract-devel / libomx-contract-dev: dnf builddep packaging/omx-dsp.spec, apt-get build-dep ./)"
command -v pkg-config >/dev/null 2>&1 || { echo "contract-include: pkg-config is missing, so the installed omx-contract cannot be found; this tree needs $need" >&2; exit 1; }
if ! pkg-config --exists omx-contract 2>/dev/null; then
  echo "contract-include: omx-contract is not installed: install $need" >&2
  exit 1
fi
have="$(pkg-config --modversion omx-contract)"
inc="$(pkg-config --variable=includedir omx-contract)"
[ -f "$inc/omxcontract/omx_contract.h" ] || { echo "contract-include: omx-contract $have is installed but $inc/omxcontract/omx_contract.h is missing" >&2; exit 1; }
printf '%s\n' "$inc"
