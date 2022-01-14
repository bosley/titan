
local {

struct shape {
  sides:int;  
};

}

enum some_enum {
  A(int),
  B(double),
  C(shape)
};


switch(some_enum){

  case A(x) {
    print("Int Value =", x);
    break;
  }
  case B(x) {
    print("double Value =", x);
    break;
  }
  case C(x) {
    print("Struct Value =", x.sides);
    break;
  }
}


