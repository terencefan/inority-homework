# Homework

## Debug

> gcc *.c -Wall -Werror -D USE_MY_MAIN_FUNCTION -D DEBUG_MODE

## Test

> gcc *.c -Wall -Werror -D USE_MY_MAIN_FUNCTION


## Valgrind

> valgrind --tool=memcheck --leak-check=yes ./a.out myregex01.txt test.txt