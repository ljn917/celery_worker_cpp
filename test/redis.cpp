#include "redis/redis.hpp"

int main() {
    redis::client c;
    
    {
        std::string key{"testing_key"};
        std::string value{"testing_key value"};
        c.set(key, value);
        auto val = c.get(key);
        std::cout << "val=" << val.value() << std::endl;
        if(val.value() != value){
            return -1;
        }
    }
    
    {
        // get something non-existent
        auto val = c.get("dasfdf934hwgf8wperh938hf89hr9hfhwwahgh984329hf");
        if(val.has_value()) {
            return -1;
        }
    }
    
    {
        std::vector<std::string> keys = {"abf", "dasdf", "dsfd"};
        for(const auto& key: keys) {
            c.set(key, "somevalue");
        }
        size_t cnt = c.del(keys);
        if(cnt != keys.size()) {
            return -1;
        }
    }
    
    {
        auto c2 = std::move(c);
    }
    
    return 0;
}
