#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

static uint64_t g_nr_guest_instr = 0;

void nr_guest_instr_add(uint32_t n) {
  g_nr_guest_instr += n;
}

void monitor_statistic() {
  Log("total guest instructions = %ld", g_nr_guest_instr);
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END || nemu_state == NEMU_ABORT) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;

  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    exec_wrapper(print_flag);
    nr_guest_instr_add(1);

#ifdef DEBUG
    /* TODO: check watchpoints here. */
		WP *watch = get_head();
		uint32_t old_val, cur_val;
		bool flag = false;
		bool success = true;
		while (watch != NULL){
			old_val = watch->Val;
			cur_val = vaddr_read(expr(watch->expr, &success),4);
			if (old_val != cur_val){
				old_val = cur_val;
				flag = true;
			}
			watch = watch->next;
		}

		if (flag == true){
			nemu_state = NEMU_STOP;
			watch = get_head();
			char *header_format = "%-*s%-*s%s\n";
			char *format = "%-*d%-*s%.8x\n";
			printf(header_format, 8, "No", 15, "Expr", "cur_value");
			while (watch != NULL){
				printf(format, 8, watch->NO, 15, watch->expr, watch->Val);
				watch = watch->next;
			}


		}

#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) {
      if (nemu_state == NEMU_END) {
        printflog("\33[1;31mnemu: HIT %s TRAP\33[0m at eip = 0x%08x\n\n",
            (cpu.eax == 0 ? "GOOD" : "BAD"), cpu.eip - 1);
        monitor_statistic();
        return;
      }
      else if (nemu_state == NEMU_ABORT) {
        printflog("\33[1;31mnemu: ABORT\33[0m at eip = 0x%08x\n\n", cpu.eip);
        return;
      }
    }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
