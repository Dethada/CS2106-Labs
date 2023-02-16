# Edge Cases

- Start a `/bin/sleep 5 &`, then wait for the sleep to exit, but dont run any other commands except
  `quit`.
  - Kills the sleep but its already exited
