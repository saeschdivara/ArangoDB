////////////////////////////////////////////////////////////////////////////////
/// @brief threads in posix
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "threads.h"

#ifdef TRI_HAVE_POSIX_THREADS

#ifdef TRI_HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#include "BasicsC/logging.h"
#include "BasicsC/tri-strings.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                            THREAD
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                     private types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief data block for thread starter
////////////////////////////////////////////////////////////////////////////////

typedef struct thread_data_s {
  void (*starter) (void*);
  void* _data;
  char* _name;
}
thread_data_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief starter function for thread
////////////////////////////////////////////////////////////////////////////////

static void* ThreadStarter (void* data) {
  thread_data_t* d;

  sigset_t all;
  sigfillset(&all);
  pthread_sigmask(SIG_SETMASK, &all, 0);

  d = data;

#ifdef TRI_HAVE_SYS_PRCTL_H
  prctl(PR_SET_NAME, d->_name, 0, 0, 0);
#endif

  // TODO: (error) Possible null pointer dereference: d - otherwise it is redundant to check if d is null at line 96
  d->starter(d->_data);

  if (d) {
    TRI_FreeString(TRI_CORE_MEM_ZONE, d->_name);
    TRI_Free(TRI_CORE_MEM_ZONE, d);
    d = NULL;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a thread
////////////////////////////////////////////////////////////////////////////////

void TRI_InitThread (TRI_thread_t* thread) {
  memset(thread, 0, sizeof(TRI_thread_t));
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the current process identifier
////////////////////////////////////////////////////////////////////////////////

TRI_pid_t TRI_CurrentProcessId () {
  return (TRI_pid_t) getpid();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the current thread process identifier
////////////////////////////////////////////////////////////////////////////////

TRI_tpid_t TRI_CurrentThreadProcessId () {
#ifdef TRI_HAVE_GETTID
  return gettid();
#else
  return getpid();
#endif
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the current thread identifier
////////////////////////////////////////////////////////////////////////////////

TRI_tid_t TRI_CurrentThreadId () {
  return (TRI_tid_t) pthread_self();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief starts a thread
////////////////////////////////////////////////////////////////////////////////

bool TRI_StartThread (TRI_thread_t* thread,
                      char const* name,
                      void (*starter)(void*),
                      void* data) {
  thread_data_t* d;
  int rc;

  d = TRI_Allocate(TRI_CORE_MEM_ZONE, sizeof(thread_data_t), false);

  d->starter = starter;
  d->_data = data;
  d->_name = TRI_DuplicateString(name);

  rc = pthread_create(thread, 0, &ThreadStarter, d);

  if (rc != 0) {
    TRI_set_errno(TRI_ERROR_SYS_ERROR);
    LOG_ERROR("could not start thread: %s ", strerror(errno));

    TRI_Free(TRI_CORE_MEM_ZONE, d);
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief trys to stops a thread
////////////////////////////////////////////////////////////////////////////////

void TRI_StopThread (TRI_thread_t* thread) {
  pthread_cancel(*thread);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief detachs a thread
////////////////////////////////////////////////////////////////////////////////

void TRI_DetachThread (TRI_thread_t* thread) {
  pthread_detach(*thread);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief waits for a thread to finish
////////////////////////////////////////////////////////////////////////////////

void TRI_JoinThread (TRI_thread_t* thread) {
  pthread_join(*thread, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sends a signal to the thread
////////////////////////////////////////////////////////////////////////////////

bool TRI_SignalThread (TRI_thread_t* thread, int signum) {
  int rc = pthread_kill(*thread, signum);

  if (rc != 0) {
    LOG_ERROR("could not send signal to thread: %s", strerror(errno));
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if we are the thread
////////////////////////////////////////////////////////////////////////////////

bool TRI_IsSelfThread (TRI_thread_t* thread) {
  return pthread_self() == *thread;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief allow cancellation
////////////////////////////////////////////////////////////////////////////////

void TRI_AllowCancelation () {
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:

