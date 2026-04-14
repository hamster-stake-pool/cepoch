# cepoch

<p align="center">
  <img src="logo/cepoch_logo.png" alt="cepoch logo" width="320" />
</p>

A lightweight C command-line tool for querying Cardano blockchain epoch and slot information — completely offline, no node, no API, no dependencies.

## Features

- Show the current epoch, absolute slot, epoch-slot, and time remaining
- Query by epoch number, absolute slot, epoch-slot, or ISO-8601 UTC timestamp
- Human-readable or JSON output
- Supports **mainnet**, **Pre-Production testnet** (`--testnet-magic 1`), and **Preview testnet** (`--testnet-magic 2`)
- No external dependencies — all calculations are done offline against built-in network parameters
- Covers both the Byron era and the Shelley+ era for each network
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
Usage: cepoch [NETWORK] [OPTION]

Network selection (default: --mainnet):
  --mainnet                  Cardano mainnet (default)
  --testnet-magic 1          Pre-Production testnet
  --testnet-magic 2          Preview testnet

Query options:
  (no query option)          show current epoch info
  --epoch N                  info for epoch N
  --slot N                   info for absolute slot N
  --epoch-slot N             info for epoch-slot N in the current epoch
  --date YYYY-MM-DDTHH:MM:SSZ  info for a UTC date/time

Output options:
  --output json              output result as JSON

Miscellaneous:
  --version                  show version information
  --help                     show this help
```

### Examples

```sh
# Current epoch (mainnet, default)
cepoch

# Explicitly select mainnet
cepoch --mainnet

# Pre-Production testnet
cepoch --testnet-magic 1

# Preview testnet
cepoch --testnet-magic 2

# Info for a specific epoch
cepoch --epoch 500

# Info for a specific epoch on the Pre-Production testnet
cepoch --testnet-magic 1 --epoch 100

# Info for an absolute slot
cepoch --slot 123456789

# Info for a specific epoch-slot within the current epoch
cepoch --epoch-slot 200000

# Info for a UTC date
cepoch --date 2024-01-15T12:00:00Z

# JSON output (combinable with any option)
cepoch --output json
cepoch --testnet-magic 1 --epoch 100 --output json

# Version information
cepoch --version
```

### Sample Output

```
Network:       mainnet
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
  "network": "mainnet",
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

## Network Parameters

All calculations are performed offline against built-in parameters — no running Cardano node or internet connection is required.

### Mainnet (`--mainnet`)

| Era | Start | Slot length | Epoch length |
|---|---|---|---|
| Byron (epochs 0–207) | 2017-09-23 21:44:51 UTC | 20 s | 21 600 slots (~5 days) |
| Shelley+ (epoch 208+) | 2020-07-29 21:44:51 UTC | 1 s | 432 000 slots (~5 days) |

### Pre-Production testnet (`--testnet-magic 1`)

| Era | Start | Slot length | Epoch length |
|---|---|---|---|
| Byron (epochs 0–3) | 2022-06-01 00:00:00 UTC | 20 s | 21 600 slots (~5 days) |
| Shelley+ (epoch 4+) | 2022-06-21 00:00:00 UTC | 1 s | 432 000 slots (~5 days) |

### Preview testnet (`--testnet-magic 2`)

| Era | Start | Slot length | Epoch length |
|---|---|---|---|
| Byron (epoch 0) | 2022-10-25 00:00:00 UTC | 20 s | 4 320 slots (~1 day) |
| Shelley+ (epoch 1+) | 2022-10-26 00:00:00 UTC | 1 s | 86 400 slots (1 day) |

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
