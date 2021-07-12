# ysen (Yemi Scripting EngiNe)
An interpreted language made for educational purposes.

## current working features
```
fun add(a, b) a + b // return and semi-colon is implicit

add(5, 5); // returns 10
add("hello ", "world"); // returns the string "hello world"
add("value: ", 200); // returns the string "value: 200".

fun add_1(a, b) ret a + b; // return and semi-colon are explicit
// same examples as above

fun print_array(arr) {
  for (var e : arr) {
    print("{}", e);
  }
}

print_array([1, 2, 3, 4]); // prints 1 2 3 4 (with EOL instead of spaces).
print_array(1..4); // Same as above but with numeric range

fun call_with_param(lambda, param) ret lambda(param)

var is_10 = call_with_param(fun (x) x + 5, 5); // lambdas are quick & simple
// is_10 has the integer value 10

var my_lambda = fun() ret "Hello!"
print(my_lambda()) // prints "Hello!"

// Object
var object = [
  'item1': ['contains', 'an', 'array'],
  'item2': 'contains a string',
  'item3': ['contains': 'another object']
]

print("Object item1: {}", object.item1);

var arr_range = 1..1000; // An array with 1000 thousand entries

for (var x : arr_range) print(x); // prints all numbers from 1 to 1000
for (var x : 1..1000) print(x); // same as above.
```

## Bootstrapping
See main.cpp for example on running a piece of script. Documentation and comments are seriously lacking at the moment.

## Goals
* Add classes
* More operators
* Better reference management (currently non-existent).
* Type system (currently non-existent apart from the possibility of specifying types for parameters like `a: int`)
* Bytecode generator

## Inspirations
* ChaiScript
* SerentityOS LibJS & SerenityOS development in general
* Many many more projects

## Side note
The project contains a lot of (probably) unsafe memory management code that has not been tested or reviewed in any sense. 
I still find bugs every other day in the `Core` library, especially with memory corruption or dangling pointers. 
I would not recommend to use this project for anything remotely serious, this is basically a playground for crazy ideas. Nothing is stable, or maintained, 
and I cannot guarantee you'll even get it to run :^)
