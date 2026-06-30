[section code]
__init_timer:
  usp 8
  mv fp[8], 0x060008000000E040          ; reference to CyclesSignalPeriod = 0xE000(base) + 0x0040(offset for device)
                                        ; MakeRef(TYPE_ULONG, F_GLOBAL, 0xE040)
  ldi r0, 150                           ; CyclesSignalPeriod = 15000000
  cp fp[8], r0

  ret

__init_pic:
  usp 16
  push  r0

  mv fp[8], 0x040008000000F100         ; reference to handlers-map[0]
                                       ; MakeRef(TYPE_UINT, F_GLOBAL, 0xF100)
  ldi r0, timer_handler                ;
  cp fp[8], r0                         ; handlers-map[0] = timer_handler

  ; InterruptsAllowed = false
  mv fp[8], 0x010008000000F00C         ; reference to InterruptsAllowed
                                       ; MakeRef(TYPE_BOOL, F_GLOBAL, 0xF00C)
  mv fp[16], 0x0100010000000000        ; value 0 (false)
  cp fp[8], fp[16]

  ; QueueInterrupts = false
  mv fp[8], 0x010008000000F010         ; reference to QueueInterrupts
  mv fp[16], 0x0100010000000000        ; value 0 (false)
  cp fp[8], fp[16]

  pop r0
  ret

; No-op: queueing stays disabled always
__pic_enable_queueing__0:
  ; QueueInterrupts = true
  mv fp[8], 0x010008000000F010         ; reference to QueueInterrupts
                                       ; MakeRef(TYPE_BOOL, F_GLOBAL, 0xF010)
  mv fp[16], 0x0100010000000001        ; value 1 (true)
                                       ; MakeRef(TYPE_BOOL, F_IMM, 0)
  cp fp[8], fp[16]
  ret

__pic_disable_queueing__0:
  ; QueueInterrupts = true
  mv fp[8], 0x010008000000F010         ; reference to QueueInterrupts
                                       ; MakeRef(TYPE_BOOL, F_GLOBAL, 0xF010)
  mv fp[16], 0x0100010000000000        ; value 0 (false)
                                       ; MakeRef(TYPE_BOOL, F_IMM, 0)
  cp fp[8], fp[16]
  ret

__pic_enable_interrupts__0:
  usp 16
  mv fp[8], 0x010008000000F00C         ; reference to InterruptsAllowed
  mv fp[16], 0x0100010000000001        ; value 1 (true)
  cp fp[8], fp[16]
  ret

__pic_disable_interrupts__0:
  usp 16
  mv fp[8], 0x010008000000F00C         ; reference to InterruptsAllowed
  mv fp[16], 0x0100010000000000        ; value 0 (false)
  cp fp[8], fp[16]
  ret

timer_handler:
  ; PIC dispatches via jmp, so fp/sp/r0-r7/rr hold the interrupted task's state.
  savec

  ; Disable interrupts immediately after savec to prevent a nested timer fire
  ; from overwriting scratch buffer 0xC000 while we read it.
  ; Direct writes — cannot call __pic_disable_interrupts__0 (it uses fp[8]/fp[16]
  ; which would corrupt the interrupted task's frame).
  mv r1, 0x0100010000000000    ; false
  mv r0, 0x010008000000F00C    ; ref to InterruptsAllowed
  cp r0, r1                    ; InterruptsAllowed = false
  mv r0, 0x010008000000F010    ; reference to QueueInterrupts
  cp r0, r1                    ; QueueInterrupts = false

  ; Copy interrupted ip from PIC to scratch buffer
  mv r0, 0x060008000000C008   ; ref to scratch buf ip slot (0xC008)
  mv r1, 0x060008000000F000   ; ref to PIC InterruptedInstructionAddress
  cp r0, r1                   ; scratch[ip] = interrupted ip

  call __call_scheduler_execute
  ; return here only if no more tasks to execute
  hlt
