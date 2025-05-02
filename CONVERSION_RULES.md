# JSON to Relational CSV Conversion Rules

This document explains the rules used by the json2relcsv compiler to convert nested JSON structures into relational CSV tables.

## Basic Principles

1. **Primary Keys**: Every table has an auto-incremented numeric primary key column named `id`
2. **Foreign Keys**: Relationships between tables are maintained using foreign key columns
3. **Normalization**: Nested structures are split into separate tables with proper relationships

## Conversion Rules

### Simple JSON Objects

A simple, flat JSON object becomes a single CSV file with columns for each property:

```json
{ "id": 1, "name": "Ali", "age": 19 }
```

Converts to a CSV file with columns: `id,id,name,age`

Note that the first `id` is the primary key, while the second `id` is the value from the JSON.

### Nested Objects

Nested objects are extracted into their own tables with foreign key relationships:

```json
{
  "post": 101,
  "author": {"uid": "u1", "name": "Sara"}
}
```

Generates two tables:
1. Root table: `id,post,author_id`
2. Authors table: `id,uid,name`

The `author_id` in the root table is a foreign key referencing the `id` in the Authors table.

### Arrays of Scalars

Arrays of scalar values (strings, numbers, booleans) are stored in junction tables:

```json
{
  "book": "Programming Guide",
  "tags": ["programming", "guide", "reference"]
}
```

Generates two tables:
1. Root table: `id,book`
2. Tags junction table: `id,root_id,index,value`
   - `root_id`: Foreign key to the root table
   - `index`: Position in the original array (0, 1, 2)
   - `value`: The array element's value

### Arrays of Objects

Arrays of objects become child tables with a foreign key to the parent:

```json
{
  "postId": 101,
  "comments": [
    {"uid": "u2", "text": "Nice!"},
    {"uid": "u3", "text": "+1"}
  ]
}
```

Generates two tables:
1. Root table: `id,postId`
2. Comments table: `id,root_id,seq,uid,text`
   - `root_id`: Foreign key to the root table
   - `seq`: Position in the original array (0, 1)

### Complex Nested Structures

Complex structures with multiple levels of nesting create multiple interrelated tables following the rules above. Each level of nesting potentially creates a new table with appropriate foreign keys to maintain relationships.

## CSV Format

- The first row of each CSV contains the column headers
- String values are enclosed in double quotes
- Special characters in strings are properly escaped
- Numeric and boolean values are not quoted
