# Specify the source files of multi-files tests
set(activity-lifecycle_sources ../catch_simgrid.cpp)
foreach(x sleep exec io comm comm_direct test-wait)
  set(activity-lifecycle_sources ${activity-lifecycle_sources} activity-lifecycle/testing_${x}.cpp)
endforeach()

foreach(x actor actor-autorestart actor-suspend actor-destroyed-with-mutex
        activity-lifecycle
        comm-get-sender comm-pt2pt comm-fault-scenarios
        cloud-interrupt-migration cloud-two-execs
      	monkey-masterworkers monkey-semaphore
        concurrent_rw
        dag-incomplete-simulation dependencies
        host-on-off host-on-off-actors host-on-off-disks host-on-off-recv host-multicore-speed-file
        io-set-bw io-stream
        basic-link-test basic-parsing-test evaluate-get-route-time evaluate-parse-time is-router
        storage_client_server listen_async pid
        trace-integration
        seal-platform
        vm-live-migration vm-suicide issue71)

  if(NOT DEFINED ${x}_sources)
      set(${x}_sources ${x}/${x}.cpp)
  endif()

  add_executable       (${x}  EXCLUDE_FROM_ALL ${${x}_sources})
  target_link_libraries(${x}  simgrid)
  set_target_properties(${x}  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${x})
  add_dependencies(tests ${x})

  foreach(file ${${x}_sources})
    set(teshsuite_src  ${teshsuite_src} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  endforeach()

  unset(${x}_sources)
endforeach()
set_property(TARGET activity-lifecycle APPEND PROPERTY INCLUDE_DIRECTORIES "${INTERNAL_INCLUDES}") # for <catch.hpp> and catch_simgrid


## Add the tests.
## Some need to be run with all factories, some don't need tesh to run
foreach(x actor actor-autorestart actor-suspend activity-lifecycle actor-destroyed-with-mutex comm-get-sender
        cloud-interrupt-migration cloud-two-execs concurrent_rw dag-incomplete-simulation dependencies io-set-bw io-stream
	      vm-live-migration vm-suicide)
  set(tesh_files    ${tesh_files}    ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.tesh)
  ADD_TESH_FACTORIES(tesh-s4u-${x} "*" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/s4u/${x} --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x} --setenv platfdir=${CMAKE_HOME_DIRECTORY}/examples/platforms --cd ${CMAKE_BINARY_DIR}/teshsuite/s4u/${x} ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x}/${x}.tesh)
endforeach()

foreach(x basic-link-test basic-parsing-test host-on-off host-on-off-actors host-on-off-disks host-on-off-recv
        comm-fault-scenarios host-multicore-speed-file is-router listen_async
        monkey-masterworkers monkey-semaphore
        pid storage_client_server trace-integration seal-platform issue71)
  set(tesh_files    ${tesh_files}    ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.tesh)
  ADD_TESH(tesh-s4u-${x}
           --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/s4u/${x}
           --setenv srcdir=${CMAKE_CURRENT_SOURCE_DIR}/${x}
           --setenv rootdir=${CMAKE_HOME_DIRECTORY}
           --setenv platfdir=${CMAKE_HOME_DIRECTORY}/examples/platforms
           --cd ${CMAKE_BINARY_DIR}/teshsuite/s4u/${x}
           ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x}/${x}.tesh)
endforeach()

# Python tesh tests
foreach(x monkey-masterworkers)
  set(tesh_files    ${tesh_files}    ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.py.tesh)
  set(teshsuite_src ${teshsuite_src} ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.py)

  if(enable_python)
    ADD_TESH(tesh-python-${x}
             --setenv srcdir=${CMAKE_CURRENT_SOURCE_DIR}/${x}
             --setenv pythoncmd=${PYTHON_EXECUTABLE}
             --setenv LD_LIBRARY_PATH=${TESH_LIBRARY_PATH}
             --setenv PYTHONPATH=${CMAKE_BINARY_DIR}/lib
             --setenv platfdir=${CMAKE_HOME_DIRECTORY}/examples/platforms
             --cd ${CMAKE_BINARY_DIR}/teshsuite/s4u/${x}
             ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x}/${x}.py.tesh)

  endif()
endforeach()


