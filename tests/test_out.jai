hello sailor// For Assassins Creed Unity they made their own programming language that compiles to C.
hello sailor is this the end of the buffer or not



printf :: (fmt: ^char, args: ..) -> void #foreign "printf" {}
scanf  :: (fmt: ^char, args: ..) -> void #foreign {}

main :: () -> int {
    defer {
        defer printf("Demo 2 \n");
        printf("Demo finished\n");
    }
    defer printf("middle\n");
    printf("Demo\n");

    printf("Enter your age: ");
    age: int = -1;
    scanf("%d", &age);
    printf("age: %d\n", age);
}
