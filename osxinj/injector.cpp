#include "injector.h"

#include <cstdio>
#include <cstring>

#include <mach/mach.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include <pthread.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/proc_info.h>
#include <libproc.h>    

Injector::Injector() : module(0), bootstrapfn(0)
{
}

Injector::~Injector()
{
    if (module)
    {
        dlclose(module);
        module = NULL;
    }
}

void Injector::inject(pid_t pid, const char *bootstrap_lib_path, const char *inject_lib_path)
{
    module = dlopen(bootstrap_lib_path, RTLD_NOW | RTLD_LOCAL);
    if (!module)
    {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return;
    }

    bootstrapfn = dlsym(module, "bootstrap");
    if (!bootstrapfn)
    {
        fprintf(stderr, "could not locate bootstrap fn\n");
        return;
    }

    mach_error_t err = mach_inject((mach_inject_entry)bootstrapfn, 
        inject_lib_path, strlen(inject_lib_path) + 1,
        pid, 0);
}

pid_t Injector::getProcessByName(const char *name)
{
    int procCnt = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    pid_t pids[1024];
    memset(pids, 0, sizeof pids);
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));

    for (int i = 0; i < procCnt; i++)
    {
        if (!pids[i]) continue;
        char curPath[PROC_PIDPATHINFO_MAXSIZE];
        char curName[PROC_PIDPATHINFO_MAXSIZE];
        memset(curPath, 0, sizeof curPath);
        proc_pidpath(pids[i], curPath, sizeof curPath);
        int len = strlen(curPath);
        if (len)
        {
            int pos = len;
            while (pos && curPath[pos] != '/') --pos;
            strcpy(curName, curPath + pos + 1);
            if (!strcmp(curName, name))
            {
                return pids[i];
            }
        }
    }
    return 0;
}
