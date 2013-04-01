#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <ao/ao.h>
#include "common.h"
#include "audio.h"

ao_device *dev = NULL;

static void help(void) {
    printf("    -d driver      set the output driver\n"
           "    -o name=value  set an arbitrary ao option\n"
           "    -i id          shorthand for -o id=<id>\n"
           "    -n name        shorthand for -o dev=<name> -o dsp=<name>\n"
          );
}

static int init(int argc, char **argv) {
    ao_initialize();
    int driver = ao_default_driver_id();
    ao_option *ao_opts = NULL;

    optind = 0;
    // some platforms apparently require optreset = 1; - which?
    char opt, *mid;
    while ((opt = getopt(argc, argv, "d:i:n:o:")) > 0) {
        switch (opt) {
            case 'd':
                driver = ao_driver_id(optarg);
                if (!driver)
                    die("could not find ao driver %s\n", optarg);
                break;
            case 'i':
                ao_append_option(&ao_opts, "id", optarg);
                break;
            case 'n':
                ao_append_option(&ao_opts, "dev", optarg);
                // Old libao versions (for example, 0.8.8) only support
                // "dsp" instead of "dev".
                ao_append_option(&ao_opts, "dsp", optarg);
                break;
            case 'o':
                mid = strchr(optarg, '=');
                if (!mid)
                    die("Expected an = in audio option %s\n", optarg);
                *mid = 0;
                ao_append_option(&ao_opts, optarg, mid+1);
                break;
            default:
                help();
                die("Invalid audio option -%c specified\n", opt);
        }
    }

    ao_sample_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.bits = 16;
    fmt.rate = 44100;
    fmt.channels = 2;
    fmt.byte_format = AO_FMT_NATIVE;

    dev = ao_open_live(driver, &fmt, NULL);

    return dev ? 0 : 1;
}

static void deinit(void) {
    if (dev)
        ao_close(dev);
}

static void start(int sample_rate) {
    if (sample_rate != 44100)
        die("unexpected sample rate!\n");
}

static void play(short buf[], int samples) {
    ao_play(dev, (char*)buf, samples*4);
}

static void stop(void) {
}

audio_output audio_ao = {
    .name = "ao",
    .help = &help,
    .init = &init,
    .deinit = &deinit,
    .start = &start,
    .stop = &stop,
    .play = &play,
    .volume = NULL
};