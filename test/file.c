#include <masc/file.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    int ret = -1;
    const char *path = "/etc/passwd";
    if (argc > 1) {
        path = argv[1];
    }
    File *f = new(File, path, "r");
    put(f);
    if (file_is_open(f)) {
        print("File size: %zu\n", file_size(f));
        Str *line;
        for (int i = 0; (line = file_readline(f)) != NULL; i++) {
            str_rstrip(line);
            print("Line %i: %O (len: %zu)\n", i, line, str_len(line));
            delete(line);
        }
        file_close(f);
        put(f);
        ret = 0;
    }
    delete(f);
    return ret;
}
