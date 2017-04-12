#include <openssl/md5.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define mkdir(x) mkdir(x, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

internalfn
size_t get_exe_path(char* buf, size_t bufsize)
{
    ssize_t res;

    res = readlink("/proc/self/exe", buf, bufsize);
    if (res >= 0) {
        return (size_t)res;
    }

    res = readlink("/proc/curproc/file", buf, bufsize);
    if (res >= 0) {
        return (size_t)res;
    }

    res = readlink("/proc/self/path/a.out", buf, bufsize);
    if (res >= 0) {
        return (size_t)res;
    }

    perror("readlink");
    strcpy(buf, ".");

    return 1;
}

