/********************************************************************************************************
 * @file    cstartup_b92.S
 *
 * @brief   This is the boot file for B92
 *
 * @author  Driver & Zigbee Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#ifdef MCU_CORE_B92

#include "version_cfg.h"

 /**
 * @brief   Set "DP_THROUGH_SWIRE_DIS" as "1" to disable the "dp_through_swire" function. Set "DP_THROUGH_SWIRE_DIS" as "0" to enable the "dp_through_swire" function.
 *          Risk description:
 *          The "dp_through_swire" function is enabled by default.When DP and DM pin are used as GPIO function and trigger "dp_through_swire" timing,
 *          there may be a problem of miswriting the chip through swire to cause the chip to crash. Therefore, disable the "dp_through_swire" function in this startup file.(set "DP_THROUGH_SWIRE_DIS" as "1").
 *          If the chip has only dp pin but no sws pin, using the above scheme (set "DP_THROUGH_SWIRE_DIS" as "1") will cause the following problems.
 *          1. During fixture production, burning a firmware program that disables the "dp_through_swire"function into unprogrammed chip will cause the problem that chip can not be burned again due to swire communication failure.
 *          2. Burning a firmware program that disables the "dp_through_swire" function with BDT tool may result in failure of debugging due to swire communication failure.
 *          If developers evaluate that the above scheme (set "DP_THROUGH_SWIRE_DIS" as "1") cannot be used, they can only set "DP_THROUGH_SWIRE_DIS" to "0" to enable the "dp_through_swire" function. However, be careful not to make DP and DM pin trigger "dp_through_swire" timing.
 *          In USB applications, 1. calling the usb_set_pin(1) interface (equivalent to usb_set_pin_en) will re-enable the dp_through_swire function 2. calling the usb_set_pin(0) interface will disable the dp_through_swire function.
 */
#ifndef DP_THROUGH_SWIRE_DIS
#define DP_THROUGH_SWIRE_DIS        1
#endif

/* If using Andes toolchain, these two Macro are defined in it's toolchain */
/* If using std gcc toolchain, these two core registers(not standard risc-v core registers)
    should be defined here. */
#ifdef STD_GCC
#define mmisc_ctl  0x7D0
#define mcache_ctl 0x7CA
#endif

/**
 * @brief   The A3 chip has a problem (the A4 doesn't) :
            If the hardware board has 1.4V dcdc inductor component and uses 1.8V GPIO,
            it is necessary to set 1P4V to DCDC mode as soon as possible after the chip is powered on,
            otherwise there will be a voltage pulse in vdd1v2 and vddo3.
 */
#define CHIP_A3_VERSION_DCDC_1P4_LDO_2P0_EN         0

.extern pm_retention_register_recover

//According to the current internal seal flash model of B91m, the maximum time between the flash awakening and the operating flash is 20us, which leaves some margin and is set to 25us.
//When adding flash later, if tRES1 is greater than 25us, this waiting time needs to be updated. (flash tRES1, which can be viewed in the comments of the variable flash_support_mid.)
//(add by weihua.zhang 20230811)
#define EFUSE_LOAD_AND_FLASH_WAKEUP_LOOP_NUM        320 //25us

//The default driver configuration is 128K to ensure that the default driver configuration can work properly on all models.
//In actual use, please configure the sram size according to your own model, otherwise the efficiency will be affected.
//the most efficient use is:
//IRAM is used to store code, DRAM is used to store data, if the data-related segments are stored in IRAM, the efficiency will be affected.
//(add by weihua.zhang, confirmed by kaixin.chen and shujuan.chu 20230725)
#define SRAM_128K      1    //128KSRAM, 128KB-IRAM(96KB-Retention)/0KB-DRAM
#define SRAM_256K      2    //256KSRAM, 256KB-IRAM(96KB-Retention)/0KB-DRAM
#define SRAM_384K      3    //384KSRAM, 256KB-IRAM(96KB-Retention)/128KB-DRAM
#define SRAM_512K      4    //512KSRAM, 256KB-IRAM(96KB-Retention)/256KB-DRAM
#define SRAM_SIZE      SRAM_128K

#if (SRAM_SIZE == SRAM_256K)
    .equ __IRAM_2_EN,   1
    .equ __DRAM_1_EN,   0
    .equ __DRAM_2_EN,   0
    .equ __DRAM_DIS,    1
#elif (SRAM_SIZE == SRAM_384K)
    .equ __IRAM_2_EN,   1
    .equ __DRAM_1_EN,   1
    .equ __DRAM_2_EN,   0
    .equ __DRAM_DIS,    0
