### Overview
This project was lots of fun to work on. I was able to put many principles taught in my operating systems class to use.

### Core Concepts
- Parsing
    - My code supports multiple commands in a line (delimited by ';'), external program execution, background execution, pipes, and I/O redirection. By using a recursive descent parser (like most other expression and shell parsers), the parser breaks the user input into sentences that fit into one of these categories, and then executes them from left to right.
- Signal and Error Handling
    - My code implements custom signal handling for `SIGINT` (Ctrl + C), and custom error messages for some other non-zero exit codes. I can do this since my shell is limited to a smaller scope of functionality, meaning these error messages likely will not overwhelm the user for anything that might be executed in my shell. Other signals and exit codes are handled by the POSIX API.

### Usage
This code uses the POSIX API (sorry Windows friends 😥). Compile with GCC using the `make all` command, and run with `./myshell`. Make will also compile a little custom program `sleep_and_echo.c` that I used when I was developing the background execution feature for my shell, which does exactly what the filename suggests. Execution follows the format `./sne <message to echo after 3 seconds>`. You can play around with the exit code and sleep time in the Makefile.

### What Next?
I may revisit my background process handler. There are performance issues when lots of background processes (50+) are running at the same time. However, I don't really see when that many background processes would be running in my shell, since this is just a fun learning project. Maybe I'll develop my own shell some day. Who knows? Some other things I might also want to add are some quality of life changes, like command history using the up arrow and tab command auto-complete. Finally, I want to see if I can figure out how to add shell scripting. That would definitely be a good challenge for another time.