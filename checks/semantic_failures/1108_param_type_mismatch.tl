//
//  Param types mismatched
//
fn function(item:u8) -> nil {
  return;
}

fn main() -> i8 {

  let x:string = "This is a string";
  function(x);

  return 0;
}
