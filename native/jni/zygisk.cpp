#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <sched.h>
#include <cstring>
#include <libgen.h>
#include <sys/wait.h>

#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

void run_daemon(int pid, int uid, const char *process, int user);
void run_scripts(int pid, int uid, const char *process, int user, bool stop);
void prepare_modules();
extern const char *MAGISKTMP;
static bool module_loaded = false;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Magisk", __VA_ARGS__)

class DynMount : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Use JNI to fetch our process name
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        bool child_zygote = args->is_child_zygote != nullptr && *(args->is_child_zygote);
        preSpecialize(process, args->uid, child_zygote);
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void preServerSpecialize(ServerSpecializeArgs *args) override {
        preServer();
    }

private:
    Api *api;
    JNIEnv *env;

    void preSpecialize(const char *process, int uid, bool child_zygote = false) {
        // Demonstrate connecting to to companion process
        int r = 0;
        int fd = api->connectCompanion();
        int pid = getpid();
        int _run_daemon = 0;
        int app_id = uid % 100000;
        if (child_zygote && app_id >= 10000 && app_id <= 19999) {
            // app zygote
            _run_daemon = 1;
        } else {
            unshare(CLONE_NEWNS);
        }
        write(fd, &_run_daemon, sizeof(_run_daemon));
        write(fd, &pid, sizeof(pid));
        write(fd, &uid, sizeof(uid));
        write(fd, process, strlen(process)+1);
        read(fd, &r, sizeof(r));
        close(fd);
        if (_run_daemon == 0)
            unshare(CLONE_NEWNS);
        // Since we do not hook any functions, we should let Zygisk dlclose ourselves
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

    void preServer() {
        int r = 0;
        int fd = api->connectCompanion();
        int zero = 0;
        write(fd, &zero, sizeof(zero));
        write(fd, &zero, sizeof(zero));
        write(fd, &zero, sizeof(zero));
        write(fd, "system_server", sizeof("system_server"));
        read(fd, &r, sizeof(r));
        close(fd);
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }
};

static int urandom = -1;

static void companion_handler(int i) {
    int done = 0;
    char MODPATH[1024];
    int pid = -1;
    int uid = -1;
    int _run_daemon = 0;
    char process[1024];
    read(i, &_run_daemon, sizeof(int));
    read(i, &pid, sizeof(int));
    read(i, &uid, sizeof(int));
    int user = uid / 100000;
    read(i, process, sizeof(process));
    if (module_loaded && _run_daemon == 0) {
        int __new_fork = fork();
        if (__new_fork == 0) {
            run_scripts(pid, uid, process, user, false);
            _exit(0);
        } else if (__new_fork > 0) {
            waitpid(__new_fork, 0, 0);
        }
	}
    write(i, &done, sizeof(int));
    //LOGD("companion_handler: [%s] PID=[%d] UID=[%d]\n", process, pid, uid);
    if (strcmp(process,"system_server") == 0 && pid == 0 && uid == 0 && !module_loaded){
        char buf[256];
        int s = readlink("/proc/self/exe", buf, sizeof(buf));
        if (s > 0) {
            MAGISKTMP = dirname(buf);
            LOGD("Magisk tmp path is %s\n", MAGISKTMP);
            prepare_modules();
            module_loaded = true;
        }
        return;
    }
    if (module_loaded && _run_daemon == 1) {
        run_daemon(pid, uid, process, user);
    }
}

REGISTER_ZYGISK_MODULE(DynMount)
REGISTER_ZYGISK_COMPANION(companion_handler)
