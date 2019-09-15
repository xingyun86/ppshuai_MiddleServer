/**
 * @file task_thread.h
 * @brief taskthread defines
 */

#pragma once

#include <header.h>
#include <base_thread.h>

class CTaskThread: public CBaseThread {
public:
	CTaskThread(const std::string & name, const std::shared_ptr<std::thread>& thread = nullptr):CBaseThread(name, thread) {
		m_locker = std::make_shared<std::mutex>();
		m_task_queue = std::make_shared<std::deque<std::string>>();
	}
	CTaskThread():CBaseThread() {
		m_locker = std::make_shared<std::mutex>();
		m_task_queue = std::make_shared<std::deque<std::string>>();
	}

private:
	std::shared_ptr<std::mutex> m_locker;
	std::shared_ptr<std::deque<std::string>> m_task_queue;
public:
	virtual std::shared_ptr<std::string> GetTaskInfo() {
		std::shared_ptr<std::string> taskinfo = nullptr;
		m_locker->lock();
		if (!m_task_queue->empty())
		{
			taskinfo = std::make_shared<std::string>(m_task_queue->front());
			m_task_queue->pop_front();
		}
		m_locker->unlock();
		return taskinfo;
	}
	virtual void AddTaskToQueue(const std::string& s) {
		m_locker->lock();
		m_task_queue->push_back(s);
		m_locker->unlock();
	}

public:
	virtual void Handler(void*) = 0;
public:
	const std::shared_ptr<std::mutex>& GetLocker() { return m_locker; }
	const std::shared_ptr<std::deque<std::string>>& GetTaskQueue() { return m_task_queue; }
};
