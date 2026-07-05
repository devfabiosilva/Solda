add_executable(test_dirty_tracking test_dirty_tracking.c)
target_link_libraries(test_dirty_tracking
    solda_core      # <- Solda Core
    solda_asserts   # <- Tests
)
add_test(NAME dirty_tracking COMMAND test_dirty_tracking)

add_executable(test_manipulator test_manipulator.c)
target_link_libraries(test_manipulator
    solda_core
    solda_asserts
)
add_test(NAME manipulator COMMAND test_manipulator)