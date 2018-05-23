# ncpm7xx-flashtool

Takes a bootblock image or a u-boot image and adds a bootblock header so that it
can be booted on a NPCM7xx BMC.

## Building

```bash
git clone git@github.com:Nuvoton-Israel/npcm7xx-flashtool.git
cd npcm7xx-flashtool
make
sudo make install
```

## Usage

```bash
create_image <--bootblock | --uboot>
             [--fiu0_drd_cfg=M] [--fiu_clk_divider=N]
             [--mc_freq=F] [--cpu_freq=F] [--mc_cfg=N]
             <input file> <output file>
```

Where `M` is an unsigned hexadecimal number which will fit in 32 bits, each `F`
is an unsigned decimal number representing a frequency in MHz from the list
below, and each `N` is an unsigned hexadecimal number which will fit in 8 bits.

Values for mc\_freq:  333, 444, 500, 600, 666, 700, 720, 750, 775, 787, 800, 825,
                      850, 900, 950, 1000, 1060
Values for cpu\_freq: 333, 500, 600, 666, 700, 720, 750, 800, 825, 850, 900,
                      950, 1000, 1060
