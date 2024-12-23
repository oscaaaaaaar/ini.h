// INI format. 
// Modify as needed. 
// Assumes ASCII character codes. 
// See end of file for license. 

// A line is either a section header or a key value pair: 
// [ section_name ]
// key = value

struct ini_line_info {
    unsigned length;
    unsigned content_first;
    unsigned content_length;
    unsigned name_first;
    unsigned name_length;
    unsigned bracket_open_first;
    unsigned bracket_close_last;
    unsigned bracket_open_count;
    unsigned bracket_close_count;
    unsigned key_length;
    unsigned val_length;
    unsigned equals_count;
};

static inline struct ini_line_info ini_get_line(const char *txt, unsigned txt_length, unsigned start_offset)
{
    struct ini_line_info line = {0};

    if (start_offset > txt_length) start_offset = txt_length;

    _Bool comment = 0;
    unsigned last_content = 0;
    unsigned val_first = 0;
    for (unsigned i = start_offset; i < txt_length; i += 1) {
        char c = txt[i];
        line.length += 1;

        if (c == '\n') break;
        if (c == ';') comment = 1;
        if (comment) continue;

        if (c == ' ') continue;
        if (c == '\t') continue;
        if (c == '\r') continue;

        if (line.content_length == 0) line.content_first = i;
        line.content_length = i - line.content_first + 1;

        if (line.equals_count != 0 && val_first == 0) val_first = i;
        if (line.equals_count != 0) line.val_length = i - val_first + 1;
        if (c == '=') line.equals_count += 1;
        if (line.equals_count == 0) line.key_length = i - line.content_first + 1;

        if (line.bracket_open_count == 1 && line.name_first == 0) line.name_first = i;

        if (c == '[' && line.bracket_open_count == 0) line.bracket_open_first = i;
        if (c == '[') line.bracket_open_count += 1;
        if (c == ']') line.bracket_close_last = i;
        if (c == ']') line.bracket_close_count += 1;

        if (c == ']' && line.name_first != 0) line.name_length = last_content - line.name_first + 1;

        last_content = i;
    }

    return line;
}

static inline _Bool ini_is_empty(const struct ini_line_info *info)
{
    return info->content_length == 0;
}

static inline _Bool ini_is_section(const struct ini_line_info *info)
{
    return
        info->bracket_open_count == 1 &&
        info->bracket_close_count == 1 &&
        info->equals_count == 0 &&
        info->bracket_open_first < info->bracket_close_last &&
        info->bracket_open_first == info->content_first &&
        info->bracket_close_last + 1 == info->content_first + info->content_length;
}

static inline _Bool ini_is_val(const struct ini_line_info *info)
{
    return
        info->equals_count == 1 &&
        info->bracket_open_count == 0 &&
        info->bracket_close_count == 0;
}

static inline const char *ini_section_name(const char *txt, const struct ini_line_info *info, unsigned *name_length)
{
    *name_length = info->name_length;
    return txt + info->name_first;
}

static inline const char *ini_key(const char *txt, const struct ini_line_info *info, unsigned *name_length)
{
    *name_length = info->key_length;
    return txt + info->content_first;
}

static inline const char *ini_val(const char *txt, const struct ini_line_info *info, unsigned *name_length)
{
    *name_length = info->val_length;
    return info->content_length - info->val_length + info->content_first + txt;
}

static inline const char *ini_error_message_for_line(const struct ini_line_info *info, unsigned *error_length)
{
    static const char backup_msg[] = "Failed to parse";

    const char *return_msg = backup_msg;
    unsigned length = sizeof(backup_msg) - 1;

    if (!error_length) {
        error_length = &length;
    }

    struct ini_line_info empty_info = {0};
    if (!info) {
        info = &empty_info;
    }

    if (info->content_length == 0) {
        static const char msg[] = "Line is empty";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->equals_count == 0 && info->bracket_open_count == 0 && info->bracket_close_count == 0) {
        static const char msg[] = "Key must be followed by an = sign";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->equals_count > 0 && (info->bracket_open_count > 0 || info->bracket_close_count > 0)) {
        static const char msg[] = "Unclear if line is a value or a section header";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_close_count > 0 && info->bracket_close_last == info->content_first) {
        static const char msg[] = "Line cannot start with a closing bracket";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_open_count > 1 && info->bracket_close_count > 1) {
        static const char msg[] = "Cannot have multiple pairs of brackets in one line";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_open_count > 0 && info->bracket_open_first != info->content_first) {
        static const char msg[] = "Cannot have text preceding an opening bracket";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_close_count > 0 && info->bracket_open_first > info->bracket_close_last) {
        static const char msg[] = "Closing bracket must come after opening bracket";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_close_count && info->bracket_close_last + 1 != info->content_first + info->content_length) {
        static const char msg[] = "Cannot have text following a closing bracket";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_open_count > info->bracket_close_count) {
        static const char msg[] = "No matching closing bracket";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->bracket_open_count < info->bracket_close_count) {
        static const char msg[] = "No matching opening bracket";
        length = sizeof(msg) - 1;
        return_msg = msg;
    } else if (info->equals_count > 1) {
        static const char msg[] = "Cannot have more than one = sign";
        length = sizeof(msg) - 1;
        return_msg = msg;
    }

    *error_length = length;
    return return_msg;
}

/*
Same terms as nothings/STB. 
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2024 Oscar
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
