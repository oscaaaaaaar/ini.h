#include "ini.h"
#include <stdio.h>

const char string[] =
    "; comment\n"
    "; \n"
    " ;\n"
    ";\n"
    "\n"
    "key=value\n"
    "key = value ; comment\n"
    "key = value\n"
    "key = ; value\n"
    "multiple word key = multiple word value\n"
    "empty value = \n"
    " = empty key\n"
    "({<!@£$%^&*>}) = ({<!@£$%^&*>})\n"
    "windows file path = C:\\windows\n"
    "expression = 1+2+3\n"
    "setting = false\n"
    "setting = true\n"
    "\n"
    "\n"
    "[section]\n"
    "[ section ] ; comment\n"
    "[ section ]\n"
    "[ multiple word section name ]\n"
    "[[section]]\n"
    "[[ section ]]\n"
    "[ [section] ]\n"
    "[ [section ]\n"
    "[ section] ]\n"
    "\n"
    "; error cases\n"
    "\n"
    "text [section] text\n"
    "] section [\n"
    " ] section [ \n"
    "[section\n"
    "section]\n"
    "text [section\n"
    "section] text\n"
    "text [section]\n"
    "[section] text\n"
    "c[section]\n"
    "[section]c\n"
    "[ key = value ]\n"
    "text [ key = value ] text\n"
    "text ] key = value [ text\n"
    "\n"
    "key = = value\n"
    "= key = value\n"
    "key = value =\n"
    "key[] = value[]\n"
    "[]key = []value\n"
    "\n"
    "key = value, [ section ]\n"
    "[ section ], key = value\n"
    "\n"
    "key ; = value\n";

void print_line_info(struct ini_line_info info, const char *txt, unsigned offset, unsigned displayed_line_number)
{
    printf(
        "line (%i):\n%.*s"
        "offset: %i, length: %i\n",

        displayed_line_number,
        info.length,
        txt+offset,
        offset,
        info.length
    );
    printf(
        "content_first: %i\n"
        "content_length: %i\n"
        "name_first: %i\n"
        "name_length: %i\n"
        "bracket_open_first: %i\n"
        "bracket_close_last: %i\n"
        "bracket_open_count: %i\n"
        "bracket_close_count: %i\n"
        "key_length: %i\n"
        "val_length: %i\n"
        "equals_count: %i\n",

        info.content_first,
        info.content_length,
        info.name_first,
        info.name_length,
        info.bracket_open_first,
        info.bracket_close_last,
        info.bracket_open_count,
        info.bracket_close_count,
        info.key_length,
        info.val_length,
        info.equals_count
    );
}

int main(int argc, char *argv[])
{
    unsigned string_length = sizeof(string) - 1;

    unsigned current_offset = 0;
    for (int i = 0; i < 500; i += 1) {
        struct ini_line_info info = ini_get_line(string, string_length, current_offset);
        if (info.length == 0) break;

        if (info.content_length) {
            print_line_info(info, string, current_offset, i+1);
            printf("\n");
        }
        
        current_offset += info.length;
    }

    printf("\n\n");

    current_offset = 0;
    for (int i = 0; i < 500; i += 1) {
        struct ini_line_info info = ini_get_line(string, string_length, current_offset);
        if (info.length == 0) break;

        if (ini_is_section(&info)) {
            unsigned length = 0;
            const char *name = ini_section_name(string, &info, &length);
            printf("[ %.*s ]", length, name);
        } else if(ini_is_val(&info)) {
            unsigned key_length = 0, val_length = 0;
            const char *key = ini_key(string, &info, &key_length);
            const char *val = ini_val(string, &info, &val_length);
            printf("%.*s = %.*s", key_length, key, val_length, val);
        } else if (ini_is_empty(&info)) {
            printf("EMPTY");
        } else {
            unsigned error_length;
            const char *error = ini_error_message_for_line(&info, &error_length);
            printf("ERROR: %.*s", error_length, error);
        }
        printf(" FROM %.*s", info.length, string+current_offset);
        
        current_offset += info.length;
    }
}
