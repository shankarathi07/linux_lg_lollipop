/*
 * Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/rpm-regulator.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_bus.h>

#include "mach/socinfo.h"
#include "acpuclock.h"
#include "acpuclock-krait.h"

static struct hfpll_data hfpll_data __initdata = {
    .mode_offset = 0x00,
    .l_offset = 0x08,
    .m_offset = 0x0C,
    .n_offset = 0x10,
    .config_offset = 0x04,
    .config_val = 0x7845C665,
    .has_droop_ctl = true,
    .droop_offset = 0x14,
    .droop_val = 0x0108C000,
    .low_vdd_l_max = 22,
    .nom_vdd_l_max = 42,
    .vdd[HFPLL_VDD_NONE] =       0,
    .vdd[HFPLL_VDD_LOW]  =  945000,
    .vdd[HFPLL_VDD_NOM]  = 1050000,
    .vdd[HFPLL_VDD_HIGH] = 1150000,
};

static struct scalable scalable[] __initdata = {
    [CPU0] = {
        .hfpll_phys_base = 0x00903200,
        .aux_clk_sel_phys = 0x02088014,
        .aux_clk_sel = 3,
        .sec_clk_sel = 2,
        .l2cpmr_iaddr = 0x4501,
        .vreg[VREG_CORE] = { "krait0", 1225000 },
        .vreg[VREG_MEM]  = { "krait0_mem", 1150000 },
        .vreg[VREG_DIG]  = { "krait0_dig", 1150000 },
        .vreg[VREG_HFPLL_A] = { "krait0_hfpll", 1800000 },
    },
    [CPU1] = {
        .hfpll_phys_base = 0x00903240,
        .aux_clk_sel_phys = 0x02098014,
        .aux_clk_sel = 3,
        .sec_clk_sel = 2,
        .l2cpmr_iaddr = 0x5501,
        .vreg[VREG_CORE] = { "krait1", 1225000 },
        .vreg[VREG_MEM]  = { "krait1_mem", 1150000 },
        .vreg[VREG_DIG]  = { "krait1_dig", 1150000 },
        .vreg[VREG_HFPLL_A] = { "krait1_hfpll", 1800000 },
    },
    [CPU2] = {
        .hfpll_phys_base = 0x00903280,
        .aux_clk_sel_phys = 0x020A8014,
        .aux_clk_sel = 3,
        .sec_clk_sel = 2,
        .l2cpmr_iaddr = 0x6501,
        .vreg[VREG_CORE] = { "krait2", 1225000 },
        .vreg[VREG_MEM]  = { "krait2_mem", 1150000 },
        .vreg[VREG_DIG]  = { "krait2_dig", 1150000 },
        .vreg[VREG_HFPLL_A] = { "krait2_hfpll", 1800000 },
    },
    [CPU3] = {
        .hfpll_phys_base = 0x009032C0,
        .aux_clk_sel_phys = 0x020B8014,
        .aux_clk_sel = 3,
        .sec_clk_sel = 2,
        .l2cpmr_iaddr = 0x7501,
        .vreg[VREG_CORE] = { "krait3", 1225000 },
        .vreg[VREG_MEM]  = { "krait3_mem", 1150000 },
        .vreg[VREG_DIG]  = { "krait3_dig", 1150000 },
        .vreg[VREG_HFPLL_A] = { "krait3_hfpll", 1800000 },
    },
    [L2] = {
        .hfpll_phys_base = 0x00903300,
        .aux_clk_sel_phys = 0x02011028,
        .aux_clk_sel = 3,
        .sec_clk_sel = 2,
        .l2cpmr_iaddr = 0x0500,
        .vreg[VREG_HFPLL_A] = { "l2_hfpll", 1800000 },
    },
};

/*
 * The correct maximum rate for 8064ab in 600 MHZ.
 * We rely on the RPM rounding requests up here.
 */
