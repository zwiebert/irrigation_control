if(NOT UNIT_TESTING) 
set(srcs
"../webapp/build/wapp.html.gz.c"
"../webapp/build/wapp.js.gz.c"
"../webapp/build/wapp.css.gz.c"
"../webapp/build/wapp.js.map.gz.c" "../webapp/build/wapp.js.map.br.c"
"../webapp/build/wapp.css.map.gz.c" "../webapp/build/wapp.css.map.br.c"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS . "./include"
   )
 
 component_compile_options(${comp_compile_opts})
component_compile_features(${comp_compile_feats})
component_compile_features(${comp_compile_feats})
component_compile_features(${comp_compile_feats})
 
 add_custom_command(
     OUTPUT "${COMPONENT_DIR}/build/wapp.html.gz.c"
            "${COMPONENT_DIR}/build/wapp.js.gz.c"
            "${COMPONENT_DIR}/build/wapp.css.gz.c"
            "${COMPONENT_DIR}/build/wapp.js.map.gz.c" "${COMPONENT_DIR}/build/wapp.js.map.br.c"
            "${COMPONENT_DIR}/build/wapp.css.map.gz.c" "${COMPONENT_DIR}/build/wapp.css.map.br.c"
             ${COMPONENT_DIR}/_force.h   
     COMMAND make all -C ${COMPONENT_DIR}
     VERBATIM)
  
add_custom_target(content_files ALL  DEPENDS ${COMPONENT_DIR}/_force.h )
add_dependencies(  ${COMPONENT_LIB} content_files)

endif()