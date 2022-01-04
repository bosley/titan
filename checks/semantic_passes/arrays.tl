fn take_it(arr:i8[100]) -> nil {

  return;
}

fn main() -> i8 {
  let a : i8 [100] = {};
  let b : i8 [100] = { 0 };
  let c : i8 [100] = { 0, 0, 0 };
  let d : i8 [100] = 0;
  let e : i8 [10] = a[7] + b[8] ;

  // Same as a[0] + b[0]. This would make 100 copies of a[0] + b[0]
  let f : i8 [100] = a + b ;
  return 0;
}
