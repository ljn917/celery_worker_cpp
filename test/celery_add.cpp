#include "celery.hpp"

#include <chrono>
#include <thread>

Json::Value add(const Json::Value& args, [[maybe_unused]] const Json::Value& kwargs) {
    int res = args[0].asInt() + args[1].asInt();
    
    Json::Value result_json = res;
    
    std::this_thread::sleep_for(std::chrono::seconds(1)); // testing publish on redis
    
    return result_json;
}

int main() {
    celery::worker_pool pool({}, {}, &add);
    
    return 0;
}
