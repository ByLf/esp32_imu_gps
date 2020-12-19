idf_component_register(SRCS "main.cpp" "wifi_start.cpp" "http_server.cpp" "gps_server.cpp"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/cacert.pem"
                                   "certs/prvtkey.pem")
