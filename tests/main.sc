
// This is a line comment

/* This is a block comment */

func test(foo f64) -> f64;

func main(x f64) -> f64 {
    ;
    var y: f64 = 1.0;
    loop {
        if (x + y != 0.0) {
            break;
        }
        else {
            return test(x - 3.0);
        }
    }
    test(666.0);
    return 3.0;
}
