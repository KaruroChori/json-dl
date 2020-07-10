The actual implementation of an interface is described in a second JSON file directly used to generate the code for the library to be imported in our host application. 

It structure is made by some meta-keys:
- **$base** is a string prefix for all the paths used in the file. It is optional, by default it is equivalent to an empty string.
- **$lang** is a string specifying the language used for the code of the implementation. It must be specified.
- **$rev** is a string defining the revision for the language used.
- **$flags** is an object defining optional key to be used as flags.

There are few other keys:
- **source** is a required string with the path location of the interface JSON file this one is based on.
- **headers** is an array of strings with the names of the header files to be included.
- **tail** is a *code* object to be appended at the end of the code generated for the library.
- **impl** is an object made by keys which are *symbols*, and for each symbol a *code* object is specified. It could be skipped, however this would require a default implementation on the original interface JSON to avoid exceptions.

A *code* object can be one of the two:
- A string containing the actual code.
- An object where the key *path* or the key *inline* can be specified. One must be and only one of them. In the former case it will be a path to an external file containing the resource. In the latter it will its content.

## Future fields & reserved names
There are fields which are not considered at the moment but can be important for future revisions of this standard:
- Some sort of **author**, **desc**, **version** for which there is no syntex specified at the moment to be placed in the root object of our JSON file.
- Some additional **author**, **desc** and **version** to be added to *code* objects as optional fields.

## Examples
```json
{
    "$lang":"c++",
    "$rev":"20",
    "$base":"./items/",
    "source":"inerface-model.json",
    "headers":["iostream","cstdlib"],
    "impl":{
        "var_1":"20.0",
        "fn_1":"return a+b;",
        "fn_2":{"path":"fn_2.fragment"}
    }
}
```