fn main() -> int {
  let a:int = 0;
  let b:int = 1;

  if(a > b) {
    let t:int = a;
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