static struct msm_bus_paths bw_level_tbl[] __initdata = {
#ifdef CONFIG_GPU_OVERCLOCK
    [0] =  BW_MBPS(640), /* At least  80 MHz on bus. */
    [1] = BW_MBPS(1064), /* At least 133 MHz on bus. */
    [2] = BW_MBPS(1600), /* At least 200 MHz on bus. */
    [3] = BW_MBPS(2128), /* At least 266 MHz on bus. */
    [4] = BW_MBPS(3200), /* At least 400 MHz on bus. */
    [5] = BW_MBPS(4264), /* At least 533 MHz on bus. */
    [6] = BW_MBPS(5290), /* At least 533 MHz on bus. */
#else
    [0] = BW_MBPS(640), /* At least  80 MHz on bus. */
    [1] = BW_MBPS(1064), /* At least 133 MHz on bus. */
    [2] = BW_MBPS(1600), /* At least 200 MHz on bus. */
    [3] = BW_MBPS(2128), /* At least 266 MHz on bus. */
    [4] = BW_MBPS(3200), /* At least 400 MHz on bus. */
    [5] = BW_MBPS(4264), /* At least 533 MHz on bus. */
#endif
};

static struct msm_bus_scale_pdata bus_scale_data __initdata = {
    .usecase = bw_level_tbl,
    .num_usecases = ARRAY_SIZE(bw_level_tbl),
    .active_only = 1,
    .name = "acpuclk-8064",
};

static struct l2_level l2_freq_tbl[] __initdata = {
    [0]  = { {  192000, PLL_8, 0, 0x00 },  825000, 825000,  1 },
    [1]  = { {  288000, HFPLL, 2, 0x20 },  975000, 975000,  1 },
    [2]  = { {  384000, HFPLL, 2, 0x24 },  975000, 975000,  1 },
    [3]  = { {  486000, HFPLL, 2, 0x28 },  975000, 975000,  2 },
    [4]  = { {  540000, HFPLL, 1, 0x16 },  975000, 975000,  2 },
    [5]  = { {  594000, HFPLL, 1, 0x18 },  975000, 975000,  2 },
    [6]  = { {  648000, HFPLL, 1, 0x1A },  975000, 975000,  4 },
    [7]  = { {  702000, HFPLL, 1, 0x1C }, 1075000, 1075000, 4 },
    [8]  = { {  756000, HFPLL, 1, 0x1E }, 1075000, 1075000, 4 },
    [9]  = { {  810000, HFPLL, 1, 0x20 }, 1075000, 1075000, 4 },
    [10] = { {  864000, HFPLL, 1, 0x22 }, 1075000, 1075000, 4 },
    [11] = { {  918000, HFPLL, 1, 0x24 }, 1075000, 1075000, 5 },
    [12] = { {  972000, HFPLL, 1, 0x26 }, 1075000, 1075000, 5 },
    [13] = { { 1026000, HFPLL, 1, 0x28 }, 1075000, 1075000, 5 },
    [14] = { { 1080000, HFPLL, 1, 0x2A }, 1075000, 1075000, 5 },
    [15] = { { 1134000, HFPLL, 1, 0x2C }, 1075000, 1075000, 5 },
    [16] = { { 1188000, HFPLL, 1, 0x2E }, 1075000, 1075000, 5 },
    { }
};

