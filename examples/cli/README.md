# Command-line tool `unir`

The `unir` tool ships in every archive (`bin/unir`, `bin\unir.exe`), in the CLI-only archives (`…-cli.zip` /
`…-cli.tar.gz`) and in the Python wheel (after `pip install` the `unir` command is on the environment's `PATH`;
`python -m unir` works too). Full option list: `unir --help`.

Messages are in the system language: English on any non-Russian system, Russian on a Russian one. `--lang en|ru`
or the `LIBSCANNER_LANG` environment variable set it explicitly. `--json` output and `--batch`/`--folder`/`--serve`
result lines are the same in both languages.

## Usage

```text
unir <image>            [options]
unir --batch  <list>    [options]   a list of paths, one per line
unir --folder <folder>  [options]   all images in a folder (not recursive)
unir --serve            [options]   one path per line from stdin
unir --license status | activate <key> | refresh | request <file> | import <file.lic> | deactivate [<file>] | fingerprint [--json]
```

## Single image

```console
$ unir label.jpg
processing: 288.0 ms
code: datamatrix 010460123450047021CeRKTFwPV1uwP<GS>917D8A<GS>92zucek5HYLlz/77HBtJ+IqIoQsGKPlZgVKiAYNC2DJK3=
```

Useful options:

```sh
unir label.jpg --codes 0            # every code in the frame (0 = as many as found)
unir label.jpg --sym dm,gs1         # only DataMatrix and GS1-128 (SSCC)
unir label.jpg --sym ean            # only EAN-13, EAN-8, UPC-A, UPC-E
unir label.jpg --budget-ms 200      # one time limit per image, shared by all symbologies
unir label.jpg --single             # unrelated images (default: a camera stream)
```

Symbology names for `--sym`: `dm` (`datamatrix`), `128` (`code128`), `gs1` (`gs1-128`, `sscc`), `ean` (all four of
`ean-13`, `ean-8`, `upc-a`, `upc-e`), `all` (default).

## Batch and folder

One process for a set of images known in advance; one line per result on stdout:
`path<TAB>exit code<TAB>milliseconds<TAB>text<TAB>symbology`; a summary line goes to stderr.

```console
$ unir --folder ./frames --codes 0
./frames/label.png	0	269.05	010460123450047021CeRKTFwPV1uwP<GS>917D8A<GS>92zucek5HYLlz/77HBtJ+IqIoQsGKPlZgVKiAYNC2DJK3=	datamatrix
```

```sh
unir --batch list.txt               # one path per line
```

Exit codes: 0 read, 1 not found or the file cannot be opened, 2 found but not read, 3 no license,
4 image exceeds the limit, 5 inconsistent frame, 6 internal error.

## Streaming (`--serve`)

Paths arrive one per line on stdin and are processed immediately; the scanner stays alive between requests.
The lines of each image end with `#END#` and are flushed at once. `path<TAB>sym=LIST` sets the symbologies for
that image only.

```console
$ mkfifo queue
$ unir --serve --codes 0 < queue &
$ printf '/path/to/label.png\tsym=dm,gs1\n' > queue
/path/to/label.png	0	206.44	010460123450047021CeRKTFwPV1uwP<GS>917D8A<GS>92zucek5HYLlz/77HBtJ+IqIoQsGKPlZgVKiAYNC2DJK3=	datamatrix
#END#
```

`--root <directory>` restricts `--serve` to images inside that directory; `--corners` adds the symbol corners as
the sixth column.

## License

```sh
unir --license status                   # trial period / license state
unir --license activate UNIR-XXXXX-XXXXX-XXXXX-XXXXX
unir --license refresh                  # check with the server now
unir --license request request.json     # offline activation: upload in your account, get license.lic
unir --license import license.lic       # accept a license file (also renewals)
unir --license deactivate               # release this machine before moving the license
unir --license fingerprint              # machine fingerprint, for support requests
unir --license status --json            # one JSON line, for scripts
```

```console
$ unir --license status --json
{"state":"trial","can_scan":true,"valid_until":1796178469,"refresh_after":0,"offline_until":0,"days_left":30,"license_id":"","activation_id":"","offline_mode":"","clock_rollback":false,"message":"Trial period: 30 days left"}
```

`state` is a stable machine-readable code: `trial`, `active`, `refresh_due`, `trial_expired`, `expired`,
`network_required`, `wrong_machine`, `invalid`.

The key does not have to be activated by hand: a text file `unir.key` with the key, placed next to `unir`
(or next to the C ABI library / your program), is activated by the library on the first run.

Documentation: <https://libscanner.com/en/docs/universal-reader/#cli>
