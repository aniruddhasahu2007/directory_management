#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME  256
#define ORDER     3       

#define TYPE_DIR  0
#define TYPE_FILE 1

typedef struct TreeNode {
    char             name[MAX_NAME];
    int              type;           
    struct TreeNode *first_child;
    struct TreeNode *next_sibling;
    struct TreeNode *parent;
} TreeNode;


TreeNode *new_tree_node(const char *name, int type, TreeNode *parent) {
    TreeNode *n = (TreeNode *)malloc(sizeof(TreeNode));
    if (!n) { printf("Memory error\n"); exit(1); }
    strncpy(n->name, name, MAX_NAME - 1);
    n->name[MAX_NAME - 1] = '\0';
    n->type         = type;
    n->first_child  = NULL;
    n->next_sibling = NULL;
    n->parent       = parent;
    return n;
}

void tree_add_child(TreeNode *parent, TreeNode *child) {
    child->parent = parent;
    if (!parent->first_child) {
        parent->first_child = child;
        return;
    }
    TreeNode *s = parent->first_child;
    while (s->next_sibling) s = s->next_sibling;
    s->next_sibling = child;
}

TreeNode *find_child(TreeNode *parent, const char *name) {
    TreeNode *c = parent->first_child;
    while (c) {
        if (strcmp(c->name, name) == 0) return c;
        c = c->next_sibling;
    }
    return NULL;
}

void tree_unlink_child(TreeNode *parent, TreeNode *child) {
    if (!parent->first_child) return;
    if (parent->first_child == child) {
        parent->first_child = child->next_sibling;
        child->next_sibling = NULL;
        return;
    }
    TreeNode *prev = parent->first_child;
    while (prev->next_sibling && prev->next_sibling != child)
        prev = prev->next_sibling;
    if (prev->next_sibling == child) {
        prev->next_sibling = child->next_sibling;
        child->next_sibling = NULL;
    }
}


void get_path(TreeNode *node, char *buf, int bufsize) {
    if (!node) { strncpy(buf, "/", bufsize); return; }
   
    TreeNode *stack[512];
    int top = 0;
    TreeNode *cur = node;
    while (cur) { stack[top++] = cur; cur = cur->parent; }
   
    buf[0] = '\0';
    for (int i = top - 1; i >= 0; i--) {
        if (strcmp(stack[i]->name, "/") == 0) {
            strncat(buf, "/", bufsize - strlen(buf) - 1);
        } else {
            if (strlen(buf) > 1)
                strncat(buf, "/", bufsize - strlen(buf) - 1);
            strncat(buf, stack[i]->name, bufsize - strlen(buf) - 1);
        }
    }
}


void display_tree(TreeNode *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) printf("    ");
    if (node->type == TYPE_DIR)
        printf("[DIR]  %s\n", node->name);
    else
        printf("[FILE] %s\n", node->name);
    display_tree(node->first_child,  depth + 1);
    display_tree(node->next_sibling, depth);
}



typedef struct BPNode {
    int          is_leaf;
    int          num_keys;
    char         keys[2 * ORDER][MAX_NAME];
    struct BPNode *children[2 * ORDER + 1];
    TreeNode     *data[2 * ORDER];         
    struct BPNode *next;                   
} BPNode;

static BPNode *bp_root = NULL;

BPNode *new_bp_node(int is_leaf) {
    BPNode *n = (BPNode *)calloc(1, sizeof(BPNode));
    if (!n) { printf("Memory error\n"); exit(1); }
    n->is_leaf = is_leaf;
    return n;
}


TreeNode *bp_search(const char *key) {
    if (!bp_root) return NULL;
    BPNode *cur = bp_root;
    while (!cur->is_leaf) {
        int i = 0;
        while (i < cur->num_keys && strcmp(key, cur->keys[i]) >= 0) i++;
        cur = cur->children[i];
    }
    for (int i = 0; i < cur->num_keys; i++)
        if (strcmp(cur->keys[i], key) == 0) return cur->data[i];
    return NULL;
}




void leaf_insert_nonfull(BPNode *leaf, const char *key, TreeNode *data) {
    int i = leaf->num_keys - 1;
    while (i >= 0 && strcmp(key, leaf->keys[i]) < 0) {
        strcpy(leaf->keys[i + 1], leaf->keys[i]);
        leaf->data[i + 1] = leaf->data[i];
        i--;
    }
    strcpy(leaf->keys[i + 1], key);
    leaf->data[i + 1] = data;
    leaf->num_keys++;
}


BPNode *split_leaf(BPNode *left, char *out_key) {
    BPNode *right = new_bp_node(1);
    int mid = ORDER;
    right->num_keys = left->num_keys - mid;
    for (int i = 0; i < right->num_keys; i++) {
        strcpy(right->keys[i], left->keys[mid + i]);
        right->data[i] = left->data[mid + i];
    }
    left->num_keys = mid;
    strcpy(out_key, right->keys[0]);
    right->next = left->next;
    left->next  = right;
    return right;
}


