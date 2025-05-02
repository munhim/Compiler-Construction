# Ubuntu Setup Guide for JSON2RelCSV

## Dependencies Installation

To use this converter on Ubuntu, you need to install the following dependencies:

```bash
# Update package lists
sudo apt update

# Install required packages
sudo apt install -y build-essential flex bison
```

## Building the Project

```bash
# Clone the repository (if using git)
# git clone https://github.com/yourusername/json2relcsv.git
# cd json2relcsv

# Or extract from archive if provided that way
# tar -xvf json2relcsv.tar.gz
# cd json2relcsv

# Compile the project
make
```

## Running the Program

You can run the program in several ways:

### Process a JSON File

```bash
# Basic usage with redirection
./json2relcsv < input.json

# Using a pipe
cat input.json | ./json2relcsv

# Print AST and write output to a specific directory
./json2relcsv < input.json --print-ast --out-dir output

# Show help message
./json2relcsv --help
```

### Process JSON from Standard Input

```bash
# Start the program
./json2relcsv

# Type or paste your JSON
{ "name": "Example", "value": 123 }

# Press Ctrl+D when finished to signal end of input
```

## Folder Structure

Recommended folder structure for using the program:

```
project/
├── bin/
│   └── json2relcsv      # The compiled executable
├── examples/
│   ├── simple.json      # Example JSON files
│   ├── nested.json
│   └── complex.json
└── output/              # Directory for output CSV files
```

## Output Files

After running the program, CSV files will be created in the specified output directory (or the current directory if no output directory is specified). Each table from the JSON structure will have its own CSV file, named according to the table it represents.

For example, a JSON structure with a nested "authors" object will generate a separate `authors.csv` file containing all author data, with proper foreign key relationships to the main table.

## Cleaning Up

To clean the project and remove all generated files:

```bash
make clean
```

This will remove all object files, the executable, and any generated CSV files.
