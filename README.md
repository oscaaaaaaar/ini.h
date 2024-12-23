Single header code for parsing INI file data. 

Modify as needed. I recommend swapping out integer types for explicitly sized ones. 

- Assumes ASCII character codes. 
- Same license terms as STB. Available under MIT license or public domain unlicense. 
- No preprocessor usage inside ini.h itself. Add an include guard if desired. 
- Standard C99. Standalone language only, the standard library is not used. 
- Zero heap allocations. 
- One additional data type: struct ini_line_info. No usage of typedef. 
- Eight static leaf functions. 

See test.c for example usage. 