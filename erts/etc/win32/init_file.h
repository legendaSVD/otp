typedef struct {
    char *key;
    char *value;
} InitEntry;
typedef struct {
    int num_entries;
    int size_entries;
    char *section_name;
    InitEntry **entries;
} InitSection;
typedef struct {
    int num_sections;
    int size_sections;
    InitSection **sections;
} InitFile;
InitFile *load_init_file(wchar_t *filename);
int store_init_file(InitFile *inif, wchar_t *filename);
InitFile *create_init_file(void);
void free_init_file(InitFile *inif);
InitSection *create_init_section(char *section_name);
int add_init_section(InitFile *inif, InitSection *inis);
int delete_init_section(InitFile *inif, char *section_name);
InitSection *lookup_init_section(InitFile *inif, char *section_name);
char *nth_init_section_name(InitFile *inif, int n);
InitSection *copy_init_section(InitSection *inis, char *new_name);
void free_init_section(InitSection *inis);
int add_init_entry(InitSection *inis, char *key, char *value);
char *lookup_init_entry(InitSection *inis, char *key);
char *nth_init_entry_key(InitSection *inis, int n);
int delete_init_entry(InitSection *inis, char *key);
#define INIT_FILE_NO_ERROR 0
#define INIT_FILE_OPEN_ERROR -1
#define INIT_FILE_WRITE_ERROR -2
#define INIT_FILE_PRESENT 0
#define INIT_FILE_NOT_PRESENT 1