BPNode *split_internal(BPNode *left, char *out_key) {
    BPNode *right = new_bp_node(0);
    int mid = ORDER - 1;
    strcpy(out_key, left->keys[mid]);
    right->num_keys = left->num_keys - mid - 1;
    for (int i = 0; i < right->num_keys; i++)
        strcpy(right->keys[i], left->keys[mid + 1 + i]);
    for (int i = 0; i <= right->num_keys; i++)
        right->children[i] = left->children[mid + 1 + i];
    left->num_keys = mid;
    return right;
}


BPNode *bp_insert_rec(BPNode *node, const char *key, TreeNode *data, char *push_key) {
    if (node->is_leaf) {
        leaf_insert_nonfull(node, key, data);
        if (node->num_keys < 2 * ORDER) return NULL;
        return split_leaf(node, push_key);
    }
   
    int i = 0;
    while (i < node->num_keys && strcmp(key, node->keys[i]) >= 0) i++;
    char up_key[MAX_NAME];
    BPNode *new_child = bp_insert_rec(node->children[i], key, data, up_key);
    if (!new_child) return NULL;
   
    for (int j = node->num_keys; j > i; j--) {
        strcpy(node->keys[j], node->keys[j - 1]);
        node->children[j + 1] = node->children[j];
    }
    strcpy(node->keys[i], up_key);
    node->children[i + 1] = new_child;
    node->num_keys++;
    if (node->num_keys < 2 * ORDER) return NULL;
    return split_internal(node, push_key);
}

void bp_insert(const char *key, TreeNode *data) {
    if (!bp_root) {
        bp_root = new_bp_node(1);
        strcpy(bp_root->keys[0], key);
        bp_root->data[0] = data;
        bp_root->num_keys = 1;
        return;
    }
    char push_key[MAX_NAME];
    BPNode *sibling = bp_insert_rec(bp_root, key, data, push_key);
    if (sibling) {
        BPNode *new_root = new_bp_node(0);
        strcpy(new_root->keys[0], push_key);
        new_root->children[0] = bp_root;
        new_root->children[1] = sibling;
        new_root->num_keys    = 1;
        bp_root = new_root;
    }
}




void leaf_remove(BPNode *leaf, const char *key) {
    int idx = -1;
    for (int i = 0; i < leaf->num_keys; i++)
        if (strcmp(leaf->keys[i], key) == 0) { idx = i; break; }
    if (idx < 0) return;
    for (int i = idx; i < leaf->num_keys - 1; i++) {
        strcpy(leaf->keys[i], leaf->keys[i + 1]);
        leaf->data[i] = leaf->data[i + 1];
    }
    leaf->num_keys--;
}

void bp_delete_rec(BPNode *node, const char *key) {
    if (!node) return;
    if (node->is_leaf) { leaf_remove(node, key); return; }
    int i = 0;
    while (i < node->num_keys && strcmp(key, node->keys[i]) >= 0) i++;
    bp_delete_rec(node->children[i], key);
}

void bp_delete(const char *key) {
    bp_delete_rec(bp_root, key);
}



static TreeNode *root_dir = NULL;
static TreeNode *cwd      = NULL;


int valid_name(const char *name) {
    if (!name || name[0] == '\0') return 0;
    for (int i = 0; name[i]; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.'))
            return 0;
    }
    return 1;
}


void read_input(char *buf, int size) {
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = '\0';
}


void free_subtree(TreeNode *node) {
    if (!node) return;
    free_subtree(node->first_child);
    free_subtree(node->next_sibling);
    if (node->type == TYPE_FILE) bp_delete(node->name);
    free(node);
}



void op_create_dir() {
    char name[MAX_NAME];
    printf("Enter directory name: ");
    read_input(name, MAX_NAME);
    if (!valid_name(name)) { printf("Error: Invalid name.\n"); return; }
    if (find_child(cwd, name)) { printf("Error: '%s' already exists.\n", name); return; }
    TreeNode *nd = new_tree_node(name, TYPE_DIR, cwd);
    tree_add_child(cwd, nd);
    printf("Directory '%s' created.\n", name);
}

void op_delete_dir() {
    char name[MAX_NAME];
    printf("Enter directory name to delete: ");
    read_input(name, MAX_NAME);
    TreeNode *target = find_child(cwd, name);
    if (!target) { printf("Error: '%s' not found.\n", name); return; }
    if (target->type != TYPE_DIR) { printf("Error: '%s' is a file, not a directory.\n", name); return; }
    tree_unlink_child(cwd, target);
    free_subtree(target->first_child);
    free(target);
    printf("Directory '%s' deleted.\n", name);
}

void op_change_dir() {
    char name[MAX_NAME];
    printf("Enter directory name (or ..): ");
    read_input(name, MAX_NAME);
    if (strcmp(name, "..") == 0) {
        if (!cwd->parent) { printf("Already at root.\n"); return; }
        cwd = cwd->parent;
        printf("Moved to '%s'.\n", cwd->name);
        return;
    }
    TreeNode *target = find_child(cwd, name);
    if (!target) { printf("Error: '%s' not found.\n", name); return; }
    if (target->type != TYPE_DIR) { printf("Error: '%s' is a file.\n", name); return; }
    cwd = target;
    printf("Changed to '%s'.\n", cwd->name);
}

