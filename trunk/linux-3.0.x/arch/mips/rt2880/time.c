/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     timer setup for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timex.h>

#include <linux/jiffies.h>
#include <linux/delay.h>

#include <asm/mipsregs.h>
#include <asm/ptrace.h>
#include <asm/hardirq.h>
#include <asm/div64.h>
#include <asm/cpu.h>
#include <asm/time.h>
#if defined (CONFIG_RALINK_SYSTICK_COUNTER) && defined (CONFIG_RALINK_MT7621)
#include <asm/gic.h>		/* to turn off(mask) each VPE's local timer interrupt */
#endif

#include <asm/rt2880/generic.h>
#include <asm/rt2880/prom.h>
#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/surfboard.h>

extern unsigned int mips_hpt_frequency;
extern u32 mips_cpu_feq;
extern u32 surfboard_sysclk;

#if defined (CONFIG_RALINK_SYSTICK_COUNTER)
/*
 *  === Ralink systick clock source device implementation ===
 */
static cycle_t ra_systick_read(struct clocksource *cs)
{
	return (*((volatile u32 *)(RALINK_COUNT)));
}

static struct clocksource ra_systick_clocksource = {
	.name		= "Ralink Systick Timer",
	.mask		= 0xffff,
	.read		= ra_systick_read,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static int ra_systick_clocksource_init(void)
{
	ra_systick_clocksource.rating = 350;
	clocksource_register_hz(&ra_systick_clocksource, 50000);

	return 0;
}

/*
 * === Ralink systick clockevent device implementation ===
 */
struct clock_event_device ra_systick;
extern int cp0_timer_irq_installed;

/* Ralink Systick clockevent handler on CPU0. */
irqreturn_t ra_systick_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd;
	u32 compare;

	cd = &ra_systick;

	compare = (*((volatile u32 *)(RALINK_COMPARE)));
	(*((volatile u32 *)(RALINK_COMPARE))) = compare;

	cd->event_handler(cd);

	return IRQ_HANDLED;
}

/* In broadcast mode, timer interrupt only happens on the first CPU. */
struct irqaction ra_systick_irqaction = {
	.handler	= ra_systick_interrupt,
	.flags		= IRQF_PERCPU | IRQF_TIMER,
	.name		= "ralink_timer",
};

static void ra_systick_set_clock_mode(enum clock_event_mode mode, struct clock_event_device *evt)
{
	/* Nothing to do ...  */
}

static int ra_systick_next_event(unsigned long delta, struct clock_event_device *evt)
{
	unsigned int cnt, next_cnt;
	int res;

	cnt = (*((volatile u32 *)(RALINK_COUNT)));
	cnt += delta;
	(*((volatile u32 *)(RALINK_COMPARE))) = cnt;

	next_cnt = (*((volatile u32 *)(RALINK_COUNT)));
	res = ((int)(next_cnt - cnt) > 0) ? -ETIME : 0;

	return res;
}

static void ra_systick_event_handler(struct clock_event_device *dev)
{
	/* shouldn't be here  */
}

#if defined (CONFIG_RALINK_MT7621)
DEFINE_SPINLOCK(ra_teststat_lock);

/*
 * Broadcast handler for MT7621 VPE0, VPE1, VPE2 and VPE3 dummy clockevent device.
 * This function is registered in arch/mips/kernel/cevt-r4k.c.
 *
 * The function is in timer irq context.
 */
void ra_systick_event_broadcast(const struct cpumask *mask)
{
	u32 reg;
	int i;
	unsigned long flags;

	/*
	 * Mailbox design.
	 *
	 * The IPI VPE sender writes the signal bit to RALINK_TESTSTAT register.
	 * So the receiver VPEs can judge "ipi_call" or "broadcast" event by
	 * RALINK_TESTSTAT register when receiving ipi_call interrupt.
	 * 
	 * Using spin_lock() to prevent other VPEs from accessing RALINK_TESTSTAT
	 * register at the same time.
	 */
	spin_lock_irqsave(&ra_teststat_lock, flags);
	reg = (*((volatile u32 *)(RALINK_TESTSTAT)));
	for_each_cpu(i, mask)
		reg |= ((0x1UL) << i);
	(*((volatile u32 *)(RALINK_TESTSTAT))) = reg;
	spin_unlock_irqrestore(&ra_teststat_lock, flags);

#if defined (CONFIG_MIPS_GIC_IPI)
	/* send IPI to other VPEs, using "ipi_call" GIC(60~63), MIPS int#2  */
	for_each_cpu(i, mask)
		gic_send_ipi(plat_ipi_call_int_xlate(i));
#endif
}
#endif

