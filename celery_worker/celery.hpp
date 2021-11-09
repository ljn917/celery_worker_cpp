#pragma once

#include <sstream>
#include <exception>
#include <functional>

#include <json/json.h>

#include "rabbitmq/rabbitmq.hpp"
#include "redis/redis.hpp"

namespace celery {

class worker {
private:
    using Func = std::function<Json::Value (const Json::Value& args, const Json::Value& kwargs)>;
    
    rabbitmq::connection broker_; // RabbitMQ for jobs
    redis::client backend_; // redis for results
    
    Json::StreamWriterBuilder json_builder_{};
    
public:
    worker(worker&)=delete;
    worker(worker&&)=default;
    worker& operator=(worker&)=delete;
    worker& operator=(worker&&)=default;
    
    explicit worker(rabbitmq::connection&& broker, redis::client&& backend) : broker_(std::move(broker)), backend_(std::move(backend)) {}
    
    void loop(Func&& f) {
        while(true) {
            const auto envelope = broker_.consume();
            
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
            backend_.store(backend_key, backend_val);
            
            // confirm job complete
            broker_.ack(envelope);
        }
    }
};

}