static struct acpu_level acpu_freq_tbl_slow[] __initdata = {
    { 1, {   192000, PLL_8, 0, 0x00 }, L2(0),   850000 },
    { 1, {   288000, HFPLL, 2, 0x20 }, L2(0),   875000 },
    //{ 0, {   384000, HFPLL, 0, 2, 0x20 }, L2(6),   875000 },
    //{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   875000 },
    { 1, {   540000, HFPLL, 1, 0x16 }, L2(5),   925000 },
    //{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),   900000 },
    //{ 0, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),  925000 },
    { 1, {   702000, HFPLL, 1, 0x1C }, L2(5),  950000 },
    //{ 0, {   756000, HFPLL, 1, 0, 0x1C }, L2(6),  975000 },
    //{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(6),  975000 },
    //{ 0, {   864000, HFPLL, 1, 0, 0x20 }, L2(6),  1000000 },
    //{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(6),  1000000 },
    //{ 0, {   972000, HFPLL, 1, 0, 0x24 }, L2(6),  1025000 },
    { 1, {  1026000, HFPLL, 1, 0x28 }, L2(5),  1050000 },
    //{ 0, {  1080000, HFPLL, 1, 0, 0x28 }, L2(15), 1075000 },
    //{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(15), 1075000 },
    { 1, {  1188000, HFPLL, 1, 0x2E }, L2(14), 1125000 },
    //{ 1, {  1242000, HFPLL, 1, 0, 0x2E }, L2(15), 1100000 },
    //{ 0, {  1296000, HFPLL, 1, 0, 0x30 }, L2(15), 1125000 },
    { 1, {  1350000, HFPLL, 1, 0x34 }, L2(14), 1150000 },
    //{ 0, {  1404000, HFPLL, 1, 0, 0x34 }, L2(15), 1137500 },
    //{ 1, {  1458000, HFPLL, 1, 0, 0x36 }, L2(15), 1137500 },
    { 1, {  1512000, HFPLL, 1, 0x3A }, L2(14), 1175000 },
    { 0, { 0 } }
};
static struct acpu_level acpu_freq_tbl_nom[] __initdata = {
    { 1, {   192000, PLL_8, 0, 0x00 }, L2(0),   800000 },
    { 1, {   288000, HFPLL, 2, 0x20 }, L2(0),   825000 },
    //{ 0, {   384000, HFPLL, 0, 2, 0x20 }, L2(6),   825000 },
    //{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   825000 },
    { 1, {   540000, HFPLL, 1, 0x16 }, L2(5),   875000 },
    //{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),   850000 },
    //{ 0, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),   875000 },
    { 1, {   702000, HFPLL, 1, 0x1C }, L2(5),   900000 },
    //{ 0, {   756000, HFPLL, 1, 0, 0x1C }, L2(6),  925000 },
    //{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(6),  925000 },
    //{ 0, {   864000, HFPLL, 1, 0, 0x20 }, L2(6),  950000 },
    //{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(6),  950000 },
    //{ 0, {   972000, HFPLL, 1, 0, 0x24 }, L2(6),  975000 },
    { 1, {  1026000, HFPLL, 1, 0x28 }, L2(5),  1000000 },
    //{ 0, {  1080000, HFPLL, 1, 0, 0x28 }, L2(15), 1025000 },
    //{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(15), 1025000 },
    { 1, {  1188000, HFPLL, 1, 0x2E }, L2(14), 1075000 },
    //{ 1, {  1242000, HFPLL, 1, 0, 0x2E }, L2(15), 1050000 },
    //{ 0, {  1296000, HFPLL, 1, 0, 0x30 }, L2(15), 1075000 },
    { 1, {  1350000, HFPLL, 1, 0x34 }, L2(14), 1100000 },
    //{ 0, {  1404000, HFPLL, 1, 0, 0x34 }, L2(15), 1087500 },
    //{ 1, {  1458000, HFPLL, 1, 0, 0x36 }, L2(15), 1087500 },
    { 1, {  1512000, HFPLL, 1, 0x3A }, L2(14), 1125000 },
    { 0, { 0 } }
};

static struct acpu_level acpu_freq_tbl_fast[] __initdata = {
    { 1, {   192000, PLL_8, 0, 0x00 }, L2(0),   750000 },
    { 1, {   288000, HFPLL, 2, 0x20 }, L2(0),   775000 },
    //{ 0, {   384000, HFPLL, 0, 2, 0x20 }, L2(6),   775000 },
    //{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   775000 },
    { 1, {   540000, HFPLL, 1, 0x16 }, L2(5),   825000 },
    //{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),   800000 },
    //{ 0, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),   825000 },
    { 1, {   702000, HFPLL, 1, 0x1C }, L2(5),   850000 },
    //{ 0, {   756000, HFPLL, 1, 0, 0x1C }, L2(6),   875000 },
    //{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(6),   875000 },
    //{ 0, {   864000, HFPLL, 1, 0, 0x20 }, L2(6),  900000 },
    //{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(6),  900000 },
    //{ 0, {   972000, HFPLL, 1, 0, 0x24 }, L2(6),  925000 },
    { 1, {  1026000, HFPLL, 1, 0x28 }, L2(5),  950000 },
    //{ 0, {  1080000, HFPLL, 1, 0, 0x28 }, L2(15), 975000 },
    //{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(15), 975000 },
    { 1, {  1188000, HFPLL, 1, 0x2E }, L2(14), 1025000 },
    //{ 1, {  1242000, HFPLL, 1, 0, 0x2E }, L2(15), 1000000 },
    //{ 0, {  1296000, HFPLL, 1, 0, 0x30 }, L2(15), 1025000 },
    { 1, {  1350000, HFPLL, 1, 0x34 }, L2(14), 1050000 },
    //{ 0, {  1404000, HFPLL, 1, 0, 0x34 }, L2(15), 1037500 },
    //{ 1, {  1458000, HFPLL, 1, 0, 0x36 }, L2(15), 1037500 },
    { 1, {  1512000, HFPLL, 1, 0x3A }, L2(14), 1075000 },
    { 0, { 0 } }
};

