idf_component_register(SRCS "wifiStack.c"
						INCLUDE_DIRS "." 
                        PRIV_REQUIRES esp_netif driver nvs_flash
					   )
					
set_target_properties(${COMPONENT_LIB} PROPERTIES  COMPILE_FLAGS -Wunused-function)	
