// SPDX-License-Identifier: MIT-0
/*
 * unir through the flat C ABI (unir_c.dll / libunir_c.so).
 * Works with any C compiler, MSVC included, and is the model for bindings
 * from other languages (C#, Delphi, Go...).
 *
 *   test_unir_c <image>
 */

#include <unir/unir_c.h>

#include <stdio.h>

int main(int argc, char** argv) {
    char version[64];
    unir_settings_t settings = UNIR_SETTINGS_INIT;
    unir_scanner_t* scanner = NULL;
    unir_scan_result_t* result = NULL;
    unir_load_status_t load = UNIR_LOAD_OK;
    int rc = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <image>\n", argv[0]);
        return 2;
    }

    unir_version(version, sizeof version);
    printf("unir %s\n", version);

    /* Unrelated images, every code in the frame, all symbologies. */
    unir_settings_multi(&settings, 0);
    settings.symbologies = UNIR_SYM_ALL;   /* or e.g. UNIR_SYM_DATAMATRIX | UNIR_SYM_GS1_128 */
    scanner = unir_scanner_create(&settings);
    if (!scanner)
        return 1;

    /* Loading and scanning under one frame deadline. The path is UTF-8 or the
     * system code page, like argv. */
    result = unir_scanner_scan_file(scanner, argv[1], &load);
    if (!result)                                     /* NULL only on an internal error */
        goto done;

    switch (unir_scan_result_status(result)) {
        case UNIR_STATUS_OK: {
            int i, n = unir_scan_result_code_count(result);
            for (i = 0; i < n; ++i) {
                /* Text may contain byte 0x1D (GS1 separator): always use the length. */
                const char* text = unir_scan_result_code_text(result, i);
                size_t len = unir_scan_result_code_text_len(result, i);
                uint32_t sym = unir_scan_result_code_symbology(result, i);
                printf("%s: %.*s\n", unir_symbology_name(sym), (int)len, text);
            }
            rc = 0;
            break;
        }
        case UNIR_STATUS_NOT_FOUND:
            printf("no code in the frame\n");
            break;
        case UNIR_STATUS_INVALID:
        case UNIR_STATUS_IMAGE_TOO_LARGE:
            fprintf(stderr, "could not read %s (load status %d)\n", argv[1], (int)load);
            break;
        case UNIR_STATUS_UNLICENSED: {
            char* status = unir_license_status_json();   /* JSON, same as "unir --license status --json" */
            fprintf(stderr, "no trial period and no license: %s\n", status ? status : "");
            if (status)
                unir_free_string(status);
            break;
        }
        default:
            printf("code found but could not be read\n");
            break;
    }

done:
    if (result)
        unir_scan_result_free(result);
    unir_scanner_destroy(scanner);
    return rc;
}
