add_executable(test_db_service test_db_service.c)
target_link_libraries(test_db_service
    solda_core      # <- Solda Core
    solda_asserts   # <- Tests
)
add_test(NAME db_service COMMAND test_db_service)

add_executable(test_manipulator test_manipulator.c)
target_link_libraries(test_manipulator
    solda_core
    solda_asserts
)
add_test(NAME manipulator COMMAND test_manipulator)