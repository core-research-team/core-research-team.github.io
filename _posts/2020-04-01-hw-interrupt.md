---
layout: post
title:  "hw interrupt"
author: "wwwlk"
comments: true
tags: [web]
---

라온화이트햇 핵심연구팀 조진호
jhjo@raoncorp.com

# HW Interrupt

인터럽트는 중단을 의미한다. 인터럽트가 발생하면 테이블에 등록된 핸들러를 실행하고, 다시 원래 코드로 돌아간다. 디바이스가 인터럽트를 발생시키면 실행하던 프로세스는 하던 일을 멈추고 인터럽트 핸들러를 실행한다. 커널은 이를 인식해 디바이스의 상태가 변한것을 인식하는 것이다.

아래부터 리눅스 커널에서 인터럽트를 처리하는 과정이다. 하드웨어 인터럽트 발생시, 인터럽트 코드를 실행하기 전 현재 하던 레지스터들을 스택에 푸시한다.

```c
static inline int __must_check
request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev)
{
	return request_threaded_irq(irq, handler, NULL, flags, name, dev);
}-
```

`/include/linux/interrupt.h` 인터럽트 초기화 합수 `request_irq`

- `irq`: 인터럽트 번호
- `handler`: 호출될 핸들러 주소
- `flags`: 속성 플래그
- `name`: 이름
- `dev`: 핸들러를 전달하는 파라미터, 보통 디바이스 드라이버 구조체

```c
ret = request_irq(res2->start, c67x00_irq, 0, pdev->name, c67x00);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto request_irq_failed;
	}
```

이런식으로 인터럽트 큐를 등록해 사용하게 된다.

```c
static irqreturn_t c67x00_irq(int irq, void *__dev)
{
	struct c67x00_device *c67x00 = __dev;
	struct c67x00_sie *sie;
	u16 msg, int_status;
	int i, count = 8;

	int_status = c67x00_ll_hpi_status(c67x00);
	if (!int_status)
		return IRQ_NONE;

	while (int_status != 0 && (count-- >= 0)) {
		c67x00_ll_irq(c67x00, int_status);
		for (i = 0; i < C67X00_SIES; i++) {
			sie = &c67x00->sie[i];
			msg = 0;
			if (int_status & SIEMSG_FLG(i))
				msg = c67x00_ll_fetch_siemsg(c67x00, i);
			if (sie->irq)
				sie->irq(sie, int_status, msg);
		}
		int_status = c67x00_ll_hpi_status(c67x00);
	}

	if (int_status)
		dev_warn(&c67x00->pdev->dev, "Not all interrupts handled! "
			 "status = 0x%04x\n", int_status);

	return IRQ_HANDLED;
}
```

인터럽트 핸들러는 파라미터로 `irq`번호와 디바이스를 받게 된다. 위에서 큐에 등록할 때 `request_threaded_irq`라는 함수에 그대로 넣는 것을 확인할 수 있었다.

```c
int request_threaded_irq(unsigned int irq, irq_handler_t handler,
			 irq_handler_t thread_fn, unsigned long irqflags,
			 const char *devname, void *dev_id)
{
	struct irqaction *action;
	struct irq_desc *desc;
	int retval;

	...

	// 1
	desc = irq_to_desc(irq);
	if (!desc)
		return -EINVAL;

	// 2
	action = kzalloc(sizeof(struct irqaction), GFP_KERNEL);
	if (!action)
		return -ENOMEM;

	action->handler = handler;
	action->thread_fn = thread_fn;
	action->flags = irqflags;
	action->name = devname;
	action->dev_id = dev_id;
```

`/kernel/irq/manage.c`에 구현되어있다.

1. `irq`번호로 인터럽트 디스크립터를 가져온다.
2. `irqaction`구조체를 만들고 값을 태워넣는다.

```c
const void *free_irq(unsigned int irq, void *dev_id)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irqaction *action;
	const char *devname;

	if (!desc || WARN_ON(irq_settings_is_per_cpu_devid(desc)))
		return NULL;

#ifdef CONFIG_SMP
	if (WARN_ON(desc->affinity_notify))
		desc->affinity_notify = NULL;
#endif

	action = __free_irq(desc, dev_id);

	if (!action)
		return NULL;

	devname = action->name;
	kfree(action);
	return devname;
}
EXPORT_SYMBOL(free_irq);
```

`free_irq`로 등록된 `irq`를 지울 수 있다.

```c
/*
 * do_IRQ handles all normal device IRQ's (the special
 * SMP cross-CPU interrupts have their own specific
 * handlers).
 */
__visible unsigned int __irq_entry do_IRQ(struct pt_regs *regs)
{
	struct pt_regs *old_regs = set_irq_regs(regs);
	struct irq_desc * desc;
	/* high bit used in ret_from_ code  */
	unsigned vector = ~regs->orig_ax;

	entering_irq();

	/* entering_irq() tells RCU that we're not quiescent.  Check it. */
	RCU_LOCKDEP_WARN(!rcu_is_watching(), "IRQ failed to wake up RCU");

	desc = __this_cpu_read(vector_irq[vector]);
	if (likely(!IS_ERR_OR_NULL(desc))) {
		if (IS_ENABLED(CONFIG_X86_32))
			handle_irq(desc, regs);
		else
			generic_handle_irq_desc(desc);
	} else {
		ack_APIC_irq();

		if (desc == VECTOR_UNUSED) {
			pr_emerg_ratelimited("%s: %d.%d No irq handler for vector\n",
					     __func__, smp_processor_id(),
					     vector);
		} else {
			__this_cpu_write(vector_irq[vector], VECTOR_UNUSED);
		}
	}

	exiting_irq();

	set_irq_regs(old_regs);
	return 1;
}
```

`/arch/arm/kernel/irq.c`에 인터럽트 벡터 테이블의 `irq`핸들러가 있다.

```c
struct pt_regs *old_regs = set_irq_regs(regs);
```

첫출의 코드로 현재 레지스터를 저장하고, 

```c
set_irq_regs(old_regs);
```

마지막에 다시 원래 프로세스로 돌아간다.

```c
struct pt_regs {
/*
 * C ABI says these regs are callee-preserved. They aren't saved on kernel entry
 * unless syscall needs a complete, fully filled "struct pt_regs".
 */
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long rbp;
	unsigned long rbx;
/* These regs are callee-clobbered. Always saved on kernel entry. */
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long rax;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsi;
	unsigned long rdi;
/*
 * On syscall entry, this is syscall#. On CPU exception, this is error code.
 * On hw interrupt, it's IRQ number:
 */
	unsigned long orig_rax;
/* Return frame for iretq */
	unsigned long rip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long rsp;
	unsigned long ss;
/* top of stack page */
};
```

레지스터 구조체를 보면 진짜 그냥 레지스터다.

```c
static void gic_handle_shared_int(bool chained)
{
	unsigned int intr, virq;
	unsigned long *pcpu_mask;
	DECLARE_BITMAP(pending, GIC_MAX_INTRS);

	...
		if (chained)
			generic_handle_irq(virq);
		else
			do_IRQ(virq);
	}
}
```

사용예시이다. 하드웨어에서 인터럽트가 발생하면 `irq desc`를 참조해 디바이스 드라이버에 작성된 인터럽트 핸들러를 실행한다.

## 전체 흐름

![/assets/acaa16ca-36b1-4732-99b9-e88e160356ac/9c59d286-b683-4819-b0a0-efd32e819f46.png](/assets/acaa16ca-36b1-4732-99b9-e88e160356ac/9c59d286-b683-4819-b0a0-efd32e819f46.png)

아키텍처별로 구현이 다르다.