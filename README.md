I’ve successfully extracted and read the key documentation files from your project. I’ll now generate a concise and professional **README** project report summarizing the purpose, setup, usage, and structure of the project.

Here’s your project README report: 👇

---

# 🛠️ C Compiler: H_CC Project Report

## 📌 Overview

This project implements a basic **C language compiler** named `H_CC`, developed as part of an academic assignment. The compiler performs lexical, syntactic, and semantic analysis and generates intermediate code. It supports a subset of C syntax and provides a CLI-based environment to compile source files.

## 👨‍💻 Authors

- **i220941**
- **i221079**

## 📁 Project Structure

- `main.c`: Entry point and controller logic.
- `ast.c/h`: Abstract Syntax Tree construction.
- `csv.c/h`: CSV parsing utility for symbol table or tokens.
- `lexer.l`: Lexical analyzer written using Flex.
- `parser.y`: Parser generated using Bison (Yacc).
- `Makefile`: Compilation rules.
- `CONVERSION_RULES.md`: Manual conversion rules for tokens and node generation.
- `SETUP_UBUNTU.md`: Setup instructions for Ubuntu systems.
- `README.md`: Instructions and project description.

## ⚙️ Setup (Ubuntu)

1. **Install dependencies:**

   ```bash
   sudo apt-get install flex bison build-essential
   ```

2. **Compile the project:**

   ```bash
   make
   ```

3. **Run the compiler:**

   ```bash
    ./json2relcsv < tests/<json file> --print-ast --out-dir output
   ```

## 🔁 Conversion Rules

Refer to `CONVERSION_RULES.md` for how C tokens and syntax are translated internally during lexical and parsing stages. This includes:

- Token representation
- AST node structure
- Conversion mapping

## 📄 Build & Clean

- **Build all:**

  ```bash
  make all
  ```

- **Clean:**

  ```bash
  make clean
  ```

## ✅ Features

- Tokenization using Flex
- Parsing with Bison
- AST generation
- Symbol table handling
- Intermediate representation
- Linux-compatible

## 🧪 Testing

Test your `.c` source files with valid subset syntax to check tokenization, AST generation, and semantic rules.

---

Would you like this in PDF or DOCX format too?
