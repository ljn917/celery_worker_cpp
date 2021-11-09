find_path(Rabbitmqc_INCLUDE_DIRS
    NAMES amqp.h
    DOC "RabbitMQ-c include dir"
    HINTS
        ${RABBITMQC_DIR}/include/rabbitmq-c
        ${RABBITMQC_DIR}/include
        /usr/include/rabbitmq-c
        /usr/include
)

find_library(Rabbitmqc_LIBRARIES
    NAMES rabbitmq
    DOC "RabbitMQ-c library"
    HINTS
        ${RABBITMQC_DIR}/lib
        /usr/lib64
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rabbitmqc
    DEFAULT_MSG
    Rabbitmqc_INCLUDE_DIRS
    Rabbitmqc_LIBRARIES
)

mark_as_advanced(Rabbitmqc_INCLUDE_DIRS Rabbitmqc_LIBRARIES)

if(Rabbitmqc_FOUND)
    if(NOT EXISTS ${Rabbitmqc_INCLUDE_DIRS}/amqp_ssl_socket.h)
        message(FATAL_ERROR "Error: Rabbitmqc lib does not have SSL support! Please upgrade.....stop")
    endif()
    add_library(Rabbitmqc::Rabbitmqc INTERFACE IMPORTED)
    set_target_properties(Rabbitmqc::Rabbitmqc
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES    "${Rabbitmqc_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES                    "${Rabbitmqc_LIBRARIES}"
    )
endif(Rabbitmqc_FOUND)
