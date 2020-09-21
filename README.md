# chobeta
## Task
Create a simple suite of `Echo Programs` that handles text in various ways

1. Echo Server
  - Written in C
  - Compile with CFLAGS: `-Wall -Wpedantic -Werror`
  - Will take three types of `ECHO` packets
    1. `TYPE 0 ECHO` packet
       - Echoes text input to `STDOUT` and sends it back to sender
    2. `TYPE 1 REVERSE` packet
       - Reverses text input, echo to `STDOUT` and sends it back to sender (i.e. `input == "hello"`, `reverse == "olleh"`)
    3. `TYPE 2 ASCII` packet
       - Takes the sum of all ASCII char in the input, echoes sum to STDOUT and sends sum back to sender (i.e. `input == "hello"`, `ascii == 523`)
  - Use *POSIX pthreads*
  - Each packet has a 'job,' save jobs for future indexing features.  Organize/identify them however you want to.
  - If 'ECHO Mirror' is found and connected, send the outputs of the 'jobs' to ECHO Mirror as well
2. Echo Client
  - Written in Python3.7+
  - Connects to Echo Server via TCP
  - Sends Echo packets of a particular type to Echo Server, gets and immediate response back from the server
  - There can be multiple Echo Clients
3. Written in Python3.7+
  - ***FINDS*** and connects to Echo Server (your choice of implementation)
  - Takes echoed output and saves in a text file (\*.txt) (you can name the txt file whatever you would like to distinguish between the 'mirrors')
  - There can be multiple Echo Mirrors

Documentation is freeform.  Use whatever you like.  LaTeX is preferred.  Keep in mind to describe what your programs do, and how you implemented them.  Also, document how to run your code and what operating system/version it runs on.

## Notes
- Write using standard C/Python3 libraries, to avoid requiring extra installs
- **Linux is strongly recommended**
