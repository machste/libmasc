#include <masc/file.h>
#include <masc/list.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    int ret = -1;
    const char *path = "/tmp/masc_file_test";
    if (argc > 1) {
        path = argv[1];
    }
    File *f = new(File, path, "w");
    put(f);
    int len = file_write(f, "Hallo Welt!\n");
    len = file_write_fmt(f, "Wrote %i bytes to %O\n", len, f);
    print("Wrote %i bytes to %O\n", len, f);
    List *l = new(List);
    for (int i = 0; i < 5; i++) {
        list_append(l, new(Str, "Count: %i\n", i));
    }
    len = file_put(f, l);
    print("Wrote %i bytes to %O\n", len, f);
    len = file_writelines(f, l);
    print("Wrote %i bytes to %O\n", len, f);
    delete(l);
    // Read tests
    delete(f);
    f = new(File, path, "r");
    put(f);
    if (file_is_open(f)) {
        print("File size: %zu\n", file_size(f));
        Str *line;
        for (int i = 0; (line = file_readline(f)) != NULL; i++) {
            str_rstrip(line);
            print("Line %i: %O (len: %zu)\n", i, line, str_len(line));
            delete(line);
        }
        print("Rewind %O\n", f);
        file_rewind(f);
        Str *s1 = file_read(f, 7);
        print("Read 7 bytes: %O\n", s1);
        Str *s2 = file_read(f, - 1);
        print("Read the remaining %i bytes: %O", str_len(s2), s2);
        delete(s1);
        delete(s2);
        file_close(f);
        put(f);
        ret = 0;
    }
    delete(f);
    return ret;
}
