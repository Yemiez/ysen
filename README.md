# Ysen: Yemi Scripting EngiNe
Ysen, an interpreted language designed for educational purposes, is a promising playground for scripting enthusiasts, despite its ongoing development.

## Table of Contents
* [Introduction](#introduction)
* [Functionalities](#functionalities)
* [Bootstrapping](#bootstrapping)
* [Future Goals](#future-goals)
* [Inspirations](#inspirations)
* [Disclaimer](#disclaimer)

## <a name="introduction"></a>Introduction
Ysen is designed as an educational tool to understand the core concepts of scripting languages. However, please bear in mind that this project is still under heavy development, and it's not recommended for serious use.

## <a name="functionalities"></a>Functionalities
Ysen offers a wide variety of features, which are outlined and demonstrated below:

### Implicit and Explicit Return
Functions in Ysen support both implicit and explicit return statements. Here's an example with an `add` function:

```
fun add(a, b) a + b // Implicit return
add(5, 5) // returns 10
```

### Array Iteration
In Ysen, you can iterate through an array using a for loop. Here's how you do it:

```
fun iterate_array(arr) {
  for (var e : arr) {
    // perform operations with e
  }
}

iterate_array([1, 2, 3, 4])
```

### Lambdas
Lambdas are quick and simple in Ysen. You can define a lambda function like this:

```
var myLambda = fun (x) x + x // myLambda is now a function that doubles its input
```

Then, you can use this lambda function to perform operations, such as doubling each number in an array:

```
fun iterate_with_lambda(arr, lambda) {
  for (var e : arr) {
    var result = lambda(e);
    // perform operations with result
  }
}

iterate_with_lambda([1, 2, 3, 4], myLambda) // doubles each number in the array
```

### Object Creation and Access
Objects in Ysen can be created and accessed like this:

```
var object = [
  'item1': ['contains', 'an', 'array'],
  'item2': 'contains a string',
  'item3': ['contains': 'another object']
]

print("Object item1: {}", object.item1)
```

### Numeric Ranges
Ysen supports the creation of numeric ranges, and they can be printed out as shown below:

```
var arr_range = 1..1000; // An array with 1000 thousand entries
for (var x : arr_range) print(x); // prints all numbers from 1 to 1000
```

## <a name="bootstrapping"></a>Bootstrapping
You can find an example of how to run a script in `main.cpp`. However, please note that the documentation and comments are still a work in progress.

## <a name="future-goals"></a>Future Goals
Ysen aims to add more features, including:
* Classes
* More operators
* Better reference management
* Type system
* Bytecode generator

## <a name="inspirations"></a>Inspirations
Ysen is inspired by various projects, particularly ChaiScript and SerenityOS LibJS & SerenityOS development.

## <a name="disclaimer"></a>Disclaimer
This project contains many untested and potentially unsafe memory management codes. Bugs are common, especially in the `Core` library with issues such as memory corruption or dangling pointers. It's not recommended for serious use, but rather as a playground for learning and experimenting with scripting concepts. There's no guarantee of stability, maintenance, or even functionality at this stage
