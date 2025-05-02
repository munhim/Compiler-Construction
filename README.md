# JSON to Relational CSV Converter

This tool reads any valid JSON file and creates CSV tables in a relational manner. It uses Flex for scanning, Yacc/Bison for parsing, and C for AST building, schema generation, and CSV output.

## Overview

The `json2relcsv` compiler transforms nested JSON structures into flat relational CSV tables with proper handling of:

- Primary and foreign keys to maintain relationships
- Nested objects become separate tables with foreign key references
- Arrays of objects become child tables with foreign keys to the parent
- Arrays of scalar values become junction tables
- Proper CSV escaping for all field values

## Build Instructions

To build the project:

```bash
# Make sure you have flex and bison installed
sudo apt-get install flex bison

# Compile the project
make
```

## Usage

```bash
# Process JSON from stdin
./json2relcsv < input.json

# Process JSON from file
cat input.json | ./json2relcsv
```

Output CSV files will be created in the current directory, with each table in its own file.

## Examples

### Simple JSON

```json
{ "id": 1, "name": "Ali", "age": 19 }
```

Will generate a single CSV file with columns for id, name, and age.

### Nested JSON with Arrays

```json
{
    "postId": 101,
    "author": {"uid": "u1", "name": "Sara"},
    "comments": [
       {"uid": "u2", "text": "Nice!"},
       {"uid": "u3", "text": "+1"}
    ]
}
```

Will generate multiple CSV files with proper relationships between tables using primary and foreign keys.

## Architecture

The compiler is organized into the following components:

1. **Scanner**: Uses Flex to tokenize the JSON input (scanner.l)
2. **Parser**: Uses Bison to parse tokens and build an AST (parser.y)
3. **AST Module**: Defines and manages the Abstract Syntax Tree (ast.h, ast.c)
4. **Schema Module**: Creates a relational schema from the AST (schema.h, schema.c)
5. **CSV Module**: Generates CSV files from the schema (csv.h, csv.c)

## Technical Details

The process works in several stages:

1. Lexical analysis and parsing using Flex/Bison
2. Building a complete AST representing the JSON structure
3. Schema generation based on the structure of the AST
4. CSV generation with proper primary/foreign key relationships
