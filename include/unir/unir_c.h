#ifndef UNIR_C_H
#define UNIR_C_H

/* ═══════════════════════════════════════════════════════════════════════════
 *  unir/unir_c.h — flat C ABI of the unir library
 *
 *  The boundary for callers other than C++ (C, Python via ctypes, C#, Go and
 *  the like): plain C99, none of the included files pulls in anything from the
 *  include/unir C++ headers. Same design as dmr/dmr_c.h of the dmr reader:
 *  only POD types cross the boundary (integers, double, float, pointers to
 *  buffers of known size, enumerations as int) and opaque handles; a C++
 *  exception never crosses it — inside it is turned into NULL or an error
 *  code.
 *
 *      unir_settings_t settings = UNIR_SETTINGS_INIT;  // struct_size and abi_tag
 *      unir_settings_single(&settings);                // preset; fields may be changed
 *      settings.symbologies = UNIR_SYM_DATAMATRIX | UNIR_SYM_GS1_128;
 *      unir_scanner_t* scanner = unir_scanner_create(&settings);  // NULL — stream()
 *      unir_image_view_t view = { ptr, w, h, stride, UNIR_PIXEL_GRAY8 };
 *      unir_scan_result_t* r = unir_scanner_scan(scanner, &view);
 *      if (r && unir_scan_result_status(r) == UNIR_STATUS_OK) {
 *          for (int i = 0; i < unir_scan_result_code_count(r); ++i) {
 *              uint32_t sym = unir_scan_result_code_symbology(r, i);
 *              size_t len = unir_scan_result_code_text_len(r, i);
 *              const char* text = unir_scan_result_code_text(r, i);
 *              ...                      // unir_symbology_name(sym) — "ean-13"
 *          }
 *      }
 *      unir_scan_result_free(r);
 *      unir_scanner_destroy(scanner);
 *
 *  Memory ownership follows one rule without exceptions: every function that
 *  RETURNS a handle or a string via return hands it over to the caller, and
 *  has a matching unir_..._free() / unir_free_string(). Pointers returned by
 *  accessor functions (unir_scan_result_code_text and the like) live no
 *  longer than the object they were taken from and are not freed separately.
 *  The exception is unir_symbology_name: the string is static and must not be
 *  freed.
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
#  if defined(UNIR_C_BUILDING)
#    define UNIR_C_API __declspec(dllexport)
#  else
#    define UNIR_C_API __declspec(dllimport)
#  endif
#else
#  if defined(UNIR_C_BUILDING)
#    define UNIR_C_API __attribute__((visibility("default")))
#  else
#    define UNIR_C_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Boundary version ───────────────────────────────────────────────────── */
/*
 * Separate from unir_version() (the library version): the ABI changes less
 * often and for other reasons. MAJOR — a compatibility break (a removed
 * function, a reordered struct field); MINOR — additions at the end. New
 * fields go only at the end of existing structures.
 */
#define UNIR_C_ABI_VERSION_MAJOR 1
#define UNIR_C_ABI_VERSION_MINOR 1

/* All three follow the snprintf contract: copy what fits into buf (always
 * NUL-terminated when buf_size > 0) and return the required length without
 * the NUL. */
UNIR_C_API size_t unir_c_abi_version(char* buf, size_t buf_size);   /* "1.1" */
/* Version of the BUILT library — as unir::version() in the C++ API: "1.2.4". */
UNIR_C_API size_t unir_version(char* buf, size_t buf_size);
/* Versions of the engines inside the build: "dmr 1.1.1, ssccr 0.1.0, qrr 0.1.0". */
UNIR_C_API size_t unir_engine_versions(char* buf, size_t buf_size);

/* ── Symbologies ────────────────────────────────────────────────────────── */
/*
 * Each symbology is a separate bit, as unir::Symbology in the C++ API:
 * unir_settings_t::symbologies is a mask of them, a code that was read
 * (unir_scan_result_code_symbology) has exactly one bit.
 */
#define UNIR_SYM_NONE        0x00u
#define UNIR_SYM_DATAMATRIX  0x01u  /* ECC 200, ISO/IEC 16022 */
#define UNIR_SYM_CODE128     0x02u  /* Code 128 without the GS1 flag */
#define UNIR_SYM_GS1_128     0x04u  /* GS1-128 (leading FNC1); without UNIR_SYM_CODE128
                                     * in the mask only SSCC (AI 00) is read */
