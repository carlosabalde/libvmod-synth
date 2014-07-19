#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "vrt.h"
#include "vqueue.h"
#include "bin/varnishd/cache.h"
#include "vcc_if.h"

static unsigned version = 0;

typedef struct synth_file {
    unsigned magic;
#define SYNTH_FILE_MAGIC 0xc93aedf5
    unsigned version;
    unsigned references;
    char *path;
    char *contents;
    unsigned long size;
    VTAILQ_ENTRY(synth_file) list;
} synth_file;

static VTAILQ_HEAD(, synth_file) files = VTAILQ_HEAD_INITIALIZER(files);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void
synth(struct sess *sp, char *contents, unsigned long size);

static synth_file *
get_synth_file(struct vmod_priv *call_priv, const char *path);

static char *
render(char *contents, unsigned long size, const char *placeholders);

int
init_function(struct vmod_priv *vcl_priv, const struct VCL_conf *conf)
{
    // Initialize the local VCL data structure.
    if (vcl_priv->priv == NULL) {
        // Every time the VMOD is loaded increase the global version. This
        // will be used to refresh cached files when the VCL is reloaded.
        AZ(pthread_mutex_lock(&mutex));
        version++;
        AZ(pthread_mutex_unlock(&mutex));

        // Set some VCL dummy state.
        vcl_priv->priv = strdup("synth");
        AN(vcl_priv->priv);
        vcl_priv->free = free;
    }
}

void
vmod_file(
    struct sess *sp, struct vmod_priv *call_priv,
    const char *path)
{
    if (path != NULL) {
        synth_file *file = get_synth_file(call_priv, path);
        if (file != NULL) {
            synth(sp, file->contents, file->size);
        }
    }
}

void
vmod_template(
    struct sess *sp, struct vmod_priv *call_priv,
    const char *path, const char *placeholders)
{
    if ((path != NULL) && (placeholders != NULL)) {
        synth_file *file = get_synth_file(call_priv, path);
        if (file != NULL) {
            char *buffer = render(file->contents, file->size, placeholders);
            synth(sp, buffer, strlen(buffer));
            free(buffer);
        }
    }
}

void
vmod_pixel(struct sess *sp)
{
    synth(
        sp,
        "\x47\x49\x46\x38\x39\x61\x01\x00\x01\x00\x80\xff\x00\xc0\xc0\xc0\x00\x00\x00\x21\xf9\x04\x01\x00\x00\x00\x00\x2c\x00\x00\x00\x00\x01\x00\x01\x00\x00\x02\x02\x44\x01\x00\x3b",
        43);
}

void
vmod_string(struct sess *sp, const char *value)
{
    if (value != NULL) {
        synth(sp, (char *)value, strlen(value));
    }
}

/******************************************************************************
 * UTILITIES.
 *****************************************************************************/

static void
synth(struct sess *sp, char *contents, unsigned long size)
{
    if (sp->step == STP_ERROR) {
        CHECK_OBJ_NOTNULL(sp, SESS_MAGIC);
        CHECK_OBJ_NOTNULL(sp->obj, OBJECT_MAGIC);

        struct vsb *vsb = SMS_Makesynth(sp->obj);
        AN(vsb);
        VSB_bcat(vsb, contents, size);
        SMS_Finish(sp->obj);
        http_Unset(sp->obj->http, H_Content_Length);
        http_PrintfHeader(sp->wrk, sp->fd, sp->obj->http, "Content-Length: %d", sp->obj->len);
    }
}

static void
free_synth_file(void *ptr)
{
    synth_file *file;

    CAST_OBJ_NOTNULL(file, ptr, SYNTH_FILE_MAGIC);

    AZ(pthread_mutex_lock(&mutex));
    file->references--;
    if (file->references > 0) {
        file = NULL;
    } else {
        VTAILQ_REMOVE(&files, file, list);
    }
    AZ(pthread_mutex_unlock(&mutex));

    if (file != NULL) {
        free(file->path);
        free(file->contents);
        FREE_OBJ(file);
    }
}

