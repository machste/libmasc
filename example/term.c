#include <masc.h>


static void *log_level_check(Str *log_level_str, Str **err_msg)
{
    Int *log_level = argparse_int(log_level_str, err_msg);
    if (log_level != NULL) {
        if (!int_in_range(log_level, 0, 7)) {
            *err_msg = str_new("invalid log level: %O!", log_level_str);
            delete(log_level);
            log_level = NULL;
        }
    }
    return log_level;
}

static void *baudrate_check(Str *baudrate_str, Str **err_msg)
{
    Int *speed = NULL;
    Int *baudrate = argparse_int(baudrate_str, err_msg);
    if (baudrate != NULL) {
        SerialSpeed s = serial_baudrate_to_speed(int_get(baudrate));
        if (s > SERIAL_SPEED_B0) {
            speed = new(Int, s);
        } else {
            *err_msg = str_new("invalid baudrate: %O!", baudrate_str);
        }
        delete(baudrate);
    }
    return speed;
}


static void *device_check(Str *path, Str **err_msg)
{
    if (!path_exists(str_cstr(path))) {
        *err_msg = str_new("device '%O' does not exist!\n", path);
        return NULL;
    }
    return new_copy(path);
}

static void serial_line_cb(MlIoPkg *self, void *data, size_t size, void *arg)
{
    Str *str = str_new_ncopy(data, size);
    str_strip(str);
    put(str);
    delete(str);
}

static void serial_eof_cb(MlIoReader *self, void *arg)
{
    log_error("Lost connection to terminal device!");
    mloop_stop();
}

void stdin_line_cb(MlIoPkg *self, void *data, size_t size, void *arg)
{
    Serial *serial = arg;
    write(serial, data, size);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    const char *prog = path_basename(argv[0]);
    // Setup argument parser
    Argparse *ap = new(Argparse, prog, "Serial Terminal");
    argparse_add_opt(ap, 'l', "log-level", "LEVEL", "1", log_level_check,
            "log level (0 - 7)");
    argparse_set_default(ap, "log-level", "6");
    argparse_add_opt(ap, 'b', "baudrate", "RATE", "1", baudrate_check,
            "set baudrate");
    argparse_set_default(ap, "baudrate", "115200");
    argparse_add_arg(ap, "path", "DEV", NULL, device_check, "device name");
    // Parse command line arguments
    Map *args = argparse_parse(ap, argc, argv);
    delete(ap);
    int log_level = int_get(map_get(args, "log-level"));
    SerialSpeed speed = int_get(map_get(args, "baudrate"));
    const char *path = str_cstr(map_get(args, "path"));
    // Setup logging
    log_init(log_level);
    log_add_stdout();
    // Init mloop
    mloop_init();
    // Setup serial terminal
    Serial *serial = new(Serial, path, speed, SERIAL_PARITY_NONE);
    if (!is_open(serial)) {
        log_error("unable to open '%s': %s", path, serial_err_msg(serial));
        goto cleanup;
    }
    mloop_io_pkg_new(serial, '\n', serial_line_cb, serial_eof_cb, NULL);
    log_debug("Terminal connected to: %O", serial);
    // Setup stdin
    Io input = init(Io, STDIN_FILENO);
    mloop_io_pkg_new(&input, '\n', stdin_line_cb, NULL, serial);
    // Run main loop
    mloop_run();
    destroy(&input);
cleanup:
    delete(serial);
    mloop_destroy();
    log_destroy();
    delete(args);
    return ret;
}