static int ra_systick_clockevent_init(void)
{
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *cd;
	unsigned int irq;
#if defined (CONFIG_RALINK_MT7621)
	unsigned int i;
	unsigned long _gic_base;
#endif

	/*
	 * With vectored interrupts things are getting platform specific.
	 * get_c0_compare_int is a hook to allow a platform to return the
	 * interrupt number of it's liking.
	 */
	irq = MIPS_CPU_IRQ_BASE + cp0_compare_irq;

	cd = &ra_systick;

	cd->features		= CLOCK_EVT_FEAT_ONESHOT;
	cd->name		= "Ralink System Tick Counter";
	clockevent_set_clock(cd, 50000);
	cd->max_delta_ns	= clockevent_delta2ns(0x7fff, cd);
	cd->min_delta_ns	= clockevent_delta2ns(0x3, cd);
#if defined (CONFIG_RALINK_MT7621)
	/* must be lower than MIPS original cd rating(300) to activate "broadcast mode" */
	cd->rating		= 250;
#else
	/* must be higher than MIPS original cd rating(300). */
	cd->rating		= 350;
#endif
	cd->irq			= irq;
	cd->cpumask		= cpumask_of(cpu);
	cd->set_next_event	= ra_systick_next_event;
	cd->set_mode		= ra_systick_set_clock_mode;
	cd->event_handler	= ra_systick_event_handler;

#if defined (CONFIG_RALINK_MT7621)
	/* Program MIPS GIC to turn off(mask) each VPE's local timer interrupt.
	 * "_gic_base" is for GIC read/write macro.
	 */
	_gic_base = (unsigned long) ioremap_nocache(GIC_BASE_ADDR, GIC_ADDRSPACE_SZ);
	for (i = 0; i < NR_CPUS /* numvpes */; i++) {
		GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);
		GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_RMASK), GIC_VPE_SMASK_TIMER_MSK);
	}
#endif

	/* install timer irq handler before MIPS BSP code. */
	if (!cp0_timer_irq_installed){
		cp0_timer_irq_installed = 1;
		setup_irq(irq, &ra_systick_irqaction);
	}

	clockevents_register_device(cd);

	/* Enable ralink system count register */
	(*((volatile u32 *)(RALINK_MCNT_CFG))) = 0x3;

	return 0;
}
#endif


#if defined (CONFIG_RALINK_MT7621)
#define LPS_PREC 8
/*
 *  Re-calibration lpj(loop-per-jiffy).
 *  (derived from kernel/calibrate.c)
 */
static int udelay_recal(void)
{
	unsigned int i, lpj = 0;
	unsigned long ticks, loopbit;
	int lps_precision = LPS_PREC;

	lpj = (1<<12);

	while ((lpj <<= 1) != 0) {
		/* wait for "start of" clock tick */
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */;

			/* Go .. */
		ticks = jiffies;
		__delay(lpj);
		ticks = jiffies - ticks;
		if (ticks)
			break;
	}

	/*
	 * Do a binary approximation to get lpj set to
	 * equal one clock (up to lps_precision bits)
	 */
	lpj >>= 1;
	loopbit = lpj;
	while (lps_precision-- && (loopbit >>= 1)) {
		lpj |= loopbit;
		ticks = jiffies;
		while (ticks == jiffies)
				/* nothing */;
		ticks = jiffies;
		__delay(lpj);
		if (jiffies != ticks)   /* longer than 1 tick */
			lpj &= ~loopbit;
	}
	printk("%d CPUs re-calibrate udelay(lpj = %d)\n", NR_CPUS, lpj);

	for(i=0; i< NR_CPUS; i++)
		cpu_data[i].udelay_val = lpj;

#if defined (CONFIG_RALINK_CPUSLEEP) && defined (CONFIG_RALINK_MT7621)
	lpj = (*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x10)));
	lpj &= ~(0xF << 8);
	lpj |=  (0xA << 8);
	(*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x10))) = lpj;
#endif

	return 0;
}
device_initcall(udelay_recal);
#endif


void __init plat_time_init(void)
{
	mips_hpt_frequency = mips_cpu_feq / 2;

#if defined (CONFIG_RALINK_SYSTICK_COUNTER)
	ra_systick_clockevent_init();
	ra_systick_clocksource_init();
#endif

#if 0
	printk("options: 0x%08lX, ases: 0x%08lX, isa_level: 0x%08X, icache.flags: 0x%08X, dcache.flags: 0x%08X,"
	       "icache.linesz: %d, dcache.linesz: %d, cp0_compare_irq: %d\n",
		cpu_data[0].options,
		cpu_data[0].ases,
		cpu_data[0].isa_level,
		cpu_data[0].icache.flags,
		cpu_data[0].dcache.flags,
		cpu_data[0].icache.linesz,
		cpu_data[0].dcache.linesz,
		cp0_compare_irq);
#endif
}

u32 get_surfboard_sysclk(void)
{
	return surfboard_sysclk;
}

EXPORT_SYMBOL(get_surfboard_sysclk);

