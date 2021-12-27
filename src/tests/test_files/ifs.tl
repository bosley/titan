
fn main() -> u8 {
  let w:u8= 0;
  let x:u8 = 1;
  let y:u8 = 0;
  let z:u8 = 0;
  
  if(w)       { let e:u8 = 0; } 
  else if (x) { let e:u8 = 0; } 
  else if (y) { let e:u8 = 0; }
  else if (z) { let e:u8 = 0; }
  else        { let e:u8 = 0; }

  if(w) {} else if (x) {} else {}
  if(w) {} else {}
  if(w) {}
}
