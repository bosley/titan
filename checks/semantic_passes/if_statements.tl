fn main() -> i8 {
  let a:u8 = 0;
  let b:u8 = 1;

  if(a > b) {
    let t:u8 = a;
    a = b;
    b = t;
  }

  if(a) {} 
  else {}

  if(b) {}
  else if (a) {}

  if(a == b) {} 
  else if (b) {}
  else if (a) {}
  else {}
  return 0;
}
