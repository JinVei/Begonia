## Begonia
Begonia is toy compiler for learning purpose. Begonia parses its program language(.bga), and use LLVM library to generate machine code.

### Dependent
- LLVM-10/LLVM-11
  - Ensure the path where the llvm installed to has export to PATH env
- ld


### BUILD & RUN
Run "```make```" on root directory of Begonia, and then our compiler would build into location of './bin/begonia'. 

Finally, Run below command to compile .bga file into Binary:

```./bin/begonia *.bga ```

NOTE: Not fully support windows plaform yet.

### Grammar

```
block := {Statement}
Statement := IfStat
        | DeclarVarStat
        | DeclarFuncStat
        | AssignStat
        | ForStat
        | WhileStat
        | RetStat
        | exp
        | ;

IfStat          := if exp '{' block '}' [elif exp '{' block '}'] [else '{' block '}'] 
DeclarVarStat   := var identifier ['=' exp] ;
DeclarFuncStat  := func identifier '(' () | (exp [',' exp])  ')' '{' block '}' ;
AssignStat      := identifier '=' exp ;
WhileStat       := while exp '{' block '}' 
RetStat         := return | return exp ["," exp];
ExprStat        := exp;

exp  := exp7 {('||') exp7}
exp7 := exp6 {(&&) exp6}
exp6 := exp5 { ('<='|'>='|'=='|'<'|'>'|'!=') exp5}
exp5 := exp4 { ('+'|'-') exp4}
exp4 := exp3 {('*'|'/'|'%') exp3}
exp3 := exp2 {('|' | '&' | '^') exp2}
exp2 := !exp1 | exp1
exp1 :=  '(' exp8 ')' | nil | false | true | number | string | identifier | funcallStat
```