#elif (SRAM_SIZE == SRAM_512K)
    .equ __IRAM_2_EN,   1
    .equ __DRAM_1_EN,   1
    .equ __DRAM_2_EN,   1
    .equ __DRAM_DIS,    0
#else
    .equ __IRAM_2_EN,   0
    .equ __DRAM_1_EN,   0
    .equ __DRAM_2_EN,   0
    .equ __DRAM_DIS,    1
#endif

    .section .vectors, "ax"
    .global     __IRAM_2_EN
    .global     __DRAM_1_EN
    .global     __DRAM_2_EN
    .global     __DRAM_DIS

    //.org  and linker's relaxation (-flto) cannot be used at the same time
    //Pop corresponds to push. Before using .option norelax, use push to save the current .option configuration
    //and then modify .option. After using norelax, use pop to restore
    .option push
    .option norelax
    .org 0x0

    .extern FILE_VERSION
    .extern MANUFACTURER_CODE_TELINK
    .extern IMAGE_TYPE

    .global _RESET_ENTRY
    .type _RESET_ENTRY,@function

    .align 2
_RESET_ENTRY:
    j        _START
    //free the 6th ~ 7th byte to store the crc type of the bin file
	
    .word (FILE_VERSION)
    .org 0x12
    .short (MANUFACTURER_CODE_TELINK)
    .short (IMAGE_TYPE)

    .org 0x18
    .word (BIN_SIZE)

    .org 0x20
    .word ('T'<<24 | 'L'<<16 | 'N'<<8 | 'K')

        .org 0x24
    .word (0x3B0097A9)             //DREAD:   cmd:1x, addr:1x, data:2x, dummy:8
    //.word (0xBB0093B9)             //X2READ:  cmd:1x, addr:2x, data:2x, dummy:4
    //.word (0x6B0097AA)             //QREAD:   cmd:1x, addr:1x, data:4x, dummy:8
    //.word (0xEB0095BA)               //X4READ:  cmd:1x, addr:4x, data:4x, dummy:6

    .option pop
    .align 2

_START:
#if 0
    // add debug, PB4 output 1
    lui     t0, 0x80140
    li      t1, 0xef
    li      t2, 0x10
    sb      t1, 0x30a(t0)        //0x8014030a  PB oen     =  0xef
    sb      t2, 0x30b(t0)        //0x8014030b  PB output  =  0x10
#endif

    /* timer rst enable. */
    li      t0, 0x801401e2 /* reg_rst2 register address. */
    li      t1, 0x39       /* bit0=1 timer enable(reset velue 0x38). */
    sb      t1, 0x0(t0)    /* (*(volatile unsigned char*)(0x801401e2))= 0x39. */

    /* timer clk enable. */
    li      t0, 0x801401e6 /* tim_ctrl2 register address. */
    li      t1, 0x31       /* bit0=1 timer enable(reset velue 0x30). */
    sb      t1, 0x0(t0)    /* (*(volatile unsigned char*)(0x801401e6))= 0x31. */

    /* set timer watchdog value: 10s. */
    li      t0, 0x8014014c /* wt_target register address. */
    li      t1, 0xe4e1c00  /* watchdog value = 0xe4e1c00(10s * 24000000) */
    sw      t1, 0x0(t0)    /* (*(volatile unsigned long*)(0x8014014c))= 0xe4e1c00. */

    /* start timer watchdog. */
    li      t0, 0x80140142 /* tim_ctrl2 register address. */
    li      t1, 0x80       /* bit7=1 watchdog enable(reset velue 0x00). */
    sb      t1, 0x0(t0)    /* (*(volatile unsigned char*)(0x80140142))= 0x80. */

#if CHIP_A3_VERSION_DCDC_1P4_LDO_2P0_EN
    /* Set the power supply mode to DCDC_LDO. */
    li      t0, 0x801401e0
    li      t1, 0x903888a0
    sw      t1, 0x0(t0)     //reg_rst = 0x903888a0;

    li      t0, 0x801401e4
    li      t1, 0x1030a8a0
    sw      t1, 0x0(t0)     //reg_clk_en = 0x1030a8a0;

    li      t0, 0x80140183
    li      t1, 0x01
    sb      t1, 0x0(t0)     //reg_ana_len = 1;

    li      t0, 0x80140180
    li      t1, 0x0a
    sb      t1, 0x0(t0)     //reg_ana_addr = 0x0a;

    li      t0, 0x80140184
    li      t1, 0x91
    sb      t1, 0x0(t0)     //reg_ana_data(0) = 0x91;

    li      t0, 0x80140188
    li      t2, 0xf0
    li      t4, 0x10
    li      t5, 0
    li      t6, 10000
