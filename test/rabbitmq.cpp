#include "rabbitmq/rabbitmq.hpp"

int main() {
    std::string exchange{""};
    std::string queue{"celery"};
    std::string message{"Test message ABC"};
    
    auto conn = rabbitmq::connection(queue);
    
    conn.publish_text(exchange, queue, message);
    
    auto envelope = conn.consume();
    conn.ack(envelope);
    std::cout << envelope.get_routing_key() << std::endl;
    std::cout << envelope.get_message_body() << std::endl;
    
    if(envelope.get_routing_key() != queue) {
        return -1;
    }
    if(envelope.get_message_body() != message) {
        return -1;
    }
    
    // test move
    auto envelope2 = std::move(envelope);
    auto conn2 = std::move(conn);
    
    return 0;
}
