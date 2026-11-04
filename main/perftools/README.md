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

The measuring tools take `meter-host meter-port` arguments pointing at a
running meter server, so **start the server first**.

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

## Logging test results (averages over a run)

Each measuring tool (`pipeperf`, `sockperf`, `rdoperf`, `fragsrcperf`)
accepts two optional trailing arguments after `meter-port`:

```
... meter-host meter-port [seconds] [logfile]
```

- `seconds` — run for this many seconds, then write a summary and exit.
  Omitted or `0` keeps the run-forever behavior (no summary).
- `logfile` — append the summary block to this file (also echoed to
  stderr). Omitted means stderr only.
- Pass `-` as `meter-host` to skip the meter GUI entirely, so you can log a
  test with no `meterserver` / X display running. (`meter-port` is still
  given positionally but ignored.)

The headline rate in the summary is the aggregate over the whole test
(total bytes ÷ total measured time), **not** the mean of the per-pass
rates; per-pass mean/min/max are included for context. Example — a
30-second socket test with no GUI, appended to `sock.log`:

```
# terminal 1 — sender
$DAQBIN/perftools/socksend <sockperf-host> 5000
# terminal 2 — measure for 30 s, log-only, append summary to sock.log
$DAQBIN/perftools/sockperf 104857600 5000 - 0 30 /path/to/sock.log
```

A summary block looks like:

```
==== sockperf summary 2026-11-04 10:51:00 ====
  samples          : 1240
  wall elapsed (s) : 30.001
  total bytes      : 133143986176
  aggregate        : 4.438e+09 bytes/sec (4232.00 MB/s)
  per-pass mean    : 4.440e+09 bytes/sec
  per-pass min/max : 4.100e+09 / 4.600e+09 bytes/sec
```

## Measuring tools (meter clients)

Each connects to the meter server and reports bytes/sec under the meter
name shown.

### `pipeperf` — Unix pipe throughput (meter `pipe`)

Reads `nbytes` from stdin and times it.

```
pipesend | pipeperf nbytes meter-host meter-port [seconds] [logfile]
```

| arg | meaning |
|-----|---------|
| `nbytes` | transfer size timed per pass |

Example:

```
# terminal 1
$DAQBIN/perftools/meterserver.tcl 2000
# terminal 2
$DAQBIN/perftools/pipesend | $DAQBIN/perftools/pipeperf 104857600 localhost 2000
```

### `sockperf` — TCP socket throughput (meter `socket`)

Listens on `my-port`, accepts one connection, and times reads from it.

```
sockperf nbytes my-port meter-host meter-port [seconds] [logfile]
```

Feed it with `socksend` (see Generators):

```
# terminal 1
$DAQBIN/perftools/meterserver.tcl 2000
# terminal 2
$DAQBIN/perftools/sockperf 104857600 5000 localhost 2000
# terminal 3
$DAQBIN/perftools/socksend <sockperf-host> 5000
```

### `rdoperf` — rate of data arriving into a ring buffer

The meter is named after the ring. Each sample empties the ring, sleeps
`ms`, then measures what accumulated. It requires the realtime clock to
have at least 10× the resolution of the sampling interval.

```
rdoperf ring ms meter-host meter-port [seconds] [logfile]
```

| arg | meaning |
|-----|---------|
| `ring` | name of the ring buffer to monitor |
| `ms`   | milliseconds between measurements |

Example:

```
$DAQBIN/perftools/meterserver.tcl 2000
$DAQBIN/perftools/rdoperf myring 100 localhost 2000
```

### `fragsrcperf` — ring fragment source throughput (meter `ringfragsrc`)

Poses as an event builder: accepts the connection, acknowledges blocks
with `OK\n`, and times the body bytes/sec.

```
fragsrcperf my-port block-count meter-host meter-port [seconds] [logfile]
```

| arg | meaning |
|-----|---------|
| `my-port` | port the fragment source connects to |
| `block-count` | received blocks per measurement |

## Standalone perf tool (no meter server)

### `bufferedoutperf` — `CBufferedOutput` write-to-file throughput

Prints the result to stdout; does not use a meter.

```
bufferedoutperf buffersize writesize mbytes fname
```

| arg | meaning |
|-----|---------|
| `buffersize` | buffer size in Kbytes |
| `writesize`  | size of each write in bytes |
| `mbytes`     | total Mbytes to write |
| `fname`      | output file |

## Data generators

Sources to feed the tools above, or other DAQ stages.

| tool | produces | usage |
|------|----------|-------|
| `pipesend` | 100 MB zero-filled blocks to stdout forever (feeds `pipeperf`) | `pipesend` (no args) |
| `socksend` | connects to host/port and streams (feeds `sockperf`) | `socksend host port` |
| `ritemMaker` | endless `PHYSICS_EVENT` ring items with body headers and incrementing timestamps, to stdout | `ritemMaker datasize sid tsinc` |
| `runmaker` | a complete run: `BEGIN_RUN`, a fixed amount of event data, then `END_RUN` | `runmaker datasize mbytes sid tsinc` |
| `fragmaker` | event-builder fragments (fragment header + ring item) to stdout, to performance-test glom | `fragmaker payload-size tstamp-delta` |

Argument notes:

- `ritemMaker` / `runmaker`: `datasize` = ring item payload size, `sid` =
  source id for the body header, `tsinc` = timestamp increment between
  events; `runmaker` adds `mbytes` = Mbytes of event data to generate.
- `fragmaker`: `payload-size` = bytes per ring item (≥ 8),
  `tstamp-delta` = timestamp tick between fragments.

## File / correctness checkers (diagnostic, not performance)

| tool | purpose |
|------|---------|
| `checkevfiles` | reads ring items from `run-NNNN-SS.evt` files (or a ring) via the data-source factory and walks them; a cookbook for reading ring-item sources |
| `evbfilecheck` | checks event-builder output files under strong assumptions (two sources, timestamp +1 per event, glom dt = 1, all sources in coincidence); reports and dumps events that break the pattern |

Run either with `--help` for its exact arguments.
