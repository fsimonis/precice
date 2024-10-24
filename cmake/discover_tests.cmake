macro(start_file)
  set(script "")
endmacro()

function(add_command NAME TEST_NAME)
  set(args "")
  foreach(arg ${ARGN})
    if(arg MATCHES "[^-./:a-zA-Z0-9_]")
      string(APPEND args " [==[${arg}]==]")
    else()
      string(APPEND args " ${arg}")
    endif()
  endforeach()
  string(APPEND script "${NAME}(${TEST_NAME} ${args})\n")
  set(script "${script}" PARENT_SCOPE)
endfunction()

function(add_test NAME)
  string(REPLACE "/" "_" test_dir "${NAME}")
  set(test_dir "${TEST_DIR}/${test_dir}")

  if (NOT EXISTS test_dir)
    FILE(MAKE_DIRECTORY ${test_dir})
  endif()

  foreach(kw Ginkgo Sockets MPIPorts MPISinglePorts MPI)
    if(NAME MATCHES ${kw})
      list(APPEND labels ${kw})
    endif()
  endforeach()
  if(labels)
    list(PREPEND labels LABELS)
  endif()

  add_command(add_test
    "[=[precice.${NAME}]=]"
    "/usr/bin/mpiexec" "-n" "4" "--map-by" ":OVERSUBSCRIBE"
   ${_TEST_EXECUTABLE} "--run_test=${NAME}" "--log_level=message"
 )
  add_command(set_tests_properties
    "[=[precice.${NAME}]=]"
    PROPERTIES
    ENVIRONMENT "OMP_NUM_THREADS=2"
    TIMEOUT "10"
    WORKING_DIRECTORY "${test_dir}"
    ${labels}
 )

 set(script "${script}" PARENT_SCOPE)
endfunction()

macro(flush_file)
  file(WRITE "${_TEST_FILE}" "${script}")
  set(flush_tests_MODE APPEND PARENT_SCOPE)
  unset(script)
endmacro()


function(discover_tests)
  cmake_parse_arguments(
    PARSE_ARGV
    0
    ""
    ""
    "TEST_EXECUTABLE;TEST_DIR;TEST_FILE"
    ""
  )

  execute_process(
    COMMAND "${_TEST_EXECUTABLE}" --list_units
    WORKING_DIRECTORY "${_TEST_DIR}"
    TIMEOUT 4
    OUTPUT_VARIABLE output
    RESULT_VARIABLE result
  )

  if(NOT ${result} EQUAL 0)
    string(REPLACE "\n" "\n    " output "${output}")
    message(FATAL_ERROR
      "Error running test executable.\n"
      "  Path: '${_TEST_EXECUTABLE}'\n"
      "  Working directory: '${_TEST_DIR}'\n"
      "  Result: ${result}\n"
      "  Output:\n"
      "    ${output}\n"
    )
  endif()

  # Turn into a CMake list
  string(REPLACE "\n" ";" output "${output}")

  start_file()
  foreach(test IN LISTS output)
    if (NOT test STREQUAL "")
      add_test("${test}")
    endif()
  endforeach()
  flush_file()
endfunction()


if(CMAKE_SCRIPT_MODE_FILE)
  discover_tests(
    TEST_EXECUTABLE "${TEST_EXECUTABLE}"
    TEST_DIR "${TEST_DIR}"
    TEST_FILE "${TEST_FILE}"
  )
endif()
