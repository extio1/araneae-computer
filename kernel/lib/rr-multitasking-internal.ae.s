; ABI:
; BFF0 -> reference to scheduler object
; C000 -> scratch context buffer (14 slots x 8 bytes):
;   +0  (index 0): unused
;   +8  (index 1): ip  <- written manually from PIC after savec
;   +16 (index 2): fp
;   +24 (index 3): sp
;   +32 (index 4): r0
;   +40 (index 5): r1
;   +48 (index 6): r2
;   +56 (index 7): r3
;   +64 (index 8): r4
;   +72 (index 9): r5
;   +80 (index 10): r6
;   +88 (index 11): r7
;   +96 (index 12): rr
;
; Stack layout:
;   0xBFF0 : reset / main stack top
;   0xBEF0 : scheduler stack top  (256 bytes gap for main's frame)
;   0xBCF0 : Task index 0 stack top  (512 bytes for scheduler)
;   0xBAF0 : Task index 1 stack top  (512 bytes per task)
;   0xB8F0 : Task index 2 stack top  (512 bytes per task)

; Called from timer_handler after savec + ip write.
; Gets current task, marks it RUNNABLE, copies scratch buf to task.context,
; then resets the scheduler stack and jumps back into execute().
__call_scheduler_execute:
  mvv r0, 0x060008000000BFF0  ; scheduler reference
  call getCurrentTask__RRScheduler__1
  mv r0, rr                   ; r0 = interrupted task reference
  push r0
  ldi r1, 1                   ; RUNNABLE state
  call setState__Task__2
  pop r0                      ; restore task reference

  call __copy_scratch_to_context__1

  ; Reset scheduler stack (no frame accumulation) and re-enter execute().
  ; setsp sets both sp and fp to the given raw address.
  mvv r1, 0x060001000000BEF0  ; raw scheduler stack top
  setsp r1                    ; sp = fp = 0xBEF0  (fresh every interrupt)
  mvv r0, 0x060008000000BFF0  ; reload scheduler reference
  jmp execute__RRScheduler__1 ; tail-jump, execute() loops forever

__save_scheduler_ref__1:
  ; r0 holds reference to Scheduler object
  mv r1, 0x060008000000BFF0
  cpr r1, r0
  ret

; Copies all 12 context slots from scratch buffer (0xC000) to task's context buffer.
; r0 = task reference
__copy_scratch_to_context__1:
  ldf r1, r0, 0x0000d0b1d3799aca  ; r1 = task.context array ref
  mv r2, 0x060008000000C000       ; r2 = scratch array ref (base 0xC000)

  ldi r3, 1
  arg r4, r2, r3
  ars r1, r3, r4   ; ip
  ldi r3, 2
  arg r4, r2, r3
  ars r1, r3, r4   ; fp
  ldi r3, 3
  arg r4, r2, r3
  ars r1, r3, r4   ; sp
  ldi r3, 4
  arg r4, r2, r3
  ars r1, r3, r4   ; r0
  ldi r3, 5
  arg r4, r2, r3
  ars r1, r3, r4   ; r1
  ldi r3, 6
  arg r4, r2, r3
  ars r1, r3, r4   ; r2
  ldi r3, 7
  arg r4, r2, r3
  ars r1, r3, r4   ; r3
  ldi r3, 8
  arg r4, r2, r3
  ars r1, r3, r4   ; r4
  ldi r3, 9
  arg r4, r2, r3
  ars r1, r3, r4   ; r5
  ldi r3, 10
  arg r4, r2, r3
  ars r1, r3, r4   ; r6
  ldi r3, 11
  arg r4, r2, r3
  ars r1, r3, r4   ; r7
  ldi r3, 12
  arg r4, r2, r3
  ars r1, r3, r4   ; rr

  ret

__save_context__Task__1:
  ; kept for API compatibility (declared as public method __save_context in Task class)
  ret

__restore_context__Task__1:
  ; r0 holds reference to task object
  ldf r1, r0, 0x0000d0b1d3799aca   ; r1 = task.context array ref
  ; Enable interrupts right before loadc to minimize the race window.
  mv r2, 0x0100010000000001        ; true
  mv r0, 0x010008000000F010        ; ref to QueueInterrupts
  cp r0, r2                        ; QueueInterrupts = true
  mv r0, 0x010008000000F00C        ; ref to InterruptsAllowed
  cp r0, r2                        ; InterruptsAllowed = true
  loadc r1
  hlt

; Called via vcall from execute() when a NEW task (state==0) is found.
; r0 = task reference.
; Switches to the per-task dedicated stack (based on scheduler's currentTaskIndex),
; enables interrupts, then jumps to __task_start__.
__run_on_own_stack__Task__1:
  ; r0 = task ref — save it while we read the scheduler
  push r0

  mvv r1, 0x060008000000BFF0          ; scheduler ref
  ldf r2, r1, 0x7c7378ac94bcb873      ; r2 = currentTaskIndex (tagged int)

  pop r0                               ; restore task ref

  ; Select per-task stack top by index (0 -> 0xBCF0, 1 -> 0xBAF0, 2 -> 0xB8F0)
  ldi r1, 0
  eq r3, r2, r1
  br r3, .stack0, .not0
.stack0:
  mvv r1, 0x060001000000BCF0
  jmp .got_stack
.not0:
  ldi r1, 1
  eq r3, r2, r1
  br r3, .stack1, .stack2
.stack1:
  mvv r1, 0x060001000000BAF0
  jmp .got_stack
.stack2:
  mvv r1, 0x060001000000B8F0
.got_stack:
  ; r1 = raw per-task stack top, r0 = task ref

  setsp r1                            ; sp = fp = task stack top

  jmp __task_start__                  ; r0 still holds task ref

; Trampoline executed on the per-task stack.
; r0 = task reference.
; Calls run() via vtable, handles normal completion, and re-enters the scheduler.
__task_start__:
  ; Enable interrupts so the task can be preempted
  mv r2, 0x0100010000000001          ; true
  mv r3, 0x010008000000F00C          ; InterruptsAllowed ref
  cp r3, r2
  mv r3, 0x010008000000F010          ; QueueInterrupts ref
  cp r3, r2

  push r0                             ; save task ref on task's own stack
  vcall r0, 0x000006531a4e41a9        ; call run()  (hash of 'run__1')
  pop r0                              ; restore task ref after run() returns

  ; Disable interrupts while we update state
  mv r1, 0x0100010000000000          ; false
  mv r2, 0x010008000000F00C          ; InterruptsAllowed ref
  cp r2, r1
  mv r2, 0x010008000000F010          ; QueueInterrupts ref
  cp r2, r1

  ; Mark task as TERMINATED (3)
  ldi r1, 3
  call setState__Task__2

  ; Switch to scheduler stack and re-enter execute() fresh
  mvv r1, 0x060001000000BEF0
  setsp r1                            ; sp = fp = scheduler stack top
  mvv r0, 0x060008000000BFF0        ; scheduler ref
  jmp execute__RRScheduler__1
