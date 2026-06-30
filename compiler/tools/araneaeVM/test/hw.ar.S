[section code]
_start:
  ; Инициализация стека
  litint r4, 0x1000  ; начальный адрес стека
  mv sp, r4
  
  ; Вызов fib(50, 1, 0)
  litint r0, 50      ; N = 50
  litint r1, 1       ; A = 1
  litint r2, 0       ; B = 0
  call fib           ; fib(N, A, B)
  
  ; Вывод результата
  mv r3, r0
  out r3
  hlt

fib:
  ; Сохраняем текущее состояние
  st [sp], fp
  mv fp, sp
  add sp, sp, 8
  
  ; Получаем параметры
  ld r0, [fp + 8]    ; N
  ld r1, [fp + 16]   ; A
  ld r2, [fp + 24]   ; B
  
  ; Проверка условия: if N <= 50
  cmpc r0, 50
  jg return_b
  
  ; Подготовка к рекурсивному вызову: fib(N+1, A+B, A)
  
  ; Вычисляем N+1
  inc r5, r0         ; r5 = N+1
  
  ; Вычисляем A+B
  add r6, r1, r2     ; r6 = A+B
  
  ; Подготавливаем аргументы для вызова
  st [sp], r5        ; первый аргумент: N+1
  st [sp + 8], r6    ; второй аргумент: A+B
  st [sp + 16], r1   ; третий аргумент: A (старое значение)
  
  ; Вызов рекурсии
  call fib
  
  ; Восстанавливаем стек
  mv sp, fp
  ld fp, [fp]
  ret

return_b:
  ; Возвращаем B
  mv r0, r2
  
  ; Восстанавливаем стек
  mv sp, fp
  ld fp, [fp]
  ret

[section heap]
; Область для данных и стека
stack_area: times 1024 db 0