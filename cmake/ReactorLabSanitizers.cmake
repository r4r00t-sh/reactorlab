# Optional AddressSanitizer / UndefinedBehaviorSanitizer flags.

function(reactorlab_enable_sanitizers target)
  if(NOT REACTORLAB_ENABLE_SANITIZERS)
    return()
  endif()
  if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    message(WARNING "Sanitizers requested but compiler is not GCC/Clang")
    return()
  endif()

  target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(${target} PRIVATE -fsanitize=address,undefined)
endfunction()
