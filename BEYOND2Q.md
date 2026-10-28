# Galaxy S10+ (beyond2q, SM-G9750) on the guacamole tree

The S10+ is the same SoC as guacamole (SM8150), so this port is a second
board on the guacamole tree, not a new port. Board header:
`shim/boards/beyond2q.h`.

## Sources

- Base DTB: the stock `boot.img` (G9750ZHU8HZE1 / TGY) carries two appended
  DTBs, SM8150 v1 and v2. The phone is v2.1, so v2 applies.
- Overlay: the live tree reports "Samsung BEYOND2Q CHNHK PROJECT Rev09". The
  touch, key and panel values match dtbo entry 1, which was read first.
- Live FDT: `dtb/beyond2q-live.dtb`, pulled 2026-10-28 from rooted Android.
  `tools/fdt-audit.py dtb/beyond2q-live.dtb --device beyond2q` passes all 20
  checks.
- Live device: rooted Android over adb. Root is Magisk in the recovery
  partition, entered with `adb reboot recovery`.

## Board facts

| Item | S10+ (beyond2q) | guacamole | Source |
|---|---|---|---|
| GIC, UFS, SPMI, USB, PHYs, CPU MPIDRs | same | same | `tools/fdt-audit.py` on the base DTB: all ok |
| restart_reason | `0x146bf65c`, `0x77665500` = download mode | same address, = ABL | live |
| cont_splash_region | `0x9C400000 + 0x02000000` (32 MiB) | `0x9C000000 + 0x02400000` | base DTB |
| Panel | S6E3HA9 AMB644WF01, 1440x3040, DSC cmd mode, dual DSI | S6E3HC2, 1440x3120, DSC cmd mode | dtbo |
| Touch | `sec,sec_ts` at I2C 0x48 on QUP SE17, SDA/SCL tlmm 55/56, IRQ tlmm 87, firmware `y771` | S6SY761 at 0x48 on SE17, IRQ tlmm 122 | dtbo |
| Touch power | `s2dos05-ldo2` (dvdd) and `s2dos05-ldo3` (avdd), `sec,regulator_boot_on` | RPMh `ldoa17` | dtbo |
| Volume up | PM8150L GPIO 12 | PM8150 GPIO 6 | dtbo |
| Bixby key | PM8150L GPIO 8 | none | dtbo |
| DDR banks | `0x80000000+0x3cc00000`, `0xc0000000+0x80000000`, `0x140000000+0xc0000000` (6 GB) | three banks, 8/12 GB | live FDT |
| Clean bank | high only (`0x140000000`, 3 GiB). The mid bank has sec_debug/ramoops, camera (320 MiB), qseecom_ta, sec_debug_rdx_bootdev | mid and high are clean | live FDT |
| MARK_PA | `0x158000000` (kernel bank + `0x18000000`) | `0x198000000` | derived |
| Volume down | PM8150 PON RESIN (`qcom,pon_2`) | PM8150 GPIO 7 | live FDT |
| Panel size | 70 x 148 mm | 76 x 163 mm | live FDT |

## Differences from guacamole

1. **Display scanout.** Guacamole relies on OnePlus ABL leaving the
   command-mode panel scanning the splash buffer. On the S10+, sboot leaves
   it idle after the splash frame. A CTL kick is accepted but the panel
   shows nothing new. The register captures and a lead theory (the RSC
   power-collapses the MDP between frames) are in
   `~/Projects/ios-beyond/NOTES.md` and `beyond2q-capture/`. This is the only
   S10+-specific display work. Everything after it is guacamole's CPU
   renderer.
2. **Boot.** Samsung has no `fastboot boot`. `shim/build.sh` makes a header-v2
   image for fastboot. The S10+ needs the stock header-v1 boot image with the
   kernel swapped in by magiskboot, keeping the appended stock DTB. It is
   flashed to BOOT with Heimdall. Recipe: ios-beyond `shim/build.sh`
   (beyond2q branch); magiskboot runs on the phone.
3. **BCB.** After `adb reboot recovery`, misc (sda10) keeps `boot-recovery`
   and every boot goes to recovery. Clear the first 32 bytes of misc before
   testing a BOOT image.
4. **Touch power and keys.** The touch rails are left on by the bootloader,
   so no RPMh or PMIC driver is needed to start. The keys need PM8150L
   (SID 4) instead of PM8150.
5. **Logs.** The kernel console log (patch 0047) goes to the last 4 MiB of
   userdata. On the S10+ that is live Android data until the iOS root is
   flashed there, and that flash erases Android, so do it deliberately.

## Status (2026-10-28)

- Board profile: `shim/boards/beyond2q.h`, `tools/devices.py['beyond2q']`,
  `tools/fdt-audit.py` MIRROR. All are checked against the live FDT.
- Shim: builds on macOS with clang. Use `--target=aarch64-linux-gnu`; add
  `-fno-jump-tables`, otherwise clang emits ABS64 string tables; lld takes no
  `--no-warn-rwx-segments`; `ufs.c` gained a freestanding `strcmp`.
  Output: `shim/out/beyond2q/Image`. The Samsung boot image still needs packing:
  stock header v1 plus the appended DTB, repacked with magiskboot.
- Kernel: `xnu/patches/0103-sweet-beyond2q-board.patch` adds MACHINE_CONFIG
  `BEYOND2Q`, which defines GUACAMOLE and SWEET too and overrides the board
  facts in `sweet_dev.h`. The touch driver gains board-overridable pins,
  no reset or 1.8 V pin on this board (it sends a sec_ts SW_RESET 0x42
  instead), and no `ldoa17` vote. The RPMh allowlist refuses every request
  when no rail is named. The gpio104 write probe is off. The staging scripts
  0019/0020/0026 accept BEYOND2Q. The patch applies cleanly on top of
  xnu-8796.141.3 + 0001-0102 + dxb. Not compiled yet: CI, `machine_config=BEYOND2Q`.
- Debug channel without the panel: the kernel's USB CDC-ECM gadget with its
  live UDP log (ios-sweet 0075-0077, `SWEET_DEV_USB=1`) should work here as is.
- Open: restarting command-mode scanout after sboot, so the framebuffer
  reaches the panel.
