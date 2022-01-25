
fn bad() -> nil {
  return;
}

fn main() -> int {

  let x:int = 3 + bad();

  return 0;
}

