// Hossein Moein
// August 9, 2023
/*
Copyright (c) 2023-2028, Hossein Moein
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Hossein Moein and/or the ThreadPool nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Hossein Moein BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <ThreadPool/SharedQueue.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// ----------------------------------------------------------------------------

namespace hmthrp
{

class   ThreadPool  {

public:

    using size_type = int;
    using time_type = time_t;

    ThreadPool() = delete;
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator = (const ThreadPool &) = delete;

    explicit
    ThreadPool(size_type thr_num,
               bool timeout_flag = true,
               time_type timeout_time = 30 * 60);
    ~ThreadPool();

    template<typename F, typename ... As>
    std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<As> ...>>
    dispatch(bool immediately, F &&routine, As && ... args);

    bool add_thread(size_type thr_num);  // Could be positive or negative

    size_type available_threads() const noexcept;
    size_type capacity_threads() const noexcept;

    bool shutdown() noexcept;

private:

    // This is the routine that is dispatched for each thread
    //
    bool thread_routine_() noexcept;
    void terminate_timed_outs_() noexcept;

    using routine_type = std::function<void()>;

    enum class WORK_TYPE : unsigned char {
        _undefined_ = 0,
        _client_service_ = 1,
        _terminate_ = 2,
        _timeout_ = 3,
    };

    struct  WorkUnit  {

        WorkUnit() = delete;
        WorkUnit(const WorkUnit &) = default;
        WorkUnit(WorkUnit &&) = default;
        explicit WorkUnit(WORK_TYPE work_t) : work_type(work_t)  {   }
        WorkUnit(WORK_TYPE work_t, routine_type &&routine)
            : func(std::forward<routine_type>(routine)),
              work_type(work_t)  {   }

        routine_type    func {  };
        const WORK_TYPE work_type;
    };

    using guard_type = std::lock_guard<std::mutex>;
    using QueueType = SharedQueue<WorkUnit>;
    using ThreadType = std::thread;

    QueueType               queue_ { };
    std::vector<ThreadType> threads_ {  };
    std::atomic<size_type>  available_threads_ { 0 };
    std::atomic<size_type>  capacity_threads_ { 0 };
    std::atomic_bool        shutdown_flag_ { false };
    const time_type         timeout_time_;
    mutable std::mutex      state_ { };
    const bool              timeout_flag_;
};

} // namespace hmthrp

// ----------------------------------------------------------------------------

#ifndef HMTHRP_DO_NOT_INCLUDE_TCC_FILES
#  include <ThreadPool/ThreadPool.tcc>
#endif // HMTHRP_DO_NOT_INCLUDE_TCC_FILES

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
