#pragma once

#include <sstream>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "db_config.hpp"

namespace celery {

class worker_pool {
private:
    using Func = std::function<Json::Value (const Json::Value& args, const Json::Value& kwargs)>;
    
    std::vector<rabbitmq::connection> broker_; // RabbitMQ for jobs
    std::vector<redis::client> backend_; // redis for results
    
    std::vector<std::thread> thread_pool;
    
    Json::StreamWriterBuilder json_builder_{};
    
public:
    worker_pool(worker_pool&)=delete;
    worker_pool(worker_pool&&)=default;
    worker_pool& operator=(worker_pool&)=delete;
    worker_pool& operator=(worker_pool&&)=default;
    
    explicit worker_pool(
        const Json::Value& broker_config,
        const Json::Value& backend_config,
        Func&& f,
        int pool_size = 0
    ) {
        if(pool_size <= 0) {
            pool_size = std::thread::hardware_concurrency();
        }
        
        for(int i = 0; i < pool_size; i++) {
            broker_.emplace_back(rabbitmq_from_config(broker_config));
            backend_.emplace_back(redis_from_config(backend_config));
            thread_pool.emplace_back(&worker_pool::loop, this, f, i);
        }
    }
    
    ~worker_pool() {
        for(auto& t: thread_pool) {
            t.join();
        }
    }
    
    void loop(Func&& f, int i) {
        while(true) {
            const auto envelope = broker_[i].consume();
            
            std::string message{envelope.get_message_body()};
            
            std::stringstream ss(std::move(message));
            Json::Value message_json;
            ss >> message_json;
            
            const Json::Value& args = message_json[0];
            const Json::Value& kwargs = message_json[1];
            
            Json::Value result;
            const std::string task_id{envelope.get_correlation_id()};
            result["task_id"] = task_id;
            result["traceback"] = Json::Value::null;
            result["children"] = Json::arrayValue;
            // result["date_done"]
            
            try {
                auto retval = f(args, kwargs);
                
                result["status"] = "SUCCESS";
                result["result"] = std::move(retval);
            } catch(std::exception& e) {
                result["status"] = "FAILURE";
                result["result"] = Json::Value::null;
                std::cerr << "celery task failed: " << e.what() << std::endl;
            }
            
            // insert retval into the result queue
            const auto backend_key = "celery-task-meta-" + task_id;
            const auto backend_val = Json::writeString(json_builder_, result);
            backend_[i].store(backend_key, backend_val);
            
            broker_[i].ack(envelope);
        }
    }
};

}
