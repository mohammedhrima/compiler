rm -rf wcc a.out
flags="-Wall -Werror -Wextra"
cc main.c -fsanitize=address -g3 -o wcc && ./wcc file.w && gcc file.s -o exe && ./exe
