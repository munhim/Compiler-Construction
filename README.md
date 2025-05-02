# JSON to Relational CSV Converter

This tool reads any valid JSON file and creates CSV tables in a relational manner. It uses Flex for scanning, Yacc/Bison for parsing, and C for AST building, schema generation, and CSV output.

## Build Instructions

To build the project:

```bash
# Make sure you have flex and bison installed
sudo apt-get install flex bison

# Compile the project
make