_ANA_CNT_WAIT:
    addi    t5, t5, 1
    bgeu    t5, t6, _ANA_TIMEROUT
    lb      t1, 0x0(t0)
    and     t3, t1, t2
    bne     t4, t3, _ANA_CNT_WAIT   //while(wait_count++ < 10000 && (!(reg_ana_buf_cnt & FLD_ANA_TX_BUFCNT))){}

_ANA_CYC_W:
    li      t0, 0x80140182
    li      t1, 0x60
    sb      t1, 0x0(t0)         //reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);

    li      t0, 0x80140182
    li      t2, 0x80
    li      t5, 0
    li      t6, 10000
_ANA_W_WAIT:
    addi    t5, t5, 1
    bgeu    t5, t6, _ANA_TIMEROUT
    lb      t1, 0x0(t0)
    and     t3, t1, t2
    beq     t3, t2, _ANA_W_WAIT //while(wait_count++ < 10000 && (reg_ana_ctrl & FLD_ANA_BUSY)){}

_ANA_W_END:
    li      t0, 0x80140182
    li      t1, 0x00
    sb      t1, 0x0(t0)         //reg_ana_ctrl = 0x00;
    j       _ANA_TIMEROUT_END

_ANA_TIMEROUT:
    li      t0, 0x801401ef
    li      t1, 0x20
    sb      t1, 0x0(t0)     //0x1401ef = 0x20; reboot
_ANA_TIMEROUT_END:
#endif

    /* Initialize global pointer */
    .option push
    .option norelax
    la     gp, __global_pointer$
    .option pop
    
#if DP_THROUGH_SWIRE_DIS
    lui     t0, 0x80100
    addi    t0 , t0 , 0x700
    li      t1, 0x00
    sb      t1, 0x501(t0)  //0x80100c01 -> 0x00  <7>: 0 swire_usb_dis 
#endif

    /* Initialize stack pointer */
    la     t0, _STACK_TOP
    mv     sp, t0

#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0
#endif

#ifdef __riscv_flen
    /* Enable FPU */
    li t0, 0x00006000
    csrrs t0, mstatus, t0
    /* Initialize FCSR */
    fscsr zero
#endif

    /* Initial machine trap-vector Base */
    la     t0, __vectors
    csrw   mtvec, t0

    /* Enable vectored external plic interrupt */
    csrsi  mmisc_ctl, 2

    /*vector mode enable bit (VECTORED) of the Feature Enable Register */
    lui     t0, 0xe4000
    li      t1, 0x02
    sw      t1, 0x0(t0)      //(*(volatile unsigned long*)(0xe4000000))= 0x02

    /* Enable I/D-Cache */
    csrr   t0,  mcache_ctl
    ori    t0,  t0,  1  #/I-Cache
    ori    t0,  t0,  2  #/D-Cache
    csrw   mcache_ctl,  t0
    fence.i

    /* Move retention reset from flash to sram */
_RETENTION_RESET_INIT:
    la     t1, _RETENTION_RESET_LMA_START
    la     t2, _RETENTION_RESET_VMA_START
    la     t3, _RETENTION_RESET_VMA_END
_RETENTION_RESET_BEGIN:
    bleu    t3, t2, _RAMCODE_INIT
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _RETENTION_RESET_BEGIN

#if 0
    /* Move retention from flash to sram */
_RETENTION_DATA_INIT:
    la     t1, _RETENTION_DATA_LMA_START
    la     t2, _RETENTION_DATA_VMA_START
    la     t3, _RETENTION_DATA_VMA_END
_RETENTION_DATA_INIT_BEGIN:
    bleu   t3, t2, _RAMCODE_INIT
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _RETENTION_DATA_INIT_BEGIN
#endif

    /* Move ramcode from flash to sram */
_RAMCODE_INIT:
    la     t1, _RAMCODE_LMA_START
    la     t2, _RAMCODE_VMA_START
    la     t3, _RAMCODE_VMA_END
_RAMCODE_INIT_BEGIN:
    bleu   t3, t2, _DATA_INIT
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _RAMCODE_INIT_BEGIN

    /* Move Data from flash to sram */
_DATA_INIT:
    la     t1, _DATA_LMA_START
    la     t2, _DATA_VMA_START
    la     t3, _DATA_VMA_END
_DATA_INIT_BEGIN:
    bleu   t3, t2, _ZERO_BSS
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _DATA_INIT_BEGIN

    /* Zero .bss section in sram */