static struct acpu_level acpu_freq_tbl_faster[] __initdata = {
    { 1, {   192000, PLL_8, 0, 0x00 }, L2(0),   750000 },
    { 1, {   288000, HFPLL, 2, 0x20 }, L2(0),   775000 },
    //{ 0, {   384000, HFPLL, 0, 2, 0x20 }, L2(6),   775000 },
    //{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   775000 },
    { 1, {   540000, HFPLL, 1, 0x16 }, L2(5),   825000 },
    //{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),   800000 },
    //{ 0, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),   825000 },
    { 1, {   702000, HFPLL, 1, 0x1C }, L2(5),   850000 },
    //{ 0, {   756000, HFPLL, 1, 0, 0x1C }, L2(6),   862500 },
    //{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(6),   862500 },
    //{ 0, {   864000, HFPLL, 1, 0, 0x20 }, L2(6),   875000 },
    //{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(6),   875000 },
    // { 0, {   972000, HFPLL, 1, 0, 0x24 }, L2(6),   900000 },
    { 1, {  1026000, HFPLL, 1, 0x28 }, L2(5),   925000 },
    //{ 0, {  1080000, HFPLL, 1, 0, 0x28 }, L2(15),  950000 },
    //{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(15),  950000 },
    { 1, {  1188000, HFPLL, 1, 0x2E }, L2(14),  1000000 },
    //{ 1, {  1242000, HFPLL, 1, 0, 0x2E }, L2(15),  975000 },
    //{ 0, {  1296000, HFPLL, 1, 0, 0x30 }, L2(15), 1000000 },
    { 1, {  1350000, HFPLL, 1, 0x34 }, L2(14), 1025000 },
    //{ 0, {  1404000, HFPLL, 1, 0, 0x34 }, L2(15), 1012500 },
    //{ 1, {  1458000, HFPLL, 1, 0, 0x36 }, L2(15), 1012500 },
    { 1, {  1512000, HFPLL, 1, 0x3A }, L2(14), 1050000 },
    { 0, { 0 } }
};

static struct pvs_table pvs_tables[NUM_SPEED_BINS][NUM_PVS] __initdata = {
    [0][PVS_SLOW]    = {acpu_freq_tbl_slow, sizeof(acpu_freq_tbl_slow),     0 },
    [0][PVS_NOMINAL] = {acpu_freq_tbl_nom,  sizeof(acpu_freq_tbl_nom),  25000 },
    [0][PVS_FAST]    = {acpu_freq_tbl_fast, sizeof(acpu_freq_tbl_fast), 25000 },
    [0][PVS_FASTER]  = {acpu_freq_tbl_faster, sizeof(acpu_freq_tbl_faster), 25000 },
};

static struct acpuclk_krait_params acpuclk_8064_params __initdata = {
    .scalable = scalable,
    .scalable_size = sizeof(scalable),
    .hfpll_data = &hfpll_data,
    .pvs_tables = pvs_tables,
    .l2_freq_tbl = l2_freq_tbl,
    .l2_freq_tbl_size = sizeof(l2_freq_tbl),
    .bus_scale = &bus_scale_data,
    .pte_efuse_phys = 0x007000C0,
    .stby_khz = 192000,
};

static int __init acpuclk_8064_probe(struct platform_device *pdev)
{
    if (cpu_is_apq8064ab() ||
        SOCINFO_VERSION_MAJOR(socinfo_get_version()) == 2) {
        acpuclk_8064_params.hfpll_data->low_vdd_l_max = 37;
        acpuclk_8064_params.hfpll_data->nom_vdd_l_max = 74;
    }
    
    return acpuclk_krait_init(&pdev->dev, &acpuclk_8064_params);
}

static struct platform_driver acpuclk_8064_driver = {
    .driver = {
        .name = "acpuclk-8064",
        .owner = THIS_MODULE,
    },
};

static int __init acpuclk_8064_init(void)
{
    return platform_driver_probe(&acpuclk_8064_driver,
                                 acpuclk_8064_probe);
}
device_initcall(acpuclk_8064_init);