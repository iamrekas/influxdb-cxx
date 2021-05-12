# CMake generated Testfile for 
# Source directory: /workspace/influxdb-cxx
# Build directory: /workspace/influxdb-cxx/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(testUdp "/workspace/influxdb-cxx/build/bin/testUdp")
set_tests_properties(testUdp PROPERTIES  TIMEOUT "60")
add_test(testPoint "/workspace/influxdb-cxx/build/bin/testPoint")
set_tests_properties(testPoint PROPERTIES  TIMEOUT "60")
add_test(testHttp "/workspace/influxdb-cxx/build/bin/testHttp")
set_tests_properties(testHttp PROPERTIES  TIMEOUT "60")
add_test(testQuery "/workspace/influxdb-cxx/build/bin/testQuery")
set_tests_properties(testQuery PROPERTIES  TIMEOUT "60")
add_test(testFactory "/workspace/influxdb-cxx/build/bin/testFactory")
set_tests_properties(testFactory PROPERTIES  TIMEOUT "60")