#define UNIR_SYM_EAN13       0x08u
#define UNIR_SYM_EAN8        0x10u
#define UNIR_SYM_UPCA        0x20u  /* 12 digits */
#define UNIR_SYM_UPCE        0x40u  /* 8 digits, not expanded to UPC-A */
#define UNIR_SYM_EAN_UPC     (UNIR_SYM_EAN13 | UNIR_SYM_EAN8 | UNIR_SYM_UPCA | UNIR_SYM_UPCE)
#define UNIR_SYM_QR          0x80u  /* QR model 2, versions 1–40 */
#define UNIR_SYM_ALL         (UNIR_SYM_DATAMATRIX | UNIR_SYM_CODE128 | UNIR_SYM_GS1_128 | UNIR_SYM_EAN_UPC | UNIR_SYM_QR)

/* Short name of a symbology (exactly one bit): "datamatrix", "code128",
 * "gs1-128", "ean-13", "ean-8", "upc-a", "upc-e", "qr"; 0 — "none"; anything
 * else — "unknown". The string is static — do not free it. */
UNIR_C_API const char* unir_symbology_name(uint32_t symbology);

/* Mask from a comma-separated list of names, like the --sym option of the
 * unir tool: "dm,128,gs1-128", "ean", "all". Accepts datamatrix|dm,
 * code128|128, gs1-128|gs1|sscc, ean-13|ean13, ean-8|ean8, upc-a|upca,
 * upc-e|upce, ean (all four EAN/UPC) and qr. Returns 1 and writes *mask; 0 — an
 * unknown name or NULL, *mask untouched. */
UNIR_C_API int unir_parse_symbologies(const char* list, uint32_t* mask);

/* ── Frame ──────────────────────────────────────────────────────────────── */

typedef enum {
    UNIR_PIXEL_GRAY8 = 0,
    UNIR_PIXEL_BGR8  = 1,
    UNIR_PIXEL_RGB8  = 2,
    UNIR_PIXEL_BGRA8 = 3,
    UNIR_PIXEL_RGBA8 = 4,
} unir_pixel_format_t;

/* A view of SOMEONE ELSE'S buffer: the library does not copy it, free it or
 * outlive it. The buffer must stay alive for the whole unir_scanner_scan()
 * call. Fields and their order match unir::ImageView in the C++ API. */
typedef struct {
    const uint8_t*       data;
    int                  width;
    int                  height;
    int                  stride;    /* 0 — packed: width * channels of the format */
    unir_pixel_format_t  format;
} unir_image_view_t;

/* A frame read from disk — the library owns its buffer. */
typedef struct unir_image unir_image_t;   /* opaque */

typedef enum { UNIR_LOAD_COLOR = 0, UNIR_LOAD_GRAY = 1 } unir_load_as_t;
typedef enum {
    UNIR_LOAD_OK               = 0,
    UNIR_LOAD_NOT_FOUND        = 1,
    UNIR_LOAD_UNREADABLE       = 2,  /* not an image, or damaged */
    UNIR_LOAD_TOO_LARGE        = 3,  /* the size in the header exceeds the limit */
    UNIR_LOAD_TIMED_OUT        = 4,  /* decoding did not finish in time
                                      * (budget_ms / timeout_ms of the settings) */
    UNIR_LOAD_INVALID_ARGUMENT = 5,  /* settings have a foreign tag or struct_size */
} unir_load_status_t;

/* Settings — below; declared in advance for unir_load_image_ex. */
typedef struct unir_settings unir_settings_t;

/*
 * Image from disk: JPEG and PNG (others — UNIR_LOAD_UNREADABLE). path is
 * UTF-8; on Windows a string that is not valid UTF-8 is read in the system
 * code page (ANSI, like argv) — both kinds work. Not Windows — the path bytes
 * as is. The same rule applies to all paths in this header. *out is NULL on
 * any failure; on UNIR_LOAD_OK — a new unir_image_t, freed with
 * unir_image_free(). Format: UNIR_LOAD_GRAY — UNIR_PIXEL_GRAY8 (enough for
 * the scanner), UNIR_LOAD_COLOR — UNIR_PIXEL_BGR8.
 *
 * _ex — with the frame size limit (max_image_side, max_image_pixels) and the
 * decoding deadline (the smaller of budget_ms and timeout_ms) from settings;
 * NULL — defaults (16384 per side, 100 MP, 10 s). unir_load_image() is the
 * same with NULL. A frame above the limit is not decoded: the check uses the
 * file header.
 */
