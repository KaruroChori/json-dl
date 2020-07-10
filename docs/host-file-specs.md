These JSON files are used to define the interface to be fulfilled by any implementation to be compatible with my host application.
It is designed to be for the most part language agnostic, however it is limited to map only variables and functions.
More complex primitives like structures, lambdas, enumerations are not sharable because of the compiling and linking process in languages as C and C++.
As such, these restrictions reflect on the design of the JSON format.
**TODO: CONTINUE DESCRIPTION**

There are few meta-key defined in our file:
- **$base** is a string prefix for all the paths used in the file. It is optional, by default it is equivalent to an empty string.
- **$lang** is a string specifying the language used for the code of the host. It must be specified.
- **$rev** is a string defining the revision for the language used.
- **$flags** is an object defining optional key to be used as flags.
- **$interface-name** is a proper symbol used as a name of the class containing the code to dynamically load the interface.

We also have five more tags:
- **constructor** which is an optional function symbol to be called automatically once the implementation is fully imported
- **destructor** like before, an optional function symbol exectured when the object wrapping an implementation gets destroyed.
- **headers** is an array of strings with the names of the header files to be included.
- **tail** is a *code* object to be appended at the end of the code generated for the host code. It can be used for example to append a main function.
- **symbols** is a complex object containing all the information to describe the format of variables and functions to be shared.

## Symbols
Symbols can be of two types: *variable* or *function*. This propery is capture by the field **stype**.
It is possible to define a **default** which means a code object which is used when the implementation file is not providing an actual implementation.
In the case of functions there are more fields to be populated:
- **args** is an array of arg/ret objects. Their order is important. It can be skipped in case there are no arguments for a function.
- **rets** is an array of arg/ret objects. In C++ only one return value is allowed, so if more are defined also **rets-pack** must be set to "tuple" or "type". If not specified it is set to *void* by default.
- **args-pack** if "tuple" arguments are packed in a tuple or a tuple-like object in case STL is not allowed. If "type" it will spawn a custom type. If "false" it will be disabled which is so by default.
- **rets-pack** if "tuple" return values are packed in a tuple or a tuple-like object in case STL is not allowed. If "type" it will spawn a custom type. If "false" it will be disabled which is so by default.
- **use-stl** if true STL or any equivalent standard library is used i.e. for tuples or optional fields.

### Code objects
A *code* object can be one of the two:
- A string containing the actual code.
- An object where the key *path* or the key *inline* can be specified. One must be and only one of them. In the former case it will be a path to an external file containing the resource. In the latter it will its content.

### Arg/ret objects
An arg/ret object can be one of the following two:
- A string specifying its type. In this case the name for arguments will be automatically assigned as *_arg_#* with *#* the position in the array.
- An object made by the following keys:
  * **type**
  * **optional**
  * **name**
  * **default** is the default value expessed as a code object. It needs **args-pack** or **rets-pack** to be set to "type".

## Future fields & reserved names
There are fields which are not considered at the moment but can be important for future revisions of this standard:
- Some sort of **author**, **desc**, **version** for which there is no syntex specified at the moment to be placed in the root object of our JSON file.
- **desc** and **version** to be added to *symbol* objects as optional fields.
- Some additional fields to document the args and the rets in functions, so that some doxygen documentation can be generated alongside the host code or by the implementation.


## Examples
```json
{
    "$interface-name":"test_1",
    "$lang":"c++",
    "$rev":"20",
    "$base":"./items/",
    "headers":["iostream","cstdlib"],
    "impl":{
        "var_1":"20.0",
        "fn_1":"return a+b;",
        "fn_2":{"path":"fn_2.fragment"}
    },

    "symbols":{
        "start":{
            "stype":"function",
            "default": "return;"
        },
        "end":{
            "stype":"function",
            "default": "return;"
        },
        "var_1":{
            "stype":"variable",
            "default":"5.27"
        },
        "fn_1":{
            "stype":"function",
            "rets":["unsigned int"],
            "args":["unsigned int", "unsigned int"]
        },
        "fn_2":{
            "stype":"function"
        },
        "ReoNA":{
            "stype":"function",
            "rets":["std::string",{"type":"int","optional":true}],
            "args":["std::string",{"type":"int","optional":true}],
            "rets-pack":"tuple",
            "args-pack":"tuple",
            "default": "return {\"Hello World\",{10}};"
        }
    },
    "constructor":"start",
    "destructor":"end",
    "tail":{
        "inline":"int main(int argc, const char* argv[]){test_1 h(argv[1]);std::cout<<\"Value is:\"<<h.banana()<<\"\\n\\n\";return 0;}"
    }
}
```