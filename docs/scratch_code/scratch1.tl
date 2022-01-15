
import io

fn say_hello(name:string) -> void {
  io::print(name);
}

fn main(argc:int, argv:string[]) -> int {

  let name:string = "Josh";

  // say_hello(name);

  return 0;
}