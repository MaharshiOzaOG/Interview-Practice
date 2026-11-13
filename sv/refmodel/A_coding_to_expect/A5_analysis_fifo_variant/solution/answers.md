# A5 — Answers

1. **Imp vs FIFO:**
   - **Imp:** `write()` is a **function** called inside the monitor's own thread. Prediction happens at once, nothing is buffered twice, and there are no extra processes.
   - **FIFO:** every item is stored in a mailbox, then a separate `run_phase` **task** wakes up to `get()` it. That costs an extra copy of the handle in the mailbox plus process switches per item, and both FIFOs keep growing if one side is slow. That is the "time consuming" part, and it matters most in emulation and long regressions.
2. **When FIFO wins:** when the checker must **wait** for something time-based (e.g. wait N cycles, wait for a register read, call a task-based model), or when you want simple "get one in, get one out" code with no queue of your own. A function `write()` can't block.
3. **Item reuse:** no. The FIFO stores the **handle**, not a copy. If the monitor reuses its object, the queued entries all change. The monitor should `create` a new item per transaction, or the scoreboard must `clone`.
