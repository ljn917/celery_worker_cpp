#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <stdexcept>
#include <optional>
#include <utility>

#include <hiredis/hiredis.h>

namespace redis {
// https://github.com/redis/hiredis/blob/master/hiredis.h
// https://github.com/redis/hiredis/blob/master/examples/example.c

struct reply_t {
    redisReply *reply_;
    
    reply_t(reply_t&)=delete;
    reply_t& operator=(reply_t&)=delete;
    
    reply_t(reply_t&& src) {
        this->reply_ = std::exchange(src.reply_, nullptr);
    }
    
    reply_t& operator=(reply_t&& src) {
        this->reply_ = std::exchange(src.reply_, nullptr);
        return *this;
    }
    
    reply_t(void *r) : reply_((redisReply*)r) {}
    reply_t(redisReply *r) : reply_(r) {}
    
    ~reply_t() {
        if(reply_) freeReplyObject(reply_);
    }
    
    long long get_int() const {
        if(reply_->type != REDIS_REPLY_INTEGER) {
            throw std::runtime_error{"Redis reply is not an int, type=" + std::to_string(reply_->type)};
        }
        return reply_->integer;
    }
    /*
    // hiredis 0.13.3 does not have dval member yet. Not in 0.14 either. It appears >= 0.15
    double get_double() const {
        if(reply_->type != REDIS_REPLY_DOUBLE) {
            throw std::runtime_error{"Redis reply is not a double, type=" + std::to_string(reply_->type)};
        }
        return reply_->dval;
    }*/
    
    std::string_view get_string() const {
        return std::string_view{reply_->str, (size_t)reply_->len};
    }
    
    bool is_null() const {
        return reply_->type == REDIS_REPLY_NIL;
    }
    
    bool is_error() const {
        return reply_->type == REDIS_REPLY_ERROR;
    }
};

struct client {
    redisContext *context;
    
    client(client&)=delete;
    client& operator=(client&)=delete;
    
    client(client&& src) {
        this->context = std::exchange(src.context, nullptr);
    }
    
    client& operator=(client&& src) {
        this->context = std::exchange(src.context, nullptr);
        return *this;
    }
    
    client(
        const std::string& ip = "127.0.0.1",
        const int port = 6379
    ) {
        context = redisConnect(ip.c_str(), port);
        
        if (context == NULL || context->err) {
            std::cerr << "redisConnect: failed" << std::endl;
            std::abort();
        }
    }
    
    ~client() {
        if(context) redisFree(context);
    }
    
    bool is_error() {
        return context->err;
    }
    
    char* get_error_message() {
        return context->errstr;
    }
    
    void throw_on_error() {
        if(is_error()) {
            std::cerr << "redis::client: error: " << get_error_message() << std::endl;
            throw std::runtime_error{get_error_message()};
        }
    }
    
    void auth(const std::string& password) {
        reply_t reply = redisCommand(context, "AUTH %b", password.c_str(), password.size());
        throw_on_error();
        if(reply.is_error()) {
            std::cerr << "redis::client: AUTH failed" << std::endl;
            throw std::runtime_error{"redis::client: AUTH failed"};
        }
    }
    
    void auth(const std::string& username, const std::string& password) {
        reply_t reply = redisCommand(context, "AUTH %b %b", username.c_str(), username.size(), password.c_str(), password.size());
        throw_on_error();
        if(reply.is_error()) {
            std::cerr << "redis::client: AUTH failed" << std::endl;
            throw std::runtime_error{"redis::client: AUTH failed"};
        }
    }
    
    void select(const int db) {
        reply_t reply = redisCommand(context, "SELECT %u", db);
        throw_on_error();
    }
    
    void set(const std::string& key, const std::string& value) {
        reply_t reply = redisCommand(context, "SET %b %b", key.c_str(), key.size(), value.c_str(), value.size());
        throw_on_error();
    }
    
    void set(const std::string& key, const std::string& value, uint32_t ex_sec) {
        reply_t reply = redisCommand(context, "SET %b %b EX %u", key.c_str(), key.size(), value.c_str(), value.size(), ex_sec);
        throw_on_error();
    }
    
    int publish(const std::string& channel, const std::string& message) {
        reply_t reply = redisCommand(context, "PUBLISH %b %b", channel.c_str(), channel.size(), message.c_str(), message.size());
        throw_on_error();
        return reply.get_int();
    }
    
    std::optional<std::string> get(const std::string& key) {
        reply_t reply = redisCommand(context, "GET %b", key.c_str(), key.size());
        throw_on_error();
        if(reply.is_null()) {
            return std::nullopt;
        }
        return std::string{reply.get_string()};
    }
    
    int del(const std::vector<std::string> keys) {
        if(keys.empty()) return 0;
        
        std::string cmd{"DEL"};
        for(const auto& key: keys) {
            cmd += " " + key;
        }
        
        reply_t reply = redisCommand(context, cmd.c_str());
        throw_on_error();
        return reply.get_int();
    }
    
    void store(const std::string& key, const std::string& value) {
        set(key, value, 86400);
        publish(key, value);
    }
};

}
