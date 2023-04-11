#include <iostream>
#include <boost/asio.hpp>
#include <algorithm>
#include <future>
#include <thread>
#include <boost/asio/spawn.hpp>
#define TASK_HANDLER 1


/**
 * schedule one of more tasks on a schedule.
 * this allows tasks to be stopped, and does
 * not leave dangling threads.
 */
class TaskExecutor{
  public:
    explicit TaskExecutor() = default;
    void stop(){
        m_context.stop();
    }
    void wait(){
        m_context.wait();
    }
#if TASK_HANDLER
    template<typename Func>
    void schedule_service_interval(Func&& func, std::chrono::milliseconds interval)
    {
        boost::asio::spawn(m_context, [this, func = std::forward<Func>(func), interval](const boost::asio::yield_context& yield){

            while(func())
            {
                auto timer = boost::asio::steady_timer(m_context);
                timer.expires_from_now(interval);
                timer.async_wait(yield);
            }
            std::cout << "inside yield token" << std::endl;
        });


    }
    template<typename Token>
    void my_coroutine1(boost::asio::basic_yield_context<Token> yield){

    }
#else
    void old_nandler(auto function, std::chrono::milliseconds interval){
        using handle_t                                                = boost::asio::steady_timer;
        auto timer                                                  = std::make_shared<handle_t>(m_context);
        std::cout << "scheduling every" << interval.count() << "ms." << std::endl;
        const std::function<void(const boost::system::error_code&)> handler =
            [ func, timer, interval, handler](const boost::system::error_code& error)
        {
            if (!error)
            {
                std::cout << "triggered" << std::endl;
                func();
                std::cout << "trigger returned." << std::endl;

                timer->expires_at(timer->expiry() + interval);
                timer->expires_at(std::chrono::steady_clock::now()+interval);
//                timer->async_wait(handler);
            }                                   [\
        };
        timer->expires_at(boost::asio::steady_timer::clock_type::now());
    }
#endif
  private:
    // this is where asio runs
    std::vector<std::future<void>> m_async_tasks;
    boost::asio::thread_pool m_context{};

};



int main(){
    TaskExecutor my_task{};
    std::cout << "starting" <<std::endl;
    int count = 0;
     my_task.schedule_service_interval([&count]()
                                      {
                                          count++;
                                          //std::cout << "hello from wpe browser"<<count<< std::endl ;
                                          return count < 200;
                                      }, std::chrono::milliseconds (0));
     my_task.wait();
     my_task.stop();

}
