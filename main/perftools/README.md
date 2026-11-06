# NSCLDAQ perftools

Tools for performance testing and basic diagnostics on individual
pieces of NSCLDAQ (pipes, sockets, ring buffers, the event builder's
inputs, file output, …).

> **Not built or installed by a normal NSCLDAQ build.** Enable them with
> `--enable-perftools` (autotools) or `-DENABLE_PERFTOOLS=ON` (cmake).
> When installed they land in `$DAQBIN/perftools`, separate from the
> normal NSCLDAQ programs.

Before running anything, source `daqsetup.bash` for the NSCLDAQ version
you want to test; it sets `DAQBIN` and the library paths. The tools are
then run as `$DAQBIN/perftools/<tool>` (the examples below use that form);
the installed binaries find the core NSCLDAQ libraries via their rpath.

All tools take GNU-style long options (`--name=value`). Every tool
supports `--help` (which lists its options and defaults) and `--version`.

## Tools at a glance

| tool | kind | summary |
|------|------|---------|
| [`pipeperf`](#pipeperf--unix-pipe-throughput-meter-pipe) | measure | throughput of a Unix pipe |
| [`sockperf`](#sockperf--tcp-socket-throughput-meter-socket) | measure | throughput of a TCP socket |
| [`rdoperf`](#rdoperf--ring-buffer-input-rate-meter-the-ring-name) | measure | rate of data arriving into a ring buffer |
| [`fragsrcperf`](#fragsrcperf--ring-fragment-source-throughput-meter-ringfragsrc) | measure | throughput of a ring fragment source (poses as an event builder) |
| [`bufferedoutperf`](#bufferedoutperf--buffered-file-write-throughput-no-meter) | measure | throughput of `io::CBufferedOutput` to a file |
| [`pipesend`](#pipesend--byte-stream-to-stdout-feeds-pipeperf) | generator | byte stream to stdout (feeds `pipeperf`) |
| [`socksend`](#socksend--byte-stream-over-tcp-feeds-sockperf) | generator | byte stream over TCP (feeds `sockperf`) |
| [`ritemMaker`](#ritemmaker--ring-item-stream-to-stdout) | generator | stream of `PHYSICS_EVENT` ring items |
| [`runmaker`](#runmaker--a-complete-run-to-stdout) | generator | a complete run: begin / data / end |
| [`fragmaker`](#fragmaker--event-builder-fragments-to-stdout-for-glom) | generator | event-builder fragments (to test glom) |
| [`checkevfiles`](#checkevfiles--physics_event-timestamp-check) | checker | checks `PHYSICS_EVENT` timestamps increment by one |
| [`evbfilecheck`](#evbfilecheck--event-builder-output-check) | checker | checks event-builder output correctness |
| [`metertest`](#metertest--meter-server-smoke-test) | test | meter-server smoke test |

## How monitoring works

A measuring tool reports its results in two independent ways, which serve
different purposes:

- **The log (`--logfile=PATH`) is the test output.** Each tool writes a full
  summary block there — sample count, wall time, total bytes, the aggregate
  rate, and the per-pass mean, min/max and standard deviation (see
  [The summary block](#the-summary-block)). This is the data you keep and
  compare across runs, and it needs no GUI or X display. Without `--logfile`
  a run records nothing, so use it for every real measurement.
- **The meter server is a live visualization, not a measurement tool.** Given
  `--meter-host`, a measuring tool also pushes its instantaneous bytes/sec to
  a meter server over a socket; `meterserver.tcl` is a Tk GUI that draws one
  needle gauge per named meter. It's useful for watching a run in real time or
  confirming data is flowing, but it shows only a moving needle — no
  statistics, nothing recorded. Don't read numbers off it.

The two are independent and combine: pass both `--logfile` and `--meter-host`
to watch the needle while the log captures the real results.

### The meter server

```
$DAQBIN/perftools/meterserver.tcl listen-port      # e.g. ... 2000
```

- Needs Tk and a working `$DISPLAY` — it's a GUI.
- The Tcl scripts are installed next to the tools in `$DAQBIN/perftools`.
  `meterserver.tcl` locates `meter.tcl` beside itself, so you can launch it
  from any directory (or by full path from a cron job or another shell).
- The window starts **empty**. A gauge is drawn only when a measuring tool
  connects and registers its meter, so an empty window means the server is
  up and waiting — not broken.
- It prints `Connected to client ...` as tools attach.
- A measuring tool connects only when you give it `--meter-host`, so **start
  the server first** in that case.

## Shared options (measuring tools)

`pipeperf`, `sockperf`, `rdoperf`, and `fragsrcperf` share these control
options; each tool's own subsection below adds the options that name *what*
to measure.

| shared option | meaning | default |
|---------------|---------|---------|
| `--nsamples=N` | number of measurement passes; the test runs exactly this many, then writes a summary and exits | `1000` |
| `--meter-host=HOST` | meter server host; **omit to disable the meter GUI** | *(none → no GUI)* |
| `--meter-port=PORT` | meter server port | `2000` |
| `--logfile=PATH` | append the run summary to this file (also echoed to stderr) | *(stderr only)* |
| `--bufsize=BYTES` | largest single read request (`pipeperf`, `sockperf`); lower it to force small reads (syscall-bound test) | pipeperf `4194304` (4 MiB), sockperf `8388608` (8 MiB) |
| `--verbose` | print each pass's measurement (off by default) | *off* |

That gives three run modes: **metered** (`--meter-host=HOST`, server
running), **metered + log** (add `--logfile=PATH`), and **log-only** (omit
`--meter-host` — no GUI or X display needed).

### The summary block

The headline rate is the aggregate over the whole test (total bytes ÷ total
measured time), **not** the mean of the per-pass rates; per-pass mean,
min/max, and standard deviation (sample, n-1, with its coefficient of
variation) are included for context. The standard deviation needs at least
two samples, so with `--nsamples=1` it reads `n/a`.

```
==== sockperf summary 2026-11-05 10:51:00 ====
  samples          : 1000
  wall elapsed (s) : 24.213
  total bytes      : 104857600000
  aggregate        : 4.438e+09 bytes/sec (4232.00 MB/s)
  per-pass mean    : 4.440e+09 bytes/sec
  per-pass min/max : 4.100e+09 / 4.600e+09 bytes/sec
  per-pass stdev   : 1.200e+08 bytes/sec (2.70 % CV)
```

## Measuring tools

### `pipeperf` — Unix pipe throughput (meter `pipe`)

Reads `--blocksize` bytes from stdin per pass and times it. With the
defaults (4 MiB block × 1000 passes) a bare run is a steady-state test. If
the sender closes the pipe early, pipeperf stops and writes the summary over
the samples collected so far.

Options (plus the shared options above): `--blocksize=BYTES` (default 4 MiB),
`--bufsize=BYTES` (default 4 MiB). A Unix pipe hands back at most ~64 KiB per
`read()`, so a `--bufsize` above that is inert here; lower it below ~64 KiB
to force small reads. `--pipe-size=BYTES` resizes the stdin pipe via
`F_SETPIPE_SZ` (kernel rounds up to a power of two, capped at
`/proc/sys/fs/pipe-max-size`, default 1 MiB) — raising it moves the read-size
knee.

```
# log-only, defaults (steady-state)
$DAQBIN/perftools/pipesend | $DAQBIN/perftools/pipeperf --logfile=pipe.log
# metered (meterserver on localhost:2000)
$DAQBIN/perftools/pipesend | $DAQBIN/perftools/pipeperf --meter-host=localhost
```

### `sockperf` — TCP socket throughput (meter `socket`)

Listens on `--port`, accepts one connection from the sender (`socksend`),
and times reads from it. Defaults: 8 MiB block × 1000 passes. If the sender
disconnects mid-run it stops and summarizes what it got. It's a **two-process
test** — start the receiver, then the sender.

Options (plus the shared options above): `--port=PORT` *(required)*;
`--blocksize=BYTES` (default 8 MiB); `--bufsize=BYTES` (default 8 MiB) caps
each `read()`; `--rcvbuf=BYTES` raises `SO_RCVBUF` to read larger chunks
(kernel-capped at `net.core.rmem_max`). A socket read is ultimately bounded
by `SO_RCVBUF`, which TCP autotunes up to `net.ipv4.tcp_rmem`'s max (~6 MiB);
the 8 MiB defaults keep the *request* from being the limiter.

```
# receiver (log-only; add --meter-host=localhost to meter it)
$DAQBIN/perftools/sockperf --port=9000 --logfile=sock.log
# sender, in another shell
$DAQBIN/perftools/socksend --host=localhost --port=9000
```

### `rdoperf` — ring buffer input rate (meter: the ring name)

Samples the named ring `--nsamples` times, `--interval-ms` apart: empty the
ring, sleep, then measure what accumulated. Requires the realtime clock to
have at least 10× the resolution of the sampling interval.

Options (plus the shared options above): `--ring=NAME` *(required)*;
`--interval-ms=MS` (default `100`). No `--bufsize` (it reads byte counts, not
data).

```
# log-only
$DAQBIN/perftools/rdoperf --ring=myring --logfile=rdo.log
# metered
$DAQBIN/perftools/rdoperf --ring=myring --meter-host=localhost
```

### `fragsrcperf` — ring fragment source throughput (meter `ringfragsrc`)

Poses as an event builder: listens on `--port`, accepts a ring fragment
source, acknowledges each block with `OK\n`, and times the body bytes/sec
over `--blocks` blocks per pass. Point a ring fragment source
(`ringFragmentSource`) at this port.

Options (plus the shared options above): `--port=PORT` *(required)*;
`--blocks=N` *(required, > 1)* — received blocks per measurement. No
`--bufsize` (each block is read into a per-block allocation).

```
$DAQBIN/perftools/fragsrcperf --port=9000 --blocks=100 --logfile=frag.log
```

### `bufferedoutperf` — buffered file-write throughput (no meter)

Writes `--mbytes` megabytes to `--filename` through `io::CBufferedOutput`,
repeated for `--nsamples` timed passes (each pass truncates and rewrites the
file), and writes one run summary to `--logfile` (same StatsLog format as the
other tools — aggregate plus per-pass mean ± σ over the passes; no output
without it). Does not use the meter server.

| option | meaning | default |
|--------|---------|---------|
| `--buffersize=K` | output buffer size in Kbytes | `8192` |
| `--writesize=BYTES` | size of each write | `8192` |
| `--nsamples=N` | number of timed passes; per-pass mean ± σ computed over them | `1` |
| `--mbytes=M` | total Mbytes to write *(required)* | — |
| `--filename=FILE` | output file *(required)* | — |
| `--fsync` | fsync to disk before stopping the timer (durable-write rate, not page cache) | *off* |
| `--logfile=PATH` | append the run summary to this file (no output without it) | — |

```
$DAQBIN/perftools/bufferedoutperf --mbytes=1024 --filename=./out.dat --logfile=bop.log
```

Without `--fsync` the data only reaches the **page cache**, so (for `--mbytes`
below RAM) the rate measures the buffering + page-cache path, not the storage
media — use `--fsync` and/or write more than RAM for the real device rate. Point
`--filename` at the target storage: a file under `/tmp` is often **tmpfs (RAM)**,
which measures memory, not disk.

## Data generators

Sources to feed the tools above, or other DAQ stages. Each writes to stdout
(except `socksend`, which connects out over TCP) and runs until its consumer
goes away. `--bufsize` sets the in-memory block each one fills and writes;
the default matches the tool's historical fixed size.

### `pipesend` — byte stream to stdout (feeds `pipeperf`)

Writes zero-filled `--bufsize`-byte blocks to stdout forever.

Options: `--bufsize=BYTES` (default `104857600`, 100 MB).

```
$DAQBIN/perftools/pipesend | $DAQBIN/perftools/pipeperf --logfile=pipe.log
```

### `socksend` — byte stream over TCP (feeds `sockperf`)

Connects to `--host`/`--port` and streams `--bufsize`-byte writes until the
consumer closes the connection (e.g. `sockperf` finishing its passes), which
it reports and treats as a clean exit (`0`); setup failures exit non-zero.

Options: `--host=HOST` *(required)*, `--port=PORT` *(required)*,
`--bufsize=BYTES` (default 100 MB), `--sndbuf=BYTES` sets `SO_SNDBUF` (pairs
with sockperf's `--rcvbuf` for tuning large transfers over a real link).

```
$DAQBIN/perftools/socksend --host=localhost --port=9000
```

### `ritemMaker` — ring-item stream to stdout

Endless `PHYSICS_EVENT` ring items with body headers and incrementing
timestamps.

Options: `--datasize=N` *(required)* payload bytes per item; `--sid=N`
(default `0`) source id in the body header; `--tsinc=N` (default `1`)
timestamp increment between events; `--bufsize=BYTES` (default `1048576`,
1 MB) buffer filled and written per pass.

```
$DAQBIN/perftools/ritemMaker --datasize=100 | some-consumer
```

### `runmaker` — a complete run to stdout

A `BEGIN_RUN`, `--mbytes` megabytes of event data, then an `END_RUN`.

Options: the same item options as `ritemMaker` (`--datasize` *(required)*,
`--sid`, `--tsinc`, `--bufsize`) plus `--mbytes=M` *(required)*, the Mbytes
of event data to generate.

```
$DAQBIN/perftools/runmaker --datasize=100 --mbytes=1024 | some-consumer
```

### `fragmaker` — event-builder fragments to stdout (for glom)

A stream of event-builder fragments (fragment header + ring item) to
performance-test glom.

Options: `--payload-size=N` *(required, ≥ 8)* bytes per ring item;
`--tsdelta=N` *(required)* timestamp tick between fragments;
`--bufsize=BYTES` (default `1048576`, 1 MB) data block size.

```
$DAQBIN/perftools/fragmaker --payload-size=100 --tsdelta=1 | glom ...
```

## File / correctness checkers (diagnostic, not performance)

### `checkevfiles` — `PHYSICS_EVENT` timestamp check

Reads the `run-NNNN-SS.evt` segments in the current directory via the
data-source factory and checks that each `PHYSICS_EVENT` body-header
timestamp increments by one; mismatches are reported to stderr.

Options: `--run=NNNN` *(required)* — the run number (segments must be in the
cwd).

```
$DAQBIN/perftools/checkevfiles --run=123
```

### `evbfilecheck` — event-builder output check

Checks event-builder output under strong assumptions (two sources, timestamp
+1 per event, glom `dt = 1`, all sources in coincidence except at end of
run); reports mismatches and dumps the offending events to `badItems.txt`.

Options: `--source=URI` *(required)* — the `file:` or `tcp:` URI to read.

```
$DAQBIN/perftools/evbfilecheck --source=file://./run-0123-00.evt
```

## Test / helper

### `metertest` — meter-server smoke test

Connects to `localhost:2000`, registers a meter named `test`, and
random-walks it. No options. Use it to confirm the meter server and GUI work
before wiring up real data paths.

```
# terminal 1
$DAQBIN/perftools/meterserver.tcl 2000
# terminal 2
$DAQBIN/perftools/metertest
```
