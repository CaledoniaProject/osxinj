#include "injector.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>

int main(int argc, char* argv[])
{
    char bootstrap_lib_path[PATH_MAX], inject_lib_path[PATH_MAX];

    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./osxinj proc_name /path/to/bootstrap.lib /path/to/hook.lib\n");
        return -1;
    }

    if (getuid() > 0)
    {
        setreuid(0, 0);
        if (getuid() > 0)
        {
            fprintf(stderr, "please run me as root\n");
            return -1;
        }
    }

    realpath(argv[2], bootstrap_lib_path);
    realpath(argv[3], inject_lib_path);

    fprintf(stderr, "bootstrap lib at %s\n", bootstrap_lib_path);
    fprintf(stderr, "inject lib at %s\n", inject_lib_path);

    Injector inj;
    
    pid_t pid = inj.getProcessByName(argv[1]);
    if (!pid)
    {
        fprintf(stderr, "Process %s not found\n", argv[1]);
        return 0;
    }
    fprintf(stderr, "pid: %u\n", pid);
    
    inj.inject(pid, bootstrap_lib_path, inject_lib_path);
    return 0;
}