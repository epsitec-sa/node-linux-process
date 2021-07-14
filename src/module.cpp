#include <node_api.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/sysctl.h>


int get_process_name_by_pid(const int pid, char* nameBuffer, int nameBufferSize)
{
    struct kinfo_proc *process = NULL;
    size_t proc_buf_size;
    int st, proc_count;
    int i = 0;
    int name[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
    
    st = sysctl(name, 4, NULL, &proc_buf_size, NULL, 0);
    if (st == -1) {
        return errno;
    }

    process = (kinfo_proc *)malloc(proc_buf_size);
    st = sysctl(name, 4, process, &proc_buf_size, NULL, 0);
    if (st == -1) {
        return errno;
    }
    
    proc_count = proc_buf_size / sizeof(struct kinfo_proc);
    while (i < proc_count) {
        if (process[i].kp_proc.p_pid == pid) {
            strcpy(nameBuffer, process[i].kp_proc.p_comm);

            free(process);
            return 1;
        }
        i++;
    }

    free(process);
    return 0;
}

// int processPid, string processName -> int
napi_value IsProcessRunning(napi_env env, napi_callback_info info)
{
  napi_status status;
  size_t argc = 2;
  napi_value argv[2];
  napi_value napiResult;

  int processPid = 0;
  int result = 1;
  char processNameA[1000];
  size_t processNameARead = 0;
  char procNameByPidBuffer[1024];


  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Failed to parse arguments");
  }

  status = napi_get_value_int32(env, argv[0], &processPid);

  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid processId was passed as argument");
  }

  status = napi_get_value_string_utf8(env, argv[1], processNameA, 1000, &processNameARead);

  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid processName was passed as argument");
  }

  if (processPid != 0)
  {
      auto res = kill(processPid, 0);

      if (res == 0) 
      {
          int ret = get_process_name_by_pid(processPid, procNameByPidBuffer, 1024);

          if (ret == 0)
          {
              // No process with the pid has been found
              result = 0;
          }
          else if (ret == 1)
          {
              // Process with pid has been found, but what about its name?
              if (strstr(procNameByPidBuffer, processNameA) == NULL)
              {
                  // Process with pid found, but different name
                  result = 0;
              }
          }
          else
          {
              // An error occurred
              result = ret;
          }
      }
      else 
      {
          int errsv = errno;
          if (errsv == ESRCH) 
          {
              // Process with pid is not running
              result = 0;
          }
          else
          {
              result = errsv;
          }
      }
  }
  
  status = napi_create_int32(env, result, &napiResult);

  if (status != napi_ok) {
    printf("---%d---", (int)status);
    napi_throw_error(env, NULL, "Unable to create return value");
  }

  return napiResult;
}


napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value fn;

  status = napi_create_function(env, NULL, 0, IsProcessRunning, NULL, &fn);
  if (status != napi_ok) {
    printf("---%d---", (int)status);
    napi_throw_error(env, NULL, "Unable to wrap IsProcessRunning native function");
  }

  status = napi_set_named_property(env, exports, "is_process_running", fn);
  if (status != napi_ok) {
    printf("---%d---", (int)status);
    napi_throw_error(env, NULL, "Unable to populate is_process_running exports");
  }

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)