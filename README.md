# cepoch

<p align="center">
  <img src="logo/cepoch_logo.png" alt="cepoch logo" width="320" />
</p>

A lightweight C command-line tool for querying Cardano blockchain epoch and slot information — completely offline, no node, no API, no dependencies.

## Features

- Show the current epoch, absolute slot, epoch-slot, and time remaining
- Query by epoch number, absolute slot, epoch-slot, or ISO-8601 UTC timestamp
- Human-readable or JSON output
- No external dependencies — all calculations are done offline using hardcoded mainnet parameters
- Covers both the Byron era (epochs 0–207) and the Shelley+ era (epoch 208+)
- Runs on Linux, macOS, and any POSIX-compatible system

## Quick Start

```sh
./configure
make
make install        # installs to /usr/local/bin
```

Then simply run:

```sh
cepoch
```

## Usage

```
Usage: cepoch [OPTION]

  (no options)                     show current Cardano epoch info
  --epoch N                        info for epoch N
  --slot N                         info for absolute slot N
  --epoch-slot N                   info for epoch-slot N in the current epoch
  --date YYYY-MM-DDTHH:MM:SSZ      info for a UTC date/time
  --output json                    output result as JSON
  --help                           show this help
```

### Examples

```sh
# Current epoch
cepoch

# Info for a specific epoch
cepoch --epoch 500

# Info for an absolute slot
cepoch --slot 123456789

# Info for a specific epoch-slot within the current epoch
cepoch --epoch-slot 200000

# Info for a UTC date
cepoch --date 2024-01-15T12:00:00Z

# JSON output (combinable with any option)
cepoch --output json
cepoch --epoch 500 --output json
```

### Sample Output

```
Epoch:         523
Absolute Slot: 127814400
Epoch Slot:    259200 / 432000
Time left:     2d 18h 0m 0s
Epoch range:   2024-09-27 21:44:51 UTC - 2024-10-02 21:44:51 UTC
Local time:    2024-09-29 21:44:51
UTC time:      2024-09-29 21:44:51 UTC
```

```json
{
  "epoch": 523,
  "absolute_slot": 127814400,
  "epoch_slot": 259200,
  "epoch_slots_total": 432000,
  "time_left_seconds": 172800,
  "time_left": "2d 0h 0m 0s",
  "epoch_start_utc": "2024-09-27 21:44:51",
  "epoch_end_utc": "2024-10-02 21:44:51",
  "epoch_start_unix": 1727473491,
  "epoch_end_unix": 1727905491,
  "local_time": "2024-09-29 21:44:51",
  "utc_time": "2024-09-29 21:44:51"
}
```

## Build Options

| Option | Description |
|---|---|
| `--prefix=PATH` | Installation prefix (default: `/usr/local`) |
| `--bindir=PATH` | Override binary directory |
| `--enable-debug` | Debug build: `-g -O0 -DDEBUG` |
| `--disable-debug` | Release build: `-O2 -DNDEBUG` (default) |

```sh
# Debug build
./configure --enable-debug
make

# Install to a custom prefix
./configure --prefix=/opt/cepoch
make
make install
```

## Running the Tests

```sh
make check
```

## Cardano Mainnet Parameters

| Era | Start | Slot length | Epoch length |
|---|---|---|---|
| Byron | 2017-09-23 21:44:51 UTC | 20 s | 21 600 slots (~5 days) |
| Shelley+ | 2020-07-29 21:44:51 UTC | 1 s | 432 000 slots (~5 days) |

All calculations are performed offline against these hardcoded parameters — no running Cardano node or internet connection is required.

## Requirements

- GCC or Clang
- `make`
- A POSIX shell (`sh`)

## License

MIT — see [LICENSE](LICENSE) for details.

Copyright (c) 2026 Neal Allen Morrison &lt;nmorrison@magister-technicus.de&gt;, Magister Technicus GmbH

---

## Support This Project

If you find `cepoch` useful, please consider delegating your ADA to:

**Hamster Stake Pool — [HAMDA]**

Your delegation supports ongoing development and helps secure the Cardano network. I would be very grateful for every delegation! Thank you.
