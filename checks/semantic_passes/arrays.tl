fn create_thing() -> int[10][10] {
  return {};
}

fn returned_item() -> int[4] {
  let x:int [4] = {0,0,0,0};
  return x;
}

fn take_it(arr:int[100]) -> nil {
  return;
}

fn take_it_too(arr:int[10][5][2]) -> nil {
  return;
}

fn main() -> int {
  let a : int [100] = {};
  let b : int [100] = { 0 };
  let c : int [100] = { 0, 0, 0 };
  let d : int [100] = 0;
  let e : int [10] = a[7] + b[8] ;

  // Same as a[0] + b[0]. This would make 100 copies of a[0] + b[0]
  let f : int [10][10] = a + b ;
  return 0;
}