UNIR_C_API unir_load_status_t unir_load_image(const char* path, unir_load_as_t as,
                                              unir_image_t** out);
UNIR_C_API unir_load_status_t unir_load_image_ex(const char* path, unir_load_as_t as,
                                                 const unir_settings_t* settings,
                                                 unir_image_t** out);

UNIR_C_API unir_image_view_t unir_image_view(const unir_image_t* image);
UNIR_C_API void              unir_image_free(unir_image_t* image);

/* ── Settings ───────────────────────────────────────────────────────────── */

/* How DataMatrix is processed (unir::Settings::Mode). */
typedef enum {
    UNIR_MODE_STREAM = 0,  /* frame stream from one camera: a symbol size hint
                            * is kept between frames (default) */
    UNIR_MODE_SINGLE = 1,  /* frames are unrelated */
    UNIR_MODE_MULTI  = 2,  /* several codes per frame */
} unir_mode_t;

/* Full search after the scout network crops (unir::Settings::NetFallback). */
typedef enum {
    UNIR_NET_FALLBACK_UNREAD = 0,  /* only classes the network found but did
                                    * not read, and a frame where it found
                                    * nothing (default) */
    UNIR_NET_FALLBACK_ALWAYS = 1,  /* always: the crops only speed up the
                                    * first read */
} unir_net_fallback_t;

/*
 * Corresponds to unir::Settings in the C++ API — same fields and defaults.
 *
 * SIZE AND TAG. The first field is struct_size: sizeof(unir_settings_t) on
 * the CALLER's side, from the header the caller was built with; the second
 * is abi_tag = UNIR_SETTINGS_TAG. The caller sets both BEFORE the preset,
 * easiest with UNIR_SETTINGS_INIT (C) or unir_settings_prepare (C++):
 *
 *     unir_settings_t s = UNIR_SETTINGS_INIT;
 *     unir_settings_single(&s);
 *
 * A preset writes no further than struct_size, and unir_scanner_create and
 * unir_load_image_ex read only those fields; the rest take the stream()
 * defaults. So a program built with an old header works with a new library
 * that has more fields at the end. A foreign tag, or struct_size below
 * UNIR_SETTINGS_MIN_SIZE or above UNIR_SETTINGS_MAX_SIZE, is rejected: the
 * preset writes nothing and returns 0, unir_scanner_create returns NULL.
 */
struct unir_settings {
    uint32_t struct_size;       /* sizeof(unir_settings_t) on the caller's side */
    uint32_t abi_tag;           /* UNIR_SETTINGS_TAG */
    uint32_t symbologies;       /* mask of UNIR_SYM_*; default — UNIR_SYM_ALL */
    int      max_codes;         /* 1 — up to the first code of any symbology,
                                 * N — up to N, 0 — all */
    int      mode;              /* unir_mode_t */
    double   budget_ms;         /* time limit per frame, ONE for all
                                 * symbologies; 0 — no limit (timeout_ms
                                 * applies) */
    double   timeout_ms;        /* safety cut-off, ms; 0 — the 60 000 ceiling;
                                 * default 10 000 */
    int      max_image_side;    /* larger frame — UNIR_STATUS_IMAGE_TOO_LARGE;
                                 * 0 — the 65535 ceiling; default 16384 */
    int64_t  max_image_pixels;  /* 0 — the 2^29 ceiling; default 100 000 000 */

    /* ── ABI 1.1 (unir 1.1.0) ── */
    const char* net_model;      /* scout network: path to the .unet model (in
                                 * the package — share/unir/net/seg16a.unet),
                                 * UTF-8 or the system encoding, as for
                                 * unir_load_image; read in
                                 * unir_scanner_create, the string can be freed
                                 * right after. NULL or "" — no network */
    float    net_conf;          /* network confidence below which a detection
                                 * is ignored; 0 — default 0.35 */
    int      net_fallback;      /* unir_net_fallback_t */
    int      threads;           /* threads for the network crops with several
                                 * codes; 0 — default 1, -1 — one per core */
};

/* Settings tag of ABI 1.x: bytes 'U' 'N' 'R' '1' (little-endian). */
#define UNIR_SETTINGS_TAG 0x31524E55u

/* Initial value of the settings for C: size and tag, the rest zeros until the
 * preset. */
#define UNIR_SETTINGS_INIT { .struct_size = (uint32_t)sizeof(unir_settings_t), .abi_tag = UNIR_SETTINGS_TAG }

