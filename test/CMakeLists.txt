add_executable( rabbitmq rabbitmq.cpp )
target_link_libraries( rabbitmq ${LIBS} )
add_test(
    NAME rabbitmq
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ./rabbitmq
)

add_executable( redis redis.cpp )
target_link_libraries( redis ${LIBS} )
add_test(
    NAME redis
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ./redis
)

add_executable( celery_add celery_add.cpp )
target_link_libraries( celery_add ${LIBS} )

add_test(
    NAME celery_add
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ../test/test_celery_add.sh ${EXECUTABLE_OUTPUT_PATH}
)