static synth_file *
get_synth_file(struct vmod_priv *call_priv, const char *path)
{
    // Initializations.
    synth_file *file;
    AN(call_priv);

    // Is the file object already cached in the PRIV_CALL pointer?
    if (call_priv->priv != NULL) {
        CAST_OBJ_NOTNULL(file, call_priv->priv, SYNTH_FILE_MAGIC);
        if (file->version == version) {
            return file;
        } else {
            free_synth_file(file);
            call_priv->priv = NULL;
        }
    }

    // Is the file object already loaded in the global list of cached files?
    AZ(pthread_mutex_lock(&mutex));
    VTAILQ_FOREACH(file, &files, list) {
        if ((file->version == version) &&
            (strcmp(path, file->path) == 0)) {
            file->references++;
            break;
        }
    }
    AZ(pthread_mutex_unlock(&mutex));
    if (file != NULL) {
        call_priv->free = free_synth_file;
        call_priv->priv = file;
        return file;
    }

    // Load the file and add to the global list of cached files (race
    // conditions may result in the same file + version loaded several
    // times).
    FILE *fp = fopen(path, "r");
    if (fp != NULL) {
        // Fetch file size.
        fseek(fp, 0, SEEK_END);
        unsigned long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // Read whole file contents.
        char *fcontents = malloc(fsize);
        AN(fcontents);
        size_t nitems = fread(fcontents, 1, fsize, fp);

        // Close file.
        fclose(fp);

        // Add to the global list of cached files.
        if (nitems == fsize) {
            ALLOC_OBJ(file, SYNTH_FILE_MAGIC);
            AN(file);

            file->version = version;
            file->references = 1;
            file->path = strdup(path);
            AN(file->path);
            file->contents = fcontents;
            file->size = fsize;

            call_priv->free = free_synth_file;
            call_priv->priv = file;

            AZ(pthread_mutex_lock(&mutex));
            VTAILQ_INSERT_HEAD(&files, file, list);
            AZ(pthread_mutex_unlock(&mutex));

            return file;
        }
    }

    // Failure.
    return NULL;
}

static char *
render(char *contents, unsigned long size, const char *placeholders)
{
    char *result;
    int allocated, used;

    // Clone the initial template.
    allocated = used = size + 1;
    result = malloc(allocated);
    AN(result);
    memcpy(result, contents, size);
    result[size] = '\0';

    // Replace placeholders.
    char *ptr, *remaining_placeholders;
    char *placeholder_name, *placeholder_value;
    unsigned placeholder_name_len, placeholder_value_len;
    ptr = remaining_placeholders = strdup(placeholders);
    AN(ptr);
    while (((placeholder_name = strsep(&remaining_placeholders, "|")) != NULL) &&
           (((placeholder_value = strsep(&remaining_placeholders, "|")) != NULL) ||
            (remaining_placeholders != NULL))) {
        // Initializations.
        if (placeholder_value == NULL) {
            placeholder_value = remaining_placeholders;
        }
        unsigned placeholder_name_len = strlen(placeholder_name);
        unsigned placeholder_value_len = strlen(placeholder_value);
        char *ptr = result;

        // Search for occurrences of the current placeholder.
        char *match;
        unsigned match_offset;
        int overflow;
        while ((match = strstr(ptr, placeholder_name)) != NULL) {
            // Store match offset (memory pointed by result may be
            // reallocated).
            match_offset = match - result;

            // Check there is enough free space. Otherwise, try
            // to allocate some more memory.
            overflow = (used + (placeholder_value_len - placeholder_name_len)) - allocated;
            if (overflow > 0) {
                allocated += (overflow < 1024) ? 1024 : overflow;
                result = realloc(result, allocated);
                AN(result);
            }

            // Relocate the remaining template.
            memmove(
                result + match_offset + placeholder_value_len,
                result + match_offset + placeholder_name_len,
                (result + used) - (result + match_offset + placeholder_name_len));

            // Replace name with value.
            memcpy(result + match_offset, placeholder_value, placeholder_value_len);

            // Adjust used memory.
            used += placeholder_value_len - placeholder_name_len;

            // Point 'ptr' to the beginning of the remaining template.
            ptr = result + match_offset + placeholder_value_len;
        }
    }

    // Free previously allocated memory.
    free(ptr);

    // Done!
    return result;
}