/* Zeroes *s and sets struct_size and abi_tag. Inline in the header: sizeof is
 * taken on the caller's side, from the caller's header. */
static inline void unir_settings_prepare(unir_settings_t* s) {
    memset(s, 0, sizeof *s);
    s->struct_size = (uint32_t)sizeof *s;
    s->abi_tag = UNIR_SETTINGS_TAG;
}

/* The smallest struct_size the library accepts: the size of the ABI 1.0
 * structure. */
#define UNIR_SETTINGS_MIN_SIZE \
    (offsetof(struct unir_settings, max_image_pixels) + sizeof(int64_t))

/* The largest struct_size: room for fields of future versions; larger is garbage. */
#define UNIR_SETTINGS_MAX_SIZE 4096u

/* Fill *out with a preset — no further than out->struct_size bytes. Return 1;
 * 0 — out == NULL, a foreign tag or struct_size out of range, *out untouched.
 * multi: max_codes — how many codes to look for, 0 — all. */
UNIR_C_API int unir_settings_stream(unir_settings_t* out);
UNIR_C_API int unir_settings_single(unir_settings_t* out);
UNIR_C_API int unir_settings_multi(unir_settings_t* out, int max_codes);

/* ── Scanner ────────────────────────────────────────────────────────────── */

typedef struct unir_scanner unir_scanner_t;   /* opaque */

/* The same numbers as unir::Status (C++ API) and the dmr reader. */
typedef enum {
    UNIR_STATUS_OK              = 0,  /* at least one code read */
    UNIR_STATUS_NOT_FOUND       = 1,  /* no symbol found */
    UNIR_STATUS_NOT_DECODED     = 2,  /* a symbol was found but not read */
    UNIR_STATUS_UNLICENSED      = 3,  /* no license and the trial is over;
                                       * the frame was not processed */
    UNIR_STATUS_IMAGE_TOO_LARGE = 4,  /* the frame exceeds max_image_side/max_image_pixels */
    UNIR_STATUS_INVALID         = 5,  /* the frame description is inconsistent: stride
                                       * below width * channels, unknown format;
                                       * for scan_file — the image was not read */
    UNIR_STATUS_FAILED          = 6,  /* processing stopped on an internal error */
    /* Not in unir::Status: an error of the boundary itself (a C++ exception
     * caught on it). */
    UNIR_STATUS_INTERNAL_ERROR  = -1,
} unir_status_t;

/*
 * settings == NULL — unir_settings_stream() (the default, like Scanner() in
 * the C++ API). NULL — an internal error or the settings were rejected (tag,
 * size).
 *
 * The first license check happens here, not on the first frame. Do not call
 * one scanner from several threads at once: in UNIR_MODE_STREAM the size hint
 * changes from frame to frame. Use one scanner per thread.
 */
UNIR_C_API unir_scanner_t* unir_scanner_create(const unir_settings_t* settings);
UNIR_C_API void            unir_scanner_destroy(unir_scanner_t* scanner);

/* The scout network is loaded: 1 — yes; 0 — net_model is not set or the file
 * was not read (why — unir_scanner_net_error; the scanner then works without
 * the network). ABI 1.1. */
UNIR_C_API int    unir_scanner_net_ready(const unir_scanner_t* scanner);
/* Why the network did not load; an empty string — it loaded or is not set.
 * snprintf contract. ABI 1.1. */
UNIR_C_API size_t unir_scanner_net_error(const unir_scanner_t* scanner, char* buf, size_t buf_size);

/* ── Result ─────────────────────────────────────────────────────────────── */

typedef struct unir_scan_result unir_scan_result_t;   /* opaque */

/* Process a frame. The caller owns the result (unir_scan_result_free).
 * NULL — an internal error or scanner/frame == NULL; "no code found" is not
 * NULL but UNIR_STATUS_NOT_FOUND. */
UNIR_C_API unir_scan_result_t* unir_scanner_scan(unir_scanner_t* scanner,
                                                 const unir_image_view_t* frame);

/* Load an image from disk and read its codes under ONE frame deadline
 * (budget_ms, timeout_ms of the scanner; loading counts against it). The
 * image is read in grayscale, the size limit comes from the scanner settings.
 * If it did not load — status UNIR_STATUS_IMAGE_TOO_LARGE or
 * UNIR_STATUS_INVALID, details in *load_status if it is not NULL. Without a
 * license the file is not opened: UNIR_STATUS_UNLICENSED,
 * *load_status == UNIR_LOAD_OK. path — as in unir_load_image. */
