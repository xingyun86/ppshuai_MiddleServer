/**
 * @file base_thread.h
 * @brief basethread defines
 */

#pragma once

#include <thread>
#include <string>

class CBaseThread {
public:
	typedef enum ThreadStatus {
		TSTYPE_STOPPED=0,
		TSTYPE_RUNNING,
	}ThreadStatus;

public:
	CBaseThread(const std::string& name, const std::shared_ptr<std::thread>& thread = nullptr) {
		m_name = name;
		m_nRunning = TSTYPE_STOPPED;
		m_thread = thread;
	}
	CBaseThread() {
		m_name = "";
		m_nRunning = TSTYPE_STOPPED;
		m_thread = nullptr;
	}
	virtual ~CBaseThread() {
		// ����߳�δ�رգ����ȹر��߳�
		Stop();
	}

private:
	std::string m_name;
	ThreadStatus m_nRunning;
	std::shared_ptr<std::thread> m_thread;

public:
	virtual void Handler(void*) = 0;
public:

	void SetThread(const std::shared_ptr<std::thread>& thread) { m_thread = thread; }
	std::shared_ptr<std::thread> GetThread() { return m_thread; }

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() { return m_name; }

	void SetRunning(ThreadStatus nRunning = TSTYPE_STOPPED) { m_nRunning = nRunning; }
	ThreadStatus GetRunning() { return m_nRunning; }
	bool IsRunning() { return (m_nRunning == TSTYPE_RUNNING); }

	// �����߳�
	void Start() {
		// ����߳�δ�رգ����ȹر��߳�
		Stop();
		SetThread(std::make_shared<std::thread>([](void* p) {
			CBaseThread* thiz = reinterpret_cast<CBaseThread*>(p);
			if (thiz != nullptr)
			{
				// �������߳�
				thiz->SetRunning(TSTYPE_RUNNING);

				// ����ҵ���߼�
				thiz->Handler(p);

				// �������߳�
				thiz->SetRunning(TSTYPE_STOPPED);
			}
			}, this));
	}
	// �����߳�
	void Stop()
	{
		if (m_thread != nullptr)
		{
			// ����߳�δ�رգ����ȹر��߳�
			SetRunning(TSTYPE_STOPPED);
			if (m_thread->joinable())
			{
				m_thread->join();
			}
		}
	}
	// ��������
	void Loop()
	{
		Start();
		if (m_thread != nullptr)
		{
			if (m_thread->joinable())
			{
				m_thread->join();
			}
		}
	}
};
