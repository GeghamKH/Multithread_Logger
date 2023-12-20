#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>

#include <chrono>
#include <sstream>

#include <memory>


class MultiLogger
{
private:

	static MultiLogger* m_logger;
	//static std::shared_ptr<MultiLogger> m_logger;
	static std::recursive_mutex m_mutex;
	std::ofstream m_logFile;


	MultiLogger() {
		m_logFile.open("multithreading_log.txt", std::ios::app);
	}

public:
	static MultiLogger* GetInstans()
	{
		if (!m_logger)
		{
			try
			{
				std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_logger = new MultiLogger();
			}
			catch (const std::bad_alloc ex)
			{
				std::cout << ex.what();
				return nullptr;
			}

		}
		return m_logger;
	}


	//For variadic template use 
	void Log()
	{
		std::lock_guard<std::recursive_mutex> lk(m_mutex);
		m_logFile << '\n';
		std::cout << std::endl;
	}

	template<class Arg, class ... Args>
	void Log(Arg value, Args... args)
	{
		std::lock_guard<std::recursive_mutex> lk(m_mutex);
 		if (!m_logFile.is_open()) return;

		std::cout << value;
		m_logFile << '\t' <<  value << '\t';

		Log(args...);
	
	}


	//If when explicit writing the copy ctor and copy assigned operators(= delete also) started from 
	//c++11 the move ctor and move assigned operator will not created implicitly

	// Move ctor | maybe don't need 
	MultiLogger(MultiLogger&& other) noexcept
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (!m_logger)
		{
			m_logger = std::exchange(other.m_logger, nullptr); // will replace other.m_logger with nullptr and return old value of other.m_logger
		}
		else
		{
			std::cerr << "Error: Attempted to move into an existing instance." << std::endl;
		}
	}

	//Move assigned operator | maybe don't need 
	MultiLogger& operator=(MultiLogger&& other) noexcept
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (!m_logger)
		{
			m_logger = std::exchange(other.m_logger, nullptr); // will replace other.m_logger with nullptr and return old value of other.m_logger
		}
		else
		{
			std::cerr << "Error: Attempted to move into an existing instance." << std::endl;
		}
	}



	// Copy ctor and copy assigned operators must be removed because this is a singltone design pattern
	MultiLogger(const MultiLogger& other) = delete;
	MultiLogger& operator =(const MultiLogger& other) = delete;

	~MultiLogger()
	{
		m_logFile.close();
	}
};

// Initialize static members
MultiLogger* MultiLogger::m_logger = nullptr;
std::recursive_mutex MultiLogger::m_mutex;






//#######################
void SomeWork(MultiLogger* logger)
{
	std::ostringstream ThreadID; 
	ThreadID << std::this_thread::get_id();

	std::string message = std::string("Time: ") + std::string(" This is a message from the Thread by ID = ") + ThreadID.str();
	std::cout << message;
	
	logger->Log(message, 123, 4.5);

}



// MAIN
#define THREADS_COUNTS 10

int main()
{
	std::thread Threads[THREADS_COUNTS];
	MultiLogger* instanse = MultiLogger::GetInstans();

	for (int i = 0; i < THREADS_COUNTS; i++)
	{
		Threads[i] = std::thread(SomeWork, instanse);
	}

	for (int i = 0; i < THREADS_COUNTS; i++)
	{
		Threads[i].join();
	}


	// Imitatation to work
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5000ms);
	// Imitatation to work




	delete instanse;
}


