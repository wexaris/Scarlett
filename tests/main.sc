
// This is a line comment

/* This is a block comment */

func test(foo f64) -> f64;

func main(x f64) -> f64 {
    ;
    var b: bool = 1 as bool;
    var int8: i8 = 1 as i8;
    var int16: i16 = 1 as i16;
    var int32: i32 = 1 as i32;
    var int64: i64 = 1 as i64;
    var uint8: u8 = 1 as u8;
    var uint16: u16 = 1 as u16;
    var uint32: u32 = 1 as u32;
    var uint64: u64 = 1 as u64;
    var float32: f32 = 1 as f32;
    var float64: f64 = 1 as f64;

    var y: f64 = 1 as f64;
    loop {
        if (x + float64 != 0.0) {
            break;
        }
        else {
            return test(x - 3.0);
        }
    }
    test(666.0);
    return 3.0;
}
