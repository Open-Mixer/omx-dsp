# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# jack2-env.sh — sourced (bash) by every run that measures against a live JACK: the externals arm's
# jack_port_get_buffer case, its sabotage and omx-rta-read's null test. They measure jack2, the JACK an audio host
# such as a Zynthian runs. A system may carry PipeWire's JACK libraries too (a Fedora image does), and the loader then
# hands jackd PipeWire's libjackserver, which runs no server, and every client PipeWire's libjack. So jack2's libdir,
# as its jack.pc names it, goes first on LD_LIBRARY_PATH, and each run names the JACK it measured and fails by name
# when the loaded library is PipeWire's.
#
#   jack2_env                     put jack2's libdir first on LD_LIBRARY_PATH (exported) and name it
#   jack2_loaded PID LIB          the path of LIB (libjack.so, libjackserver.so) mapped in process PID, or nothing
#   jack2_name LABEL PID LIB      print `jack: LABEL <path>`; return 1, printing `jack: FAIL ...`, when the path
#                                 is PipeWire's or LIB is not mapped

jack2_env() {
  local d
  d="$(pkg-config --variable=libdir jack 2>/dev/null)" || d=""
  if [ -z "$d" ]; then
    echo "jack: no jack.pc, no jack2 libdir to put first"
    return 1
  fi
  export LD_LIBRARY_PATH="$d${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  echo "jack: jack2's libdir first: $d; $(jackd --version 2>&1 | grep -m1 -i 'version' || echo 'jackd --version said nothing')"
}

jack2_loaded() {
  awk -v lib="/$2" 'index($6, lib) { print $6; exit }' "/proc/$1/maps" 2>/dev/null
}

jack2_name() {
  local p
  p="$(jack2_loaded "$2" "$3")"
  if [ -z "$p" ]; then
    echo "jack: FAIL $1 has no $3 mapped"
    return 1
  fi
  case "$p" in
    *pipewire*) echo "jack: FAIL $1 loaded PipeWire's $3 ($p), not jack2's"; return 1 ;;
  esac
  echo "jack: $1 $p"
}
