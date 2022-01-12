fn math() -> nil {
  let x:i8 = 1 + 2 - 3 / 4 * 5 % 6;
  x = x ** 3;
  x += 1;
  x /= 1;
  x *= 4;
  x %= 4;
  x **= 4;
  return;
}

fn cond() -> nil {
  let x:i8 = 0; 
  let y:i8 = 1;
  if(x >  y){}
  if(x >= y){}
  if(x <  y){}
  if(x <= y){}
  if(x == y){}
  if(x != y){}
  if(!y){}
  if(-y){}
  if(x || y){}
  if(x && y){}
}

fn shifts_and_stuff() -> nil {
  let x:i8 = 1 << 3;
  x = x >> 3;
  x <<= 1;
  x >>= 1;
  x = x ^ 3;
  x = x | 3;
  x = x & 3;
  x = ~1;
  x ^= 3;
  x |= 3;
  x &= 3;
  x ~= 3;
}

fn main() -> i8 {
  return 0;
}
