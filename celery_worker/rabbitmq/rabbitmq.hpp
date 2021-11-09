#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_ssl_socket.h>
//#include <rabbitmq-c/amqp.h>
//#include <rabbitmq-c/tcp_socket.h>

namespace rabbitmq {

inline std::string_view to_string_view(const amqp_bytes_t& b) {
    return std::string_view{(char*)b.bytes, b.len};
}

// http://alanxz.github.io/rabbitmq-c/docs/0.5.0/structamqp__envelope__t.html
struct envelope {
    amqp_envelope_t *envelope_;
    
    envelope(envelope&)=delete;
    envelope& operator=(envelope&)=delete;
    
    envelope(envelope&& src) {
        this->envelope_ = std::exchange(src.envelope_, nullptr);
    }
    
    envelope& operator=(envelope&& src) {
        this->envelope_ = std::exchange(src.envelope_, nullptr);
        return *this;
    }
    
    envelope() {
        envelope_ = new amqp_envelope_t();
    }
    
    ~envelope() {
        if(envelope_) {
            amqp_destroy_envelope(envelope_);
            delete envelope_;
        }
    }
    
    amqp_envelope_t& get() {
        return *envelope_;
    }
    
    amqp_channel_t get_channel() const {
        return envelope_->channel;
    }
    
    std::string_view get_consumer_tag() const {
        return to_string_view(envelope_->consumer_tag);
    }
    
    uint64_t get_delivery_tag() const {
        return envelope_->delivery_tag;
    }
    
    bool get_redelivered() const {
        return envelope_->redelivered;
    }
    
    std::string_view get_exchange() const {
        return to_string_view(envelope_->exchange);
    }
    
    std::string_view get_routing_key() const {
        return to_string_view(envelope_->routing_key);
    }
    
    std::string_view get_message_body() const {
        return to_string_view(envelope_->message.body);
    }
    
    const amqp_basic_properties_t& get_message_properties() const {
        return envelope_->message.properties;
    }
    
    std::string_view get_correlation_id() const {
        return to_string_view(envelope_->message.properties.correlation_id);
    }
};

// Multiple channels in rabbitmq-c is not very convenient, ignore it for now
// https://github.com/alanxz/rabbitmq-c/blob/master/examples/amqp_listenq.c
struct connection {
    amqp_connection_state_t conn_; // a pointer type
    amqp_socket_t *socket_;
    static constexpr amqp_channel_t channel_id_{1};
    
    connection(connection&)=delete;
    connection& operator=(connection&)=delete;
    
    connection(connection&& src) {
        this->conn_ = std::exchange(src.conn_, nullptr);
        this->socket_ = std::exchange(src.socket_, nullptr);
    }
    
    connection& operator=(connection&& src) {
        this->conn_ = std::exchange(src.conn_, nullptr);
        this->socket_ = std::exchange(src.socket_, nullptr);
        return *this;
    }
    
    connection(
        const std::string& queue_name,
        const std::string& hostname = "127.0.0.1",
        const int port = 5672,
        const std::string& username = "guest",
        const std::string& password = "guest",
        const std::string& vhost = "/",
        const bool use_tls = false
    ) {
        conn_ = amqp_new_connection();
        if(!conn_) {
            std::cerr << "amqp_new_connection: failed" << std::endl;
            std::abort();
        }
        
        if(use_tls) {
            socket_ = amqp_ssl_socket_new(conn_);
            amqp_ssl_socket_set_verify_peer(socket_, 1);
            amqp_ssl_socket_set_verify_hostname(socket_, 1);
        } else {
            socket_ = amqp_tcp_socket_new(conn_);
        }
        if(!socket_) {
            std::cerr << "ERROR: creating socket" << std::endl;
            std::abort();
        }
        
        int status = amqp_socket_open(socket_, hostname.c_str(), port);
        if(status) {
            std::cerr << "ERROR: opening socket" << std::endl;
            std::abort();
        }
        
        const auto login_status = amqp_login(
            conn_,
            vhost.c_str(),
            0, // channel_max
            131072, // frame_max
            0, // heartbeat
            AMQP_SASL_METHOD_PLAIN,
            username.c_str(),
            password.c_str()
        );
        if(AMQP_RESPONSE_NORMAL != login_status.reply_type) {
            std::cerr << "ERROR: amqp_login" << std::endl;
            std::abort();
        }
        
        // create channel
        amqp_channel_open(conn_, channel_id_);
        if(AMQP_RESPONSE_NORMAL != amqp_get_rpc_reply(conn_).reply_type) {
            std::cerr << "ERROR: amqp_channel_open" << std::endl;
            std::abort();
        }
        
        // bind queue
        amqp_basic_consume(
            conn_,
            channel_id_, // channel id
            amqp_cstring_bytes(queue_name.c_str()), // queue
            amqp_empty_bytes, // consumer_tag
            0, // no_local
            0, // no_ack, when set to 0, we need to send ack; otherwise, remove the message directly.
            0, // exclusive
            amqp_empty_table // arguments
        );
        if(AMQP_RESPONSE_NORMAL != amqp_get_rpc_reply(conn_).reply_type) {
            std::cerr << "ERROR: amqp_basic_consume" << std::endl;
            std::abort();
        }
    }
    
    ~connection() {
        if(!conn_) return;
        
        if(amqp_channel_close(conn_, channel_id_, AMQP_REPLY_SUCCESS).reply_type != AMQP_RESPONSE_NORMAL) {
            std::cerr << "amqp_channel_close: failed" << std::endl;
            std::abort();
        }
        
        if(amqp_connection_close(conn_, AMQP_REPLY_SUCCESS).reply_type != AMQP_RESPONSE_NORMAL) {
            std::cerr << "amqp_connection_close: failed" << std::endl;
            std::abort();
        }
        
        // also delete socket_
        int status = amqp_destroy_connection(conn_);
        if(status < 0) {
            std::cerr << "amqp_destroy_connection: failed" << std::endl;
            std::abort();
        }
    }
    
    envelope consume() {
        envelope res;
        
        amqp_maybe_release_buffers(conn_);
        
        auto status = amqp_consume_message(conn_, res.envelope_, nullptr, 0);
        if(AMQP_RESPONSE_NORMAL != status.reply_type) {
            throw std::runtime_error{"consume_message: Cannot read message from amqp"};
        }
        
        return res;
    }
    
    bool ack(const envelope& msg) {
        return AMQP_STATUS_OK == amqp_basic_ack(conn_, msg.get_channel(), msg.get_delivery_tag(), false);
    }
    
    // http://alanxz.github.io/rabbitmq-c/docs/0.2/structamqp__basic__properties__t.html
    // return true on success
    bool publish(
        const std::string& exchange,
        const std::string& routingkey,
        const std::string& messagebody,
        const amqp_basic_properties_t *properties
    ) {
        auto status = amqp_basic_publish(
            conn_,
            channel_id_, // channel id
            amqp_cstring_bytes(exchange.c_str()),
            amqp_cstring_bytes(routingkey.c_str()),
            0, // mandatory
            0, // immediate
            properties,
            amqp_cstring_bytes(messagebody.c_str())
        );
        return AMQP_RESPONSE_NORMAL != status;
    }
    
    // send plain text
    bool publish_text(
        const std::string& exchange,
        const std::string& routingkey,
        const std::string& messagebody
    ) {
        amqp_basic_properties_t props;
        props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
        props.content_type = amqp_cstring_bytes("text/plain");
        props.delivery_mode = 2; // persistent delivery mode
        
        return publish(exchange, routingkey, messagebody, &props);
    }
};

}