_ZERO_BSS:
    lui    t0, 0
    la     t2, _BSS_VMA_START
    la     t3, _BSS_VMA_END
_ZERO_BSS_BEGIN:
    bleu   t3, t2, _ZERO_AES
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _ZERO_BSS_BEGIN

    /* Zero .aes section in sram */
_ZERO_AES:
    lui    t0, 0
    la     t2, _AES_VMA_START
    la     t3, _AES_VMA_END
_ZERO_AES_BEGIN:
    bleu   t3, t2, _FILL_STK
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _ZERO_AES_BEGIN

    /* Fill the remaining section in sram */
_FILL_STK:
#if 0
    lui    t0, 0x55555
    addi   t0, t0, 0x555
    la     t2, _BSS_VMA_END   //_BSS_VMA_END must be 4-byte aligned
    la     t3, _STACK_TOP     //_STACK_TOP   must be 4-byte aligned
_FILL_STK_BEGIN:
    bleu   t3, t2, _MAIN_FUNC
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _FILL_STK_BEGIN
#endif

    /* Jump to the main function */
_MAIN_FUNC:
    nop

    la     t0, main
    jalr   t0

    nop
    nop
    nop
    nop
    nop
_END:
    j    _END


 .section .retention_reset, "ax"
    .option push
    .option norelax
    .global _IRESET_ENTRY
    .type _IRESET_ENTRY,@function

    .align 2
_IRESET_ENTRY:
    /* Decide whether this is an NMI or cold reset */
    j       _ISTART

    .org 0x22
_ISTART:
#if 0
    // add debug, PB4 output 1
    lui     t0, 0x80140
    li      t1, 0xef
    li      t2, 0x10
    sb      t1, 0x30a(t0)        //0x8014030a  PB oen     =  0xef
    sb      t2, 0x30b(t0)        //0x8014030b  PB output  =  0x10
#endif

    /* timer rst enable. */
    li      t0, 0x801401e2 /* reg_rst2 register address. */
    li      t1, 0x39       /* bit0=1 timer enable(reset velue 0x38). */
    sb      t1, 0x0(t0)    /* (*(volatile unsigned char*)(0x801401e2))= 0x39. */

    /* timer clk enable. */
    li      t0, 0x801401e6 /* tim_ctrl2 register address. */
    li      t1, 0x31       /* bit0=1 timer enable(reset velue 0x30). */
    sb      t1, 0x0(t0)    /* (*(volatile unsigned char*)(0x801401e6))= 0x31. */

    /* set timer watchdog value: 10s. */
    li      t0, 0x8014014c /* wt_target register address. */
    li      t1, 0xe4e1c00  /* watchdog value = 0xe4e1c00(10s * 24000000) */
    sw      t1, 0x0(t0)    /* (*(volatile unsigned long*)(0x8014014c))= 0xe4e1c00. */

    /* start timer watchdog. */
    li      t0, 0x80140142 /* tim_ctrl2 register address. */
    li      t1, 0x80       /* bit7=1 watchdog enable(reset velue 0x00). */
    sb      t1, 0x0(t0)    /* (*(volatile unsigned char*)(0x80140142))= 0x80. */

    /* Initialize global pointer */
    la     gp, __global_pointer$
    .option pop

    /* Initialize stack pointer */
    la     t0, _STACK_TOP
    mv     sp, t0

#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0
#endif

#ifdef __riscv_flen
    /* Enable FPU */
    li t0, 0x00006000
    csrrs t0, mstatus, t0
    /* Initialize FCSR */
    fscsr zero
#endif

    /* Initial machine trap-vector Base */
    la     t0, __vectors
    csrw   mtvec, t0

    /* Enable vectored external plic interrupt */
    csrsi  mmisc_ctl, 2

    /*vector mode enable bit (VECTORED) of the Feature Enable Register */
    lui     t0, 0xe4000
    li      t1, 0x02
    sw      t1, 0x0(t0)      //(*(volatile unsigned long*)(0xe4000000))= 0x02

    /* Enable I/D-Cache */
    csrr   t0,  mcache_ctl
    ori    t0,  t0,  1  #/I-Cache
    ori    t0,  t0,  2  #/D-Cache
    csrw   mcache_ctl,  t0
    fence.i

 /* after retention, digital register will be lost, restore some register values that need to be used */
    la     t0, pm_retention_register_recover
    jalr   t0


    /* flash wakeup */
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t1, 0x0e
    sb      t1, 0x78c(t0)       //xip_stop      :0xA3FFFF1c = 0x0e
