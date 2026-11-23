#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""curve-vectors-oracle.py: writes vectors/curve-points.json, the contract of omx_curve_points_json.

Every expected number is a CLOSED FORM evaluated in 60-digit decimal arithmetic from the request's own terms. The
library is never run to write a vector (the typed-contracts spec, section 6.1): this file shares no line with
omx_curve.h, and a kernel that drifts from its law is red against it, not written back into it.

The laws, each exact in the design's own terms:
  transfer curve  y = x + g(x) + makeup, g the static gain computer of the family's mode: above (dynamics, limiter,
                  de-esser: slope 1/ratio - 1, a limiter's ratio infinite, a soft knee the quadratic over [-W/2, W/2])
                  and below (a gate, hard knee: slope ratio - 1, held at the range);
  head filter     the cookbook high-pass (RBJ) section's |H(e^jw)| at each grid frequency, a 12 dB/oct filter one
                  section at Q = 1/sqrt(2), a 24 dB/oct filter two at the Butterworth Qs 1/(2 cos(pi/8)), 1/(2 cos(3 pi/8));
  bell / notch    the gain at the centre: a bell's is its gain within 5e-5 dB, a notch's is the floor of the drawing (-120 dB);
  delay stems     echo k at k x time, mix x feedback^(k-1) in dB, the dry tap at 1 - mix, a leg per ping-pong turn.
