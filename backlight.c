#include <dirent.h>
#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define PACKAGE_NAME "backlight"
#define PACKAGE_VERSION "0.0.1"

#define SYS_NODE    "/sys/class/backlight"

struct device {
    char *path;
    int max;
    int current;
};

static int usage(int code)
{
    printf("Usage: %s [option] [command]\n"
           "\n"
           "Options:\n"
           " -h    This help text\n"
           " -v    Show program version\n"
           "\n"
           "Commands:\n"
           " val   Set brightness val%%, range is 0~100\n"
           "\n", PACKAGE_NAME);

    return code;
}

static int version(void)
{
    puts(PACKAGE_VERSION);
    return 0;
}

static int is_digit_str(const char *s)
{
    int i;
    for (i = 0; s[i]!= '\0'; i++) {
        if (isdigit(s[i]) == 0)
        return 0;
    }
    return 1;
}

static int value(char *path, char *file, int val)
{
    FILE *fp;
    char *fn, *mode;
    size_t len;
    int rc;

    len = strlen(path) + strlen(file) + 2;
    fn = alloca(len);
    snprintf(fn, len, "%s/%s", path, file);

    if (val == -1)
        mode = "r";
    else
        mode = "w";

    fp = fopen(fn, mode);
    if(!fp) {
        warn("Failed open %s", fn);
        return -1;
    }

    if (val == -1)
        rc = fscanf(fp, "%d", &val);
    else
        rc = fprintf(fp, "%d\n", val);
    if (!rc)
        warn("Failed %s value %s %s", val==-1 ? "reading":"writing", val==-1 ? "from":"to", fn);
    fclose(fp);
    return val;
}

static int set(char *path, char *file, int val)
{
    int rc;
    return value(path, file, val);
}

static int get(char *path, char *file)
{
    return value(path, file, -1);
}

static int brightness_set(struct device *dev, int val)
{
    int rc = -1;
    if (val >= 0 && val <=100)
        rc = set(dev->path, "brightness", dev->max * val / 100);
    return rc;
}

static int brightness_get(struct device *dev)
{
    double val = get(dev->path, "brightness") * 100.0 / dev->max;
    return round(val);
}

static int locate(struct device *dev)
{
    DIR *dir;
    struct dirent *d;
    int found = 0;
    size_t len;

    dir = opendir(SYS_NODE);
    if (!dir)
        return 0;

    while((d = readdir(dir))) {
        if (d->d_name[0] == '.')
            continue;
        len = strlen(SYS_NODE) + strlen(d->d_name) + 2;
        dev->path = malloc(len);
        if (!dev->path)
            return 0;
        snprintf(dev->path, len, "%s/%s", SYS_NODE, d->d_name);
        dev->max = get(dev->path, "max_brightness");
        dev->current = get(dev->path, "brightness");
        found = 1;
        break;
    }
    closedir(dir);
    return found;
}

int main(int argc, char *argv[])
{
    struct device dev;
    int c;
    long val;

    while((c = getopt(argc, argv, "hv")) != EOF) {
        switch (c) {
        case 'h':
            return usage(0);
        case 'v':
            return version();
        default:
            return usage(1);
        }
    }

    if (!locate(&dev)) 
        err(1, "System has no backlight control");

    if (argc == 2 && is_digit_str(argv[1])) {
        brightness_set(&dev, strtol(argv[1], NULL, 10));
    }

    printf("%d\n", brightness_get(&dev));
    return 0;
} 