# Monkey tests are launched directly, not with tesh
set(_monkey-semaphore_disable_python 1) # Semaphore not exposed to python
foreach(x  monkey-masterworkers monkey-semaphore)
  ADD_TEST(monkey-s4u-${x} "${PYTHON_EXECUTABLE}" ${CMAKE_HOME_DIRECTORY}/tools/simgrid-monkey -- ${VALGRIND_WRAPPER_UNBOXED} ${CMAKE_BINARY_DIR}/teshsuite/s4u/${x}/${x})
  if(enable_python)
    if(NOT DEFINED _${x}_disable_python)
      ADD_TEST(monkey-python-${x} "${PYTHON_EXECUTABLE}" ${CMAKE_HOME_DIRECTORY}/tools/simgrid-monkey "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.py)
      set_tests_properties(monkey-python-${x} PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/lib")
    endif()
  endif()
endforeach()

# The output is not relevant
ADD_TEST(tesh-s4u-comm-pt2pt    ${CMAKE_BINARY_DIR}/teshsuite/s4u/comm-pt2pt/comm-pt2pt    ${CMAKE_HOME_DIRECTORY}/examples/platforms/cluster_backbone.xml)

if(enable_coverage)
  foreach (example evaluate-get-route-time evaluate-parse-time)
    ADD_TEST(cover-${example} ${CMAKE_CURRENT_BINARY_DIR}/${example}/${example} ${CMAKE_HOME_DIRECTORY}/examples/platforms/g5k.xml)
  endforeach()
endif()

# NS-3 specific tests
if(SIMGRID_HAVE_NS3)
  foreach(x ns3-simultaneous-send-rcv ns3-from-src-to-itself)
    add_executable       (${x}  EXCLUDE_FROM_ALL ${x}/${x}.cpp)
    target_link_libraries(${x}  simgrid)
    set_target_properties(${x}  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${x})
    add_dependencies(tests-ns3 ${x})
    ADD_TESH(tesh-s4u-${x} --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x} --setenv platfdir=${CMAKE_HOME_DIRECTORY}/examples/platforms --cd ${CMAKE_BINARY_DIR}/teshsuite/s4u/${x} ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x}/${x}.tesh)
  endforeach()
endif()

# BMF specific tesh tests
foreach(x cloud-two-execs concurrent_rw)
  set(tesh_files    ${tesh_files}    ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}-bmf.tesh)

  if(Eigen3_FOUND)
    ADD_TESH(tesh-s4u-${x}-bmf
             --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/s4u/${x}
             --setenv srcdir=${CMAKE_CURRENT_SOURCE_DIR}/${x}
             --setenv rootdir=${CMAKE_HOME_DIRECTORY}
             --setenv platfdir=${CMAKE_HOME_DIRECTORY}/examples/platforms
             --cd ${CMAKE_BINARY_DIR}/teshsuite/s4u/${x}
             ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/${x}/${x}-bmf.tesh)

  endif()
endforeach()

foreach(x ns3-simultaneous-send-rcv ns3-from-src-to-itself)
  set(teshsuite_src ${teshsuite_src} ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.cpp)
  set(tesh_files    ${tesh_files}    ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.tesh)
endforeach()

# test for code coverage
ADD_TEST(test-help-version ${VALGRIND_WRAPPER_UNBOXED} ${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test/basic-parsing-test
  --version --help --help-aliases --help-models --help-tracing)
ADD_TEST(test-help-logs    ${VALGRIND_WRAPPER_UNBOXED} ${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test/basic-parsing-test
  --help-logs --help-log-categories)

ADD_TESH(tesh-parser-bypass   --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/basic-parsing-test --setenv srcdir=${CMAKE_HOME_DIRECTORY} basic-parsing-test-bypass.tesh)
ADD_TESH(tesh-parser-sym-full --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/s4u/basic-parsing-test basic-parsing-test-sym-full.tesh)

ADD_TEST(tesh-parser-full-links01    ${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test/basic-parsing-test ${CMAKE_HOME_DIRECTORY}/teshsuite/platforms/two_clusters.xml FULL_LINK)
ADD_TEST(tesh-parser-full-links02    ${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test/basic-parsing-test ${CMAKE_HOME_DIRECTORY}/teshsuite/platforms/two_clusters_one_name.xml FULL_LINK)
ADD_TEST(tesh-parser-one-link-g5k    ${CMAKE_BINARY_DIR}/teshsuite/s4u/basic-parsing-test/basic-parsing-test ${CMAKE_HOME_DIRECTORY}/examples/platforms/g5k.xml ONE_LINK)

set(teshsuite_src ${teshsuite_src}  ${CMAKE_SOURCE_DIR}/teshsuite/catch_simgrid.hpp PARENT_SCOPE)
set(tesh_files    ${tesh_files}     ${CMAKE_CURRENT_SOURCE_DIR}/basic-parsing-test/basic-parsing-test-sym-full.tesh
                                    ${CMAKE_CURRENT_SOURCE_DIR}/basic-parsing-test/basic-parsing-test-bypass.tesh PARENT_SCOPE)
set(xml_files     ${xml_files}      ${CMAKE_CURRENT_SOURCE_DIR}/activity-lifecycle/testing_platform.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1.0-hbp1.0-hbp1.0.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1.0-hbp3.0-hbp4.0.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1.5-hbp1.5.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c0s0-c0s1.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c0s0-c1s0.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c0s1-c0s2.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c0s1-c2s2.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c0s2-c1s0.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c0s2-c1s1.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c1s1-c1s2.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp1-c1s1-c3s2.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/trace-integration/test-hbp2.5-hbp1.5.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/vm-live-migration/platform.xml
                                    ${CMAKE_CURRENT_SOURCE_DIR}/issue71/platform_bad.xml
				    PARENT_SCOPE)