void op_create_file() {
    char name[MAX_NAME];
    printf("Enter file name: ");
    read_input(name, MAX_NAME);
    if (!valid_name(name)) { printf("Error: Invalid name.\n"); return; }
    if (bp_search(name)) { printf("Error: File '%s' already exists in the system.\n", name); return; }
    if (find_child(cwd, name)) { printf("Error: '%s' already exists here.\n", name); return; }
    TreeNode *fn = new_tree_node(name, TYPE_FILE, cwd);
    tree_add_child(cwd, fn);
    bp_insert(name, fn);
    printf("File '%s' created.\n", name);
}

void op_delete_file() {
    char name[MAX_NAME];
    printf("Enter file name to delete: ");
    read_input(name, MAX_NAME);
    TreeNode *target = find_child(cwd, name);
    if (!target) { printf("Error: '%s' not found in current directory.\n", name); return; }
    if (target->type != TYPE_FILE) { printf("Error: '%s' is a directory.\n", name); return; }
    tree_unlink_child(cwd, target);
    bp_delete(name);
    free(target);
    printf("File '%s' deleted.\n", name);
}

void op_search_file() {
    char name[MAX_NAME];
    printf("Enter file name to search: ");
    read_input(name, MAX_NAME);
    TreeNode *result = bp_search(name);
    if (!result) { printf("File '%s' not found.\n", name); return; }
    char path[4096];
    get_path(result, path, sizeof(path));
    printf("Found: %s\n", path);
}

void op_display_tree() {
    printf("\n--- Directory Tree ---\n");
    display_tree(root_dir, 0);
    printf("----------------------\n");
}

void op_display_path() {
    char name[MAX_NAME];
    printf("Enter file name: ");
    read_input(name, MAX_NAME);
    TreeNode *result = bp_search(name);
    if (!result) { printf("File '%s' not found.\n", name); return; }
    char path[4096];
    get_path(result, path, sizeof(path));
    printf("Path: %s\n", path);
}

void op_move_file() {
    char fname[MAX_NAME], dname[MAX_NAME];
    printf("Enter file name to move: ");
    read_input(fname, MAX_NAME);
    printf("Enter destination directory name: ");
    read_input(dname, MAX_NAME);

    TreeNode *file = find_child(cwd, fname);
    if (!file) { printf("Error: File '%s' not found in current directory.\n", fname); return; }
    if (file->type != TYPE_FILE) { printf("Error: '%s' is not a file.\n", fname); return; }

   
   
    TreeNode *dest = NULL;
   
    TreeNode *stack[4096];
    int top = 0;
    stack[top++] = root_dir;
    while (top > 0) {
        TreeNode *cur = stack[--top];
        if (cur->type == TYPE_DIR && strcmp(cur->name, dname) == 0) { dest = cur; break; }
        if (cur->first_child)  stack[top++] = cur->first_child;
        if (cur->next_sibling) stack[top++] = cur->next_sibling;
    }
    if (!dest) { printf("Error: Destination directory '%s' not found.\n", dname); return; }
    if (find_child(dest, fname)) { printf("Error: File '%s' already exists in '%s'.\n", fname, dname); return; }

    tree_unlink_child(cwd, file);
    tree_add_child(dest, file);
    file->parent = dest;
   
    printf("Moved '%s' to '%s'.\n", fname, dname);
}



int main() {
   
    root_dir = new_tree_node("/", TYPE_DIR, NULL);
    cwd = root_dir;

    int choice;
    char input[16];

    while (1) {
        char path[4096];
        get_path(cwd, path, sizeof(path));
        printf("\n========================================\n");
        printf(" File Directory Management System\n");
        printf(" CWD: %s\n", path);
        printf("========================================\n");
        printf(" 1. Create Directory\n");
        printf(" 2. Delete Directory\n");
        printf(" 3. Change Directory (cd)\n");
        printf(" 4. Create File\n");
        printf(" 5. Delete File\n");
        printf(" 6. Move File\n");
        printf(" 7. Search File (B+ Tree)\n");
        printf(" 8. Display File Path\n");
        printf(" 9. Display Directory Tree\n");
        printf(" 0. Exit\n");
        printf("========================================\n");
        printf("Enter choice: ");

        read_input(input, sizeof(input));
        if (input[0] == '\0') { printf("Invalid choice.\n"); continue; }
       
        char *end;
        choice = (int)strtol(input, &end, 10);
        if (end == input) { printf("Invalid choice.\n"); continue; }

        switch (choice) {
            case 1: op_create_dir();   break;
            case 2: op_delete_dir();   break;
            case 3: op_change_dir();   break;
            case 4: op_create_file();  break;
            case 5: op_delete_file();  break;
            case 6: op_move_file();    break;
            case 7: op_search_file();  break;
            case 8: op_display_path(); break;
            case 9: op_display_tree(); break;
            case 0:
                printf("Exiting. Goodbye!\n");
                free_subtree(root_dir);
                return 0;
            default:
                printf("Invalid choice. Enter 0-9.\n");
        }
    }
}