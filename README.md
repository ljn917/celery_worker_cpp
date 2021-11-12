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

Chain (not supported)

```
[{"payload_bytes":536,"redelivered":false,"exchange":"","routing_key":"celery","message_count":0,"properties":{"reply_to":"21cea31e-2ea0-3aca-a1e3-1db8da88fc7c","correlation_id":"350612ec-80a7-4e18-9fcf-9ad1bd15dfed","priority":0,"delivery_mode":2,"headers":{"argsrepr":"(4, 10)","eta":"undefined","expires":"undefined","group":"undefined","group_index":"undefined","id":"350612ec-80a7-4e18-9fcf-9ad1bd15dfed","kwargsrepr":"{}","lang":"py","origin":"gen1258261@ljn","parent_id":"undefined","retries":0,"root_id":"350612ec-80a7-4e18-9fcf-9ad1bd15dfed","shadow":"undefined","task":"tasks.add","timelimit":["undefined","undefined"]},"content_encoding":"utf-8","content_type":"application/json"},"payload":"[[4, 10], {}, {\"callbacks\": null, \"errbacks\": null, \"chain\": [{\"task\": \"tasks.add\", \"args\": [30], \"kwargs\": {}, \"options\": {\"task_id\": \"3a0b08eb-888b-4af0-aa4e-2b0c35d91055\", \"reply_to\": \"21cea31e-2ea0-3aca-a1e3-1db8da88fc7c\"}, \"subtask_type\": null, \"chord_size\": null, \"immutable\": false}, {\"task\": \"tasks.add\", \"args\": [20], \"kwargs\": {}, \"options\": {\"task_id\": \"86dc52ea-86e7-4994-9ad1-8d9fda3e27cf\", \"reply_to\": \"21cea31e-2ea0-3aca-a1e3-1db8da88fc7c\"}, \"subtask_type\": null, \"chord_size\": null, \"immutable\": false}], \"chord\": null}]","payload_encoding":"string"}]

# graph
350612ec-80a7-4e18-9fcf-9ad1bd15dfed(2)
     86dc52ea-86e7-4994-9ad1-8d9fda3e27cf(1)
          3a0b08eb-888b-4af0-aa4e-2b0c35d91055(0)

# redis results
127.0.0.1:6379> get celery-task-meta-350612ec-80a7-4e18-9fcf-9ad1bd15dfed
"{\"status\": \"SUCCESS\", \"result\": 14, \"traceback\": null, \"children\": [[[\"86dc52ea-86e7-4994-9ad1-8d9fda3e27cf\", null], null]], \"date_done\": \"2021-11-11T20:44:26.380804\", \"task_id\": \"350612ec-80a7-4e18-9fcf-9ad1bd15dfed\"}"
127.0.0.1:6379> get celery-task-meta-86dc52ea-86e7-4994-9ad1-8d9fda3e27cf
"{\"status\": \"SUCCESS\", \"result\": 34, \"traceback\": null, \"children\": [[[\"3a0b08eb-888b-4af0-aa4e-2b0c35d91055\", null], null]], \"date_done\": \"2021-11-11T20:44:26.401050\", \"parent_id\": \"350612ec-80a7-4e18-9fcf-9ad1bd15dfed\", \"task_id\": \"86dc52ea-86e7-4994-9ad1-8d9fda3e27cf\"}"
127.0.0.1:6379> get celery-task-meta-3a0b08eb-888b-4af0-aa4e-2b0c35d91055
"{\"status\": \"SUCCESS\", \"result\": 64, \"traceback\": null, \"children\": [], \"date_done\": \"2021-11-11T20:44:26.402422\", \"parent_id\": \"86dc52ea-86e7-4994-9ad1-8d9fda3e27cf\", \"task_id\": \"3a0b08eb-888b-4af0-aa4e-2b0c35d91055\"}"
```

Callback

```
[{"payload_bytes":205,"redelivered":false,"exchange":"","routing_key":"celery","message_count":0,"properties":{"reply_to":"d6fcc464-fe94-3d0a-8ab2-1f4a8c9aa164","correlation_id":"2d1db8ad-1d2f-4423-9c3f-b85e43cf7f6d","priority":0,"delivery_mode":2,"headers":{"argsrepr":"(4, 4)","eta":"undefined","expires":"undefined","group":"undefined","group_index":"undefined","id":"2d1db8ad-1d2f-4423-9c3f-b85e43cf7f6d","kwargsrepr":"{}","lang":"py","origin":"gen1263097@ljn","parent_id":"undefined","retries":0,"root_id":"2d1db8ad-1d2f-4423-9c3f-b85e43cf7f6d","shadow":"undefined","task":"tasks.add","timelimit":["undefined","undefined"]},"content_encoding":"utf-8","content_type":"application/json"},"payload":"[[4, 4], {}, {\"callbacks\": [{\"task\": \"tasks.add\", \"args\": [10], \"kwargs\": {}, \"options\": {}, \"subtask_type\": null, \"immutable\": false, \"chord_size\": null}], \"errbacks\": null, \"chain\": null, \"chord\": null}]","payload_encoding":"string"}]

# redis results
127.0.0.1:6379> get celery-task-meta-209954d8-5285-4b0e-a28f-5005c6598d3a
"{\"status\": \"SUCCESS\", \"result\": 18, \"traceback\": null, \"children\": [], \"date_done\": \"2021-11-11T21:34:49.360491\", \"parent_id\": \"2d1db8ad-1d2f-4423-9c3f-b85e43cf7f6d\", \"task_id\": \"209954d8-5285-4b0e-a28f-5005c6598d3a\"}"
127.0.0.1:6379> get celery-task-meta-2d1db8ad-1d2f-4423-9c3f-b85e43cf7f6d
"{\"status\": \"SUCCESS\", \"result\": 8, \"traceback\": null, \"children\": [[[\"209954d8-5285-4b0e-a28f-5005c6598d3a\", null], null]], \"date_done\": \"2021-11-11T21:34:49.358510\", \"task_id\": \"2d1db8ad-1d2f-4423-9c3f-b85e43cf7f6d\"}"
```


# References
- https://blog.petrzemek.net/2017/06/25/consuming-and-publishing-celery-tasks-in-cpp-via-amqp/
- Celery Message Protocol https://docs.celeryproject.org/en/latest/internals/protocol.html
- Result https://docs.celeryproject.org/en/latest/_modules/celery/backends/rpc.html#RPCBackend
- Chain https://github.com/celery/celery/blob/master/celery/canvas.py#L588
