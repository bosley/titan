
fn bad() -> nil {
  return;
}

fn main() -> i8 {

  let x:i8 = 3 + bad();

  return 0;
}

