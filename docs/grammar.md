
```
    <entry> := <import> 
             | <entry> <function-statement>+

    <import> := 'import' <string>
              | <import> 'import' <string>

    <function-statement> := 'fn' <identifier> '(' <function-params> ')' '->' <identifier> <statement-block>

    <function-params> := <identifier> ':' <identifier>
                       | <function-params> ',' <identifier> ':' <identifier>

    <statement-block> := '{' <statement>+ '}'

    <statement> := <assignment> 
                | <if-statement> 
                | <while-statement>
                | <for-statement>
                | <return-statement>
                | <expression>

    <assignment> := 'let' <identifier> ':' <identifier> [<lit-accessors>+] '=' <expression> ';'

    <lit-accessors> := '[' <number> ']'

    <expr-accessors> := '[' <expression> ']'

    <if-statement> := 'if' <conditional> <statement-block> [<else-if>+] [<else>]

    <else-if> := 'else' 'if' <statement-block>

    <else> := 'else' <statement-block>

    <while-statement> := 'while' <conditional> <statemebt-block>

    <for-statement> := 'for' '(' <assignment> ';' <expression> ';' <expression> ')' <statement-block>

    <conditional> := '(' expression ')'

    <identifier> := <letter> | <identifier> <letter> | <identifier> <number> | <identifier> '_'

    <letter> := ALPHANUM

    <number> := DECIMAL_NUMBER

    <float> := <number> '.' <number>+

    <string> := '"' <chars>+ '"'

    <expression> := <prefix> [<infix+>]

    <expression-list> := <expression>
                       | ',' <expression-list>

    <prefix> := <identifier>
              | <number> 
              | <float>
              | <string>
              | <prefix> 
              | '(' <expression> ')'
              | '{' <expression-list> '}'

    <infix> := '=' <expression>
             | '=' '=' <expression>
             | '!' '=' <expression>
             | '<' <expression>
             | '>' <expression>
             | '<' '=' <expression>
             | '>' '=' <expression>
             | '+' <expression>
             | '-' <expression>
             | '/' <expression>
             | '*' <expression>
             | '%' <expression>
             | <call-expr>
             | <expr-accessors>

    <call-expr> := <identifier> '(' [<call_parameter_list>] ')' 

    <call_parameter_list> := <expression> 
                           | <call_parameter_list> ',' <expression>

```
