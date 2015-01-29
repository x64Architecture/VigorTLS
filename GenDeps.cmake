add_custom_target(includes
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/crypto/objects"
    COMMAND perl objects.pl objects.txt obj_mac.num ${PROJECT_SOURCE_DIR}/include/openssl/obj_mac.h
    COMMAND perl obj_dat.pl ${PROJECT_SOURCE_DIR}/include/openssl/obj_mac.h obj_dat.h
)
