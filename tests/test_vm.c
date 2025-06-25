#include "../vm.h"
#include "test_macros.h"
#include <stdlib.h>

TEST(test_vm_init) {
    VM vm;
    vm_init(&vm);
    ASSERT_EQ(vm.stack_top, vm.stack, "%p");
    ASSERT_EQ(vm.frame_count, 0, "%d");
}

int main(void) {
    RUN_TEST(test_vm_init);

    printf("✔︎ All vm tests passed.\n");
    return 0;
}
