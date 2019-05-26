

// comment

/*
    multiline comment
// inner comment
    /*
        Nested block comment
    */
*/

foo: int = 4;

SOME_CONST :: 2;

Some_Struct :: struct {
    flags: int = 0;
    foo: int;
    some_variable: bool;
};

test :: () -> string {
    x: int = 4;
    {
        hello: string = "sailor";
    }
    a: int = x;
    y: int = 4; // @todo when removed undeclared identifier below
    b: int = x * y;
    c: int = x / y * x;
    d: int = (c * x) % 2;

    e: int = x++;
    f: int = --x;

    g: bool = true;
    h: bool = !g;

    some_struct: Some_Struct = null;
    j: bool = some_struct.some_variable;
    k: bool = some_struct.some_variable;
    //l: bool = some_struct.a.b.c.d.e.f.g;
    m: double = 124.0;
    n: char = #char "\n";
    o: bool = xx 1;
    //p: ^int = 0xB8000;
    //q := 123;
    q : int = 123;
    q = 55;
    r: int = a + 5;
    s: bool = a < 5;

    // test identifier aligner
    //some_struct: Some_Struct = null;
    a_struct:    Some_Struct = null;
    b/  _struct: Some_Struct = null;
    c/
        _struct: Some_Struct = null;
    d/  _struct: Some_Struct = null;

    e/  _some_struct: Some_Struct = null;
    e/  _e/  _struct: Some_Struct = null;

    return "Hello Sailor";
}

test_print :: () -> bool {
    hello: string = "sailor";

    x: int = 1;
    a: bool = true;
    while a {
        hello: string = "sailor";
        x++;
        a = !a;
    }

    i: int = 0;
    while a && (i < 10) {
        hello: string = "Hello Sailor!";
        i++;
    }

    return true;
}

test_return :: () -> int {
    f: float = 123.0;
    g := 123.0;
    h := #char "!";
    i := "Hello Sailor!";
    return 123;
}

test_if :: () -> bool {
    x: bool = cast(bool) true;
    if x == true {
        return true;
    } else {
        return false;
    }
    y: int = 10;
    if y > 15 {
        return true;
    } else if y < 9 {
        return false;
    } else {
        return true;
    }
}

test_params :: (x: int, y: int) -> int {
    z: int = x * y;
    return z;
}

printf :: (_string: ^char, args: ..) -> void #foreign "printf" {}

fibonacci :: (x: int) -> int {
    if x == 0 || x == 1 return x;
    return fibonacci(x - 1) + fibonacci(x - 2);
}

main :: () -> int {
    printf("Hello Sailor!\n");
    x: int = 5;
    printf("test x = %d; %s\n", x, "Hello Sailor!!!");
    defer test();
    test_print();

    // test printf
    i: int = 0;
    while i < 10 {
        printf("Hello Sailor!\n");
        i++;
    }

    f: int = fibonacci(10);
    printf("Fibonacci of 10 is %d", f);
}


// #run main();

