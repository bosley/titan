//
//  Param types mismatched
//
fn function(item:int) -> nil {
  return;
}

fn main() -> int {

  let x:string = "This is a string";
  function(x);

  return 0;
}
