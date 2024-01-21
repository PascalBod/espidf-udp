This python program waits on port 3333 for UDP messages. Every received message is printed, with the IP address and port of the sending application.

Must be run with python3.

To check that the server works, the *nc* command can be used, on Linux:
```shell
$ nc -u localhost 3333
```

If run from another computer, `localhost` must be replaced by the IP address of the computer where the python program runs.
