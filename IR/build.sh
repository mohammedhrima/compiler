flags="-Wall -Werror -Wextra"
cc main.c -fsanitize=address -g3 -o wcc && ./wcc