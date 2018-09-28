#include <masc/path.h>
#include <masc/iter.h>
#include <masc/int.h>
#include <masc/json.h>
#include <masc/print.h>


static void print_usage(void)
{
    fprint(stderr, "usage: filetree PATH\n");
}

Map *path_tree(Map *tree, const char *dir)
{
    if (tree == NULL) {
        tree = new(Map);
    }
    List *items = path_list(dir);
    list_sort(items, cmp);
    Iter *itr = new(Iter, items);
    for (Str *item = next(itr); item != NULL; item = next(itr)) {
        Str *path = path_join(dir, str_cstr(item));
        PathType type = path_type(str_cstr(path));
        if (type == PATH_DIR) {
            Map *subtree = new(Map);
            path_tree(subtree, str_cstr(path));
            map_set(tree, str_cstr(item), subtree);
        } else if (type == PATH_LINK) {
            Str *link = path_readlink(str_cstr(path));
            map_set(tree, str_cstr(item), link);
        } else { 
            Int *fsize = new(Int, path_getsize(str_cstr(path)));
            map_set(tree, str_cstr(item), fsize);
        }
        delete(path);
    }
    delete(itr);
    delete(items);
    return tree;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 2) {
        print_usage();
        return -1;
    }
    const char *path = argv[1];
    // Check if path exists
    if (!path_exists(path)) {
        fprint(stderr, "Error: path '%s' does not exist!\n", path);
        return -1;
    }
    // Check if path is a directory
    if (!path_is_dir(path)) {
        fprint(stderr, "Error: path '%s' is not a directory!\n", path);
        return -1;
    }
    Map *tree = path_tree(NULL, path);
    Json *js = new(Json, tree);
    json_pretty_print(js);
    delete_objs(js, tree);
    return ret;
}
