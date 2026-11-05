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

## How monitoring works

Most measuring tools display nothing useful themselves. They push a
bytes/sec number to a **meter server** over a socket. The meter server
(`meterserver.tcl`) is a Tk GUI that draws one needle gauge per named
meter.

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
- Each measuring tool also prints its measurements to stderr, so you get
  numbers even without the GUI.

A measuring tool talks to the meter server only when you give it
`--meter-host`, so **start the server first** in that case. Omit
`--meter-host` and the tool skips the GUI entirely (see *log-only* below).

### Quick smoke test (no data pipeline needed)

```
# terminal 1
$DAQBIN/perftools/meterserver.tcl 2000
# terminal 2
$DAQBIN/perftools/metertest
```

`metertest` connects to `localhost:2000`, creates a meter named `test`,
and random-walks it. Use it to confirm the server and GUI work before
wiring up real data paths.

## Running the measuring tools

The four measuring tools — `pipeperf`, `sockperf`, `rdoperf`,
`fragsrcperf` — share the same control options and differ only in the
required options that name what to measure:

| shared option | meaning | default |
|---------------|---------|---------|
| `--nsamples=N` | number of measurement passes; the test runs exactly this many then writes a summary and exits | `1000` |
| `--meter-host=HOST` | meter server host; **omit to disable the meter GUI** | *(none → no GUI)* |
| `--meter-port=PORT` | meter server port | `2000` |
| `--logfile=PATH` | append the run summary to this file (also echoed to stderr) | *(stderr only)* |
| `--bufsize=BYTES` | largest single read request (`pipeperf`, `sockperf`); lower it to force small reads (syscall-bound test) | pipeperf `4194304` (4 MiB), sockperf `8388608` (8 MiB) |
| `--verbose` | print each pass's measurement (off by default) | *off* |

Tool-specific options (required unless a default is noted):

| tool | options | meter name |
|------|---------|------------|
| `pipeperf` | `--blocksize=BYTES` read per pass — *optional, default 4 MiB (steady-state)* | `pipe` |
| `sockperf` | `--port=PORT` (required); `--blocksize`/`--bufsize` default 8 MiB; `--rcvbuf=BYTES` sets SO_RCVBUF | `socket` |
| `rdoperf` | `--ring=NAME` (plus `--interval-ms=MS`, default `100`) | *(the ring name)* |
| `fragsrcperf` | `--port=PORT` (listen), `--blocks=N` (>1, per measurement) | `ringfragsrc` |

Because the control options are shared, one example covers all three
run modes; swap `sockperf` and its required options for any other tool:

```
# sender for the socket test (any mode)
$DAQBIN/perftools/socksend --host=<sockperf-host> --port=5000

# 1) metered — needs meterserver running on localhost:2000
$DAQBIN/perftools/sockperf --port=5000 --blocksize=104857600 --meter-host=localhost

# 2) metered + log — same, plus a summary appended to sock.log
$DAQBIN/perftools/sockperf --port=5000 --blocksize=104857600 \
    --meter-host=localhost --logfile=sock.log

# 3) log-only — no --meter-host, so no GUI / X display needed
$DAQBIN/perftools/sockperf --port=5000 --blocksize=104857600 --logfile=sock.log
```

Shorten or lengthen any run with `--nsamples` (e.g. `--nsamples=5000`).

The headline rate in the summary is the aggregate over the whole test
(total bytes ÷ total measured time), **not** the mean of the per-pass
rates; per-pass mean, min/max, and standard deviation (sample, n-1, with
its coefficient of variation) are included for context. The standard
deviation needs at least two samples, so with `--nsamples=1` it reads
`n/a`. A summary block looks like:

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

### Per-tool notes

- **`pipeperf`** — Unix pipe throughput. Reads `--blocksize` bytes from
  stdin per pass; with the defaults (4 MiB block, 1000 passes) a bare run
  is a steady-state test. If the sender closes the pipe early, pipeperf
  stops and writes the summary over the samples collected so far. Feed it
  with `pipesend`:

  ```
  # defaults: steady-state, log-only
  $DAQBIN/perftools/pipesend | $DAQBIN/perftools/pipeperf --logfile=run.log
  # metered (meterserver on localhost:2000)
  $DAQBIN/perftools/pipesend | \
      $DAQBIN/perftools/pipeperf --meter-host=localhost
  ```

