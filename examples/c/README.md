# C example — dynamic linking with unir_c (C ABI)

`unir_c` is the same library behind a flat C interface (`unir/unir_c.h`). The compiler runtime is linked into it
statically, so it works with MSVC, Clang, GCC and any language that can call C functions.

## Linux

```sh
export PKG_CONFIG_PATH=/path/to/unir-1.2.5-linux-x64/lib/pkgconfig
cc main.c -o test_unir_c $(pkg-config --cflags --libs unir_c) -Wl,-rpath,'$ORIGIN'
cp /path/to/unir-1.2.5-linux-x64/lib/libunir_c.so .
./test_unir_c label.jpg
```

## Windows, MinGW-w64 GCC

```sh
export PKG_CONFIG_PATH=/c/path/to/unir-1.2.5-windows-x64/lib/pkgconfig
gcc main.c -o test_unir_c.exe $(pkg-config --cflags --libs unir_c)
cp /c/path/to/unir-1.2.5-windows-x64/bin/unir_c.dll .
```

## Windows, MSVC

The package has no `.lib` file; generate the import library from the DLL. In an MSYS2 shell:

```sh
gendef unir_c.dll          # package mingw-w64-x86_64-tools; writes unir_c.def
```

Then in the Developer Command Prompt for VS (x64):

```bat
lib /def:unir_c.def /machine:x64 /out:unir_c.lib
cl /I C:\path\to\unir-1.2.5-windows-x64\include main.c unir_c.lib
```

At run time `unir_c.dll` must be next to the `.exe` (or on `PATH`); after purchase the key file `unir.key`
goes next to `unir_c.dll` / `libunir_c.so`.

Full C ABI reference: <https://libscanner.com/en/docs/universal-reader/#install-dynamic>