UNIR_C_API unir_scan_result_t* unir_scanner_scan_file(unir_scanner_t* scanner, const char* path,
                                                      unir_load_status_t* load_status);

UNIR_C_API void          unir_scan_result_free(unir_scan_result_t* result);
UNIR_C_API unir_status_t unir_scan_result_status(const unir_scan_result_t* result);
/* 1 — at least one engine was cut off by the frame deadline or did not get to start. */
UNIR_C_API int           unir_scan_result_timed_out(const unir_scan_result_t* result);
UNIR_C_API double        unir_scan_result_elapsed_ms(const unir_scan_result_t* result);
/* Codes in reading order: one code — SSCC and EAN/UPC, DataMatrix, other
 * Code 128; several — linear symbologies, then DataMatrix. */
UNIR_C_API int           unir_scan_result_code_count(const unir_scan_result_t* result);

/* Symbology of the code — one UNIR_SYM_* bit; index out of range — UNIR_SYM_NONE. */
UNIR_C_API uint32_t unir_scan_result_code_symbology(const unir_scan_result_t* result, int index);

/* Symbol data. A leading FNC1 (the GS1 flag) is not returned; an FNC1 inside
 * is byte 0x1D (GS) as is. The text may contain byte 0, so the length MUST be
 * taken from unir_scan_result_code_text_len, not strlen(). The pointer lives
 * no longer than result; index out of range — NULL and 0. */
UNIR_C_API const char* unir_scan_result_code_text(const unir_scan_result_t* result, int index);
UNIR_C_API size_t      unir_scan_result_code_text_len(const unir_scan_result_t* result, int index);

/* Box — in pixels of the original frame, axis-aligned. */
UNIR_C_API void unir_scan_result_code_box(const unir_scan_result_t* result, int index,
                                          int* x, int* y, int* width, int* height);
/* corners_xy — eight floats: x0,y0,x1,y1,x2,y2,x3,y3, corners in order around
 * the symbol. DataMatrix: corner 3 is the one the detector took for the corner
 * of the solid L-shaped finder. Code 128, GS1-128, EAN/UPC: corner 0 is on the
 * start side, 1 on the stop side along the same edge of the bars, 2 and 3 on
 * the opposite edge. */
UNIR_C_API void unir_scan_result_code_corners(const unir_scan_result_t* result, int index,
                                              float corners_xy[8]);
/* DataMatrix: modules per side of the symbol (0 — unknown or a linear
 * symbology). */
UNIR_C_API int   unir_scan_result_code_module_count(const unir_scan_result_t* result, int index);
/* Linear symbologies: module width, pixels (0 — unknown or DataMatrix). */
UNIR_C_API float unir_scan_result_code_module_px(const unir_scan_result_t* result, int index);

/* ── License ────────────────────────────────────────────────────────────── */
/*
 * Return the same JSON as "unir --license ... --json" (and the dmr reader):
 *   state     — {"state", "can_scan", "valid_until", "refresh_after",
 *                "offline_until", "days_left", "license_id", "activation_id",
 *                "offline_mode", "clock_rollback", "message"};
 *   operation — {"ok", "error", "message", "status": {state}}.
 * A failed operation (wrong key, no network) is ordinary JSON with "ok": false.
 * Every non-NULL result must be released with unir_free_string() after use.
 * NULL — an internal error. Paths — as in unir_load_image (UTF-8 or ANSI).
 */

/* Configure before the first network call; optional. server_url / app_version
 * — NULL: the build default; timeout_ms <= 0 — 15 000; auto_check — check in
 * with the server by itself (once a day). */
UNIR_C_API void unir_license_configure(const char* server_url, const char* app_version,
                                       int timeout_ms, int auto_check);

UNIR_C_API char* unir_license_status_json(void);
UNIR_C_API char* unir_license_activate_json(const char* key);           /* UNIR-XXXXX-... */
UNIR_C_API char* unir_license_refresh_json(void);
UNIR_C_API char* unir_license_deactivate_json(const char* reason);      /* reason may be NULL */
UNIR_C_API char* unir_license_deactivate_to_file_json(const char* path, const char* reason);
UNIR_C_API char* unir_license_write_activation_request_json(const char* path);
UNIR_C_API char* unir_license_import_license_file_json(const char* path);
UNIR_C_API char* unir_license_fingerprint_json(void);                   /* {"fingerprint": "hex"} */

UNIR_C_API void unir_free_string(char* s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UNIR_C_H */