_MSPI_WAIT_1:
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    lb      t2, 0x798(t0)
    li      t3, 0x80
    beq     t3, t2, _MSPI_WAIT_1//read reg_mspi_status FLD_MSPI_BUSY(0xA3FFFF28[bit7])
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t1, 0x06
    sb      t1, 0x78c(t0)       //xip_disable       :0xA3FFFF1c = 0x06
    lui     t0, 0x80140
    li      t1, 0x3f
    sb      t1, 0x331(t0)       //mspi ie enable    :0x80140331 = 0x3f
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t2, 0x80
    sb      t2, 0x778(t0)       //mspi_ctrl1        :0xA3FFFF08 = 0x80
    li      t3, 0xab
    sb      t3, 0x774(t0)       //wakeup_cmd        :0xA3FFFF04 = 0xab
_MSPI_WAIT_2:
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    lb      t2, 0x798(t0)
    li      t3, 0x80
    beq     t3, t2, _MSPI_WAIT_2//read reg_mspi_status FLD_MSPI_BUSY(0xA3FFFF28[bit7])
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t1, 0x0a
    sb      t1, 0x78c(t0)       //xip_enable        :0xA3FFFF1c = 0x0a

    /*flash wakeup need delay about 20us */
    /*efuse load need delay about 0us */
    li      t0, 0
    li      t1, EFUSE_LOAD_AND_FLASH_WAKEUP_LOOP_NUM
_WAIT_EFUSE_LOAD_AND_FLASH_WAKEUP_FINISH:
    addi    t0, t0 , 1
    bgeu    t1, t0 , _WAIT_EFUSE_LOAD_AND_FLASH_WAKEUP_FINISH

_MULTI_ADDRESS_BEGIN:
    lui     t0, 0xA3FFF
    addi    t0, t0 , 0x790
    la      t1, g_pm_mspi_cfg
    lw      t2, 0(t1)
    sw      t2, 0x790(t0)       //g_pm_mspi_cfg->0xA3FFFF20

#if 0
    /* Zero .bss section in sram */
_IZERO_BSS:
    lui    t0, 0
    la     t2, _BSS_VMA_START
    la     t3, _BSS_VMA_END
_IZERO_BSS_BEGIN:
    bleu   t3, t2, _IZERO_AES
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IZERO_BSS_BEGIN
#endif

    /* Zero .aes section in sram */
_IZERO_AES:
    lui    t0, 0
    la     t2, _AES_VMA_START
    la     t3, _AES_VMA_END
_IZERO_AES_BEGIN:
    bleu   t3, t2, _IFILL_STK
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IZERO_AES_BEGIN

    /* Fill the remaining section in sram */
_IFILL_STK:
#if 0
    lui    t0, 0x55555
    addi   t0, t0, 0x555
    la     t2, _BSS_VMA_END   //_BSS_VMA_END must be 4-byte aligned
    la     t3, _STACK_TOP     //_STACK_TOP   must be 4-byte aligned
_IFILL_STK_BEGIN:
    bleu   t3, t2, _IDATA_INIT
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IFILL_STK_BEGIN
#endif

    /* Move Data from flash to sram */
//Read the flash as late as possible to lengthen the time from the wake up to the operation of the flash.
_IDATA_INIT:
#if 0
    la     t1, _DATA_LMA_START
    la     t2, _DATA_VMA_START
    la     t3, _DATA_VMA_END
_IDATA_INIT_BEGIN:
    bleu   t3, t2, _IMAIN_FUNC
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _IDATA_INIT_BEGIN
#endif

    /* Jump to the main function */
_IMAIN_FUNC:
    nop

    la     t0, main
    jalr   t0

    nop
    nop
    nop
    nop
    nop
_IEND:
    j    _IEND


.text
    .global default_irq_entry
    .align 2

default_irq_entry:
1:  j 1b

       .weak trap_handler

 trap_handler:
1:  j 1b

    .macro INTERRUPT num
    .weak entry_irq\num
    .set entry_irq\num, default_irq_entry
    .long entry_irq\num
    .endm

#define VECTOR_NUMINTRS         47

    .section .ram_code, "ax"

    .global __vectors
    .balign 256


__vectors:
    /* Trap vector */
    .long trap_entry

    /* PLIC interrupt vector */
    .altmacro
    .set irqno, 1
    .rept VECTOR_NUMINTRS/*  .rept  .endr  */
    INTERRUPT %irqno
    .set irqno, irqno+1
    .endr
#endif
