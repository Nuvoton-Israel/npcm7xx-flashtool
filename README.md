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
             <input file> <output file>
```

Where `M` is an unsigned hexadecimal number which will fit in 32 bits and `N` is
a unsigned hexadecimal number which will fit in 8 bits.
