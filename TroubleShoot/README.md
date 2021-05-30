## TROUBLE SHOOT

### 1 Dump ASM Code
```objdump -d {obj_file}```

### 2 Dump Symbol table
```readelf -a {obj_file}```

### 3 Make a breakpoint in entrypoint of Executable with GDB
```(gdb) b *0x401000```

### 4 Show ASM Code which current executed with GDB
```(gdb) layout asm```

### 5 Process must run out with a System Call of ```exit()```, Or it would throw error: ```Segmentation Fault```