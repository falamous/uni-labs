#include "struct.h"
#include "util.h"
#include <string.h>

struct file_mapped fmalloc(FILE *file, char *s) {
        size_t start;
        size_t len;
        fseek(file, 0, SEEK_END);
        start = ftell(file);

        len = strlen(s) + 1 + sizeof(size_t) * 2;
        fputc(1, file);
        fwrite(&start, 1, sizeof(start), file);
        fwrite(&len, 1, sizeof(len), file);
        fwrite(s, 1, len - 1 - sizeof(size_t) * 2, file);

        return (struct file_mapped){.file = file, .start = start};
}

void ffree(struct file_mapped *fm) {
        fseek(fm->file, fm->start, SEEK_SET);
        fputc(0, fm->file);
}

char *freadstr(struct file_mapped *fm) {
        size_t start;
        size_t len;
        char *buf;
        fseek(fm->file, fm->start, SEEK_SET);
        fgetc(fm->file);
        fread(&start, 1, sizeof(start), fm->file);
        fread(&len, 1, sizeof(len), fm->file);
        buf = xmalloc(len);
        fread(buf, 1, len - 1 - sizeof(size_t) * 2, fm->file);
        buf[len - 1 - sizeof(size_t) * 2] = '\0';
        return buf;
}

ChainedDict *fdefragment(FILE *file) {
        ChainedDict *res;
        size_t i;
        size_t j;
        size_t start;
        size_t len;
        size_t new_start;
        char *buf;
        int in_use;

        res = xmalloc(sizeof(ChainedDict));
        chaineddict_init(res, 1024, idhash, valcmp, NULL, NULL);
        i = 0;
        j = 0;
        fseek(file, i, SEEK_SET);
        while ((in_use = fgetc(file)) != EOF) {

                fread(&start, 1, sizeof(start), file);
                fread(&len, 1, sizeof(len), file);
                if (in_use) {
                        buf = xmalloc(len);
                        fread(buf, 1, len - 1 - sizeof(size_t) * 2, file);
                        fseek(file, j, SEEK_SET);
                        new_start = ftell(file);

                        chaineddict_set(res, (Val){.i = start}, (void *)new_start);
                        fwrite(&in_use, 1, 1, file);
                        fwrite(&new_start, 1, sizeof(new_start), file);
                        fwrite(&len, 1, sizeof(len), file);
                        fwrite(buf, 1, len - 1 - sizeof(size_t) * 2, file);
                        j += len;
                        free(buf);
                }
                i += len;

                fseek(file, i, SEEK_SET);
        }
        return res;
}