- **`sockperf`** — TCP socket throughput. Listens on `--port`, accepts one
  connection, times reads from it; with the defaults (8 MiB block) only
  `--port` is required. A socket read is ultimately capped by the kernel
  receive buffer (SO_RCVBUF, which TCP autotunes up to ~6 MiB by default); the
  8 MiB `--blocksize`/`--bufsize` defaults keep the *request* from being the
  limiter, and `--rcvbuf=BYTES` raises SO_RCVBUF to read even larger chunks
  (kernel-capped at `net.core.rmem_max`). Lower `--bufsize` to test small-read
  overhead. If the sender disconnects mid-run it stops and summarizes what it
  got. Feed it with `socksend` (above).

- **`rdoperf`** — rate of data arriving into a ring buffer. The meter is
  named after the ring. Each sample empties the ring, sleeps
  `--interval-ms`, then measures what accumulated; it requires the realtime
  clock to have at least 10× the resolution of the sampling interval.

  ```
  $DAQBIN/perftools/rdoperf --ring=myring --interval-ms=100 --meter-host=localhost
  ```

- **`fragsrcperf`** — ring fragment source throughput. Poses as an event
  builder: accepts the connection, acknowledges blocks with `OK\n`, and
  times the body bytes/sec over `--blocks` blocks per measurement.

## Standalone perf tool (no meter server)

### `bufferedoutperf` — `CBufferedOutput` write-to-file throughput

Prints the result to stdout; does not use a meter.

```
$DAQBIN/perftools/bufferedoutperf --mbytes=1024 --filename=/path/out.dat
```

| option | meaning | default |
|--------|---------|---------|
| `--buffersize=K` | buffer size in Kbytes | `8192` |
| `--writesize=BYTES` | size of each write | `8192` |
| `--mbytes=M` | total Mbytes to write *(required)* | — |
| `--filename=FILE` | output file *(required)* | — |

## Data generators

Sources to feed the tools above, or other DAQ stages. Each writes to
stdout (except `socksend`, which connects out over TCP).

| tool | produces | usage |
|------|----------|-------|
| `pipesend` | zero-filled blocks to stdout forever (feeds `pipeperf`) | `pipesend [--bufsize=104857600]` |
| `socksend` | connects to a host/port and streams (feeds `sockperf`) | `socksend --host=HOST --port=PORT [--bufsize=104857600] [--sndbuf=BYTES]` |
| `ritemMaker` | endless `PHYSICS_EVENT` ring items with body headers and incrementing timestamps | `ritemMaker --datasize=N [--sid=0] [--tsinc=1] [--bufsize=1048576]` |
| `runmaker` | a complete run: `BEGIN_RUN`, a fixed amount of event data, then `END_RUN` | `runmaker --datasize=N --mbytes=M [--sid=0] [--tsinc=1] [--bufsize=1048576]` |
| `fragmaker` | event-builder fragments (fragment header + ring item), to performance-test glom | `fragmaker --payload-size=N --tsdelta=T [--bufsize=1048576]` |

Argument notes:

- `--datasize` = ring item payload size, `--sid` = source id for the body
  header, `--tsinc` = timestamp increment between events; `runmaker` adds
  `--mbytes` = Mbytes of event data to generate.
- `fragmaker`: `--payload-size` = bytes per ring item (≥ 8),
  `--tsdelta` = timestamp tick between fragments.
- `--bufsize` sets the in-memory block each generator fills and writes;
  the default matches each tool's historical fixed size.

## File / correctness checkers (diagnostic, not performance)

| tool | purpose | usage |
|------|---------|-------|
| `checkevfiles` | reads `run-NNNN-SS.evt` segments in the cwd via the data-source factory and checks that `PHYSICS_EVENT` timestamps increment by one | `checkevfiles --run=NNNN` |
| `evbfilecheck` | checks event-builder output under strong assumptions (two sources, timestamp +1 per event, glom dt = 1, all sources in coincidence); reports and dumps events that break the pattern | `evbfilecheck --source=<uri>` |

Run either with `--help` for its exact arguments.
