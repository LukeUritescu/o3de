/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace AZ
{
    inline TaskToken::TaskToken(TaskGraph& parent, uint32_t index)
        : m_parent{ parent }
        , m_index{ index }
    {
    }

    template<typename... JT>
    void TaskToken::Precedes(JT&... tokens)
    {
        (PrecedesInternal(tokens), ...);
    }

    template <typename... JT>
    void TaskToken::Follows(JT&... tokens)
    {
        (tokens.PrecedesInternal(*this), ...);
    }

    inline bool TaskGraphEvent::IsSignaled()
    {
        return m_semaphore.try_acquire_for(AZStd::chrono::milliseconds{ 0 });
    }

    inline void TaskGraphEvent::Wait()
    {
        m_semaphore.acquire();
    }

    inline void TaskGraphEvent::Signal()
    {
        m_semaphore.release();
    }

    template<typename Lambda>
    TaskToken TaskGraph::AddTask(TaskDescriptor const& desc, Lambda&& lambda)
    {
        AZ_Assert(!m_submitted, "Cannot mutate a TaskGraph that was previously submitted or in flight.");

        m_tasks.emplace_back(desc, AZStd::forward<Lambda>(lambda));

        return { *this, aznumeric_cast<uint32_t>(m_tasks.size() - 1) };
    }

    template <typename... Lambdas>
    AZStd::array<TaskToken, sizeof...(Lambdas)> TaskGraph::AddTasks(TaskDescriptor const& descriptor, Lambdas&&... lambdas)
    {
        return { AddTask(descriptor, AZStd::forward<Lambdas>(lambdas))... };
    }

    inline void TaskGraph::Detach()
    {
        m_retained = false;
    }
} // namespace AZ
