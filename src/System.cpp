/*
 * Copyright (c) 2016 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/xia-chu/ZLMediaKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#if !defined(_WIN32)
#include <limits.h>
#include <sys/resource.h>
#include <sys/wait.h>
#if !defined(ANDROID)
#endif//!defined(ANDROID)
#endif//!defined(_WIN32)

#include <cstdlib>

#include "Util/logger.h"
#include "Util/uv_errno.h"
#include "System.h"

using namespace std;
using namespace toolkit;

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

string System::execute(const string &cmd) {
    FILE *fPipe = NULL;
    fPipe = popen(cmd.data(), "r");
    if(!fPipe){
        return "";
    }
    string ret;
    char buff[1024] = {0};
    while(fgets(buff, sizeof(buff) - 1, fPipe)){
        ret.append(buff);
    }
    pclose(fPipe);
    return ret;
}


void System::startDaemon(bool &kill_parent_if_failed) {
    kill_parent_if_failed = true;
#ifndef _WIN32
    static pid_t pid;
    do {
        pid = fork();
        if (pid == -1) {
            WarnL << "fork失败:" << get_uv_errmsg();
            //休眠1秒再试
            sleep(1);
            continue;
        }

        if (pid == 0) {
            //子进程
            return;
        }

        //父进程,监视子进程是否退出
        DebugL << "启动子进程:" << pid;
        signal(SIGINT, [](int) {
            WarnL << "收到主动退出信号,关闭父进程与子进程";
            kill(pid, SIGINT);
            exit(0);
        });

        do {
            int status = 0;
            if (waitpid(pid, &status, 0) >= 0) {
                WarnL << "子进程退出";
                //休眠3秒再启动子进程
                sleep(3);
                //重启子进程，如果子进程重启失败，那么不应该杀掉守护进程，这样守护进程可以一直尝试重启子进程
                kill_parent_if_failed = false;
                break;
            }
            DebugL << "waitpid被中断:" << get_uv_errmsg();
        } while (true);
    } while (true);
#endif // _WIN32
}