Each row carries the tolerance MEASURED against the library (test/curve-vectors.mjs --report) and rounded up: the
transfer curves and the notch floor are exact (0), the delay's levels differ by 4e-15 dB (a log10 and a pow), a 12 dB/oct
head filter by 4e-10 dB (the cookbook's formula against the section's coefficients), a 24 dB/oct one by 4.4e-8 dB (the
kernel's seven-digit Butterworth Qs), a bell's centre by 8e-6 dB (the matched pair meets the analogue gain to a part in a
million). A tolerance is never widened to absorb a second implementation's miss: that is a finding.

Usage: curve-vectors-oracle.py [--check] <out.json>   (--check compares the file to a fresh render and exits 1 on a difference)
"""
import json
import sys
from decimal import Decimal as D, getcontext

getcontext().prec = 60
PI = D("3.14159265358979323846264338327950288419716939937510582097494")


def dsin_cos(x):
    """sin and cos by Taylor series after reduction to [-pi, pi]."""
    two_pi = 2 * PI
    x = x - two_pi * (x / two_pi).to_integral_value()
    s = c = D(0)
    k = 0
    t = D(1)
    while True:
        if k % 4 == 0:
            c += t
        elif k % 4 == 1:
            s += t
        elif k % 4 == 2:
            c -= t
        else:
            s -= t
        k += 1
        t = t * x / k
        if abs(t) < D(10) ** -55 and k > 10:
            break
    return s, c


def db(lin):
    return 20 * lin.log10()


def lv(x0, x1, i, n):
    return D(x0) + (D(x1) - D(x0)) * i / (n - 1)


# ---- transfer curves -------------------------------------------------------------------------------------
def gain_above(x, thr, ratio, knee):
    xs = D(x) - D(thr)
    half = D(knee) / 2
    slope = (D(1) / D(ratio) - 1) if ratio is not None else D(-1)
    if knee > 0 and xs < half:
        return D(0) if xs <= -half else slope * (xs + half) ** 2 / (2 * D(knee))
    return D(0) if xs <= -half else slope * xs


def gain_below(x, thr, ratio, rng):
    xs = D(x) - D(thr)
    g = D(0) if xs >= 0 else (D(ratio) - 1) * xs
    return max(g, D(rng))


def transfer_row(name, family, terms, x0, x1, n):
    thr = next(t["value"] for t in terms if t["role"] == "threshold")
    ratio = next((t["value"] for t in terms if t["role"] == "ratio"), None)
    knee = next((t["value"] for t in terms if t["role"] == "knee"), 0)
    mk = next((t["value"] for t in terms if t["role"] == "makeup"), 0)
    rng = next((t["value"] for t in terms if t["role"] == "range"), None)
    pts = []
    for i in range(n):
        x = lv(x0, x1, i, n)
        g = gain_below(x, thr, ratio, rng) if family == "gate" else gain_above(x, thr, ratio, knee)
        pts.append({"x": float(x), "y": float(x + g + D(mk))})
    return {
        "name": name,
        "request": {"kind": "transfer_curve", "family": family, "sampleRate": 48000, "points": n,
                    "extents": {"min": x0, "max": x1}, "terms": terms},
        "tolerance": {"x": 0, "y": 0},
        "expect": {"points": pts, "exact": True},
    }


def t(role, value, unit, **extra):
    return dict({"role": role, "value": value, "unit": unit}, **extra)


# ---- head filters --------------------------------------------------------------------------------------
def hp_section_db(f, f0, q, fs):
    w0 = 2 * PI * D(f0) / D(fs)
    s0, c0 = dsin_cos(w0)
    alpha = s0 / (2 * q)
    a0 = 1 + alpha
    b0 = (1 + c0) / 2 / a0
    b1 = -(1 + c0) / a0
    a1 = -2 * c0 / a0
    a2 = (1 - alpha) / a0
    w = 2 * PI * D(f) / D(fs)
    s1, c1 = dsin_cos(w)
    s2, c2 = dsin_cos(2 * w)
    num = (b0 + b1 * c1 + b0 * c2) ** 2 + (b1 * s1 + b0 * s2) ** 2
    den = (1 + a1 * c1 + a2 * c2) ** 2 + (a1 * s1 + a2 * s2) ** 2
    return 10 * (num / den).log10()


def eq_head_row(name, slope24, f0, fs, n):
    f_lo, f_hi = D(20), min(D(20000), D("0.999") * D(fs) / 2)
    if slope24:
        qs = [D(1) / (2 * dsin_cos(PI / 8)[1]), D(1) / (2 * dsin_cos(3 * PI / 8)[1])]
    else:
        qs = [D(1) / D(2).sqrt()]
    pts = []
    for i in (0, n // 4, n // 2, (3 * n) // 4, n - 1):
        f = f_lo * (f_hi / f_lo) ** (D(i) / (n - 1))
        pts.append({"x": float(f), "y": float(sum(hp_section_db(f, f0, q, fs) for q in qs))})
    return {
        "name": name,
        "request": {"kind": "eq_response", "family": "eq", "sampleRate": fs, "points": n,
                    "terms": [t("hpf_freq", f0, "Hz"), t("hpf_slope", 24 if slope24 else 12, "dB")]},
        "tolerance": {"x": 1e-15, "y": 1e-7 if slope24 else 1e-9},
        "expect": {"points": pts},
    }


def eq_centre_row(name, shape, gain, f0, q, fs, floor_db=None):
    # the matched pole and zero pairs meet the analogue gain at the centre to a part in a million (measured 8e-6 dB
    # at Q 1, 3e-6 dB at Q 2), not exactly; a notch's null is exact, and held at the drawing's floor
    y = float(gain) if floor_db is None else float(floor_db)
    terms = [t("band_freq", f0, "Hz", shape=shape), t("band_q", q, "none"), t("band_on", 1, "none")]
    if floor_db is None:
        terms.insert(1, t("band_gain", gain, "dB"))
    return {
        "name": name,
        "request": {"kind": "eq_response", "family": "eq", "sampleRate": fs, "points": 32, "terms": terms},
        "tolerance": {"x": 0, "y": 5e-5 if floor_db is None else 0},
        "expect": {"points": [{"x": float(f0), "y": y}]},
    }


# ---- delay stems ---------------------------------------------------------------------------------------
def delay_row(name, time_ms, mix, fb, pingpong):
    stems = []
    dry = D(1) - D(mix)
    if dry > 0 and db(dry) > -60:
        stems.append({"x": 0.0, "y": float(db(dry)), "leg": "both"})
    k = 1
    while len(stems) < 32:
        lin = D(mix) * D(fb) ** (k - 1)
        if not (lin > 0 and db(lin) > -60):
            break
        stems.append({"x": float(D(time_ms) * k), "y": float(db(lin)),
                      "leg": "both" if not pingpong else ("left" if k & 1 else "right")})
        k += 1
    return {
        "name": name,
        "request": {"kind": "time_response", "family": "delay", "sampleRate": 48000, "points": 8,
                    "terms": [t("time", time_ms, "ms"), t("feedback", fb, "none"), t("mix", mix, "none"),
                              t("pingpong", 1 if pingpong else 0, "none")]},
        "tolerance": {"x": 0, "y": 1e-13},
        "expect": {"stems": stems, "exact": True},
    }


# ---- refusals: the code each missing or refused term is answered with ----------------------------------
def refusal(name, request, code):
    return {"name": name, "request": request, "expect": {"code": code}}


def base(kind="transfer_curve", family="dynamics", **kw):
    r = {"kind": kind, "family": family, "sampleRate": 48000, "points": 8, "terms": []}
    r.update(kw)
    return r


def rows():
    dyn = [t("threshold", -20, "dB"), t("ratio", 4, "ratio")]
    out = [
        transfer_row("dynamics hard knee", "dynamics", dyn, -60, 0, 13),
        transfer_row("dynamics soft knee with make-up", "dynamics",
                     dyn + [t("knee", 8, "dB"), t("makeup", 3, "dB")], -60, 0, 25),
        transfer_row("de-esser", "deesser", [t("threshold", -30, "dB"), t("ratio", 2.5, "ratio")], -60, 0, 7),
        transfer_row("limiter", "limiter", [t("threshold", -6, "dB")], -40, 0, 9),
        transfer_row("gate hard knee", "gate",
                     [t("threshold", -40, "dB"), t("ratio", 3, "ratio"), t("range", -30, "dB")], -80, -20, 13),
        eq_head_row("high-pass 12 dB/oct at 100 Hz, 48 kHz", False, 100, 48000, 24),
        eq_head_row("high-pass 24 dB/oct at 250 Hz, 96 kHz", True, 250, 96000, 24),
        eq_centre_row("bell +6 dB at its centre", "bell", 6, 1000, 1, 48000),
        eq_centre_row("bell -9 dB at its centre, 192 kHz", "bell", -9, 4000, 2, 192000),
        eq_centre_row("notch at its centre is the drawing's floor", "notch", None, 3000, 8, 48000, floor_db=-120),
        delay_row("delay, half mix, half feedback", 250, 0.5, 0.5, False),
        delay_row("delay, ping-pong, light feedback", 120, 0.4, 0.3, True),
        refusal("a transfer curve with no threshold", base(terms=[t("ratio", 4, "ratio")]), "termMissing"),
        refusal("a transfer curve with no ratio", base(terms=[t("threshold", -20, "dB")]), "termMissing"),
        refusal("a threshold in kHz", base(terms=[t("threshold", -20, "kHz"), t("ratio", 4, "ratio")]), "unitRefused"),
        refusal("a sample rate of zero", base(sampleRate=0, terms=dyn), "valueNotFinite"),
        refusal("one grid point", base(points=1, terms=dyn), "pointsRange"),
        refusal("an eq display on the dynamics family", base(kind="eq_response", terms=dyn), "kindUnknown"),
        refusal("an eq with no band and no head filter", base(kind="eq_response", family="eq", terms=[]), "termMissing"),
        refusal("a band with no shape", base(kind="eq_response", family="eq",
                                             terms=[t("band_freq", 1000, "Hz"), t("band_gain", 3, "dB")]), "termMissing"),
        refusal("a 18 dB/oct head filter", base(kind="eq_response", family="eq",
                                                terms=[t("hpf_freq", 100, "Hz"), t("hpf_slope", 18, "dB")]), "kindUnknown"),
        refusal("a delay with no feedback", base(kind="time_response", family="delay",
                                                 terms=[t("time", 100, "ms")]), "termMissing"),
    ]
    return out


def render():
    doc = {
        "description": "The contract of omx_curve_points_json (curve-io.schema.json): each row a request, the answer a "
                       "closed form gives, and the tolerance its arithmetic earns. Written by tools/curve-vectors-oracle.py "
                       "in 60-digit decimal arithmetic, never by the library. A row of `exact` lists every point or stem "
                       "in order; any other lists points found by their x. A row with a code is a refusal.",
        "rows": rows(),
    }
    return json.dumps(doc, indent=1, sort_keys=False) + "\n"


def main(argv):
    check = "--check" in argv
    paths = [a for a in argv[1:] if a != "--check"]
    if len(paths) != 1:
        sys.stderr.write(__doc__)
        return 2
    text = render()
    if check:
        have = open(paths[0]).read()
        if have != text:
            sys.stderr.write("curve-vectors-oracle: %s is not what the oracle writes\n" % paths[0])
            return 1
        return 0
    open(paths[0], "w").write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
