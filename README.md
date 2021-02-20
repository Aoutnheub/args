Simple library for parsing command-line arguments.

# Installation

1. Clone the repository

```bash
$ git clone https://github.com/Aoutnheub/args.git
```

2. Create build directory and go in it

```bash
$ mkdir build && cd build
```

3. Create makefiles

```bash
$ cmake ..
```

4. Build

```bash
$ cmake --build . --config Release
```

5. Install

```bash
$ cmake --install . --config Release
```

# Usage

## Defining arguments

Create a parser:

```c++
ArgParser parser(
    "Example", // name (optional)
    "This is an example program" // definition (optional)
);
```

- Add flags

    ```c++
    parser.addFlag(
        "flag", // name
        "This is a flag", // description (optional)
        'f' // abbreviation (optional)
    );
    ```

- Add options

    ```c++
    parser.addOption(
        "option", // name
        "This is an option", // description (optional)
        'o', // abbreviation (optional)
        "value1", // default value (optional)
        {"value1", "value2"} // allowed values (optional)
    );
    ```

- Add commands

    ```c++
    parser.addCommand(
        "command", // name
        "This is a command" // description (optional)
    );
    ```

> Defining an argument with an existing name will throw an `args::duplicate_argument` exception

## Parsing arguments

```c++
ArgResults results = parser.parse({"some", "arguments"});
```

> Passing an option without a value will throw an `args::missing_value` exception  
> Passing an option with a value that's not in _allowed_ will throw an `args::invalid_value` exception  
> Passing an undefind argument will throw an `args::invalid_argument` exception  
> Multiple flag abbreviations can be concatenated into one

## Reading values

- For flags

    ```c++
    results.flag["flag"];
    ```

- For options

    ```c++
    results.option["option"];
    ```

- For commands

    ```c++
    results.command;
    ```

The rest of the arguments are counted as positional and can be accessed with `results.positional`.

## Info

Use `std::cout << parser.help` to print the usage for all the arguments.

```c++
Program name - description

COMMANDS

    command
        description

FLAGS

    --flag1, -f
        description

OPTIONS

    --option1, -o value1|value2|value3
        description
```

# To do

- Make `--option=value` valid usage
- Make description split into multiple lines if it's too long
