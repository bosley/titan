
```
    <entry> := <function-statement>+

    <function-statement> := 'fn' <identifier> <function-params> '->' <identifier> '{' <statements> '}'

    <statements> := <statement> 
                  | <statements> <statement>

    <satement> := <assignment> 
                | <if-statement> 
                | <loop> 
                | <expression>

    <assignment> := 'let' <identifier> ':' <identifier> '=' <expression> ';'
                  | <identifier> '=' <expression> ';'

    <if-statement> := 'if' '(' <expression> ')' '{' <statements> '}' <else-if> 
                    | 'if' '(' <expression> ')' '{' <statements> '}' <else>
                    | 'if' '(' <expression> ')' '{' <statements> '}'

    <else-if> := 'else' 'if' '(' <expression> ')' '{' <statements> '}' <else-if>
               | 'else' 'if' '(' <expression> ')' '{' <statements> '}' <else>
               | 'else' 'if' '(' <expression> ')' '{' <statements> '}'

    <else> := 'else' '{' <statements> '}'

    <loop> := 'while' '(' <expression> ')' '{' <statements> '}' 
            | 'for' '(' <assigment> <expression> ';' <assignment> ')' '{' <statements> '}'


    <identifier> := <letter> | <identifier> <letter> | <identifier> <number> | <identifier> '_'

    <letter> := ALPHANUM

    <number> := DECIMAL_NUMBER

    <float> := <number> '.' <number>+

    <expression> := <term>
                  | <expression> '+' <term>
                  | <expression> '-' <term>
                  | <expression> '<' '=' <term>
                  | <expression> '>' '=' <term>
                  | <expression> '<' <term>
                  | <expression> '> '<term>
                  | <expression> '=' '=' <term>
                  | <expression> '!' '=' <term>

    <term> := <factor>
            | <term> '*' <factor>
            | <term> '/' <factor>
            | <term> '*' '*' <factor>
            | <term> '%' <factor>
            | <term> '<' '<' <factor>
            | <term> '>' '>' <factor>
            | <term> '^' <factor>
            | <term> '|' <factor>
            | <term> '&' <factor>
            | <term> '|' '|' <factor>
            | <term> '&' '&' <factor>

    <factor> := <primary>
              | <function_call>
              | '(' <expression> ')'
              | '~' <factor>
              | '!' <factor>

    <primary> := <number>
               | <float>
               | <identifier>
                  
    <function_call> := <identifier> '(' ')' 
                     | <identifier> '(' <call_parameter_list> ')'

    <call_parameter_list> := <primary>
                           | <call_parameter_list> <primary>
```