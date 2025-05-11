# 📦 JSON-to-Relational Schema Converter

This C-based project converts JSON structures into a normalized relational database schema using **Flex** and **Bison**. It parses JSON input and analyzes it to generate relational tables, columns, and foreign key relationships.

---

## 🚀 Features

- Parses JSON using Flex and Bison
- Converts nested JSON objects into normalized relational tables
- Handles arrays of objects and scalars
- Infers foreign key relationships between nested elements
- Deduplicates tables based on structure
- Generates schema information suitable for SQL-style databases

---

## 🛠️ Technologies Used

- **C Language**
- **Flex** (Lexical Analysis)
- **Bison** (Parsing)
- **Abstract Syntax Tree (AST)**
- **Schema Normalization Logic**

---

## 🔄 Conversion Rules

| JSON Element               | Relational Schema Equivalent       |
|---------------------------|------------------------------------|
| Scalar field              | Column in a table                  |
| Object field              | New table + foreign key reference  |
| Array of objects          | Child table with foreign key       |
| Array of scalars          | Junction table with parent key     |

---

## ⚙️ How It Works

1. **Tokenization (Flex):** JSON input is read and tokenized into recognizable structures.
2. **Parsing (Bison):** Tokens are parsed to create an Abstract Syntax Tree (AST).
3. **AST Traversal:** AST nodes are recursively visited to identify schema components.
4. **Schema Generation:** 
    - Tables are created for each unique object structure.
    - Scalar fields become columns.
    - Arrays of objects create child tables with foreign keys.
    - Arrays of scalars form junction tables.
    - Duplicate object structures are detected and reused.

---

## 🧠 Advanced Features

- **Structure Signatures:** Ensures that duplicate JSON structures are not redundantly turned into tables.
- **Foreign Key Inference:** Nested objects and arrays automatically create referential links.
- **Indexing:** Elements in arrays are indexed to preserve order.
- **Flexible Design:** Easily extensible for more JSON features or schema variations.

---

## 🏁 Build & Run

### Step 1: Compile

```bash
make clean
make
```

### Step 2: Run

```bash
./json2relcsv < tests/test5.json --print-ast --out-dir output
```

The schema is printed to standard output. Redirect to a file if needed.

---

## 📌 Use Cases

- Automatically generate SQL schemas from JSON APIs
- Normalize deeply nested JSON for relational databases
- Aid in ETL (Extract, Transform, Load) pipelines
- Educational projects involving compiler construction or parsing

---

---


Developed as part of a compiler construction assignment using Flex and Bison.
