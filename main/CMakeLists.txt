idf_component_register(SRCS "main.c" "wifi_start.c" "http_server.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/cacert.pem"
                                   "certs/prvtkey.pem")
