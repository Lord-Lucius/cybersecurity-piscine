# libft

Extended C standard library for 42 projects. Includes libc reimplementations, additional string/memory utilities, `ft_printf` and `get_next_line`.

## Structure

```
include/libft.h      public header
src/ctype            character classification & case
src/string           string manipulation, atoi/atol, itoa, split
src/memory           memory operations, calloc
src/io               fd-based output
src/printf           ft_printf (%c %s %p %d %i %u %x %X %%)
src/gnl              get_next_line (override BUFFER_SIZE with -D BUFFER_SIZE=n)
```

## Build

```sh
make        # builds libft.a
make clean  # removes obj/
make fclean # removes obj/ and libft.a
make re
```

Link with `-L <path> -lft`, compile with `-I <path>/include`.
