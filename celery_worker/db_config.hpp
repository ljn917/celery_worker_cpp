#pragma once

#include <json/json.h>

#include "rabbitmq/rabbitmq.hpp"
#include "redis/redis.hpp"

namespace celery {

rabbitmq::connection rabbitmq_from_config(const Json::Value& config) {
    std::string queue_name{"celery"};
    std::string hostname{"127.0.0.1"};
    int port{5672};
    std::string username{"guest"};
    std::string password{"guest"};
    std::string vhost{"/"};
    bool use_tls{false};
    
    if(config.isMember("queue_name")) queue_name = config["queue_name"].asString();
    if(config.isMember("hostname")) hostname = config["hostname"].asString();
    if(config.isMember("port")) port = config["port"].asInt();
    if(config.isMember("username")) username = config["username"].asString();
    if(config.isMember("password")) password = config["password"].asString();
    if(config.isMember("vhost")) vhost = config["vhost"].asString();
    if(config.isMember("use_tls")) use_tls = config["use_tls"].asBool();
    
    return rabbitmq::connection(
        queue_name,
        hostname,
        port,
        username,
        password,
        vhost,
        use_tls
    );
}

redis::client redis_from_config(const Json::Value& config) {
    std::string ip{"127.0.0.1"};
    int port{6379};
    std::string username;
    std::string password;
    int db{0};
    
    if(config.isMember("ip")) ip = config["ip"].asString();
    if(config.isMember("port")) port = config["port"].asInt();
    if(config.isMember("username")) username = config["username"].asString();
    if(config.isMember("password")) password = config["password"].asString();
    if(config.isMember("db")) db = config["db"].asInt();
    
    auto client = redis::client(ip, port);
    
    if(!password.empty()) {
        if(username.empty()) {
            client.auth(password);
        } else {
            client.auth(username, password);
        }
    }
    
    if(db != 0) client.select(db);
    
    return client;
}

}
