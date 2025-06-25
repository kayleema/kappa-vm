cmake_minimum_required(VERSION 3.30)
project(kappavm C)

set(CMAKE_C_STANDARD 11)

set(VM_SOURCES
    vm.c
    chunk.c
    assembler.c
    vm.h
    value.h
    common.h
    opcode.h
    chunk.h
    assembler.h
)

add_executable(kappavm main.c ${VM_SOURCES})

enable_testing()

add_executable(vm_tests
        tests/test_vm.c
        tests/test_macros.h
        ${VM_SOURCES}
)
add_test(NAME vm_test COMMAND vm_tests)

add_executable(opcode_tests
        tests/test_opcode.c
        tests/test_macros.h
        ${VM_SOURCES}
)
add_test(NAME opcode_test COMMAND opcode_tests)

add_executable(execution_tests
        tests/test_execution.c
        tests/test_macros.h
        ${VM_SOURCES}
)
add_test(NAME execution_test COMMAND execution_tests)

add_executable(chunk_tests
        tests/test_chunk.c
        tests/test_macros.h
        ${VM_SOURCES}
)
add_test(NAME chunk_test COMMAND chunk_tests)

add_executable(cli_test
        tests/test_cli.c
        chunk.c
        assembler.c
        value.h common.h opcode.h chunk.h assembler.h
)
add_test(NAME cli_test COMMAND cli_test)

add_executable(assembler_tests
        tests/test_assembler.c
        assembler.c
        chunk.c
        ${VM_SOURCES}
)
add_test(NAME assembler_tests COMMAND assembler_tests)
