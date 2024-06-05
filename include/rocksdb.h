#ifndef ROCKSDB_H
#define ROCKSDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

typedef struct rocksdb_options_s rocksdb_options_t;
typedef struct rocksdb_open_s rocksdb_open_t;
typedef struct rocksdb_close_s rocksdb_close_t;
typedef struct rocksdb_slice_s rocksdb_slice_t;
typedef struct rocksdb_range_s rocksdb_range_t;
typedef struct rocksdb_delete_range_s rocksdb_delete_range_t;
typedef struct rocksdb_iterator_s rocksdb_iterator_t;
typedef struct rocksdb_batch_s rocksdb_batch_t;
typedef struct rocksdb_s rocksdb_t;

typedef void (*rocksdb_open_cb)(rocksdb_open_t *req, int status);
typedef void (*rocksdb_close_cb)(rocksdb_close_t *req, int status);
typedef void (*rocksdb_delete_range_cb)(rocksdb_delete_range_t *req, int status);
typedef void (*rocksdb_iterator_cb)(rocksdb_iterator_t *iterator, int status);
typedef void (*rocksdb_batch_cb)(rocksdb_batch_t *batch, int status);

typedef enum {
  rocksdb_compaction_style_level = 0,
  rocksdb_compaction_style_universal = 1,
  rocksdb_compaction_style_fifo = 2,
  rocksdb_compaction_style_none = 3,
} rocksdb_compaction_style_t;

/** @version 0 */
struct rocksdb_options_s {
  int version;

  /** @since 0 */
  bool read_only;

  /** @since 0 */
  bool create_if_missing;

  /** @since 0 */
  int max_background_jobs;

  /** @since 0 */
  uint64_t bytes_per_sync;

  /** @since 0 */
  rocksdb_compaction_style_t compaction_style;

  /** @since 0 */
  bool enable_blob_files;

  /** @since 0 */
  uint64_t min_blob_size;

  /** @since 0 */
  uint64_t blob_file_size;

  /** @since 0 */
  bool enable_blob_garbage_collection;

  /** @since 0 */
  uint64_t table_block_size;

  /** @since 0 */
  bool table_cache_index_and_filter_blocks;

  /** @since 0 */
  uint32_t table_format_version;
};

struct rocksdb_open_s {
  uv_work_t worker;

  rocksdb_t *db;

  rocksdb_options_t options;

  char path[4096 + 1 /* NULL */];

  char *error;

  rocksdb_open_cb cb;

  void *data;
};

struct rocksdb_close_s {
  uv_work_t worker;

  rocksdb_t *db;

  char *error;

  rocksdb_close_cb cb;

  void *data;
};

struct rocksdb_slice_s {
  const char *data;
  size_t len;
};

struct rocksdb_range_s {
  rocksdb_slice_t gt;
  rocksdb_slice_t gte;

  rocksdb_slice_t lt;
  rocksdb_slice_t lte;
};

struct rocksdb_delete_range_s {
  uv_work_t worker;

  rocksdb_t *db;

  rocksdb_range_t range;

  char *error;

  rocksdb_delete_range_cb cb;

  void *data;
};

struct rocksdb_iterator_s {
  uv_work_t worker;

  rocksdb_t *db;

  void *handle; // Opaque iterator pointer

  rocksdb_range_t range;
  bool reverse;

  size_t len;
  size_t capacity;

  rocksdb_slice_t *keys;
  rocksdb_slice_t *values;

  char *error;

  rocksdb_iterator_cb cb;

  void *data;
};

struct rocksdb_batch_s {
  uv_work_t worker;

  rocksdb_t *db;

  size_t len;

  const rocksdb_slice_t *keys;
  rocksdb_slice_t *values;

  union {
    // A single error for the write or delete batch.
    char *error;

    // A list of errors for the read batch.
    char **errors;
  };

  rocksdb_batch_cb cb;

  void *data;
};

struct rocksdb_s {
  uv_loop_t *loop;

  void *handle; // Opaque database pointer
};

int
rocksdb_init (uv_loop_t *loop, rocksdb_t *db);

int
rocksdb_open (rocksdb_t *db, rocksdb_open_t *req, const char *path, const rocksdb_options_t *options, rocksdb_open_cb cb);

int
rocksdb_close (rocksdb_t *db, rocksdb_close_t *req, rocksdb_close_cb cb);

rocksdb_slice_t
rocksdb_slice_init (const char *data, size_t len);

void
rocksdb_slice_destroy (rocksdb_slice_t *slice);

rocksdb_slice_t
rocksdb_slice_empty (void);

int
rocksdb_delete_range (rocksdb_t *db, rocksdb_delete_range_t *req, rocksdb_range_t range, rocksdb_delete_range_cb cb);

int
rocksdb_iterator_init (rocksdb_t *db, rocksdb_iterator_t *iterator);

int
rocksdb_iterator_open (rocksdb_iterator_t *iterator, rocksdb_range_t range, bool reverse, rocksdb_iterator_cb cb);

int
rocksdb_iterator_close (rocksdb_iterator_t *iterator, rocksdb_iterator_cb cb);

int
rocksdb_iterator_refresh (rocksdb_iterator_t *iterator, rocksdb_range_t range, bool reverse, rocksdb_iterator_cb cb);

int
rocksdb_iterator_read (rocksdb_iterator_t *iterator, rocksdb_slice_t *keys, rocksdb_slice_t *values, size_t capacity, rocksdb_iterator_cb cb);

int
rocksdb_batch_init (rocksdb_t *db, rocksdb_batch_t *batch);

int
rocksdb_batch_read (rocksdb_batch_t *batch, const rocksdb_slice_t *keys, rocksdb_slice_t *values, char **errors, size_t len, rocksdb_batch_cb cb);

int
rocksdb_batch_write (rocksdb_batch_t *batch, const rocksdb_slice_t *keys, const rocksdb_slice_t *values, size_t len, rocksdb_batch_cb cb);

int
rocksdb_batch_delete (rocksdb_batch_t *batch, const rocksdb_slice_t *keys, const rocksdb_slice_t *values, size_t len, rocksdb_batch_cb cb);

#ifdef __cplusplus
}
#endif

#endif // ROCKSDB_H
