# 📁 File Directory Management System (FDMS)

A terminal-based File Directory Management System built in C using two core data structures:
**General Tree** for directory hierarchy and **B+ Tree** for fast file searching.

---

## 🧠 Idea Behind the Project

Every operating system (Windows, Linux, macOS) manages files and folders internally using tree structures.
This project simulates that same concept in a simple CLI application written in C.

The idea is to answer one question:

> *How does a computer know where your file is, and how does it find it so fast?*

Two data structures work together to solve this:

| Data Structure | Role |
|---|---|
| General Tree | Stores the folder and file hierarchy (like a real file system) |
| B+ Tree | Indexes all file names so search is fast (O log n) |

---

## 🗂️ Project Structure

```
project/
 └── fdms.c       ← entire program in one file
```

---

## ⚙️ How to Compile and Run

**On Linux / macOS:**
```bash
gcc -std=c99 -Wall -o fdms fdms.c
./fdms
```

**On Windows (MinGW / VS Code terminal):**
```bash
gcc -std=c99 -Wall -o fdms.exe fdms.c
fdms.exe
```

---

## 🖥️ Menu

```
========================================
 File Directory Management System
 CWD: /
========================================
 1. Create Directory
 2. Delete Directory
 3. Change Directory (cd)
 4. Create File
 5. Delete File
 6. Move File
 7. Search File (B+ Tree)
 8. Display File Path
 9. Display Directory Tree
 0. Exit
========================================
Enter choice:
```

> CWD means **Current Working Directory** — it updates live as you navigate.

---

## 🌳 Data Structure 1 — General Tree

### What it does
Stores the entire directory and file structure as a tree.
Each node is either a **directory** or a **file**.

### Representation
Uses **Child-Sibling** (Left-Child Right-Sibling) representation.
Each node has only 3 pointers regardless of how many children it has.

```
struct TreeNode {
    name          ← folder or file name
    type          ← 0 = directory, 1 = file
    first_child   → points to first child node
    next_sibling  → points to next sibling node
    parent        → points to parent (used for path building)
}
```

### Visual Example

```
Suppose you have this structure:

/
├── Docs
│   ├── resume.txt
│   └── notes.txt
└── Photos

In memory it looks like this:

        [/]
         |  (first_child)
       [Docs] ──(next_sibling)──► [Photos]
         |  (first_child)
    [resume.txt] ──(next_sibling)──► [notes.txt]
```

### Why Child-Sibling?
- A directory can have **any number** of children
- Child-sibling uses only **2 pointers per node** instead of an array
- Memory efficient and easy to traverse

---

## 🔍 Data Structure 2 — B+ Tree

### What it does
Indexes all **file names** in the system so you can search any file in O(log n) time
without traversing the entire directory tree.

### Key Properties
- **Order 3** (each node holds at most 5 keys)
- **Internal nodes** store only routing keys (no file data)
- **Leaf nodes** store file names + pointer to the actual TreeNode
- All leaf nodes are **linked together** (like a linked list) for efficient traversal

### Structure

```
Internal Node:
┌────────────────────────────┐
│  key1  |  key2  |  key3   │  ← routing keys only
├────────────────────────────┤
│ child0 | child1 | child2  │  ← pointers to children
└────────────────────────────┘

Leaf Node:
┌──────────────────────────────────────────┐
│  "notes.txt"  |  "resume.txt"            │  ← file name keys
│  → TreeNode*  |  → TreeNode*             │  ← pointer to tree node
└──────────────────────────────────────────┘
        │
        └──► next leaf node (linked list)
```

### Visual Example

```
Suppose 3 files exist: budget.txt, notes.txt, resume.txt

B+ Tree (order 3):

         [ notes.txt ]
        /              \
[budget.txt]      [notes.txt | resume.txt]
     ↓                   ↓           ↓
  TreeNode*           TreeNode*   TreeNode*
```

### Why B+ Tree?
- Search is **O(log n)** — much faster than scanning all files one by one
- All data lives in leaf nodes — internal nodes are small and fast to traverse
- Naturally sorted — easy to extend for range searches later

---

## 🔄 How Both Trees Work Together

