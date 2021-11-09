# Disclaimer
This is only a toy project and is likely to be abandoned very soon. Use at your own risks.

# TODO
- Abstraction for the following types using template or virtual. And concepts for them.
    + broker message type: args, kwargs, task_id, task
    + broker backend: `consume` and `ack` method
    + result message type: json-serializable
    + result backend: `store` method
- Worker needs to check the `task` field in the header.

# Requirements
- celery `pip install "celery[librabbitmq,redis]"` (or install `celery`, `ampq`, `redis` separately)
- rabbitmq-server
- rabbitmq-c
- redis
- hiredis
- jsoncpp

# Setting up celery
Install celery
https://docs.celeryproject.org/en/stable/getting-started/first-steps-with-celery.html

Start the RabbitMQ server
https://docs.celeryproject.org/en/stable/getting-started/backends-and-brokers/rabbitmq.html#broker-rabbitmq

Start the redis server
https://docs.celeryproject.org/en/stable/getting-started/backends-and-brokers/redis.html#broker-redis

# Testing
Run the python celery worker

```
celery -A tasks worker --loglevel=INFO
```

Or run the C++ celery worker

```
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
../bin_release/celery_add
```

Submit a job

```
python run_add.py
```

# Some development notes
## See what is in redis
```
redis-cli

KEYS *
GET some_key

# example
GET celery-task-meta-0d5e9e01-d676-48b1-9c7f-60c74cb60ad0
"{\"status\": \"SUCCESS\", \"result\": 8, \"traceback\": null, \"children\": [], \"date_done\": \"2021-11-09T04:25:37.443814\", \"task_id\": \"0d5e9e01-d676-48b1-9c7f-60c74cb60ad0\"}"

# need double quotes for spaces
SET some_key some_data

PUBSUB CHANNELS

PUBLISH channel message

# delete all keys in all db
redis-cli FLUSHALL

# delete all keys in the specified db
redis-cli -n <database_number> FLUSHDB
```

## See what is in rabbitmq
```
sudo rabbitmqctl status

sudo rabbitmqctl list_users
sudo rabbitmqctl list_vhosts
sudo rabbitmqctl list_queues
```

```
# https://www.rabbitmq.com/management-cli.html
sudo rabbitmq-plugins enable rabbitmq_management

wget http://localhost:15672/cli/rabbitmqadmin

sudo python rabbitmqadmin list exchanges
sudo python rabbitmqadmin list queues

sudo python rabbitmqadmin get queue=celery count=10
+-------------+----------+---------------+-----------------------------------------------------------------------------------+---------------+------------------+-------------+
| routing_key | exchange | message_count |                                      payload                                      | payload_bytes | payload_encoding | redelivered |
+-------------+----------+---------------+-----------------------------------------------------------------------------------+---------------+------------------+-------------+
| celery      |          | 0             | [[4, 4], {}, {"callbacks": null, "errbacks": null, "chain": null, "chord": null}] | 81            | string           | False       |
+-------------+----------+---------------+-----------------------------------------------------------------------------------+---------------+------------------+-------------+
# the payload is [args, kwargs, embed], see Celery Message Protocol

# https://www.rabbitmq.com/management.html#http-api
# https://rawcdn.githack.com/rabbitmq/rabbitmq-server/v3.9.8/deps/rabbitmq_management/priv/www/api/index.html
# endpoint is /api/queues/$vhost/$queue_name/get, '%2F' is /
curl -u 'guest:guest' -H 'content-type:application/json' -X POST localhost:15672/api/queues/%2F/celery/get -d '{"count":1,"ackmode":"ack_requeue_true","encoding":"auto"}'

[{"payload_bytes":81,"redelivered":false,"exchange":"","routing_key":"celery","message_count":0,"properties":{"reply_to":"53c62f3b-02d5-35e2-9de6-6ec08c1e906d","correlation_id":"0d5e9e01-d676-48b1-9c7f-60c74cb60ad0","priority":0,"delivery_mode":2,"headers":{"argsrepr":"(4, 4)","eta":"undefined","expires":"undefined","group":"undefined","group_index":"undefined","id":"0d5e9e01-d676-48b1-9c7f-60c74cb60ad0","kwargsrepr":"{}","lang":"py","origin":"gen917383@ljn","parent_id":"undefined","retries":0,"root_id":"0d5e9e01-d676-48b1-9c7f-60c74cb60ad0","shadow":"undefined","task":"tasks.add","timelimit":["undefined","undefined"]},"content_encoding":"utf-8","content_type":"application/json"},"payload":"[[4, 4], {}, {\"callbacks\": null, \"errbacks\": null, \"chain\": null, \"chord\": null}]","payload_encoding":"string"}]

```

# References
- https://blog.petrzemek.net/2017/06/25/consuming-and-publishing-celery-tasks-in-cpp-via-amqp/
- Celery Message Protocol https://docs.celeryproject.org/en/latest/internals/protocol.html
- Result https://docs.celeryproject.org/en/latest/_modules/celery/backends/rpc.html#RPCBackend
