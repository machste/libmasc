#include <masc/path.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    const char *path = "./some/test/path";
    if (argc > 1) {
        path = argv[1];
    }
    print("Path: %s\n", path);
    // Test isabs
    print("isabs: %s\n", path_is_abs(path) ? "true" : "false");
    // Test split
    Array *split = path_split(path);
    print("split: %O\n", split);
    delete(split);
    // Test basename
    print("basename: %s\n", path_basename(path));
    // Test dirname
    Str *dirname = path_dirname(path);
    print("dirname: %O\n", dirname);
    delete(dirname);
    // Test join
    Str *joined = path_join(path, "../more/stuff/");
    print("join(path, \"../more/path/\": %O\n", joined);
    // Test normpath
    Str *normpath = path_normpath(str_cstr(joined));
    delete(joined);
    print("normpath: %O\n", normpath);
    delete(normpath);
    // Test abspath
    Str *abspath = path_abspath(path);
    print("abspath: %O\n", abspath);
    delete(abspath);
    // Test is_file, is_dir, is_link and exists
    print("exists: %s\n", path_exists(path) ? "true" : "false");
    print("is_file: %s\n", path_is_file(path) ? "true" : "false");
    print("is_dir: %s\n", path_is_dir(path) ? "true" : "false");
    print("is_link: %s\n", path_is_link(path) ? "true" : "false");
    // Test readlink
    Str *link = path_readlink(path);
    print("readlink: %s -> %O\n", path, link);
    delete(link);
    // Test list
    List *list = path_list(path);
    print("list: %O\n", list);
    delete(list);
    return 0;
}
