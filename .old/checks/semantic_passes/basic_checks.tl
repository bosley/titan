fn implicit_integer_casting() -> nil {
  let a:u8  = 0;
  let b:u16 = 0;
  let c:u32 = 0;
  let d:u64 = 0;
  let e:i8  = 0;
  let f:i16 = 0;
  let g:i32 = 0;
  let h:i64 = 0;
  a = e; a = f; a = g; a = h;
  b = e; b = f; b = g; b = h;
  c = e; c = c; c = g; c = h;
  d = e; d = d; d = g; d = h;
  e = a; e = b; e = c; e = d;
  f = a; f = b; f = c; f = d;
  g = a; g = b; g = c; g = d;
  h = a; h = b; h = c; h = d;
  return;
}

fn implicit_float_casting() -> nil {
  let x:float = 3.14159;
  let a:u8  = x;
  let b:u16 = x;
  let c:u32 = x;
  let d:u64 = x;
  let e:i8  = x;
  let f:i16 = x;
  let g:i32 = x;
  let h:i64 = x;
  x = a; x = b; x = c; x = d;
  x = e; x = f; x = g; x = h;
  return;
}

fn string_concat() -> nil {
  let a:string = "This is a string";
  let b:string = " there are many like it";
  let c:string = a + b + " but this one is mine";
  return;
}

fn numbers_to_strings() -> nil {
  let a:u8  = 0;
  let b:u16 = 0;
  let c:u32 = 0;
  let d:u64 = 0;
  let e:i8  = 0;
  let f:i16 = 0;
  let g:i32 = 0;
  let h:i64 = 0;
  let i:float = 2.71828;
  let x:string = a; x = b; x = c; x = d; 
  x = e; x = f; x = g; x = h; x = i;
  return;
}

fn main() -> i8 { return 0; }