```
CREATE FILE "resume.txt" under /Docs
─────────────────────────────────────
Step 1: Create TreeNode { name="resume.txt", type=FILE }
Step 2: Add node as child of /Docs in General Tree
Step 3: Insert key "resume.txt" → TreeNode* into B+ Tree

SEARCH FILE "resume.txt"
─────────────────────────────────────
Step 1: Query B+ Tree with key "resume.txt"
Step 2: B+ Tree returns TreeNode* in O(log n)
Step 3: Follow parent pointers in General Tree to build path
Step 4: Output → /Docs/resume.txt

DELETE FILE "resume.txt"
─────────────────────────────────────
Step 1: Remove node from General Tree (unlink from parent)
Step 2: Delete key "resume.txt" from B+ Tree
Step 3: Free memory
```

---

## 📋 Features and Behaviour

| Operation | What Happens |
|---|---|
| Create Directory | New node (type=DIR) added under current directory |
| Delete Directory | Entire subtree deleted; all file keys removed from B+ Tree |
| Change Directory | CWD pointer moves to named child or back to parent with `..` |
| Create File | Node (type=FILE) added under CWD; key inserted into B+ Tree |
| Delete File | Node removed from CWD; key deleted from B+ Tree |
| Move File | Node unlinked from current parent; linked to destination; parent pointer updated |
| Search File | B+ Tree search by name; returns full absolute path |
| Display Path | Follows parent pointers from file node up to root |
| Display Tree | Pre-order traversal of General Tree printed with indentation |

---

## ⚠️ Error Handling

| Situation | Message |
|---|---|
| Invalid or empty name | `Error: Invalid name.` |
| Duplicate directory in same folder | `Error: 'name' already exists.` |
| Duplicate file anywhere in system | `Error: File 'name' already exists in the system.` |
| Navigate to non-existent directory | `Error: 'name' not found.` |
| Navigate into a file | `Error: 'name' is a file.` |
| Delete a file as directory or vice versa | Appropriate type mismatch error |
| Move file to missing destination | `Error: Destination directory 'name' not found.` |
| Search for non-existent file | `File 'name' not found.` |
| Invalid menu input | `Invalid choice.` |

---

## 📌 Constraints and Rules

- Only **standard C libraries** used: `stdio.h`, `stdlib.h`, `string.h`
- Single `.c` file — no external dependencies
- **Root directory `/` cannot be deleted**
- Valid name characters: letters, digits, `-` `_` `.` only
- File names are **globally unique** across the entire system (enforced by B+ Tree)
- All memory is freed cleanly on exit

---

## 💡 Sample Session

```
CWD: /
> 1  →  Create Directory  →  "Docs"
> 1  →  Create Directory  →  "Photos"
> 3  →  Change Directory  →  "Docs"

CWD: /Docs
> 4  →  Create File  →  "resume.txt"
> 4  →  Create File  →  "notes.txt"
> 3  →  Change Directory  →  ".."

CWD: /
> 9  →  Display Tree
        [DIR]  /
            [DIR]  Docs
                [FILE] resume.txt
                [FILE] notes.txt
            [DIR]  Photos

> 7  →  Search File  →  "notes.txt"
        Found: /Docs/notes.txt

> 6  →  Move File  →  "notes.txt"  →  destination: "Photos"
        Moved 'notes.txt' to 'Photos'

> 9  →  Display Tree
        [DIR]  /
            [DIR]  Docs
                [FILE] resume.txt
            [DIR]  Photos
                [FILE] notes.txt
```

---

## 🚀 Future Enhancements

- [ ] Persistent storage — save and load tree from a file
- [ ] Rename file or directory
- [ ] Wildcard search (e.g. `*.txt`)
- [ ] File metadata — size, date created
- [ ] Multi-user with permissions
- [ ] GUI version

---

## 📚 Concepts Used

- General Tree with Child-Sibling representation
- B+ Tree indexing and search
- Dynamic memory allocation (`malloc`, `free`)
- Recursive tree traversal
- Pointer-based data structure design
- Modular C programming

---

*Academic Project — Data Structures and Algorithms | C Programming*
